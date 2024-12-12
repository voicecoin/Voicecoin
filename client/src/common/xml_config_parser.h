#ifndef _BCUS_COMMON_XML_CONFIG_PARSER_H_
#define _BCUS_COMMON_XML_CONFIG_PARSER_H_
#include <string>
#include <vector>
#include "tinyxml2.h"

namespace bcus {

class xml_config_parser {
public:
    typedef tinyxml2::XMLElement* element;
    enum { RETURN_KEY = 1 };
    xml_config_parser();
    int parse_file(const char *file);
    int parse_buffer(const char *buffer);
    int parse_detail_buffer(const char *buffer);

    /** if path == NULL return root */
    std::string get_first_child_key(const char *path = NULL, element ele = NULL);

    /** if path == NULL return root */
    std::string get_attribute(const char *name, const char *path = NULL, element ele = NULL);
    std::string get_parameter(const char *path, element ele = NULL);
    std::string get_parameter(const char *path, const char *defaultvalue, element ele = NULL);
    int get_parameter(const char *path, int defaultvalue, element ele = NULL);
    std::vector<std::string> get_parameters(const char *path, element ele = NULL, int return_key_ = 0);
    std::vector<element> get_elements(const char *path, element ele = NULL);
    element get_element(const char *path, element ele = NULL);

    const std::string& get_error_message()const{return error_;}
    std::string get_string();

private:
    void set_xml_error();
private:
    tinyxml2::XMLDocument xmldoc_;
    tinyxml2::XMLElement *root_;
    std::string error_;
};

}
#endif
