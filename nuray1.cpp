#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <random>
#include <cstdint>
#include <iomanip> 

const int AES_BLOCK_SIZE = 16; // 


uint8_t gmul(uint8_t a, uint8_t b) {
    uint8_t p = 0;
    while (b) {
        if (b & 1) p ^= a;
        a = (a << 1) ^ (a & 0x80 ? 0x1b : 0);
        b >>= 1;
    }
    return p;
}


void gmixColumn(unsigned char* r) {
    unsigned char a[4];
    unsigned char b[4];
    unsigned char h;


    for (unsigned char c = 0; c < 4; c++) {
        a[c] = r[c]; 
        h = r[c] & 0x80; 
        b[c] = r[c] << 1; 
        if (h) b[c] ^= 0x1b; 
    }

    
    r[0] = b[0] ^ a[3] ^ a[2] ^ b[1] ^ a[1];
    r[1] = b[1] ^ a[0] ^ a[3] ^ b[2] ^ a[2];
    r[2] = b[2] ^ a[1] ^ a[0] ^ b[3] ^ a[3];
    r[3] = b[3] ^ a[2] ^ a[1] ^ b[0] ^ a[0];
}


std::vector<std::vector<std::vector<uint8_t>>> generateMatrices(int count) {
    std::vector<std::vector<std::vector<uint8_t>>> matrices(count, std::vector<std::vector<uint8_t>>(4, std::vector<uint8_t>(4)));
    std::random_device rd; std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dis(0, 255);
    for (auto& matrix : matrices)
        for (auto& row : matrix)
            for (auto& byte : row)
                byte = dis(gen);
    return matrices;
}


void saveMatricesToFile(const std::vector<std::vector<std::vector<uint8_t>>>& matrices, const std::string& filename) {
    std::ofstream file(filename);
    for (size_t i = 0; i < matrices.size(); ++i) {
        file << "Matrix " << i + 1 << ":\n";
        for (const auto& row : matrices[i]) {
            for (const auto& byte : row) file << static_cast<int>(byte) << " ";
            file << "\n";
        }
        file << "\n";
    }
}


void processAndSaveResults(const std::vector<std::vector<std::vector<uint8_t>>>& matrices, const std::string& resultFile, const std::string& timeFile) {
    std::ofstream outputFile(resultFile);
    std::ofstream timeOutputFile(timeFile);
    for (size_t i = 0; i < matrices.size(); ++i) {
        auto state = matrices[i];

        
        auto start = std::chrono::high_resolution_clock::now();

       
        for (int col = 0; col < 4; ++col) {
            unsigned char column[4];
            for (int row = 0; row < 4; ++row) {
                column[row] = state[row][col];
            }

            gmixColumn(column);

            for (int row = 0; row < 4; ++row) {
                state[row][col] = column[row];
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        
        timeOutputFile << "Matrix " << i + 1 << ": " << std::fixed << std::setprecision(9) << duration.count() << " seconds\n";

       
        outputFile << "Matrix " << i + 1 << " after MixColumns:\n";
        for (const auto& row : state) {
            for (const auto& byte : row) outputFile << static_cast<int>(byte) << " ";
            outputFile << "\n";
        }
        outputFile << "\n";
    }
}

int main() {
    const std::vector<int> dataSizes = {1 * 1024 * 1024, 10 * 1024 * 1024, 100 * 1024 * 1024}; // Размеры данных: 1 МБ, 10 МБ, 100 МБ
    for (int dataSize : dataSizes) {
        int numMatrices = dataSize / AES_BLOCK_SIZE;
        auto matrices = generateMatrices(numMatrices);
        saveMatricesToFile(matrices, "initial_matrices_" + std::to_string(dataSize) + "B.txt");
        processAndSaveResults(matrices, "results_" + std::to_string(dataSize) + "B.txt", "processing_times_" + std::to_string(dataSize) + "B.txt");
    }
    return 0;
}
