#include "parser.h"

using namespace parser;

shared_ptr<JsonNode> Parser::parseJson (const string &jsonStr) {
    if (jsonStr.length() > STR_MAX_LEN) {
        throw length_error ("too long input JSON");
    }

    _jsonStr = dropWhitespaces (jsonStr);
    position = 0;

    try {
        jsonObj = move (_parse ());
        if (position < _jsonStr.length()-1) {
            throw JsonGrammExcept ("early end of json", position, _jsonStr[position]);
        }
    }
    catch (const JsonGrammExcept& e) {
#ifdef DEBUG
        std::cerr << "GRAMMAR ERROR: " << e.what() << endl;
#endif        
        throw;
    }
    catch (const length_error& e) {
#ifdef DEBUG
        std::cerr << "LENGTH ERROR: " << e.what() << endl;
#endif
        throw;
    }

    // Set class members to start condition
    _jsonStr = string();
    position = 0;    

    return jsonObj;
}

shared_ptr<JsonNode> Parser::parseJsonFromFile (const string &fileName) {
    std::ifstream inputFile;
    inputFile.open (fileName);
    if (!inputFile.is_open()) {
        throw std::invalid_argument ("cannot open input file");
    }

    std::streampos begin, end;
    begin = inputFile.tellg();
    inputFile.seekg (0, std::ios::end);
    end = inputFile.tellg();
    
    if (end - begin > STR_MAX_LEN) {
        throw std::invalid_argument ("too long input file");
    }

    inputFile.seekg (0, std::ios::beg);

    auto strStream = std::ostringstream{};

    strStream << inputFile.rdbuf();

    auto jsonString = strStream.str();

    return parseJson (jsonString);
}

shared_ptr<JsonNode> Parser::getJson () noexcept {
    return jsonObj;
}

void Parser::printJson (shared_ptr<JsonNode> jsonNode) {
    if (jsonNode) {
        jsonNode->printNode();
    }
    else if (jsonObj) {
        DEBUG_MSG ("Print internal object");
        jsonObj->printNode();
    }
    else {
        throw invalid_argument ("No objects, which can print");
    }
}

void printSpaces (uint32_t depth) {
    for (uint32_t i = 0; i < 2*depth; i++) {
        cout << " ";
    }
}

void JsonNode::printNode (uint32_t depth) {
    if (std::holds_alternative<JSONObj>(value)) {
        // If empty object, print just "{}"
        cout << "{";
        if (std::get<JSONObj>(value).size() == 0) {
            cout << "}";
            if (depth == 0) {
                cout << endl;
            }
            return;
        }
        cout << endl;

        // i to print commas, not for last element
        size_t i = 0;

        // add space to properties of object
        depth++;
        for (const auto& [key, val] : std::get<JSONObj> (value)) {
            assert (val);
            printSpaces (depth);
            cout << "\"" << key << "\"" << ": ";
            val->printNode (depth);
            if (i++ != std::get<JSONObj> (value).size() - 1) {
                cout << ", ";
            }
            cout << endl;
        }
        printSpaces (--depth);
        cout << "}";
        if (depth == 0) {
            cout << endl;
        }
    }
    else if (std::holds_alternative<JSONArr>(value)) {
        // If empty object, print just "[]"
        cout << "[";
        if (std::get<JSONArr>(value).size() == 0) {
            cout << "]";
            return;
        }
        cout << endl;

        // i is used to print commas, not for last element, like 
        // ["str1", "str2", "str3"]
        size_t i = 0;

        // add space to elements of array
        depth++;
        for (const auto& elem : std::get<JSONArr>(value)) {
            assert (elem);
            printSpaces(depth);
            elem->printNode (depth);
            if (i++ != std::get<JSONArr> (value).size() - 1) {
                cout << ", ";
            }
            cout << endl;
        }
        printSpaces (--depth);
        cout << "]";
    }
    else if (std::holds_alternative<string>(value)) {
        cout << "\"" << std::get<string> (value) << "\"";
    }
    else {
        throw std::logic_error ("unknown type");
    }
}

string Parser::dropWhitespaces (const string &jsonStr) const {
    string retStr = "";

    char strMark = 0;
    for (auto c : jsonStr) {
        if (!isspace (c)) {
            retStr.push_back (c);
        }

        if (c == DOUB_QUOTAT || c == ONE_QUOTAT) {
            if (c == strMark) {
                strMark = 0;
            }
            else if (strMark == 0) {
                strMark = c;
            }
        }

        if (strMark && isspace (c)) {
            retStr.push_back(c);
        }
    }

    DEBUG_MSG ("Clear string:");
    DEBUG_MSG (retStr);

    return retStr;
}

