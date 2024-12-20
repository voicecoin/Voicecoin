// Copyright (c) 2017-2018 VoiceExpert Squall
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#include "rpcprotocol.h"

#include "clientversion.h"
#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"
#include "utiltime.h"
#include "version.h"

#include <stdint.h>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;

//! Number of bytes to allocate and read at most at once in post data
const size_t POST_READ_SIZE = 256 * 1024;

/**
 * HTTP protocol
 * 
 * This ain't Apache.  We're just using HTTP header for the length field
 * and to be compatible with other JSON-RPC implementations.
 */

string HTTPPost(const string& strMsg, const map<string,string>& mapRequestHeaders, const string &host)
{
    ostringstream s;
    s << "POST / HTTP/1.1\r\n"
      << "User-Agent: voicecoin-json-rpc/" << FormatFullVersion() << "\r\n"
      << "Host: " << host << "\r\n"
      << "Content-Type: application/json\r\n"
      << "Content-Length: " << strMsg.size() << "\r\n"
      << "Connection: close\r\n"
      << "Accept: application/json\r\n";
    BOOST_FOREACH(const PAIRTYPE(string, string)& item, mapRequestHeaders)
        s << item.first << ": " << item.second << "\r\n";
    s << "\r\n" << strMsg;

    return s.str();
}

static string rfc1123Time()
{
    return DateTimeStrFormat("%a, %d %b %Y %H:%M:%S +0000", GetTime());
}

static const char *httpStatusDescription(int nStatus)
{
    switch (nStatus) {
        case HTTP_OK: return "OK";
        case HTTP_BAD_REQUEST: return "Bad Request";
        case HTTP_FORBIDDEN: return "Forbidden";
        case HTTP_NOT_FOUND: return "Not Found";
        case HTTP_INTERNAL_SERVER_ERROR: return "Internal Server Error";
        default: return "";
    }
}

string HTTPError(int nStatus, bool keepalive, bool headersOnly)
{
    if (nStatus == HTTP_UNAUTHORIZED)
        return strprintf("HTTP/1.0 401 Authorization Required\r\n"
            "Date: %s\r\n"
            "Server: voicecoin-json-rpc/%s\r\n"
            "WWW-Authenticate: Basic realm=\"jsonrpc\"\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 296\r\n"
            "\r\n"
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\r\n"
            "\"http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd\">\r\n"
            "<HTML>\r\n"
            "<HEAD>\r\n"
            "<TITLE>Error</TITLE>\r\n"
            "<META HTTP-EQUIV='Content-Type' CONTENT='text/html; charset=ISO-8859-1'>\r\n"
            "</HEAD>\r\n"
            "<BODY><H1>401 Unauthorized.</H1></BODY>\r\n"
            "</HTML>\r\n", rfc1123Time(), FormatFullVersion());

    return HTTPReply(nStatus, httpStatusDescription(nStatus), keepalive,
                     headersOnly, "text/plain");
}

string HTTPReplyHeader(int nStatus, bool keepalive, size_t contentLength, const char *contentType)
{
    return strprintf(
            "HTTP/1.1 %d %s\r\n"
            "Date: %s\r\n"
            "Connection: %s\r\n"
            "Content-Length: %u\r\n"
            "Content-Type: %s\r\n"
            "Server: voicecoin-json-rpc/%s\r\n"
            "\r\n",
        nStatus,
        httpStatusDescription(nStatus),
        rfc1123Time(),
        keepalive ? "keep-alive" : "close",
        contentLength,
        contentType,
        FormatFullVersion());
}

string HTTPReply(int nStatus, const string& strMsg, bool keepalive,
                 bool headersOnly, const char *contentType)
{
    if (headersOnly)
    {
        return HTTPReplyHeader(nStatus, keepalive, 0, contentType);
    } else {
        return HTTPReplyHeader(nStatus, keepalive, strMsg.size(), contentType) + strMsg;
    }
}

bool ReadHTTPRequestLine(std::basic_istream<char>& stream, int &proto,
                         string& http_method, string& http_uri)
{
    string str;
    getline(stream, str);

    // HTTP request line is space-delimited
    vector<string> vWords;
    boost::split(vWords, str, boost::is_any_of(" "));
    if (vWords.size() < 2)
        return false;

    // HTTP methods permitted: GET, POST
    http_method = vWords[0];
    if (http_method != "GET" && http_method != "POST")
        return false;

    // HTTP URI must be an absolute path, relative to current host
    http_uri = vWords[1];
    if (http_uri.size() == 0 || http_uri[0] != '/')
        return false;

    // parse proto, if present
    string strProto = "";
    if (vWords.size() > 2)
        strProto = vWords[2];

    proto = 0;
    const char *ver = strstr(strProto.c_str(), "HTTP/1.");
    if (ver != NULL)
        proto = atoi(ver+7);

    return true;
}

