#include "xml_config_parser.h"
#include <algorithm>

#if defined(WIN32) || defined(WINDOWS)
# ifndef snprinf
#  define snprintf _snprintf
# endif
#endif

namespace bcus {

xml_config_parser::xml_config_parser() : root_(NULL) {
}
void xml_config_parser::set_xml_error() {
    if (xmldoc_.Error()) {
        char szcode[16] = {0};
        snprintf(szcode, 15, "id=%d ", xmldoc_.ErrorID());
        error_ = "XMLDocument error ";
        error_.append(szcode);
        error_.append(tinyxml2::GetError(xmldoc_.ErrorID()));
        error_.append(" str1=");
        if (xmldoc_.GetErrorStr1()) {
            error_.append(xmldoc_.GetErrorStr1());
        }
        error_.append(" str2=");
        if (xmldoc_.GetErrorStr2()) {
            error_.append(xmldoc_.GetErrorStr2());
        }
    }
}
int xml_config_parser::parse_file(const char *file) {
    if (tinyxml2::XML_SUCCESS != xmldoc_.LoadFile(file)) {
        set_xml_error();
        return -1;
    }
    if( NULL == (root_ = xmldoc_.RootElement())) {
        error_ = "no root node!";
        return -1;
    }
    return 0;
}
int xml_config_parser::parse_buffer(const char *buffer) {
    xmldoc_.Parse(buffer);
    if (xmldoc_.Error()) {
        set_xml_error();
        return -1;
    }
    if( NULL == (root_ = xmldoc_.RootElement())) {
        error_ = "no root node!";
        return -1;
    }
    return 0;
}
int xml_config_parser::parse_detail_buffer(const char *buffer) {
    std::string str = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
    str.append(buffer);
    return parse_buffer(str.c_str());
}
std::string xml_config_parser::get_string() {
    tinyxml2::XMLPrinter printer;
    xmldoc_.Accept(&printer);
    return printer.CStr();
}
std::string xml_config_parser::get_first_child_key(const char *path, xml_config_parser::element ele) {
    ele = get_element(path, ele);
    if (ele == NULL) {
        return "";
    }
    tinyxml2::XMLElement *sub = ele->FirstChildElement();
    if (sub == NULL) {
        return "";
    }
    return sub->Value();
}
std::string xml_config_parser::get_attribute(const char *name, const char *path, xml_config_parser::element ele) {
    ele = get_element(path, ele);
    if (ele == NULL) {
        return "";
    }
    const char *value = ele->Attribute(name);
    if (NULL == value) {
        //fprintf(stderr, "xml_config_parser::%s, no attribute[%s] for [%s]\n", __FUNCTION__, name, path);
        return "";
    }
    return value;
}
std::string xml_config_parser::get_parameter(const char *path, xml_config_parser::element ele) {
    ele = get_element(path, ele);
    if (ele == NULL) {
        return "";
    }
    tinyxml2::XMLPrinter printer;
    ele->InnerAccept(&printer);
    return printer.CStr();
}
int xml_config_parser::get_parameter(const char *path, int defaultvalue, xml_config_parser::element ele) {
    std::string result = get_parameter(path, ele);
    return result.empty() ? defaultvalue : atoi(result.c_str());
}
std::string xml_config_parser::get_parameter(const char *path, const char *defaultvalue, xml_config_parser::element ele) {
    std::string result = get_parameter(path, ele);
    return result.empty() ? defaultvalue : result;
}
std::vector<std::string> xml_config_parser::get_parameters(const char *path, xml_config_parser::element ele, int return_key_) {
    std::vector<std::string> result;
    ele = get_element(path, ele);
    if (ele == NULL) {
        return result;
    }
    const char *p = strrchr(path, '/');
    std::string key = (p == NULL) ? path : p + 1;
    do {
        tinyxml2::XMLPrinter printer;
        if (return_key_ == RETURN_KEY)
            ele->Accept(&printer);
        else
            ele->InnerAccept(&printer);
        result.push_back(printer.CStr());
    } while(NULL != (ele = ele->NextSiblingElement(key.c_str())));

    return result;
}

std::vector<xml_config_parser::element> xml_config_parser::get_elements(const char *path, xml_config_parser::element ele) {
    std::vector<element> result;
    ele = get_element(path, ele);
    if (ele == NULL) {
        return result;
    }
    const char *p = strrchr(path, '/');
    std::string key = (p == NULL) ? path : p + 1;
    do {
        result.push_back(ele);
    } while(NULL != (ele = ele->NextSiblingElement(key.c_str())));

    return result;
}
xml_config_parser::element xml_config_parser::get_element(const char *path, xml_config_parser::element ele) {
    if (ele == NULL) {
        ele = root_;
    }
    if (path == NULL || *path == 0) {
        return ele;
    }
    if (ele == NULL){
        fprintf(stderr, "xml_config_parser::%s, root_ is NULL\n", __FUNCTION__);
        return NULL;
    }
    std::string key;
    const char *last_pos = path;
    const char *curr_pos = path;
    const char *end = path + strlen(path);
    for (curr_pos = strchr(last_pos, '/'); curr_pos != NULL && curr_pos < end; curr_pos = strchr(last_pos, '/')) {
        key.assign(last_pos, curr_pos - last_pos);
        last_pos = curr_pos + 1;

        if (key.empty()) {
            fprintf(stderr, "xml_config_parser::%s, key is empty\n", __FUNCTION__);
            return NULL;
        }
        if (NULL == (ele = ele->FirstChildElement(key.c_str()))) {
            fprintf(stderr, "xml_config_parser::%s, no element for key[%s]\n", __FUNCTION__, key.c_str());
            return NULL;
        }
    }
    key = last_pos;
    if(key.empty() || NULL == (ele = ele->FirstChildElement(key.c_str()))) {
        fprintf(stderr, "xml_config_parser::%s, key[%s] is empty or no element for key\n", __FUNCTION__, key.c_str());
        return NULL;
    }
    return ele;
}

}
