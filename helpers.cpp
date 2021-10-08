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

    // Map all headers from a key to a value
    while( true )
    {
        tail = head + 1;
        while( *head++ != '\r' );
        mid = strstr( tail, ":" );   

        // Look for the failed strstr   
        if( tail > mid )
            break;

        http_request[ string( ( char * ) msg ).substr( tail - ( char *)msg , ( mid ) - tail  ) ] = string( ( char * ) msg ).substr( mid + 2 - ( char *) msg , ( head - 3 ) - mid );
    }
    return http_request;
}


void send_file(int socket, char *file, bool fileSwitch=false)
{
	char *sendbuf;
	FILE *requested_file;
	printf("Received request for file: %s on socket %d\n\n", file + 1, socket);
	cout << file << endl;
	if (fileSwitch) { requested_file = fopen(file + 1, "rb"); }
	else { requested_file = fopen(file + 1, "r"); }
	
	if (requested_file == NULL){
		cout << "404 " << endl;
		
	}
	else 
	{
		printf("Hit else #1\n");
		fseek (requested_file, 0, SEEK_END);
		int fileLength = ftell(requested_file);
		rewind(requested_file);
		
		sendbuf = (char*) malloc (sizeof(char)*fileLength);
		size_t result = fread(sendbuf, 1, fileLength, requested_file);
		
		if (result > 0) {
            write(socket , sendbuf , result);
			// send(socket, sendbuf, result, 0);		
		}		
		else { printf("Send error."); exit(1); }
	}
	
	fclose(requested_file);
}