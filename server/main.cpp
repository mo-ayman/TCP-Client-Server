
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <unordered_map>
#include <sys/mman.h>
#include <sys/stat.h>
#include <atomic>
#include <thread>
#define MAX 32768
#define SA struct sockaddr

using namespace std;

unordered_map<string, string> mime_types = {
	{"html", "text/html"},
	{"htm", "text/html"},
	{"txt", "text/plain"},
	{"css", "text/css"},
	{"js", "application/javascript"},
	{"json", "application/json"},
	{"xml", "application/xml"},
	{"swf", "application/x-shockwave-flash"},
	{"flv", "video/x-flv"},
	{"png", "image/png"},
	{"jpe", "image/jpeg"},
	{"jpeg", "image/jpeg"},
	{"jpg", "image/jpeg"},
	{"gif", "image/gif"},
	{"bmp", "image/bmp"},
	{"ico", "image/vnd.microsoft.icon"},
	{"tiff", "image/tiff"},
	{"tif", "image/tiff"},
	{"svg", "image/svg+xml"},
	{"svgz", "image/svg+xml"},
	{"zip", "application/zip"},
	{"rar", "application/x-rar-compressed"},
	{"exe", "application/x-msdownload"},
	{"msi", "application/x-msdownload"},
	{"cab", "application/vnd.ms-cab-compressed"},
	{"mp3", "audio/mpeg"},
	{"qt", "video/quicktime"},
	{"mov", "video/quicktime"},
	{"pdf", "application/pdf"},
	{"ps", "application/postscript"},
	{"eps", "application/postscript"},
	{"ai", "application/postscript"},
	{"rtf", "application/rtf"},
	{"m3u", "audio/x-mpegurl"},
	{"wma", "audio/x-ms-wma"},
	{"wax", "audio/x-ms-wax"},
	{"wmv", "video/x-ms-wmv"},
	{"asf", "video/x-ms-asf"},
	{"asx", "video/x-ms-asf"},
	{"wm", "video/x-ms-wm"},
	{"wmx", "video/x-ms-wmx"},
	{"wvx", "video/x-ms-wvx"},
	{"avi", "video/x-msvideo"},
	{"movie", "video/x-sgi-movie"},
	{"doc", "application/msword"},
	{"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
	{"ppt", "application/vnd.ms-powerpoint"},
	{"pptx", "application/vnd.openxmlformats"},
	{"xls", "application/vnd.ms-excel"},
	{"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
	{"odt", "application/vnd.oasis.opendocument.text"},
	{"ods", "application/vnd.oasis.opendocument.spreadsheet"},
	{"", "application/octet-stream"}
};

// Total number of connections being handled by the server
static int *conn_count;

// A function to handle a single connection. Usually it's called in a separate process.
void handle_conn(int connfd)
{
	char buff[MAX];
	while (true) {
		int n = 0;
		bzero(buff, MAX);
		size_t unread = read(connfd, buff, sizeof(buff));
		if (buff[0] == 0) {
			printf("Client disconnected\n");
			break;
		}
		std::stringstream ss;
		// Extract the method of the request
		while (buff[n] != ' ') ss << buff[n++];
		n += 2;
		std::string method = ss.str();
		ss.str(std::string());
		// Extract the path of the request excluding the leading '/'
		while (buff[n] != ' ') ss << buff[n++];
		n++;
		std::string path = ss.str();
		ss.str(std::string());
		// Extract the HTTP version
		while (buff[n] != '\r') ss << buff[n++];
		n += 2;
		std::string protocol = ss.str();
		// Hashmap to store headers sent by the client
		unordered_map<std::string, std::string> headers;
		while (buff[n] != '\r') {
			ss.str(string());
			while (buff[n] != ':') ss << buff[n++];
			n += 2;
			string key = ss.str();
			ss.str(string());
			while (buff[n] != '\r') ss << buff[n++];
			n += 2;
			string value = ss.str();
			headers[key] = value;
		}
		n += 2;
			
		printf("%s /%s %s\n", method.c_str(), path.c_str(), protocol.c_str());
		for (auto it = headers.begin(); it != headers.end(); it++) {
			printf("%s: %s\n", it->first.c_str(), it->second.c_str());
		}
		printf("\n");
		
		if (method == "GET") {
			// Open the file in read binary mode
			FILE* fp = fopen(path.c_str(), "rb");
			if (fp == NULL) {
				bzero(buff, MAX);
				ss.str(string());
				ss << "HTTP/1.1 404 Not Found\r\n";
				ss << "Content-Type: text/html\r\n";
				ss << "Content-Length: 72\r\n";
				ss << "Connection: Keep-Alive\r\n";
				ss << "\r\n";
				ss << "<html><body><h1 style='color: hotpink;'>404 Not Found</h1></body></html>";
				strcpy(buff, ss.str().c_str());
				write(connfd, buff, ss.str().length());
			} else {
				bzero(buff, MAX);
				struct stat st;
				stat(path.c_str(), &st);
				size_t size = st.st_size;
				size_t ext_pos = path.find_last_of('.');
				string ext = ext_pos == string::npos ? "" : path.substr(ext_pos + 1);
				string mime_type = mime_types.find(ext) == mime_types.end() ? mime_types[""] : mime_types[ext];
				ss.str(string());
				ss << "HTTP/1.1 200 OK\r\n";
				ss << "Content-Type: " << mime_type << "\r\n";
				ss << "Content-Length: " << size << "\r\n";
				ss << "Connection: Keep-Alive\r\n";
				ss << "\r\n";
				strcpy(buff, ss.str().c_str());
				write(connfd, buff, ss.str().length());
				while (size > 0) {
					bzero(buff, MAX);
					size_t read_size = fread(buff, 1, MAX, fp);
					write(connfd, buff, read_size);
					size -= read_size;
				}
				fclose(fp);
			}
		} else if (method == "POST") {
			char buff2[MAX];
			bzero(buff2, MAX);
			ss.str(string());
			ss << "HTTP/1.1 200 OK\r\n";
			ss << "Content-Length: 0\r\n";
			ss << "Connection: Keep-Alive\r\n";
			ss << "\r\n";
			strcpy(buff2, ss.str().c_str());
			write(connfd, buff2, ss.str().length());
			size_t size = stoi(headers["Content-Length"]);
			unread -= n;
			while (size > 0) {
				while (unread > 0) {
					for (int i = n; i < unread + n; i++) {
						cout << buff[i];
					}
					size -= unread;
					unread = 0;
				}
				if (size > 0) {
					bzero(buff, MAX);
					size_t read_size = read(connfd, buff, MAX);
					unread = read_size;
					n = 0;
				}
			}
			cout << endl;
		} else {
			bzero(buff, MAX);
			ss << "HTTP/1.1 405 Method Not Allowed\r\n";
			ss << "Content-Type: text/html\r\n";
			ss << "\r\n";
			ss << "<html><body><h1 style='color: hotpink;'>405 Method Not Allowed</h1></body></html>";
			strcpy(buff, ss.str().c_str());
			write(connfd, buff, sizeof(buff));
		}
		fd_set readfds {};
		struct timeval tv {};
		int retval;
		FD_ZERO(&readfds);
		FD_SET(connfd, &readfds);
		// Set timeout dynamically based on the total number of connections
		tv.tv_sec = (8 / *conn_count) * 5;
		tv.tv_usec = 0;
		// Wait for a new request or timeout if no new request is received
		retval = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);
		if (retval == -1) {
			printf("Error in select()\n");
			break;
		} else if (retval == 0) {
			printf("Timeout occurred!\n");
			break;
		}
	}
}


int main(int argc, char** argv)
{
	int sockfd;
	int port = 8080;
	struct sockaddr_in servaddr;

	if (argc > 1) {
		port = stoi(argv[1]);
	}

	conn_count = (int*) mmap(NULL, sizeof *conn_count, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	*conn_count = 0;

	sockfd = socket(AF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	
	bzero(&servaddr, sizeof(servaddr));

	// Set server address and port
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	// Bind the socket with the server address
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("Failed to bind a socket on port %d!\n", port);
		printf("Try to run the server with another port.\n");
		exit(0);
	}

	// Listen for incoming connections
	if ((listen(sockfd, 64)) != 0) {
		printf("Failed to listen on port %d!\n", port);
		exit(0);
	} else {
		printf("Listening on port %d...\n", port);
	}

	while (true) {
		int connfd = accept(sockfd, NULL, NULL);
		if (connfd < 0) {
			continue;
		}
		pid_t pid = fork();
		if (pid == 0) {
			// Close sockfd as we are now in the process that handles the connection
			close(sockfd);
			// Increment the number of connections
			*conn_count += 1;
			// Handle the connection
			handle_conn(connfd);
			// Decrement the number of connections
			*conn_count -= 1;
			exit(0);
		}
		else {
			// Close connfd as we are now in the parent process
			close(connfd);
		}
	}

	// Close the socket as we are exiting the server
	close(sockfd);
}