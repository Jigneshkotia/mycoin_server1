#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <iostream>
#include <fstream>
#include <ctime>
#include "json.hpp"
#include "sha256.h"

using json = nlohmann::json;

struct Transaction {
    std::string sender;
    std::string receiver;
    double amount;
    bool is_confirmed;
};

struct Block {
    int index;
    std::string prev_hash;
    std::string hash;
    std::vector<Transaction> transactions;
    long long timestamp;
    int nonce;
};

class Blockchain {
public:
    std::vector<Block> chain;
    std::deque<Transaction> mempool;
    std::map<std::string, double> confirmed_balance;
    std::map<std::string, double> pending_balance;

    Blockchain() {
        load_blockchain_from_file();

        if (chain.empty()) {
            std::cout << "⚡ Creating Genesis Block...\n";
            Block genesis;
            genesis.index = 0;
            genesis.prev_hash = "0";
            genesis.hash = "GENESIS_HASH";
            genesis.timestamp = time(0);
            genesis.nonce = 0;
            chain.push_back(genesis);

            save_blockchain_to_file();
        }

        load_mempool_from_file();
    }

    // ================= MEMPOOL =================

    void load_mempool_from_file() {
        std::ifstream file("data/mempool.json");
        if (!file.is_open()) return;

        json j;
        file >> j;

        for (const auto& item : j) {
            Transaction tx = {
                item["sender"],
                item["receiver"],
                item["amount"],
                item["is_confirmed"]
            };

            mempool.push_back(tx);

            pending_balance[tx.sender] -= tx.amount;
            pending_balance[tx.receiver] += tx.amount;
        }
    }

    void add_transaction(std::string sender, std::string receiver, double amount) {
        Transaction tx = {sender, receiver, amount, false};
        mempool.push_back(tx);

        pending_balance[sender] -= amount;
        pending_balance[receiver] += amount;
    }

    // ================= PROOF OF WORK =================

    std::string proof_of_work(Block& block, int difficulty) {
        std::string target(difficulty, '0');

        while (true) {
            std::string tx_data = "";
            for (auto& tx : block.transactions) {
                tx_data += tx.sender + tx.receiver + std::to_string(tx.amount);
            }

            std::string data =
                std::to_string(block.index) +
                block.prev_hash +
                tx_data +
                std::to_string(block.timestamp) +
                std::to_string(block.nonce);

            std::string hash = SHA256::hash(data);

            if (hash.substr(0, difficulty) == target) {
                return hash;
            }

            block.nonce++;

            if (block.nonce % 100000 == 0) {
                std::cout << "Trying nonce: " << block.nonce << std::endl;
            }
        }
    }

    // ================= MINING =================

    void mine_pending_transactions(std::string miner_address) {
        if (mempool.empty()) {
            std::cout << "⚠️ No transactions to mine\n";
            return;
        }

        Block new_block;
        new_block.index = chain.size();
        new_block.prev_hash = chain.back().hash;
        new_block.timestamp = time(0);
        new_block.nonce = 0;

        int count = 0;
        while (!mempool.empty() && count < 5) {
            Transaction tx = mempool.front();
            tx.is_confirmed = true;

            new_block.transactions.push_back(tx);

            // ✅ update balances
            confirmed_balance[tx.sender] -= tx.amount;
            confirmed_balance[tx.receiver] += tx.amount;

            pending_balance[tx.sender] += tx.amount;
            pending_balance[tx.receiver] -= tx.amount;

            mempool.pop_front();
            count++;
        }

        // 🏆 Mining reward
        Transaction reward = {"SYSTEM", miner_address, 10.0, true};
        new_block.transactions.push_back(reward);
        confirmed_balance[miner_address] += 10.0;

        int difficulty = 3;

        std::cout << "⛏️ Mining started...\n";

        new_block.hash = proof_of_work(new_block, difficulty);

        std::cout << "✅ Block mined!\n";
        std::cout << "Hash: " << new_block.hash << std::endl;
        std::cout << "Nonce: " << new_block.nonce << std::endl;

        chain.push_back(new_block);

        // 🔥 SAVE BLOCKCHAIN
        save_blockchain_to_file();
    }

    // ================= FILE STORAGE =================

    void save_blockchain_to_file() {
        json j = json::array();

        for (const auto& block : chain) {
            json block_json;
            block_json["index"] = block.index;
            block_json["prev_hash"] = block.prev_hash;
            block_json["hash"] = block.hash;
            block_json["timestamp"] = block.timestamp;
            block_json["nonce"] = block.nonce;

            json txs = json::array();
            for (const auto& tx : block.transactions) {
                txs.push_back({
                    {"sender", tx.sender},
                    {"receiver", tx.receiver},
                    {"amount", tx.amount},
                    {"is_confirmed", tx.is_confirmed}
                });
            }

            block_json["transactions"] = txs;
            j.push_back(block_json);
        }

        std::ofstream file("data/blockchain.json");
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
            std::cout << "💾 Blockchain saved\n";
        }
    }

    void load_blockchain_from_file() {
        std::ifstream file("data/blockchain.json");
        if (!file.is_open()) return;

        json j;
        file >> j;

        chain.clear();

        for (const auto& block_json : j) {
            Block block;

            block.index = block_json["index"];
            block.prev_hash = block_json["prev_hash"];
            block.hash = block_json["hash"];
            block.timestamp = block_json["timestamp"];
            block.nonce = block_json["nonce"];

            for (const auto& tx_json : block_json["transactions"]) {
                Transaction tx = {
                    tx_json["sender"],
                    tx_json["receiver"],
                    tx_json["amount"],
                    tx_json["is_confirmed"]
                };
                block.transactions.push_back(tx);
            }

            chain.push_back(block);
        }

        std::cout << "📦 Blockchain loaded from file\n";
    }

    // ================= BALANCE =================

    double get_total_balance(std::string user) {
        return confirmed_balance[user] + pending_balance[user];
    }
};

#endif