bool isAvailableChar (char c) {

    if (c >= ' ' && c < '\177') {
        return true;
    }
    return false;
}

string Parser::parseString () {
    char startPos = _jsonStr[position++];
    string resStr;

    // if string starts from ", then should over by "
    // similarly for '
    while (_jsonStr[position] != startPos && position < _jsonStr.length()) {
        auto c = _jsonStr[position];
        // Check, that symbol is not dot, comma, another
        if (!isAvailableChar (c)) {
            throw JsonGrammExcept ("Wrong character like a part of string", position, c);
        }
        position++;
        resStr += c;
    }
    if (position >= _jsonStr.length()) {
        throw JsonGrammExcept("cannot find the end of string", position);
    }
    return resStr;
}

unique_ptr<JsonNode> Parser::parseObject () {
    auto retNode = make_unique<JsonNode>();
    retNode->value = JSONObj{};
    while (_jsonStr[position] != CL_CURLY_BR && position < _jsonStr.length()) {

        if (!(_jsonStr[position] == DOUB_QUOTAT || 
                _jsonStr[position] == ONE_QUOTAT)) {
            throw JsonGrammExcept ("Object should start from quotation mark", 
                    position, _jsonStr[position]);
        }

        auto nameProp = parseString ();
        position++;
        if (position >= _jsonStr.length()) {
            throw JsonGrammExcept ("cannot find the end of object", position);
        }
        if (_jsonStr[position] != COLON) {
            throw JsonGrammExcept ("Wrong object structure: cannot find colon", 
                    position, _jsonStr[position]);
        }

        position++;
        if (position >= _jsonStr.length()) {
            throw JsonGrammExcept ("cannot find the end of object", position);
        }
        
        std::get<JSONObj> (retNode->value).insert ({nameProp, _parse()});
        
        position++;
        if (position >= _jsonStr.length()) {
            throw JsonGrammExcept ("cannot find the end of object", position);
        }

        if (_jsonStr[position] != COMMA && _jsonStr[position] != CL_CURLY_BR) {
            throw JsonGrammExcept ("Wrong grammar", position, _jsonStr[position]);
        }
        else if (_jsonStr[position] == COMMA) {
            position++;
            if (position >= _jsonStr.length()) {
                throw JsonGrammExcept ("cannot find the end of object", position);
            }
        }
    }
    return retNode;
}

unique_ptr<JsonNode> Parser::parseArray () {
    auto retNode = make_unique<JsonNode>();
    retNode->value = JSONArr{};
    while (_jsonStr[position] != CL_SQUAR_BR && 
            position < _jsonStr.length()) {
        if (_jsonStr[position] == COMMA) {
            position++;
            if (position >= _jsonStr.length()) {
                throw JsonGrammExcept ("cannot find the end of array", position);
            }
            continue;
        }
        std::get<JSONArr> (retNode->value).push_back (_parse());

        position++;
        if (position >= _jsonStr.length()) {
            throw JsonGrammExcept ("cannot find the end of array", position);
        }

        if (_jsonStr[position] != COMMA && _jsonStr[position] != CL_SQUAR_BR) {
            throw JsonGrammExcept ("Wrong grammar", position, _jsonStr[position]);
        }
    }
    if (position >= _jsonStr.length()) {
        throw JsonGrammExcept("cannot find the end of array", position);
    }
    return retNode;
}

unique_ptr<JsonNode> Parser::_parse () {
    auto retObj = make_unique<JsonNode>();
    for (; position < _jsonStr.length(); position++) {
        if (position == 0 && _jsonStr[position] != OP_CURLY_BR) {
            throw JsonGrammExcept ("invalid JSON object", 
                    position, _jsonStr[position]);
        }
        if (_jsonStr[position] == OP_CURLY_BR) {
            position++;
            retObj = std::move (parseObject ());
            break;
        }
        else if (_jsonStr[position] == DOUB_QUOTAT || 
                _jsonStr[position] == ONE_QUOTAT) {
            retObj = make_unique<JsonNode> ();
            retObj->value = parseString();
            break;
        }
        else if (_jsonStr[position] == OP_SQUAR_BR) {
            position++;
            retObj = std::move (parseArray ());
            break;
        }
        else {
            throw JsonGrammExcept ("Wrong structure of JSON: \
                    cannot find correct element", position, _jsonStr[position]);
        }
    }

    assert (retObj);
    return retObj;
}
