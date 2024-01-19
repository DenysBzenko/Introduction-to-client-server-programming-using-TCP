#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <fstream>
#include <string>
#pragma comment(lib, "ws2_32.lib")

void receiveFile(SOCKET clientSocket, const std::string& filename) {
    std::streamsize size = 0;
    recv(clientSocket, reinterpret_cast<char*>(&size), sizeof(size), 0);
    if (size > 0) {
        std::ofstream outFile("D:\\KSE\\paradigm\\Introduction to client-server programming using TCP\\Introduction-to-client-server-programming-using-TCP\\client1\\client\\clientfolder" + filename, std::ios::binary);
        char buffer[1024];
        while (size > 0) {
            int bytesRead = recv(clientSocket, buffer, std::min<size_t>(size, sizeof(buffer)), 0);
            outFile.write(buffer, bytesRead);
            size -= bytesRead;
        }
        outFile.close();
    }
}

void sendFile(SOCKET clientSocket, const std::string& filename) {
    std::ifstream file("D:\\KSE\\paradigm\\Introduction to client-server programming using TCP\\Introduction-to-client-server-programming-using-TCP\\client1\\client\\clientfolder"  + filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Unable to open file" << std::endl;
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

void sendCommandAndGetResponse(SOCKET clientSocket, const std::string& command) {
    send(clientSocket, command.c_str(), command.length(), 0);
    char buffer[4096];
    memset(buffer, 0, 4096);
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        std::cout << "Server response: " << std::string(buffer, bytesReceived) << std::endl;
    }
}

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);

    iResult = connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "connect failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    while (true) {
        std::string command;
        std::cout << "Enter command ( 'EXIT' to quit): ";
        std::getline(std::cin, command);

        if (command == "EXIT") {
            break;
        }

        if (command.rfind("GET", 0) == 0) {
            sendCommandAndGetResponse(clientSocket, command);
            std::string filename = command.substr(4);
            receiveFile(clientSocket, filename);
        }
        else if (command.rfind("PUT", 0) == 0) {
            std::string filename = command.substr(4);
            sendFile(clientSocket, filename);
            sendCommandAndGetResponse(clientSocket, command);
        }
        else {
            sendCommandAndGetResponse(clientSocket, command);
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
