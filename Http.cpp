#include "Http.h"

#include <sstream>
#include <iostream>

bool HttpRequest::parseRequest(const std::string &request)
{
    std::istringstream stream(request);
    std::string line;

    // Parse the first line (method, path, version)
    if (!std::getline(stream, line))
        return false;

    std::istringstream firstLine(line);
    if (!(firstLine >> method >> path >> version))
        return false;

    // Parse headers
    while (std::getline(stream, line) && !line.empty() && line != "\r")
    {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos)
        {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);

            // Remove spaces
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t\r") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r") + 1);

            headers[key] = value;
        }
    }

    // Read request body
    std::string bodyLine;
    while (std::getline(stream, bodyLine))
    {
        body += bodyLine + "\n";
    }
    if (!body.empty())
    {
        body.pop_back(); // Remove the last \n
    }

    return true;
}

HttpResponse::HttpResponse(int code, const std::string &text)
    : statusCode(code), statusText(text)
{
    headers["Content-Type"] = "application/json";
    headers["Connection"] = "close";
}

std::string HttpResponse::toString() const
{
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";

    for (const auto &header : headers)
    {
        response << header.first << ": " << header.second << "\r\n";
    }
    response << "Content-Length: " << body.length() << "\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}
