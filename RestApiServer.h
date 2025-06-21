#pragma once

#include <iostream>
#include <string>
#include <map>
#include <mutex>

#include "Http.h"

class RestApiServer
{
public:
    RestApiServer();
    ~RestApiServer();

    bool start(int port);
    void run();
    void stop();
    void handleNewConnection();
    void handleClientData(int clientSocket);

private:
    int serverSocket;
    int epollFd;
    std::map<std::string, std::string> dataStore; // Data store for the REST API
    std::mutex storeMutex;

    bool setNonBlocking(int fd);
    HttpResponse handleGetRequest(const std::string &path);
    HttpResponse handlePostRequest(const std::string &path, const std::string &body);
    HttpResponse handlePutRequest(const std::string &path, const std::string &body);
    HttpResponse handleDeleteRequest(const std::string &path);
    HttpResponse processRequest(const HttpRequest &request);
};