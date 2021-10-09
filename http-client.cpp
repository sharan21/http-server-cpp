#include "helpers.cpp"
#define max_buf_size 5120
#define server_port 4950
using namespace std;

void service_socket(char sender_buffer[], int server_sock_fd)
{
    char receiver_buffer[max_buf_size];
    int message_size = recv(server_sock_fd, receiver_buffer, max_buf_size, 0);
    receiver_buffer[message_size] = '\0';
}

int main()
{
/*********************** INIT GLOBAL FILES/OBJECTS *************************/
    long int done = 0;
    int connected_threads = 0, n;
    ifstream input_file;
    fstream exps_file;
	input_file.open("inp-params.txt", ios::in);
    exps_file.open("exps.txt", ios::app);

	input_file >> n;
    cout << "#client: " << n << endl;
    omp_set_num_threads(n);

#pragma omp parallel
    {
/**************************** INIT THREAD LOCAL VARS ***************************/
        int id = omp_get_thread_num();        
        int server_sock_fd, last_fd, i;
        struct sockaddr_in server_addr;
        struct timeval tv = { 0, 10 }; //after 1 second select() will timeout
        fd_set all_fds, current_fds;
        #pragma omp barrier
/********************************* SOCKET CREATION ******************************/
        // usleep(id * 100); //to prevent DDoSsing yourself by sequentially adding threads to server
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
if(id == 0)
    cout << "Waiting for all clients to connect..." << endl;
#pragma omp barrier // wait for all clients to connect to server

#pragma omp critical
        {
            connected_threads++;
        }
        string msg = "GET /index.html HTTP/1.1 \r\n\r\n";
        float mean;
        if(id == 0){ //thread to compute and log statistics
            cout << "All clients connected! #clients: " << n << endl;
            int sec = 0;
            while(true){
                long int t1 = done;
                sleep(1);
                mean += done - t1;
                cout << done - t1 << " responses/second" << endl;
                sec ++;
                if(sec % 10 == 0){
                    cout << "------ Mean requests processed per 10 seconds: " <<  mean/10 << " ------" << endl;
                    mean = 0;
                }
            }
        }
        else{ // client threads
            while (true) {    
                send(server_sock_fd, msg.c_str(), strlen(msg.c_str()), 0); 
                // #pragma omp critical
                // {
                //     cout << id << " sent request:" << msg << endl;
                // }   
                char receiver_buffer[max_buf_size];
                int message_size = recv(server_sock_fd, receiver_buffer, max_buf_size, 0);
                receiver_buffer[message_size] = '\0';
                // #pragma omp critical
                // {
                //     cout << id << " received object of size: " << message_size << endl;
                // }    
                #pragma omp critical
                {
                    done += 1;
                }
                usleep(1000);
            }

        }
    
       
    } 
    return 0;
}
