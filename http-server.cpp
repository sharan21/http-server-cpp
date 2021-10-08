// Server side C program to demonstrate HTTP Server programming
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <map>
using namespace std;



map<string, string> parse_header( void *msg )
{

    map<string, string> http_request;
    char *head = (char *) msg;
    char *mid;
    char *tail = head;

    if( sizeof( msg ) == 0 )
    {
        return http_request;
    }

    // Find request type
    while( *head++ != ' ');
    http_request[ "Type" ] = std::string( ( char * ) msg ).substr( 0 , ( head - 1) - tail );

    // Find path
    tail = head;
    while( *head++ != ' ');
    http_request[ "Path" ] = std::string( ( char * ) msg ).substr( tail - ( char *)msg , ( head - 1) - tail );

    // Find HTTP version
    tail = head;
    while( *head++ != '\r');
    http_request[ "Version" ] = std::string( ( char * ) msg ).substr( tail - ( char *)msg , ( head - 1) - tail );

    // Map all headers from a key to a value
    while( true )
    {
        tail = head + 1;
        while( *head++ != '\r' );
        mid = strstr( tail, ":" );   

        // Look for the failed strstr   
        if( tail > mid )
            break;

        http_request[ std::string( ( char * ) msg ).substr( tail - ( char *)msg , ( mid ) - tail  ) ] = std::string( ( char * ) msg ).substr( mid + 2 - ( char *) msg , ( head - 3 ) - mid );
    }

    
    return http_request;
}

using namespace std;

#define PORT 8080
int main(int argc, char const *argv[])
{
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // Only this line has been changed. Everything is same.
    string content = "<!DOCTYPE html><html><head><title>Example</title></head><body><p>This is an example of a simple HTML page with one paragraph.</p></body></html>";
    string hello = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length:" + to_string(content.length()) + "\n\n" + content;

    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        
        char buffer[30000] = {0};
        valread = read(new_socket, buffer, 30000);
        // cout << buffer << endl;
        map<string, string> http_request = parse_header(buffer);

        // Determine if successful
        std::cout << http_request[ "Host" ] << std::endl; 
        std::cout << http_request[ "Upgrade-Insecure-Requests" ] << std::endl; 
        std::cout << http_request[ "Accept" ] << std::endl; 
        std::cout << http_request[ "User-Agent" ] << std::endl; 
        std::cout << http_request[ "Accept-Language" ] << std::endl; 
        std::cout << http_request[ "Accept-Encoding" ] << std::endl; 
        std::cout << http_request[ "Connection" ] << std::endl; 


        write(new_socket , hello.c_str() , strlen(hello.c_str()));
        printf("------------------Hello message sent-------------------");
        close(new_socket);
    }
    return 0;
}