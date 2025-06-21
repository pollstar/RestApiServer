#pragma once

#include <string>
#include <map>

class HttpRequest 
{
public:
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    
    bool parseRequest(const std::string& request); 
};

class HttpResponse 
{
public:
    int statusCode;
    std::string statusText;
    std::map<std::string, std::string> headers;
    std::string body;
    
    HttpResponse(int code = 200, const std::string& text = "OK");
    std::string toString() const;
};