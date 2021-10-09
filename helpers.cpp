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
    cout<<(char*)msg<<endl;

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
    string content = "<!DOCTYPE html><html><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"stylesheet\" href=\"/w3css/3/w3.css\"><link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.6.3/css/font-awesome.min.css\"><body><!-- Navigation --><nav class=\"w3-bar w3-black\">  <a href=\"#home\" class=\"w3-button w3-bar-item\">Home</a>  <a href=\"#band\" class=\"w3-button w3-bar-item\">Band</a>  <a href=\"#tour\" class=\"w3-button w3-bar-item\">Tour</a>  <a href=\"#contact\" class=\"w3-button w3-bar-item\">Contact</a></nav><!-- Slide Show --><section>  <img class=\"mySlides\" src=\"img_band_la.jpg\"  style=\"width:100%\">  <img class=\"mySlides\" src=\"img_band_ny.jpg\"  style=\"width:100%\">  <img class=\"mySlides\" src=\"img_band_chicago.jpg\"  style=\"width:100%\"></section><!-- Band Description --><section class=\"w3-container w3-center w3-content\" style=\"max-width:600px\">  <h2 class=\"w3-wide\">THE BAND</h2>  <p class=\"w3-opacity\"><i>We love music</i></p>  <p class=\"w3-justify\">We have created a fictional band website. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.</p></section><!-- Band Members --><section class=\"w3-row-padding w3-center w3-light-grey\">  <article class=\"w3-third\">    <p>John</p>    <img src=\"img_bandmember.jpg\" alt=\"Random Name\" style=\"width:100%\">    <p>John is the smartest.</p>  </article>  <article class=\"w3-third\">    <p>Paul</p>    <img src=\"img_bandmember.jpg\" alt=\"Random Name\" style=\"width:100%\">    <p>Paul is the prettiest.</p>  </article>  <article class=\"w3-third\">    <p>Ringo</p>    <img src=\"img_bandmember.jpg\" alt=\"Random Name\" style=\"width:100%\">    <p>Ringo is the funniest.</p>  </article></section><!-- Footer --><footer class=\"w3-container w3-padding-64 w3-center w3-black w3-xlarge\">  <a href=\"#\"><i class=\"fa fa-facebook-official\"></i></a>  <a href=\"#\"><i class=\"fa fa-pinterest-p\"></i></a>  <a href=\"#\"><i class=\"fa fa-twitter\"></i></a>  <a href=\"#\"><i class=\"fa fa-flickr\"></i></a>  <a href=\"#\"><i class=\"fa fa-linkedin\"></i></a>  <p class=\"w3-medium\">  Powered by <a href=\"https://www.w3schools.com/w3css/default.asp\" target=\"_blank\">w3.css</a>  </p></footer><script>// Automatic Slideshow - change image every 3 secondsvar myIndex = 0;carousel();function carousel() {  var i;  var x = document.getElementsByClassName(\"mySlides\");  for (i = 0; i < x.length; i++) {     x[i].style.display = \"none\";  }  myIndex++;  if (myIndex > x.length) {myIndex = 1}  x[myIndex-1].style.display = \"block\";  setTimeout(carousel, 3000);}</script></body></html>";
    string failed_msg = "HTTP/1.1 404 NOTFOUND\nContent-Type: text/html\nContent-Length:" + to_string(0) + "\n\n";
    // send(socket, msg.c_str(), strlen(msg.c_str()), 0);	
    // return;
	// char *sendbuf = "HTTP/1.1 200 OK\r\n \r\n";
    // string res, msg;

	FILE *requested_file;
	printf("Received request for file: %s on socket %d\n\n", file, socket);
	requested_file = fopen(file, "rb");
	
	if (requested_file == NULL){
		cout << "404 " << endl;
        // string msg = "HTTP/1.1 404 NOTFOUND \r\n \r\n";
        send(socket, failed_msg.c_str(), strlen(failed_msg.c_str()), 0);	
        cout << "HERE";
        // free(sendbuf);
	}
	else 
	{
		fseek (requested_file, 0, SEEK_END);
		int fileLength = ftell(requested_file);
		rewind(requested_file);
        
		char sendbuf[sizeof(char)*fileLength + 1];
		size_t result = fread(sendbuf, 1, fileLength, requested_file);
		if (result <= 0) {
            printf("Send error."); exit(1);
        }
        string msg = "HTTP/1.1 404 NOTFOUND\nContent-Type: text/html\nContent-Length:" + to_string(result) + "\n\n" + sendbuf;
		// msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + to_string(res.length()) + "\r\n" + res;
        send(socket, msg.c_str(), strlen(msg.c_str()), 0);	 
        fclose(requested_file);
	}
    
	
}