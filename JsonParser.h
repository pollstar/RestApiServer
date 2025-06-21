#pragma once

#include <string>
#include <map>

class JsonParser
{
public:
    static std::map<std::string, std::string> parseSimpleJson(const std::string &json);
    static std::string createJsonResponse(const std::map<std::string, std::string>& data);  
};