int ReadHTTPStatus(std::basic_istream<char>& stream, int &proto)
{
    string str;
    getline(stream, str);
    vector<string> vWords;
    boost::split(vWords, str, boost::is_any_of(" "));
    if (vWords.size() < 2)
        return HTTP_INTERNAL_SERVER_ERROR;
    proto = 0;
    const char *ver = strstr(str.c_str(), "HTTP/1.");
    if (ver != NULL)
        proto = atoi(ver+7);
    return atoi(vWords[1].c_str());
}

int ReadHTTPHeaders(std::basic_istream<char>& stream, map<string, string>& mapHeadersRet)
{
    int nLen = 0;
    while (true)
    {
        string str;
        std::getline(stream, str);
        if (str.empty() || str[0] == '\r' || str[0] == '\n')
            break;
        string::size_type nColon = str.find(":");
        if (nColon != string::npos)
        {
            string strHeader = str.substr(0, nColon);
            boost::trim(strHeader);
            boost::to_lower(strHeader);
            string strValue = str.substr(nColon+1);
            boost::trim(strValue);
            mapHeadersRet[strHeader] = strValue;
            if (strHeader == "content-length")
                nLen = atoi(strValue.c_str());
        }
    }
    return nLen;
}


int ReadHTTPMessage(std::basic_istream<char>& stream, map<string,
                    string>& mapHeadersRet, string& strMessageRet,
                    int nProto, size_t max_size)
{
    mapHeadersRet.clear();
    strMessageRet.erase();

    // Read header
    int nLen = ReadHTTPHeaders(stream, mapHeadersRet);
    if (nLen < 0 || (size_t)nLen > max_size)
        return HTTP_INTERNAL_SERVER_ERROR;

    // Read message
    if (nLen > 0)
    {
        vector<char> vch;
        size_t ptr = 0;
        while (ptr < (size_t)nLen)
        {
            size_t bytes_to_read = std::min((size_t)nLen - ptr, POST_READ_SIZE);
            vch.resize(ptr + bytes_to_read);
            stream.read(&vch[ptr], bytes_to_read);
            if (!stream) // Connection lost while reading
                return HTTP_INTERNAL_SERVER_ERROR;
            ptr += bytes_to_read;
        }
        strMessageRet = string(vch.begin(), vch.end());
    }

    if(nLen == 0) {
      while(stream.good() && --max_size > 0)
        strMessageRet.push_back(stream.get());
    }

    string sConHdr = mapHeadersRet["connection"];

    if ((sConHdr != "close") && (sConHdr != "keep-alive"))
    {
        if (nProto >= 1)
            mapHeadersRet["connection"] = "keep-alive";
        else
            mapHeadersRet["connection"] = "close";
    }

    return HTTP_OK;
}

/**
 * JSON-RPC protocol.  Bitcoin speaks version 1.0 for maximum compatibility,
 * but uses JSON-RPC 1.1/2.0 standards for parts of the 1.0 standard that were
 * unspecified (HTTP errors and contents of 'error').
 * 
 * 1.0 spec: http://json-rpc.org/wiki/specification
 * 1.2 spec: http://jsonrpc.org/historical/json-rpc-over-http.html
 * http://www.codeproject.com/KB/recipes/JSON_Spirit.aspx
 */

string JSONRPCRequest(const string& strMethod, const UniValue& params, const UniValue& id)
{
    UniValue request(UniValue::VOBJ);
    request.push_back(Pair("method", strMethod));
    request.push_back(Pair("params", params));
    request.push_back(Pair("id", id));
    return request.write(0, 0, GetBoolArg("-legacyrpc", true)) + "\n";
}

UniValue JSONRPCReplyObj(const UniValue& result, const UniValue& error, const UniValue& id)
{
    UniValue reply(UniValue::VOBJ);
    if (!error.isNull())
        reply.push_back(Pair("result", NullUniValue));
    else
        reply.push_back(Pair("result", result));
    reply.push_back(Pair("error", error));
    reply.push_back(Pair("id", id));
    return reply;
}

string JSONRPCReply(const UniValue& result, const UniValue& error, const UniValue& id)
{
    UniValue reply = JSONRPCReplyObj(result, error, id);
    static bool legacy = GetBoolArg("-legacyrpc", true);
    return reply.write(0, 0, legacy) + "\n";
}

UniValue JSONRPCError(int code, const string& message)
{
    UniValue error(UniValue::VOBJ);
    error.push_back(Pair("code", code));
    error.push_back(Pair("message", message));
    return error;
}
