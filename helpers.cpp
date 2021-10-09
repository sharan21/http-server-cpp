#include <arpa/inet.h>
#include <cstdlib>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <list>
#include <netinet/in.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <unordered_map>
#include "omp.h"
using namespace std;

void print_request(map<string, string> req){
    for(auto item: req){
        cout << item.first << ":" << item.second << endl;
    }
}

map<string, string> parse_header( void *msg )
{
    map<string, string> http_request;
    char *head = (char *) msg;
    char *mid;
    char *tail = head;

    if( sizeof( msg ) == 0 ){
        return http_request;
    }

    if(string((char*)msg).find("GET") == string::npos){
        cout<<"Not a get request\n";
        return http_request;
    }

    // Find request type
    while( *head++ != ' ');
    http_request[ "Type" ] = string( ( char * ) msg ).substr( 0 , ( head - 1) - tail );

    // Find path
    tail = head;
    while( *head++ != ' ');
    http_request[ "Path" ] = string( ( char * ) msg ).substr( tail - ( char *)msg , ( head - 1) - tail );

    // Find HTTP version
    tail = head;
    while( *head++ != '\r');
    http_request[ "Version" ] = string( ( char * ) msg ).substr( tail - ( char *)msg , ( head - 1) - tail );

    return http_request;
}


void send_file(int socket, char *file, bool fileSwitch=false)
{
    string path = string(file, strlen(file));
    string type = path.substr(path.find(".")+1);
	FILE *requested_file;
	printf("Received request for file: %s on socket %d\n\n", file, socket);
	requested_file = fopen(file, "rb");
	if (requested_file == NULL){
		cout << "404" << endl;
        string failed_msg = "HTTP/1.1 404 NOTFOUND\n\n \n\n";
        send(socket, failed_msg.c_str(), strlen(failed_msg.c_str()), 0);	
	}
	else 
	{
		fseek (requested_file, 0, SEEK_END);
		int fileLength = ftell(requested_file);
		rewind(requested_file);
		char sendbuf[sizeof(char)*fileLength + 1];
		size_t len = fread(sendbuf, 1, fileLength, requested_file);
		if (len <= 0) {
            printf("Send error."); exit(1);
        }
        string msg = "HTTP/1.1 200 OK\nContent-Type: text/" + type + "\nContent-Length:" + to_string(len) + "\n\n" + sendbuf;
        send(socket, msg.c_str(), strlen(msg.c_str()), 0);	 
        fclose(requested_file);
	}
}