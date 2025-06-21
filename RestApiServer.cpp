#include "RestApiServer.h"

#include <vector>
#include <sstream>
#include <thread>
#include <algorithm>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include "JsonParser.h"

RestApiServer::RestApiServer()
    : serverSocket(-1), epollFd(-1)
{
}

RestApiServer::~RestApiServer()
{
    stop();
}

bool RestApiServer::start(int port)
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    // Set SO_REUSEADDR
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "Failed to set SO_REUSEADDR: " << strerror(errno) << std::endl;
        close(serverSocket);
        return false;
    }

    if (!setNonBlocking(serverSocket))
    {
        std::cerr << "Failed to set non-blocking mode" << std::endl;
        close(serverSocket);
        return false;
    }

    // Set up address and port
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        close(serverSocket);
        return false;
    }

    // Start listening
    if (listen(serverSocket, SOMAXCONN) < 0)
    {
        std::cerr << "Failed to listen on socket: " << strerror(errno) << std::endl;
        close(serverSocket);
        return false;
    }

    // Create epoll
    epollFd = epoll_create1(0);
    if (epollFd < 0)
    {
        std::cerr << "Failed to create epoll instance: " << strerror(errno) << std::endl;
        close(serverSocket);
        return false;
    }

    // Add server socket to epoll
    epoll_event event{};
    event.events = EPOLLIN; // | EPOLLET; // Edge-triggered
    event.data.fd = serverSocket;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) < 0)
    {
        std::cerr << "Failed to add server socket to epoll: " << strerror(errno) << std::endl;
        close(serverSocket);
        close(epollFd);
        return false;
    }

    return true;
}

void RestApiServer::run()
{
    const int MAX_EVENTS = 64;
    epoll_event events[MAX_EVENTS];

    while (true)
    {
        int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        if (numEvents < 0)
        {
            if (errno == EINTR)
                continue;
            std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
            break;
        }

        for (int i = 0; i < numEvents; ++i)
        {
            int fd = events[i].data.fd;

            if (fd == serverSocket)
            {
                // New connection
                handleNewConnection();
            }
            else
            {
                // Data from client
                handleClientData(fd);
            }
        }
    }
}

void RestApiServer::stop()
{
    if (epollFd >= 0)
    {
        close(epollFd);
        epollFd = -1;
    }
    if (serverSocket >= 0)
    {
        close(serverSocket);
        serverSocket = -1;
    }
}

void RestApiServer::handleNewConnection()
{
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);

    int clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientLen);
    if (clientSocket < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        }
        return;
    }

    if (!setNonBlocking(clientSocket))
    {
        std::cerr << "Failed to set client socket non-blocking" << std::endl;
        close(clientSocket);
        return;
    }

    epoll_event event{};
    event.events = EPOLLIN | EPOLLET; // Edge-triggered
    event.data.fd = clientSocket;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event) < 0)
    {
        std::cerr << "Failed to add client socket to epoll: " << strerror(errno) << std::endl;
        close(clientSocket);
        return;
    }

    std::cout << "New client connected: " << inet_ntoa(clientAddr.sin_addr) << std::endl;
}

void RestApiServer::handleClientData(int clientSocket)
{
    char buffer[4096];
    std::string requestData;

    while (true)
    {
        ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            requestData += buffer;
        }
        else if (bytesRead == 0)
        {
            // Client closed the connection
            break;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // No more data
                break;
            }
            else
            {
                std::cerr << "Read error: " << strerror(errno) << std::endl;
                break;
            }
        }
    }

    if (!requestData.empty())
    {
        // Process request
        HttpRequest request;
        if (request.parseRequest(requestData))
        {
            HttpResponse response = processRequest(request);
            std::string responseStr = response.toString();

            // Send response
            ssize_t totalSent = 0;
            ssize_t responseLen = responseStr.length();

            while (totalSent < responseLen)
            {
                ssize_t sent = write(clientSocket, responseStr.c_str() + totalSent,
                                     responseLen - totalSent);
                if (sent < 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        continue;
                    }
                    else
                    {
                        std::cerr << "Write error: " << strerror(errno) << std::endl;
                        break;
                    }
                }
                totalSent += sent;
            }
        }
    }

    // Close connection
    epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, nullptr);
    close(clientSocket);
}

