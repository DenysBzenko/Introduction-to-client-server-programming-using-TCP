#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <fstream>
#include <string>
#pragma comment(lib, "ws2_32.lib")


class FileClient {
public:
    static void receiveFile(SOCKET clientSocket, const std::string& filename) {
        std::streamsize size = 0;
        recv(clientSocket, reinterpret_cast<char*>(&size), sizeof(size), 0);
        if (size > 0) {
            std::ofstream outFile("D:\\KSE\\paradigm\\Introduction to client-server programming using TCP\\Introduction-to-client-server-programming-using-TCP\\client1\\client\\clientfolder\\" + filename, std::ios::binary);
            char buffer[1024];
            while (size > 0) {
                int bytesRead = recv(clientSocket, buffer, std::min<size_t>(size, sizeof(buffer)), 0);
                outFile.write(buffer, bytesRead);
                size -= bytesRead;
            }
            outFile.close();
        }
    }

    static void sendFile(SOCKET clientSocket, const std::string& filename) {
        std::ifstream file("D:\\KSE\\paradigm\\Introduction to client-server programming using TCP\\Introduction-to-client-server-programming-using-TCP\\client1\\client\\clientfolder\\" + filename, std::ios::binary | std::ios::ate);
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

    static void sendCommandAndGetResponse(SOCKET clientSocket, const std::string& command) {
        send(clientSocket, command.c_str(), command.length(), 0);
        char buffer[4096];
        memset(buffer, 0, 4096);
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << "Server response: " << std::string(buffer, bytesReceived) << std::endl;
        }
    }
};

class Client {
private:
    SOCKET clientSocket;
    sockaddr_in serverAddr;

public:
    Client() : clientSocket(INVALID_SOCKET) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
    }

    ~Client() {
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
        }
        WSACleanup();
    }

    bool connectToServer(const std::string& ipAddress, int port) {
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
            return false;
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);

        
        if (InetPtonA(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr) != 1) {
            std::cerr << "InetPtonA failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            return false;
        }

        if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "connect failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            return false;
        }
        return true;
    }

    void run() {
        while (true) {
            std::string command;
            std::cout << "Enter command ('EXIT' to quit): ";
            std::getline(std::cin, command);

            if (command == "EXIT") {
                break;
            }

            sendCommandAndGetResponse(command);
            if (command.rfind("GET", 0) == 0) {
                std::string filename = command.substr(4);
                FileClient::receiveFile(clientSocket, filename);
            }
            else if (command.rfind("PUT", 0) == 0) {
                std::string filename = command.substr(4);
                FileClient::sendFile(clientSocket, filename);
                sendCommandAndGetResponse(command);
            }
        }
    }

private:
    void sendCommandAndGetResponse(const std::string& command) {
        send(clientSocket, command.c_str(), command.length(), 0);
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << "Server response: " << std::string(buffer, bytesReceived) << std::endl;
        }
    }
};

int main() {
    try {
        Client client;
        if (client.connectToServer("127.0.0.1", 12345)) {
            client.run();
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
