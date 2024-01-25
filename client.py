import socket
import sys

class FileClient:
    def receive_file(client_socket, filename):
        pass
    def send_file(client_socket, filename):
        pass
class Client:
    def __init__(self):
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def connect_to_server(self, ip_address, port):
        try:
            self.client_socket.connect((ip_address, port))
            return True
        except socket.error as e:
            print(f"Connection error: {e}")
            return False

    def run(self):
        while True:
            command = input("Enter command ('EXIT' to quit): ")
            if command == "EXIT":
                break

            self.client_socket.sendall(command.encode())
            response = self.client_socket.recv(4096)
            print(f"Server response: {response.decode()}")

            if command.startswith("GET"):
                filename = command[4:]
                FileClient.receive_file(self.client_socket, filename)
            elif command.startswith("PUT"):
                filename = command[4:]
                FileClient.send_file(self.client_socket, filename)

    def close(self):
        self.client_socket.close()

if __name__ == "__main__":
    client = Client()
    if client.connect_to_server("127.0.0.1", 12345):
        try:
            client.run()
        finally:
            client.close()
