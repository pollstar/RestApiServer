#include "JsonParser.h"

#include <sstream>

std::map<std::string, std::string> JsonParser::parseSimpleJson(const std::string &json)
{
    std::map<std::string, std::string> result;

    // Simple JSON parser for basic string fields
    size_t pos = 0;
    while (pos < json.length())
    {
        size_t keyStart = json.find('"', pos);
        if (keyStart == std::string::npos)
            break;

        size_t keyEnd = json.find('"', keyStart + 1);
        if (keyEnd == std::string::npos)
            break;

        std::string key = json.substr(keyStart + 1, keyEnd - keyStart - 1);

        size_t colonPos = json.find(':', keyEnd);
        if (colonPos == std::string::npos)
            break;

        size_t valueStart = json.find('"', colonPos);
        if (valueStart == std::string::npos)
            break;

        size_t valueEnd = json.find('"', valueStart + 1);
        if (valueEnd == std::string::npos)
            break;

        std::string value = json.substr(valueStart + 1, valueEnd - valueStart - 1);
        result[key] = value;

        pos = valueEnd + 1;
    }

    return result;
}

std::string JsonParser::createJsonResponse(const std::map<std::string, std::string> &data)
{
    std::ostringstream json;
    json << "{";
    bool first = true;
    for (const auto &pair : data)
    {
        if (!first)
            json << ",";
        json << "\"" << pair.first << "\":\"" << pair.second << "\"";
        first = false;
    }
    json << "}";
    return json.str();
}

