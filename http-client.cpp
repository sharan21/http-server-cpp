#include "helpers.cpp"
#define max_buf_size 5120
#define server_port 4950
using namespace std;

string msg = "GET / HTTP/1.0\n";

void service_socket(char sender_buffer[], int server_sock_fd, long int n = -1)
{
    char receiver_buffer[max_buf_size];
    int message_size = recv(server_sock_fd, receiver_buffer, max_buf_size, 0);
    receiver_buffer[message_size] = '\0';
    // parse_received_votes(receiver_buffer, server_sock_fd, s_node, recv_snode, n);
}

int main()
{

/*********************** INIT GLOBAL FILES/OBJECTS *************************/

#pragma region

    int connected_threads = 0;

#pragma endregion

#pragma omp parallel
    {
/**************************** INIT THREAD LOCAL VARS ***************************/

#pragma region

        int id = omp_get_thread_num();
        int server_sock_fd, last_fd, i, dest_thread;

        char sender_buffer[max_buf_size];

        struct sockaddr_in server_addr;
        struct timeval tv = { 1, 0 }; //after 1 second select() will timeout

        fd_set all_fds, current_fds;

#pragma endregion

/********************************* SOCKET CREATION ******************************/

#pragma region

        //to prevent DDoSsing yourself by sequentially adding threads to server
        usleep(id * 100);

        if ((server_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            cout << "failed to create socket" << endl;
            exit(1);
        }

#pragma endregion

/******************************** INIT SERVER ADDR ******************************/

#pragma region

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);

#pragma endregion

/********************************** CONNECT TO SERVER ****************************/

#pragma region

        if (connect(server_sock_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0) {
            cout << "failed to connect to server" << endl;
            exit(1);
        }

#pragma endregion

/******************************* CONFIG THE FD BIT ARRAYS  *************************/

#pragma region

        FD_ZERO(&all_fds);
        FD_ZERO(&current_fds);
        FD_SET(server_sock_fd, &all_fds); //set server fd to 1
        last_fd = server_sock_fd; //we only need to listen to the server port

#pragma endregion

/******************************	 CONFIG NODE AND SNO *********************************/

#pragma omp critical
        {
            connected_threads++;
        }
        while (connected_threads != n);

        while (true) {
            // string to_send = create_vote_message(my_snode);
            strcpy(sender_buffer, msg.c_str());
            send(server_sock_fd, sender_buffer, strlen(sender_buffer), 0); // me -> server -> other clients
        }

    } 

    return 0;
}
