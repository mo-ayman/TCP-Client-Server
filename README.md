<h1 align="center">TCP Client-Server</h1>
<div align="center">
    <img src="./imgs/intro-img.jpg" alt="TCP/IP" />
</div>

## Overview
This project presents a basic implementation of a TCP client-server system in C++. The server listens for incoming connections on a predefined port, and the client communicates with the server. Unix sockets were utilized for low-level networking logic.

### TCP Server

#### Pseudocode
1. Listen for requests on the predefined port.
2. Accept the incoming new connection.
3. Fork the process to handle the new connection without blocking receiving more new connections.
    a. Read the first 32kb chunk of data sent by the client.
    b. If the client sent zero, then close the connection.
    c. Extract method, path, and HTTP version.
    d. If the method is GET, send the requested file if it exists after adjusting response headers; otherwise, send a 404 page.
    e. If the method is POST, send a 200 OK message, then read the payload sent by the client.
    f. Keep the connection open for (8 / total_clients_count) * 5 seconds. If after that period no more requests are received, close the connection. Otherwise, go to step a.
4. Go to step 3.

#### File Structure
- **main.c**
  - Implementation of the pseudocode.
  - Contains a dictionary for common mime types, allowing the server to set the “Content-Type” header to the right value.

#### Timeout Heuristic
Simple and efficient formula: (8 / total_clients_count) * 5. The total number of clients is a value shared across all processes handling different connections.

### TCP Client

#### Pseudocode
1. Open socket.
2. Connect with the server with an IP address taken from arguments.
3. Read requests line in the file `in.txt` line by line.
4. Parse requests and add headers using the `Header` class.
5. Add the host attribute to the header.
6. For a GET request:
    a. Send the header and wait until the server sends the file.
    b. Receive the response header using the `ResponseHeader` class.
    c. Open a new file to save the received file.
    d. Save the file.
7. For a POST request:
    a. Add content length and content type to the header.
    b. Send the header.
    c. Load the file that should be sent.
    d. Send it to the server.
    e. Receive the request response from the server.
8. Close the connection.

#### File Structure
- **Header.h**
  - Header parsing code.
  - Contains the `Header` class as the parent for `RequestHeader` and `ResponseHeader`.
  - `RequestHeader` class to create a header when sending a request.
  - `ResponseHeader` to parse headers when receiving a response.
  - Function to send the content type based on file extension.
- **Client.cpp**
  - The main file of the client side.
  - Executed from the terminal, passing the server IP and port number.
  - Parses requests from the `in.txt` file.
  - Sends requests based on the method: "GET" or "POST".
- **in.txt**
  - File of executed commands.

## Usage
1. Compile the server and client.
2. Run the server on a machine with a specified IP address.
3. Execute the client, providing the server's IP address and port number.
4. Monitor the communication between the client and server.

## License
This project is licensed under the [MIT License](LICENSE).
