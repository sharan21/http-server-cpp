#include "helpers.cpp"
#define max_buf_size 5120
#define server_port 4950
using namespace std;

string msg = "GET / HTTP/1.0\r\n \r\n";
int n = 100;

void service_socket(char sender_buffer[], int server_sock_fd)
{
    char receiver_buffer[max_buf_size];
    int message_size = recv(server_sock_fd, receiver_buffer, max_buf_size, 0);
    receiver_buffer[message_size] = '\0';
}

int main()
{
/*********************** INIT GLOBAL FILES/OBJECTS *************************/
    int connected_threads = 0;
#pragma omp parallel
    {
/**************************** INIT THREAD LOCAL VARS ***************************/
        int id = omp_get_thread_num();        
        int server_sock_fd, last_fd, i;
        struct sockaddr_in server_addr;
        struct timeval tv = { 1, 0 }; //after 1 second select() will timeout
        fd_set all_fds, current_fds;
/********************************* SOCKET CREATION ******************************/
        usleep(id * 100); //to prevent DDoSsing yourself by sequentially adding threads to server
        if ((server_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            cout << "failed to create socket" << endl;
            exit(1);
        }
/******************************** INIT SERVER ADDR ******************************/
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
/********************************** CONNECT TO SERVER ****************************/
        if (connect(server_sock_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0) {
            cout << "failed to connect to server" << endl;
            exit(1);
        }
/******************************* CONFIG THE FD BIT ARRAYS  *************************/
        FD_ZERO(&all_fds);
        FD_ZERO(&current_fds);
        FD_SET(server_sock_fd, &all_fds); //set server fd to 1
        last_fd = server_sock_fd; //we only need to listen to the server port
/******************************	 SEND AND RECEIVE *********************************/

#pragma omp critical
        {
            connected_threads++;
        }
        while (connected_threads != n);

        while (true) {    
            cout << "here" << endl;
            send(server_sock_fd, msg.c_str(), strlen(msg.c_str()), 0); 
            #pragma omp critical
            {
                cout << id << " sent message" << endl;
            }   
            char receiver_buffer[max_buf_size];
            int message_size = recv(server_sock_fd, receiver_buffer, max_buf_size, 0);
            receiver_buffer[message_size] = '\0';
            #pragma omp critical
            {
                cout << id << " received object of size: " << message_size << endl;
            }    
            sleep(2);
        }
    } 
    return 0;
}
