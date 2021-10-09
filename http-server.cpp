#include "helpers.cpp"
#define max_buf_size 5120
#define server_port 4950
#define max_n_threads 1023
using namespace std;

char receiver_buffer[max_buf_size];
int curr_no_of_clients = 0;

string content = "<!DOCTYPE html><html><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"stylesheet\" href=\"/w3css/3/w3.css\"><link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.6.3/css/font-awesome.min.css\"><body><!-- Navigation --><nav class=\"w3-bar w3-black\">  <a href=\"#home\" class=\"w3-button w3-bar-item\">Home</a>  <a href=\"#band\" class=\"w3-button w3-bar-item\">Band</a>  <a href=\"#tour\" class=\"w3-button w3-bar-item\">Tour</a>  <a href=\"#contact\" class=\"w3-button w3-bar-item\">Contact</a></nav><!-- Slide Show --><section>  <img class=\"mySlides\" src=\"img_band_la.jpg\"  style=\"width:100%\">  <img class=\"mySlides\" src=\"img_band_ny.jpg\"  style=\"width:100%\">  <img class=\"mySlides\" src=\"img_band_chicago.jpg\"  style=\"width:100%\"></section><!-- Band Description --><section class=\"w3-container w3-center w3-content\" style=\"max-width:600px\">  <h2 class=\"w3-wide\">THE BAND</h2>  <p class=\"w3-opacity\"><i>We love music</i></p>  <p class=\"w3-justify\">We have created a fictional band website. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.</p></section><!-- Band Members --><section class=\"w3-row-padding w3-center w3-light-grey\">  <article class=\"w3-third\">    <p>John</p>    <img src=\"img_bandmember.jpg\" alt=\"Random Name\" style=\"width:100%\">    <p>John is the smartest.</p>  </article>  <article class=\"w3-third\">    <p>Paul</p>    <img src=\"img_bandmember.jpg\" alt=\"Random Name\" style=\"width:100%\">    <p>Paul is the prettiest.</p>  </article>  <article class=\"w3-third\">    <p>Ringo</p>    <img src=\"img_bandmember.jpg\" alt=\"Random Name\" style=\"width:100%\">    <p>Ringo is the funniest.</p>  </article></section><!-- Footer --><footer class=\"w3-container w3-padding-64 w3-center w3-black w3-xlarge\">  <a href=\"#\"><i class=\"fa fa-facebook-official\"></i></a>  <a href=\"#\"><i class=\"fa fa-pinterest-p\"></i></a>  <a href=\"#\"><i class=\"fa fa-twitter\"></i></a>  <a href=\"#\"><i class=\"fa fa-flickr\"></i></a>  <a href=\"#\"><i class=\"fa fa-linkedin\"></i></a>  <p class=\"w3-medium\">  Powered by <a href=\"https://www.w3schools.com/w3css/default.asp\" target=\"_blank\">w3.css</a>  </p></footer><script>// Automatic Slideshow - change image every 3 secondsvar myIndex = 0;carousel();function carousel() {  var i;  var x = document.getElementsByClassName(\"mySlides\");  for (i = 0; i < x.length; i++) {     x[i].style.display = \"none\";  }  myIndex++;  if (myIndex > x.length) {myIndex = 1}  x[myIndex-1].style.display = \"block\";  setTimeout(carousel, 3000);}</script></body></html>";
string msg = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length:" + to_string(content.length()) + "\n\n" + content;

void service_socket(int client_fd, fd_set *all_fds, int server_soc_fd, int last_fd)
{
	int rec_buffer_size;	
	rec_buffer_size = recv(client_fd, receiver_buffer, max_buf_size, 0);
	if (rec_buffer_size == 0)
	{ //client wants to close connection
		close(client_fd);
		FD_CLR(client_fd, all_fds); //set 0 to bit corresponding to this clients fd
		curr_no_of_clients--;
	}
	else
	{   
        map<string, string> request = parse_header(receiver_buffer);
        // print_request(request);
        cout << "serving GET " << request["Path"] << " to client with port " << client_fd << endl;
		char * filehere = new char(request["Path"].length()+1);
		strcpy (filehere, request["Path"].c_str());
		send(client_fd, msg.c_str(), strlen(msg.c_str()), 0);	
		// send_file(client_fd, filehere);
		memset(receiver_buffer, 0, max_buf_size);
	}
}

void accept_client_connection(fd_set *all_fds, int *last_fd, int server_soc_fd, struct sockaddr_in client_addr)
{
	int client_fd;
	socklen_t client_addr_len;
	client_addr_len = sizeof(struct sockaddr_in);

	if (curr_no_of_clients == max_n_threads)
	{
		cout << "Reached Max no of clients, cannot accept more!" << endl;
		return;
	}

	if ((client_fd = accept(server_soc_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
	{
		cout << "Failed to accept client" << endl;
		exit(1);
	}
	else
	{
		FD_SET(client_fd, all_fds); //set the client fd to 1
		curr_no_of_clients++;
		if (client_fd > *last_fd) // update the last fd/ no of active dfs
			*last_fd = client_fd;
		// cout << "connected to new client with port " << client_fd << endl;
	}
}

int main()
{
	int flag = 1, server_soc_fd = 0, last_fd, i; // last fd stores the
	fd_set all_fds, current_fds;
	struct sockaddr_in server_addr, client_addr;
	FD_ZERO(&all_fds);	   //a bit array of all the socket fds
	FD_ZERO(&current_fds); //a bit array of the servicable fds

	if ((server_soc_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "Failed to init socket" << endl;
		exit(1);
	}

	// INIT SERVER ADDR
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(4950);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);

	//INIT SOCKET
	if (setsockopt(server_soc_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
	{
		cout << " Failed to set socket options" << endl;
		exit(1);
	}

	//BIND SOCKET (removed check for c++11)
	bind(server_soc_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

	//START LISTENING
	if (listen(server_soc_fd, 10) < 0)
	{
		cout << "Failed to start listening" << endl;
		exit(1);
	}

	cout << "Started listening on port 4950...";
	cout.flush(); //empty stdout buffer
	FD_SET(server_soc_fd, &all_fds); //set server fd to 1 since server is now active
	last_fd = server_soc_fd;		 //currently only server fd is present, so last_fd = 1

	while (true)
	{
		current_fds = all_fds; //update current set of fds, incase an fd has been added/ closed
		if (select(last_fd + 1, &current_fds, NULL, NULL, NULL) < 0)
		{ //once an fd is set, condition will break
			cout << "error occured during select()" << endl;
			exit(1);
		}
		for (i = 0; i < last_fd + 1; i++)
		{ //iterate through list of fds
			
			if (FD_ISSET(i, &current_fds))
			{							//check which socket sent data
				if (i == server_soc_fd) // server accepting new client
					accept_client_connection(&all_fds, &last_fd, server_soc_fd, client_addr);
				else
					service_socket(i, &all_fds, server_soc_fd, last_fd); // a client has sent data to server
			}
		}
	}
	return 0;
}