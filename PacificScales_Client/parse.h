#ifndef CD_PARSER_H
#define CD_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

/* 
 * some string processing methods
 */
namespace string_tools {

struct parser {
    
    parser();
    static std::vector<std::string> split(const std::string& sentence, const std::string& delimiter);
    static std::string ltrim(const std::string &s);
    static std::string rtrim(const std::string &s);
    static std::string trim(const std::string &s);

    static std::string fix_negative(const std::string &s);

    // static uint32_t get32(const uint8_t* buffer);
    // static void put32(uint8_t* buffer, uint32_t value);

    static uint32_t Parse_uint32(const char* token);
    static bool Parse_boolean(const char* token);

};

}  // namespace string_tools
#endif /* CD_PARSER_H */
