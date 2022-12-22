
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> 
#include <stdlib.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

string get_content_type(string file_path);

// Parent class header
class Header {
    protected:
        unordered_map<string, string> header;
        string http_version;
    public:
        Header(){
            http_version = "";
        };
        // ~Header() {
        //     delete this;
        // }
        void add(string key, string value) {
            header[key] = value;
        }

        string get(string key) {
            if(header.find(key) != header.end()) {
                return header[key];
            } else {
                return "";
            }
        }


        void add_content_length(int length) {
            header["Content-Length"] = to_string(length);
        }

        void add_content_type(string file_path) {
            header["Content-Type"] = get_content_type(file_path);
        }

        void add_host(string host, int port) {
            header["Host"] = host + ":" + to_string(port);
        }

        void add_connection(string connection) {
            header["Connection"] = connection;
        }


        string get_http_version() {
            return http_version;
        }




};

// class request header
class RequestHeader : public Header
{
    private:
        string method;
        string path;


    public:
        RequestHeader() : Header() {
            method = "";
            path = "";
        };
        // ~RequestHeader() {
        //     delete this;
        // }
        void add_request_line(string method, string path, string version) {
            this->method = method;
            this->path = path;
            this->http_version = version;
        }

        void send(int socket_fd) {
            stringstream ss;    
            ss << method << " " << path << " " << http_version << "\r\n";
            for(auto it = header.begin(); it != header.end(); it++) {
                ss << it->first << ": " << it->second << "\r\n";
            }
            ss << "\r\n";

            // send header
            write(socket_fd, ss.str().c_str(), ss.str().size());
        }

        void print_request() {
            // cout << "request header" << endl;
            cout << method << " " << path << " " << http_version << endl;
            for(auto it = header.begin(); it != header.end(); it++) {
                cout << it->first << ": " << it->second << endl;
            }
            cout << endl;
        }



        // Getters
        string get_method() {
            return method;
        }

        string get_path() {
            return path;
        }


};

// class response header
class ResponseHeader : public Header 
{
    private:
        string status_message;
        string status_code;
    public:
        ResponseHeader() : Header() {
            status_message = "";
            status_code = "";
        }
        // ~ResponseHeader() {
        //     delete this;
        // }

        void set_response_line(string version, string status_code, string status_message) {
            this->http_version = version;
            this->status_code = status_code;
            this->status_message = status_message;
        }

        void receive(int socket_fd) {


           // response header line

   
            char c;
            // // string http_version, status_code, status_message;
            while(true) {
                read(socket_fd, &c, sizeof(c));
                if(c == ' ' || c == '\r') {
                    break;
                }
                http_version += c;
            }
            if(c == ' ') {

                while(true) {
                    read(socket_fd, &c, sizeof(c));
                    if(c == ' ' || c == '\r') {
                        if(c == '\r') {
                            read(socket_fd, &c, sizeof(c));
                        }
                        break;
                    }
                    status_code += c;
                }
                if(c == ' ') {
                    while(true) {
                        read(socket_fd, &c, sizeof(c));
                        if(c == '\r') {
                            read(socket_fd, &c, sizeof(c));
                            break;
                        }
                        status_message += c;
                    }
                }
            }


            if(status_code == "404") {
                return;
            }

            string key, value;
            while(true) {
                key = "";
                value = "";
                while(true) {
                    read(socket_fd, &c, sizeof(c));
                    
                    if(c == ':' || c == '\r') {
                        if(c == '\r') {
                            read(socket_fd, &c, sizeof(c));
                        }
                        break;
                    }
                    key += c;
                }
                if(c == '\n') {
                    break;
                }
                while(true) {
                    read(socket_fd, &c, sizeof(c));
                    if(c == '\r' ) {
                        read(socket_fd, &c, sizeof(c));
                        break;
                    }
                    value += c;
                }
                if(key == "" && value == "") {
                    break;
                }
                header[key] = value;
            }
     
        }

        void print_response() {
            // cout << "response header" << endl;
            cout << http_version << " " << status_code << " " << status_message << endl;
            for(auto it = header.begin(); it != header.end(); it++) {
                cout << it->first << ": " << it->second << endl;
            }
            cout << endl;
        }

        string get_status_code() {
            return status_code;
        }

        string get_status_message() {
            return status_message;
        }


};

string get_content_type(string file_path) {
    static unordered_map<string, string> file_types = {
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

    // extract file extension
    int start = file_path.find_last_of(".");
    string extension = file_path.substr(start + 1, file_path.size() - start - 1);

    // map file extension to content type
    if(file_types.find(extension) != file_types.end()) {
        return file_types[extension];
    } else {
        return file_types[""];
    }

}
