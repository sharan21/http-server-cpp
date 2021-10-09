#include <pthread.h>
#include "helpers.cpp"
#define max_buf_size 5120
#define server_port 4950
#define max_n_threads 2048
using namespace std;

int curr_no_of_clients[10] = {0}, last_fd[10], server_soc_fd = 0;
fd_set all_fds[10], current_fds[10], current_master_fds, master_fds;

string content = "<!DOCTYPE html><html><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"stylesheet\" href=\"/w3css/3/w3.css\"><link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.6.3/css/font-awesome.min.css\"><body><!-- Navigation --><nav class=\"w3-bar w3-black\">  <a href=\"#home\" class=\"w3-button w3-bar-item\">Home</a>  <a href=\"#band\" class=\"w3-button w3-bar-item\">Band</a>  <a href=\"#tour\" class=\"w3-button w3-bar-item\">Tour</a>  <a href=\"#contact\" class=\"w3-button w3-bar-item\">Contact</a></nav><!-- Slide Show --><section>  <img class=\"mySlides\" src=\"img_band_la.jpg\"  style=\"width:100%\">  <img class=\"mySlides\" src=\"img_band_ny.jpg\"  style=\"width:100%\">  <img class=\"mySlides\" src=\"img_band_chicago.jpg\"  style=\"width:100%\"></section><!-- Band Description --><section class=\"w3-container w3-center w3-content\" style=\"max-width:600px\">  <h2 class=\"w3-wide\">THE BAND</h2>  <p class=\"w3-opacity\"><i>We love music</i></p>  <p class=\"w3-justify\">We have created a fictional band website. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.</p></section><!-- Band Members --><section class=\"w3-row-padding w3-center w3-light-grey\">  <article class=\"w3-third\">    <p>John</p>    <img src=\"img_bandmember.jpg\" alt=\"Random Name\" style=\"width:100%\">    <p>John is the smartest.</p>  </article>  <article class=\"w3-third\">    <p>Paul</p>    <img src=\"img_bandmember.jpg\" alt=\"Random Name\" style=\"width:100%\">    <p>Paul is the prettiest.</p>  </article>  <article class=\"w3-third\">    <p>Ringo</p>    <img src=\"img_bandmember.jpg\" alt=\"Random Name\" style=\"width:100%\">    <p>Ringo is the funniest.</p>  </article></section><!-- Footer --><footer class=\"w3-container w3-padding-64 w3-center w3-black w3-xlarge\">  <a href=\"#\"><i class=\"fa fa-facebook-official\"></i></a>  <a href=\"#\"><i class=\"fa fa-pinterest-p\"></i></a>  <a href=\"#\"><i class=\"fa fa-twitter\"></i></a>  <a href=\"#\"><i class=\"fa fa-flickr\"></i></a>  <a href=\"#\"><i class=\"fa fa-linkedin\"></i></a>  <p class=\"w3-medium\">  Powered by <a href=\"https://www.w3schools.com/w3css/default.asp\" target=\"_blank\">w3.css</a>  </p></footer><script>// Automatic Slideshow - change image every 3 secondsvar myIndex = 0;carousel();function carousel() {  var i;  var x = document.getElementsByClassName(\"mySlides\");  for (i = 0; i < x.length; i++) {     x[i].style.display = \"none\";  }  myIndex++;  if (myIndex > x.length) {myIndex = 1}  x[myIndex-1].style.display = \"block\";  setTimeout(carousel, 3000);}</script></body></html>";
string msg = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length:" + to_string(content.length()) + "\n\n" + content;

