#ifndef SHA256_H
#define SHA256_H

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

class SHA256 {
public:
    static std::string hash(const std::string input) {
        unsigned char hash[32];
        SHA256_CTX ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, (unsigned char*)input.c_str(), input.size());
        sha256_final(&ctx, hash);
        std::stringstream ss;
        for(int i = 0; i < 32; i++) ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        return ss.str();
    }

private:
    typedef unsigned int uint32;
    typedef struct { unsigned char data[64]; uint32 datalen; unsigned long long bitlen; uint32 state[8]; } SHA256_CTX;

    static void sha256_init(SHA256_CTX *ctx) {
        ctx->datalen = 0; ctx->bitlen = 0;
        ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85;
        ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
        ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
        ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
    }

    static void sha256_transform(SHA256_CTX *ctx, unsigned char data[]) {
        uint32 a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];
        for (i = 0, j = 0; i < 16; ++i, j += 4) m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
        for ( ; i < 64; ++i) m[i] = (rotr(m[i-2], 17) ^ rotr(m[i-2], 19) ^ (m[i-2] >> 10)) + m[i-7] + (rotr(m[i-15], 7) ^ rotr(m[i-15], 18) ^ (m[i-15] >> 3)) + m[i-16];
        a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3]; e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];
        for (i = 0; i < 64; ++i) {
            t1 = h + (rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)) + ((e & f) ^ (~e & g)) + K[i] + m[i];
            t2 = (rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
            h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }
        ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d; ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
    }

    static void sha256_update(SHA256_CTX *ctx, unsigned char data[], size_t len) {
        for (size_t i = 0; i < len; i++) {
            ctx->data[ctx->datalen] = data[i]; ctx->datalen++;
            if (ctx->datalen == 64) { sha256_transform(ctx, ctx->data); ctx->bitlen += 512; ctx->datalen = 0; }
        }
    }

    static void sha256_final(SHA256_CTX *ctx, unsigned char hash[]) {
        uint32 i = ctx->datalen;
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
        ctx->bitlen += ctx->datalen * 8;
        ctx->data[63] = ctx->bitlen; ctx->data[62] = ctx->bitlen >> 8; ctx->data[61] = ctx->bitlen >> 16;
        sha256_transform(ctx, ctx->data);
        for (i = 0; i < 4; i++) { hash[i] = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff; hash[i+4] = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff; /* ... etc */ }
        // Simple finalization loop truncated for brevity - usually provided by crypto libs
        for(i=0; i<8; i++) for(int j=0; j<4; j++) hash[i*4+j] = (ctx->state[i] >> (24-j*8)) & 0xff;
    }
    static uint32 rotr(uint32 x, uint32 n) { return (x >> n) | (x << (32 - n)); }
    static const uint32 K[64];
};
const unsigned int SHA256::K[] = { 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };

#endif