bool RestApiServer::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

HttpResponse RestApiServer::handleGetRequest(const std::string &path)
{
    std::lock_guard<std::mutex> lock(storeMutex);

    if (path == "/api/data")
    {
        // Return all data
        std::string json = JsonParser::createJsonResponse(dataStore);
        HttpResponse response(200, "OK");
        response.body = json;
        return response;
    }
    else if (path.find("/api/data/") == 0)
    {
        // Return specific item
        std::string key = path.substr(10); // Remove "/api/data/"
        if (dataStore.find(key) != dataStore.end())
        {
            std::map<std::string, std::string> item;
            item[key] = dataStore[key];
            std::string json = JsonParser::createJsonResponse(item);
            HttpResponse response(200, "OK");
            response.body = json;
            return response;
        }
        else
        {
            HttpResponse response(404, "Not Found");
            response.body = "{\"error\":\"Item not found\"}";
            return response;
        }
    }

    HttpResponse response(404, "Not Found");
    response.body = "{\"error\":\"Endpoint not found\"}";
    return response;
}

HttpResponse RestApiServer::handlePostRequest(const std::string &path, const std::string &body)
{
    std::lock_guard<std::mutex> lock(storeMutex);

    if (path == "/api/data")
    {
        // Parse JSON and save data
        auto jsonData = JsonParser::parseSimpleJson(body);

        if (jsonData.empty())
        {
            HttpResponse response(400, "Bad Request");
            response.body = "{\"error\":\"Invalid JSON\"}";
            return response;
        }

        // Save data
        for (const auto &pair : jsonData)
        {
            dataStore[pair.first] = pair.second;
        }

        HttpResponse response(201, "Created");
        response.body = "{\"message\":\"Data saved successfully\"}";
        return response;
    }

    HttpResponse response(404, "Not Found");
    response.body = "{\"error\":\"Endpoint not found\"}";
    return response;
}

HttpResponse RestApiServer::handlePutRequest(const std::string &path, const std::string &body)
{
    std::lock_guard<std::mutex> lock(storeMutex);

    if (path.find("/api/data/") == 0)
    {
        std::string key = path.substr(10); // Remove "/api/data/"
        auto jsonData = JsonParser::parseSimpleJson(body);

        if (jsonData.find("value") != jsonData.end())
        {
            dataStore[key] = jsonData["value"];
            HttpResponse response(200, "OK");
            response.body = "{\"message\":\"Data updated successfully\"}";
            return response;
        }
        else
        {
            HttpResponse response(400, "Bad Request");
            response.body = "{\"error\":\"Missing 'value' field\"}";
            return response;
        }
    }

    HttpResponse response(404, "Not Found");
    response.body = "{\"error\":\"Endpoint not found\"}";
    return response;
}

HttpResponse RestApiServer::handleDeleteRequest(const std::string &path)
{
    std::lock_guard<std::mutex> lock(storeMutex);

    if (path.find("/api/data/") == 0)
    {
        std::string key = path.substr(10); // Remove "/api/data/"
        if (dataStore.erase(key) > 0)
        {
            HttpResponse response(200, "OK");
            response.body = "{\"message\":\"Data deleted successfully\"}";
            return response;
        }
        else
        {
            HttpResponse response(404, "Not Found");
            response.body = "{\"error\":\"Item not found\"}";
            return response;
        }
    }

    HttpResponse response(404, "Not Found");
    response.body = "{\"error\":\"Endpoint not found\"}";
    return response;
}

HttpResponse RestApiServer::processRequest(const HttpRequest &request)
{
    std::cout << "Processing " << request.method << " " << request.path << std::endl;

    if (request.method == "GET")
    {
        return handleGetRequest(request.path);
    }
    else if (request.method == "POST")
    {
        return handlePostRequest(request.path, request.body);
    }
    else if (request.method == "PUT")
    {
        return handlePutRequest(request.path, request.body);
    }
    else if (request.method == "DELETE")
    {
        return handleDeleteRequest(request.path);
    }
    else
    {
        HttpResponse response(405, "Method Not Allowed");
        response.body = "{\"error\":\"Method not allowed\"}";
        return response;
    }
}
