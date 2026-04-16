#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
#include <string>

#include "json.hpp"
#include "blockchain.h"
#include "endpoints.h"
#include "server_utils.h"

using json = nlohmann::json;
using namespace std;

// Global blockchain
Blockchain my_blockchain;


int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // FIXED PORT HANDLING
    int port = 8080;
    char* port_env = getenv("PORT");
    if (port_env != nullptr) {
        port = atoi(port_env);
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // ERROR CHECKING
    if (::bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (::listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    cout << "Server running on port " << port << endl;

    while (true) {
        int client = accept(server_fd, nullptr, nullptr);

        char buffer[8192] = {0};
        int valread = read(client, buffer, sizeof(buffer));

        if (valread <= 0) {
            close(client);
            continue;
        }

        string req(buffer);
        cout << "Request:\n" << req << endl;

        // Parse method & path
        size_t first_space = req.find(' ');
        size_t second_space = req.find(' ', first_space + 1);

        if (first_space == string::npos || second_space == string::npos) {
            close(client);
            continue;
        }

        string method = req.substr(0, first_space);
        string path = req.substr(first_space + 1, second_space - first_space - 1);

        // Handle CORS preflight
        if (method == "OPTIONS") {
            string response = ServerUtils :: build_response("");
            send(client, response.c_str(), response.size(), 0);
            close(client);
            continue;
        }

        // Extract JSON body
        json body = ServerUtils :: extract_json(req);

        // Route request
        string response_body;
        try {
            response_body = handle_request(path, method, body);
        } catch (...) {
            response_body = R"({"error": "Something went wrong"})";
        }

        // Send response
        string full_response = ServerUtils :: build_response(response_body);
        send(client, full_response.c_str(), full_response.size(), 0);

        close(client);
    }

    close(server_fd);
    return 0;
}