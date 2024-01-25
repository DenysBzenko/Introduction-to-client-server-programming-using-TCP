 # Introduction-to-client-server-programming-using-TCP
This project implements a simple TCP-based file server and a corresponding client written in C++ using the WinSock API. The server and client use sockets to exchange data and perform basic file operations, such as browsing the file list, uploading, sending, deleting, and retrieving file information.

## Functions

- **View file list**: The client can get a list of all files in a specific directory on the server.
- **File Download**: The client can download a file from the server.
- **Sending files**: The client can send a file to the server.
- **File deletion**: The client can delete a file on the server.
- **Get file information**: The client can get information about when it was last modified, how much it weighs.

## How do you get started with this project?

### Dowload 

```bash
git clone https://github.com/DenysBzenko/Introduction-to-client-server-programming-using-TCP.git
```
-And just compile the code

### Server

The server automatically starts listening for incoming connections on the port specified in the code (for example, 12345).

### Client 

After starting the client, you can enter the following COMMANDS: For example:
- `LIST` to get a list of files.
- `DELETE <filename>`  to delete a file.
- `INFO <filename>` to get information about the file.
