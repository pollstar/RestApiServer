# HTTP REST API Server in C++

A simple HTTP server with REST API, JSON support, and non-blocking sockets for Linux.

## Features

- ✅ Non-blocking sockets with epoll
- ✅ REST API supporting GET, POST, PUT, DELETE
- ✅ JSON parsing and generation
- ✅ Multithreaded request handling
- ✅ Simple in-memory data storage

## Building the Project

### Requirements

- Linux (Ubuntu/Debian/CentOS/etc.)
- GCC 7+ or Clang 6+
- CMake 3.10+
- pthread library

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake
```

**CentOS/RHEL:**
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake
```

### Build

```bash
# Create build directory
mkdir build
cd build

# Configure the project
cmake ..

# Compile
make

# Or build in release mode
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Running

```bash
# From the build directory
./bin/rest_server

# Or using make target
make run
```

The server will start on port 8080.

## API Endpoints

### GET /api/data
Get all data
```bash
curl -X GET http://localhost:8080/api/data
```

### GET /api/data/{key}
Get a specific item
```bash
curl -X GET http://localhost:8080/api/data/mykey
```

### POST /api/data
Create new data (JSON in request body)
```bash
curl -X POST http://localhost:8080/api/data \
  -H "Content-Type: application/json" \
  -d '{"name": "John", "city": "Kiev"}'
```

### PUT /api/data/{key}
Update an item
```bash
curl -X PUT http://localhost:8080/api/data/name \
  -H "Content-Type: application/json" \
  -d '{"value": "Jane"}'
```

### DELETE /api/data/{key}
Delete an item
```bash
curl -X DELETE http://localhost:8080/api/data/name
```

## Usage Examples

### Creating Data
```bash
# Create a user
curl -X POST http://localhost:8080/api/data \
  -H "Content-Type: application/json" \
  -d '{"username": "user123", "email": "user@example.com"}'

# Response: {"message":"Data saved successfully"}
```

### Retrieving Data
```bash
# Get all data
curl -X GET http://localhost:8080/api/data
# Response: {"username":"user123","email":"user@example.com"}

# Get a specific field
curl -X GET http://localhost:8080/api/data/username
# Response: {"username":"user123"}
```

### Updating Data
```bash
# Update email
curl -X PUT http://localhost:8080/api/data/email \
  -H "Content-Type: application/json" \
  -d '{"value": "newemail@example.com"}'

# Response: {"message":"Data updated successfully"}
```

### Deleting Data
```bash
# Delete user
curl -X DELETE http://localhost:8080/api/data/username
# Response: {"message":"Data deleted successfully"}
```

## Architecture

### Main Components

1. **RestApiServer** - main server class
   - Socket and epoll management
   - Handling incoming connections
   - Request routing

2. **HttpRequest** - HTTP request parsing
   - Extracting method, path, headers
   - Parsing request body

3. **HttpResponse** - HTTP response formation
   - Setting status codes
   - Managing headers
   - Serializing response

4. **JsonParser** - simple JSON parser
   - Parsing JSON strings into map
   - Generating JSON from map

### Non-blocking Architecture

The server uses:
- **epoll** for monitoring multiple file descriptors
- **Non-blocking sockets** to prevent blocking
- **Edge-triggered mode** for efficient event handling

### Thread Safety

- Data storage is protected by a mutex
- Each request is handled atomically
- Safe access to shared state

## Configuration and Extension

### Changing the Port

In the `main.cpp` file, change the line:
```cpp
if (!server.start(8080)) {  // Replace 8080 with the desired port
```

### Adding New Endpoints

Add handling in the `handle*Request` methods:
```cpp
if (path == "/api/newendpoint") {
    // Your logic
}
```

### Improving the JSON Parser

The current parser is simple. For production, consider:
- nlohmann/json
- RapidJSON
- jsoncpp

### Logging

For production, add:
- Structured logging
- Log rotation
- Different log levels

## Performance

### Optimizations

- Compile with optimization flags (`-O2`, `-O3`)
- Use static linking if needed
- Profile with `perf` or `valgrind`

### Load Testing

```bash
# Using Apache Bench
ab -n 1000 -c 10 http://localhost:8080/api/data

# Using wrk
wrk -t4 -c100 -d30s http://localhost:8080/api/data
```

## Debugging

### Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Using GDB
```bash
gdb ./bin/rest_server
(gdb) run
```

### Memory Leak Check
```bash
valgrind --leak-check=full ./bin/rest_server
```

## Limitations

- Simple JSON parser (string fields only)
- In-memory storage (data not persisted)
- No SSL/TLS support
- Basic error handling

## Possible Improvements

- [ ] HTTPS support
- [ ] Database integration
- [ ] More advanced JSON parser
- [ ] Middleware system
- [ ] Configuration files
- [ ] Metrics and monitoring
- [ ] Rate limiting
- [ ] Authentication and authorization

## License

MIT License