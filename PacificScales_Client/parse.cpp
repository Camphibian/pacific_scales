#include <string.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "parse.h"
/** @file parse.cpp
 * @brief Inherit base parsing capabilities for yaml config model
 */

namespace string_tools {
/*
 * Inherit this class in your parser subclass
 */
parser::parser() {
}


/**
 * @brief Split a delimited string into a vector of strings
 * Delimeter is inclusive
 * @param sentence 
 * @param delimiter 
 * @return std::vector<std::string> 
 */
std::vector<std::string> parser::split(const std::string& sentence, const std::string& delimiter) {
    std::vector<std::string> tokens;

    auto start = 0U;
    auto end = sentence.find(delimiter);
    while (end != std::string::npos) {
        //std::cout << sentence.substr(start, end - start) << " ";
        tokens.push_back(sentence.substr(start, end - start));
        start = end + delimiter.length();
        end = sentence.find(delimiter, start);
    }

    //std::cout << sentence.substr(start, end) << std::endl << std::flush;
    tokens.push_back(sentence.substr(start, end));

    // std::cout << "'" << sentence << "'=( ";
    // for (auto s : tokens)
    //     std::cout << "'" << s << "' ";
    // std::cout << ")" << std::endl;
    return tokens;
}

const std::string WHITESPACE = " \n\r\t\f\v";
 
std::string parser::ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}
 
std::string parser::rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
 
std::string parser::trim(const std::string &s) {
    return rtrim(ltrim(s));
}

std::string parser::fix_negative(const std::string &s) {
    auto end = s.find("-");
    if (end != std::string::npos) {
		// remove any whitespace between the value and the minus
		return "-" + trim(s.substr(end+1));
	}
	return s;
}

// uint32_t parser::get32(const uint8_t* buffer) {
//     uint32_t value = 0;
//     for (int i = 3; i >= 0; i--)
//         value = (value << 8) + buffer[i];
//     return value;
// }

// void parser::put32(uint8_t* buffer, uint32_t value) {
//   *buffer++ = value & 0xff;
//   *buffer++ = (value >> 8) & 0xff;
//   *buffer++ = (value >> 16) & 0xff;
//   *buffer   = (value >> 24) & 0xff;
// }


/*
 * @param token string representation of unsigned decimal or hex number less than 2^32
 * @return uint32_t
 * @throws std::invalid_argument
 * @throws std::out_of_range
 */
uint32_t parser::Parse_uint32(const char* token) {
	std::cout << "Parse_uint32(" << token << ")" << std::endl << std::flush;

    auto x = (strncmp(token, "0x", 2) == 0) ? std::stoul(token, nullptr, 16): std::stoul(token);
    return x;
};

/*
 * match a string to a boolean form
 * @param token a string form of boolean type
 * @return boolean value
 * @throws not found
 */
bool parser::Parse_boolean(const char* token) {
    static std::map<std::string, bool> map = {
        {"false",   false},
        {"true",    true}
    };

	std::cout << "Parse_boolean(" << token << ")" << std::endl << std::flush;

    if (map.find(token) == map.end()) {
        throw new std::runtime_error("parser::Parse_boolean");
    }
    return map[token];
};

}  // namespace klendathu

