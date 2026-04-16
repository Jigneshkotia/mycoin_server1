#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <string>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include "json.hpp"
#include "blockchain.h"

using namespace std;
using json = nlohmann::json;

extern Blockchain my_blockchain;

class ServerUtils {
public:
    static void save_users(const json& users) {
        ofstream file("data/users.json");

        if (file.is_open()) {
            file << users.dump(4); // pretty print
            file.close();
            cout << "Users saved successfully\n";
        } else {
            cerr << "Failed to open users.json\n";
        }
    }

    static void save_mempool_to_file() {
        std::cout << "DEBUG: Saving " << my_blockchain.mempool.size() << " transactions to file..." << std::endl;
        
        json mempool_json = json::array();
        for (const auto& tx : my_blockchain.mempool) {
            mempool_json.push_back({
                {"sender", tx.sender},
                {"receiver", tx.receiver},
                {"amount", tx.amount},
                {"is_confirmed", tx.is_confirmed}
            });
        }

        std::ofstream file("data/mempool.json");
        if (file.is_open()) {
            file << mempool_json.dump(4);
            file.close(); // IMPORTANT: Close the file to flush the buffer!
            std::cout << "DEBUG: Successfully wrote to file." << std::endl;
        } else {
            std::cerr << "DEBUG: FAILED TO OPEN FILE!" << std::endl;
        }
    }

    static json load_users() {
        std::ifstream file("data/users.json");
        if (!file.is_open()) return json::object(); // Return empty object if file doesn't exist
        json j;
        file >> j;
        return j;
    }

    static string build_response(const string &body, int status = 200) {
            string status_text = (status == 200) ? "200 OK" : "400 Bad Request";

            return "HTTP/1.1 " + status_text + "\r\n"
                "Content-Type: application/json\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                "Access-Control-Allow-Headers: Content-Type\r\n"
                "Content-Length: " + to_string(body.size()) + "\r\n"
                "\r\n" +
                body;
        }

    static json extract_json(const std::string &req) {
        size_t header_end = req.find("\r\n\r\n");
        if (header_end == std::string::npos) return json{};

        // Find Content-Length
        size_t content_length_pos = req.find("Content-Length:");
        if (content_length_pos == std::string::npos) return json{};

        size_t line_end = req.find("\r\n", content_length_pos);
        std::string len_str = req.substr(content_length_pos + 15, line_end - (content_length_pos + 15));

        int content_length = std::stoi(len_str);

        // Extract exact body
        std::string body = req.substr(header_end + 4, content_length);

        std::cout << "Extracted Body: [" << body << "]" << std::endl;

        try {
            return json::parse(body);
        } catch (...) {
            std::cout << "JSON PARSE FAILED\n";
            return json{};
        }
    }
};

#endif