#include <iostream>
#include <WinSock2.h>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <chrono>
#pragma comment(lib, "ws2_32.lib")

void handleGetCommand(SOCKET clientSocket, const std::string& filename) {
    std::string fullPath = "D:\\KSE\\paradigm\\Introduction to client-server programming using TCP\\Introduction-to-client-server-programming-using-TCP\\server\\Server\\serverfolder\\" + filename;
    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::string errorMsg = "File not found";
        send(clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    send(clientSocket, reinterpret_cast<const char*>(&size), sizeof(size), 0);

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer))) {
        send(clientSocket, buffer, file.gcount(), 0);
    }

    file.close();
}

void handleList(SOCKET clientSocket) {
    std::string directory = "D:\\KSE\\paradigm\\Introduction to client-server programming using TCP\\Introduction-to-client-server-programming-using-TCP\\server\\Server\\serverfolder";
    std::ostringstream oss;

    if (!std::filesystem::exists(directory)) {
        oss << "Directory not found";
    }
    else {
        try {
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                oss << entry.path().filename().string() << "\n";
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            oss << "Error accessing directory: " << e.what();
        }
    }

    std::string fileList = oss.str();
    send(clientSocket, fileList.c_str(), fileList.size(), 0);
}


void handlePut(SOCKET clientSocket, const std::string& filename) {
    std::string fullPath = "D:\\KSE\\paradigm\\Introduction to client-server programming using TCP\\Introduction-to-client-server-programming-using-TCP\\server\\Server\\serverfolder\\" + filename;
    std::ofstream file(fullPath, std::ios::binary);
    char buffer[1024];
    int bytesReceived;
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        file.write(buffer, bytesReceived);
    }
    file.close();
    std::string response = "File received";
    send(clientSocket, response.c_str(), response.size(), 0);
}

void handleDelete(SOCKET clientSocket, const std::string& filename) {
    std::string fullPath = "D:\\KSE\\paradigm\\Introduction to client-server programming using TCP\\Introduction-to-client-server-programming-using-TCP\\server\\Server\\serverfolder\\" + filename;
    std::string response;
    if (std::filesystem::remove(fullPath)) {
        response = "File deleted successfully";
    }
    else {
        response = "Error deleting file";
    }
    send(clientSocket, response.c_str(), response.size(), 0);
}

void handleInfo(SOCKET clientSocket, const std::string& filename) {
    std::string fullPath = "D:\\KSE\\paradigm\\Introduction to client-server programming using TCP\\Introduction-to-client-server-programming-using-TCP\\server\\Server\\serverfolder\\" + filename;
    std::ostringstream oss;
    if (std::filesystem::exists(fullPath)) {
        auto ftime = std::filesystem::last_write_time(fullPath);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

        char timeString[26];
        std::tm tm;
        localtime_s(&tm, &cftime);
        asctime_s(timeString, sizeof(timeString), &tm);

        oss << "File: " << filename << "\nSize: " << std::filesystem::file_size(fullPath) << " bytes\n";
        oss << "Last modified: " << timeString;
    }
    else {
        oss << "File not found";
    }
    std::string fileInfo = oss.str();
    send(clientSocket, fileInfo.c_str(), fileInfo.size(), 0);
}

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    iResult = bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    iResult = listen(serverSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port 12345" << std::endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        char recvbuf[1024];
        int recvbuflen = 1024;
        int result = recv(clientSocket, recvbuf, recvbuflen, 0);

        if (result > 0) {
            std::string command(recvbuf, result);
            if (command.rfind("GET", 0) == 0) {
                std::string filename = command.substr(4);
                handleGetCommand(clientSocket, filename);
            }
            else if (command.rfind("LIST", 0) == 0) {
                handleList(clientSocket);
            }
            else if (command.rfind("PUT", 0) == 0) {
                std::string filename = command.substr(4);
                handlePut(clientSocket, filename);
            }
            else if (command.rfind("DELETE", 0) == 0) {
                std::string filename = command.substr(7);
                handleDelete(clientSocket, filename);
            }
            else if (command.rfind("INFO", 0) == 0) {
                std::string filename = command.substr(5);
                handleInfo(clientSocket, filename);
            }
        }

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
