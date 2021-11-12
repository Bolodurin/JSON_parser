#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <stack>
#include <exception>
#include <variant>
#include <cassert>
#include <sstream>
#include <fstream>

#include <iostream>

#ifdef DEBUG
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while (false)
#else
#define DEBUG_MSG(str) do { } while (false)
#endif // DEBUG

#ifndef DEBUG
#define NDEBUG
#endif

using std::string;
using std::unique_ptr;
using std::map;
using std::vector;
using std::shared_ptr;
using std::length_error;
using std::invalid_argument;
using std::make_unique;
using std::variant;
using std::cout;
using std::endl;
using std::stack;

namespace parser {
    const uint32_t STR_MAX_LEN = 1 << 25;
    const size_t   START_STR_SIZE = 10;

    enum ServiceSymbols {
        OP_CURLY_BR = '{',
        CL_CURLY_BR = '}',
        OP_SQUAR_BR = '[',
        CL_SQUAR_BR = ']',
        DOUB_QUOTAT = '\"',
        ONE_QUOTAT  = '\'',
        COLON       = ':',
        COMMA       = ',',
    };

    class JsonNode;
    using JSONObj = map<string, unique_ptr<JsonNode>>;
    using JSONArr = vector<unique_ptr<JsonNode>>;

    //------------------------------------------------//
    class JsonGrammExcept : public std::runtime_error {
    
    private:
        string message_;

    public:
        JsonGrammExcept (const string& message, uint32_t position, 
                const char& unexChar = '?') : std::runtime_error (message) {
            std::ostringstream ostream;
            ostream << message << ": position without spaces - " << position 
                    << ", unexpected character - \'" << unexChar << "\'\n";
            message_ = ostream.str();
        }
        const char* what() const throw() {
            return message_.c_str();
        }
    };

    //---------------------------------------------//
    //TODO add extension to take access to elements of JSON structure
    /**
     * @brief Class with API to parse JSON
     * 
     * It saves last parsed JSON structure
     */
    class Parser {
        shared_ptr<JsonNode> jsonObj; ///< Pointer to last parsed JSON object
        uint32_t position; ///< Internal variable to parse obj
        string _jsonStr; ///< Input JSON string without spaces
    
        unique_ptr<JsonNode> _parse ();
        string dropWhitespaces (const string &jsonStr) const;
        unique_ptr<JsonNode> parseObject ();
        string parseString ();
        unique_ptr<JsonNode> parseArray ();

    public:
        /**
         * @brief function for parsing JSON
         * 
         * @param jsonStr - string in format JSON
         * @return shared_ptr<JsonNode> - pointer on upper node of 
         *   JSON structure
         * 
         * @throw length_error if input string is too long
         * @throw JsonGrammExcept if an grammar error in JSON input
         */
        shared_ptr<JsonNode> parseJson (const string &jsonStr);

        /**
         * @brief parse JSON from file
         * 
         * @param fileName name of file with input JSON
         * @return shared_ptr<JsonNode> - pointer to return JSON object
         * 
         * @throw invalid_argument cannot open file or filr too long
         * @throw length_error if input string is too long
         * @throw JsonGrammExcept if an grammar error in JSON input
         */
        shared_ptr<JsonNode> parseJsonFromFile (const string &fileName);
        
        /**
         * @brief print JSON structure in human-view format
         * 
         * If argument is emply, it tries to print internal struct
         * 
         * @param jsonObj - pointer to JSON object, which want to print, 
         *   or nothing, if want to print internal object
         * 
         * @throw invalid_argument no JSON to print
         * @throw logic_error internal structure error
         */
        void printJson (shared_ptr<JsonNode> jsonObj = nullptr);
        
        /**
         * @brief Get the JSON object (may be nullptr, if not defined)
         * 
         * @return shared_ptr<JsonNode> pointer to JSON object
         */
        shared_ptr<JsonNode> getJson () noexcept;
    };

    //---------------------------------------//
    struct JsonNode {
        variant<JSONObj, JSONArr, string> value;
        void printNode (uint32_t depth = 0);
    };
    //---------------------------------------//
}

#endif // _PARSER_H_