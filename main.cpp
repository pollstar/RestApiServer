#include "RestApiServer.h"

int main() {
    RestApiServer server;
    
    if (!server.start(8080)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "REST API Server running on http://localhost:8080" << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET    /api/data       - Get all data" << std::endl;
    std::cout << "  GET    /api/data/{key} - Get specific item" << std::endl;
    std::cout << "  POST   /api/data       - Create new data (JSON body)" << std::endl;
    std::cout << "  PUT    /api/data/{key} - Update item (JSON: {\"value\": \"...\"})" << std::endl;
    std::cout << "  DELETE /api/data/{key} - Delete item" << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;
    
    server.run();
    
    return 0;
}