void service_socket(int client_fd, fd_set *all_fds, int server_soc_fd, int last_fd, int thread_number)
{
	int rec_buffer_size;	
	char *receiver_buffer = new char[max_buf_size];
	rec_buffer_size = recv(client_fd, receiver_buffer, max_buf_size, 0);
	if (rec_buffer_size == 0)
	{ //client wants to close connection
		close(client_fd);
		FD_CLR(client_fd, all_fds); //set 0 to bit corresponding to this clients fd
		curr_no_of_clients[thread_number]--;
		int sum =0;
		for(int i=0;i<10;i++){
			sum+= curr_no_of_clients[i];
			cout<< curr_no_of_clients[i]<< ", ";
		}
		cout<<"Total : "<<sum;
	}
	else
	{   
        map<string, string> request = parse_header(receiver_buffer);
        // print_request(request);
        //cout << "serving GET " << request["Path"] << " to client with port " << client_fd << endl;
		if(request.size()<3){
			return;
		}
		char * filehere = new char[2048]{'\0'};
		getcwd(filehere, 2048);
		strcpy (filehere + strlen(filehere), request["Path"].c_str());
		//cout<<filehere<<endl;
		// send(client_fd, msg.c_str(), strlen(msg.c_str()), 0);	
		send_file(client_fd, filehere);
	}
	free(receiver_buffer);
}

void accept_client_connection(struct sockaddr_in client_addr)
{
	int client_fd;
	socklen_t client_addr_len;
	client_addr_len = sizeof(struct sockaddr_in);

	if ((client_fd = accept(server_soc_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
	{
		cout << "Failed to accept client" << endl;
		exit(1);
	}
	else
	{
		int min_thread = 0;
		for(int i=1;i<10;i++){
			if(curr_no_of_clients[i]<curr_no_of_clients[min_thread])
				min_thread = i;
		}
		cout<<"Accepting connection to thread "<<min_thread<<endl;
		FD_SET(client_fd, &all_fds[min_thread]); //set the client fd to 1
		curr_no_of_clients[min_thread]++;
		if (client_fd > last_fd[min_thread]) // update the last fd/ no of active dfs
			last_fd[min_thread] = client_fd;
		// cout << "connected to new client with port " << client_fd << endl;
	}
}

void* client_handler(void *thread){
	int thread_number = *((int*) thread);
	while (true)
	{
		current_fds[thread_number] = all_fds[thread_number]; //update current set of fds, incase an fd has been added/ closed
		struct timeval tv = { 0, 1000 }; //after 1 second select() will timeout
		int rVal = select(last_fd[thread_number] + 1, &current_fds[thread_number], NULL, NULL, &tv);
		if (rVal < 0)
		{ //once an fd is set, condition will break
			cout << "error occured during select()" << endl;
			exit(1);
		}
		else if(rVal >0){
			for (int i = 0; i < last_fd[thread_number] + 1; i++)
			{ //iterate through list of fds
				
				if (FD_ISSET(i, &current_fds[thread_number]))
				{
					service_socket(i, &all_fds[thread_number], server_soc_fd, last_fd[thread_number], thread_number); // a client has sent data to server
				}
			}
		}
		else{
			usleep(1000);
		}
	}
}

int main()
{
	int flag = 1, i; // last fd stores the
	struct sockaddr_in server_addr, client_addr;
	pthread_t threads[10];
	for(int i=0;i<10;i++){
		FD_ZERO(&all_fds[i]);	   //a bit array of all the socket fds
		FD_ZERO(&current_fds[i]); //a bit array of the servicable fds
	}
	FD_ZERO(&master_fds);
	FD_ZERO(&current_master_fds);

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
	for(int i=0;i<10;i++){
		//FD_SET(server_soc_fd, &all_fds[i]); //set server fd to 1 since server is now active
		last_fd[i] = server_soc_fd;		 //currently only server fd is present, so last_fd = 1
	}
	FD_SET(server_soc_fd, &master_fds);

	for(int i=0;i<10;i++){
		int *arg = new int();
		*arg = i;
		pthread_create(&threads[i], 0, client_handler, arg);
	}

	while (true)
	{
			accept_client_connection(client_addr);
			cout << "#clients: ";
			int sum =0;
			for(int i=0;i<10;i++){
				sum+= curr_no_of_clients[i];
				cout<< curr_no_of_clients[i]<< ", ";
			}
			cout<<"Total : "<<sum;
	}
	return 0;
}