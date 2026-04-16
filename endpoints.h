#ifndef ENDPOINTS_H
#define ENDPOINTS_H

#include "json.hpp"
#include "server_utils.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using json = nlohmann::json;

string handle_request(string path, string method, json body) {
    // Remove trailing slash
    // if (!path.empty() && path.back() == '/') {
    //     path.pop_back();
    // }

    cout<< "mai hu donn !!!!!!!!!!!!!!" << path << method << body << endl;

    // Placeholder for Add transaction to Mempool
    if (path == "/add_tx" && method == "POST") {
        try {
            string sender = body.value("sender", "");
            string receiver = body.value("receiver", "");
            double amount = body.value("amount", 0.0);

            // Basic validation
            if (sender.empty() || receiver.empty() || amount <= 0) {
                return json({
                    {"status", "error"},
                    {"message", "Invalid transaction data"}
                }).dump();
            }

            // Load users
            json users = ServerUtils::load_users();

            bool senderFound = false;
            bool receiverFound = false;

            double senderBalance = 0.0;

            // Find sender & receiver
            for (const auto& user : users) {
                if (user.value("privateKey", "") == sender) {
                    senderFound = true;

                    double confirmed = user.value("confirmed_coins", 0.0);
                    double pending = user.value("pending_coins", 0.0);

                    senderBalance = confirmed + pending;
                }

                if (user.value("publicKey", "") == receiver) {
                    receiverFound = true;
                }
            }

            // Sender not found
            if (!senderFound) {
                return json({
                    {"status", "error"},
                    {"message", "Sender does not exist"}
                }).dump();
            }

            // Receiver not found
            if (!receiverFound) {
                return json({
                    {"status", "error"},
                    {"message", "Receiver does not exist"}
                }).dump();
            }

            // Insufficient balance
            if (senderBalance < amount) {
                return json({
                    {"status", "error"},
                    {"message", "Insufficient balance"}
                }).dump();
            }

            // Transaction is valid → add to mempool
            my_blockchain.add_transaction(sender, receiver, amount);

            // Update pending balance (VERY IMPORTANT)
            for (auto& user : users) {
                if (user["privateKey"] == sender) {
                    user["pending_coins"] = user.value("pending_coins", 0.0) - amount;
                }
                if (user["publicKey"] == receiver) {
                    user["pending_coins"] = user.value("pending_coins", 0.0) + amount;
                }
            }

            // Save updated users
            ServerUtils::save_users(users);

            // Save mempool
            ServerUtils::save_mempool_to_file();

            cout << "\n[+] Transaction added: " << sender << " -> " << receiver << " (" << amount << ")" << endl;

            return json({
                {"status", "success"},
                {"message", "Transaction added to mempool"}
            }).dump();

        } catch (const std::exception& e) {
            return json({
                {"status", "error"},
                {"message", e.what()}
            }).dump();
        }
    }

    // Placeholder for Signup
    if(path == "/signup" && method == "POST"){
        try
        {
            string privateKey = body.value("privateKey", "");
            string publicKey = body.value("publicKey", "");
            bool isFullNode = body.value("isFullNode", false);

            // 🔥 Basic validation
            if (privateKey.empty() || publicKey.empty()) {
                return json({
                    {"status", "error"},
                    {"message", "Private key and Public key are required"}
                }).dump();
            }

            json users = ServerUtils::load_users();

            // 🔍 Check for duplicates (both keys must be unique)
            for (const auto& user : users) {
                std::string existingPrivate = user.value("privateKey", "");
                std::string existingPublic = user.value("publicKey", "");

                if (existingPrivate == privateKey || existingPrivate == publicKey) {
                    return json({
                        {"status", "error"},
                        {"message", "Private key already in use"}
                    }).dump();
                }

                if (existingPublic == publicKey || existingPublic == privateKey) {
                    return json({
                        {"status", "error"},
                        {"message", "Public key already in use"}
                    }).dump();
                }
            }

            // 🆕 Create new user
            json newUser = {
                {"privateKey", privateKey},
                {"publicKey", publicKey},
                {"confirmed_coins", 100.0},
                {"pending_coins", 0.0},
                {"isFullNode", isFullNode}
            };

            // ➕ Add user
            users.push_back(newUser);

            // 💾 Save to file
            ServerUtils::save_users(users);

            return json({
                {"status", "success"},
                {"message", "User created successfully"},
                {"user", newUser}
            }).dump();
        }
        catch(const std::exception& e)
        {
            return json({{"status", "error"}, {"message", e.what()}}).dump();
        }
        
    }
    
    // Placeholder for login 
    if (path == "/login" && method == "POST") { 
        cout<< "ha bhai login function me aagya mai !!!!!!!!"<<endl;
        string privateKey = body.value("privateKey", "");
        json users = ServerUtils :: load_users();

        bool found = false;
        json userData;

        // Iterate through users in the JSON object (assuming it's a list or map)
        // If users.json is a list of objects:
        for (const auto& user : users) {
            if (user.value("privateKey", "") == privateKey) {
                userData = user;
                found = true;
                break;
            }
        }

        if (found) {
            cout << "response chale gya !!!"<<endl;
            cout<< userData <<endl;
            cout << userData.dump()<<endl;
            return userData.dump();
        } else {
            cout<< "user nhi mila wo chale gya !!!!!"<<endl;
            return json({{"status", "error"}, {"message", "User not found"}}).dump();
        }

        cout<< " mai befaltu me call hogya bhai !!!!! "<<endl;
    }

    // Placeholder for start mining 
    if (path == "/mine" && method == "POST") {
        try {
            string minerKey = body.value("privateKey", "");

            if (minerKey.empty()) {
                return json({
                    {"status", "error"},
                    {"message", "Private key required"}
                }).dump();
            }

            // 🔥 Load users
            json users = ServerUtils::load_users();

            bool minerFound = false;
            bool isFullNode = false;

            // 🔍 Find miner
            for (auto& user : users) {
                if (user["privateKey"] == minerKey) {
                    minerFound = true;
                    isFullNode = user.value("isFullNode", false);
                    break;
                }
            }

            // ❌ Not found
            if (!minerFound) {
                return json({
                    {"status", "error"},
                    {"message", "Miner not found"}
                }).dump();
            }

            // ❌ Not a full node
            if (!isFullNode) {
                return json({
                    {"status", "error"},
                    {"message", "Only full nodes can mine"}
                }).dump();
            }

            // ❌ No transactions
            if (my_blockchain.mempool.empty()) {
                return json({
                    {"status", "error"},
                    {"message", "No transactions to mine"}
                }).dump();
            }

            // 🔥 Take first 5 transactions BEFORE mining
            vector<Transaction> minedTxs;
            int count = 0;
            for (auto& tx : my_blockchain.mempool) {
                if (count >= 5) break;
                minedTxs.push_back(tx);
                count++;
            }

            // 🚀 Mine block
            my_blockchain.mine_pending_transactions(minerKey);

            // 🔥 Update USERS.json balances
            for (auto& tx : minedTxs) {
                for (auto& user : users) {

                    // Sender update
                    if (user["privateKey"] == tx.sender) {
                        user["confirmed_coins"] = user.value("confirmed_coins", 0.0) - tx.amount;
                        user["pending_coins"] = user.value("pending_coins", 0.0) + tx.amount;
                    }

                    // Receiver update (PUBLIC KEY)
                    if (user["publicKey"] == tx.receiver) {
                        user["confirmed_coins"] = user.value("confirmed_coins", 0.0) + tx.amount;
                        user["pending_coins"] = user.value("pending_coins", 0.0) - tx.amount;
                    }
                }
            }

            // 💰 Mining reward
            for (auto& user : users) {
                if (user["privateKey"] == minerKey) {
                    user["confirmed_coins"] = user.value("confirmed_coins", 0.0) + 10.0;
                }
            }

            // 💾 Save users
            ServerUtils::save_users(users);

            // 💾 Save updated mempool
            ServerUtils::save_mempool_to_file();

            cout << "[⛏️] Block mined by: " << minerKey << endl;

            return json({
                {"status", "success"},
                {"message", "Block mined successfully"},
                {"transactions_mined", minedTxs.size()}
            }).dump();

        } catch (...) {
            return json({
                {"status", "error"},
                {"message", "Mining failed"}
            }).dump();
        }
    }

    // 📦 Get full blockchain
    if (path == "/blockchain" && method == "GET") {
        try {
            std::ifstream file("data/blockchain.json");

            // ❌ File not found
            if (!file.is_open()) {
                return json({
                    {"status", "error"},
                    {"message", "Blockchain file not found"}
                }).dump();
            }

            // ❌ Empty file check
            if (file.peek() == std::ifstream::traits_type::eof()) {
                return json({
                    {"status", "success"},
                    {"data", json::array()}  // return empty array
                }).dump();
            }

            json blockchain;
            file >> blockchain;

            return json({
                {"status", "success"},
                {"data", blockchain}
            }).dump();

        } catch (const std::exception& e) {
            return json({
                {"status", "error"},
                {"message", e.what()}
            }).dump();
        }
    }

    cout<< " bhai tera koi bhi path match nhi hua !!! "<<endl;
    return R"({"error": "Endpoint not found"})";
}

#endif