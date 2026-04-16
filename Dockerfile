FROM gcc:latest

WORKDIR /app

# Download the single-header JSON library directly from the official source
RUN apt-get update && apt-get install -y curl && \
    curl -o json.hpp https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp

# Copy your local project files (main.cpp, blockchain.h, endpoints.h, sha256.h)
COPY . .

# Compile
RUN g++ -std=c++11 -o server main.cpp

EXPOSE 8080
CMD ["./server"]