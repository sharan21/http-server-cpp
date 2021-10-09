#include <pthread.h>
#include <sys/poll.h>
#include "helpers.cpp"
#define max_buf_size 5120
#define server_port 4950
#define max_n_threads 2048
using namespace std;

int curr_no_of_clients[10] = {0}, nfds[10]={0}, server_soc_fd = 0;
struct pollfd all_fds[10][2048], master_fds;

void service_socket(int index, int thread_number)
{
	int rec_buffer_size;	
	int client_fd = all_fds[thread_number][index].fd;
	char *receiver_buffer = new char[max_buf_size];
	rec_buffer_size = recv(client_fd, receiver_buffer, max_buf_size, 0);
	if (rec_buffer_size == 0)
	{ //client wants to close connection
		close(client_fd);
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
		all_fds[min_thread][nfds[min_thread]].fd = client_fd;
		all_fds[min_thread][nfds[min_thread]].events = POLLIN;
		nfds[min_thread]++;
		curr_no_of_clients[min_thread]++;
		// cout << "connected to new client with port " << client_fd << endl;
	}
}

void* client_handler(void *thread){
	int thread_number = *((int*) thread);
	while (true)
	{
		int timeout = 10; //after 1 second select() will timeout
		int rVal = poll(all_fds[thread_number], nfds[thread_number], timeout);
		if (rVal < 0)
		{ //once an fd is set, condition will break
			cout << "error occured during poll()" << endl;
			exit(1);
		}
		else if(rVal >0){
			for (int i = 0; i < nfds[thread_number] + 1; i++)
			{ //iterate through list of fds
				if(all_fds[thread_number][i].revents == 0)
        			continue;
				if(all_fds[thread_number][i].revents != POLLIN)
				{
					printf("  Error! revents = %d\n", all_fds[thread_number][i].revents);
					break;
      			}
				service_socket(i,  thread_number); // a client has sent data to server
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
	memset(all_fds, 0 , sizeof(all_fds));

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
	
	master_fds.fd = server_soc_fd;
  	master_fds.events = POLLIN;

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