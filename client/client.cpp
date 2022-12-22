// Client side C/C++ program to demonstrate Socket
// programming
#include <bits/stdc++.h>
#include "header.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>


#define PORT 8081

using namespace std;

vector<string> parse_line(string line) {

    int start = 0;
    int end = line.find(" ");
    vector<string> token;
    while (end != -1) {
        token.push_back( line.substr(start, end - start) );
        start = end + 1;
        end = line.find(" ", start);
    }
    token.push_back( line.substr(start, end - start) );

    if(token[0] == "client_get") {
        token[0] = "GET";
    } else if(token[0] == "client_post") {
        token[0] = "POST";
    }

    return token;

}

size_t get_file_size(string file_name) {
    struct stat stat_buf;
    int rc = stat(file_name.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}


int main(int argc, char const* argv[])
{
    fstream file;
    file.open("in.txt", ios::in);


    char const * server_ip = argv[1];
    int port_number = PORT;
    if(argc == 3) 
        port_number = stoi(argv[2]);


	int sock = 0, valread, client_fd;
	struct sockaddr_in serv_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_number);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}


    if ((client_fd
        = connect(sock, (struct sockaddr*)&serv_addr,
                sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    if(file.is_open()) {
        string line;
        while(getline(file, line)) {
            std::stringstream ss;
            ss.str(string());

            vector<string> parsed = parse_line(line);


            
            if(parsed[0] == "GET") {
                // Request line 
                RequestHeader request_header;
                request_header.add_request_line(parsed[0], parsed[1], "HTTP/1.1");
                // Host
                if(parsed.size() == 4) 
                    port_number = stoi(parsed[3]);
                request_header.add_host(parsed[2], port_number);
                request_header.print_request();

                // send header
                request_header.send(sock);

                cout << "Request header sent" << endl;

                // response header
                ResponseHeader response_header;
                cout << "Waiting for response header" << endl;
                response_header.receive(sock);
                cout << "Response header received" << endl;
                response_header.print_response();

                if(response_header.get_status_code() != "200") continue;

                // read content length
                int content_length = stoi(response_header.get("Content-Length"));

                // read content byte by byte until content length
                string file_name = parsed[1].substr(1, parsed[1].size() - 1);
                FILE *fp = fopen(file_name.c_str(), "wb");
                byte buffer;
                cout << "Receiving file" << endl;
                for(int i = 0; i < content_length; i++) {
                    read(sock, &buffer, 1);
                    // writ as binary
                    fwrite(&buffer, sizeof(buffer), 1, fp);
                }

                cout << "File received" << endl;
                cout << endl;
                fclose(fp);

                continue;

            } else if(parsed[0] == "POST") {
                RequestHeader request_header;
                request_header.add_request_line(parsed[0], parsed[1], "HTTP/1.1");
                // Host
                if(parsed.size() == 4) 
                    port_number = stoi(parsed[3]);
                request_header.add_host(parsed[2], port_number);

                request_header.print_request();

                // add content length
                string file_name = parsed[1].substr(1, parsed[1].size() - 1);
                FILE *fp = fopen(file_name.c_str(), "rb");
                size_t content_length = get_file_size(file_name);
                request_header.add_content_length(content_length);

                // add content type
                request_header.add_content_type(file_name);

                // send header
                request_header.send(sock);

                // response header

                int MAX = 1024;
                char buff[MAX];
                size_t size = content_length;

                while (size > 0) {
					bzero(buff, MAX);
					size_t read_size = fread(buff, 1, MAX, fp);
					write(sock, buff, read_size);
					size -= read_size;
				}
                ResponseHeader response_header;
                response_header.receive(sock);
                response_header.print_response();
                fclose(fp);

            }

        }

        close(client_fd);
        close(sock);

    }
	return 0;
}
