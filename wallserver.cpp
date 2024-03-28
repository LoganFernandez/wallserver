#include <iostream>
//socket setup taken from https://www.geeksforgeeks.org/socket-programming-cc/
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <deque>
#include "messages.h"
#define DEFAULT_PORT 5514
#define DEFAULT_QUEUE_SIZE 20
#define MAX_POST_LENGTH 80
using namespace std;

int main(int argc, char* argv[]){
    int queue_size;
    int port;
    if(argc == 2){
        string arg1 = argv[1];
        queue_size = stoi(arg1);
        port = DEFAULT_PORT;
    }
    else if(argc == 3){
        string arg1 = argv[1];
        string arg2 = argv[2];
        queue_size = stoi(arg1);
        port = stoi(arg2);
    }
    else{
        queue_size = DEFAULT_QUEUE_SIZE;
        port = DEFAULT_PORT;
    }
    //EX8 VARS
    bool isKill = false;
    bool isQuit = false;
    bool isClear = true;
    deque<string> postQueue;

    //Server stay keeps listening until isKill is true
    while (isKill == false)
    {

        int server_fd, client_fd;
        struct sockaddr_in address;
        int opt = 1;
        socklen_t addrlen = sizeof(address);

        // Creating socket file descriptor
        if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
    
        // Forcefully attaching socket to the port 8080
        if(setsockopt(server_fd, SOL_SOCKET,
                    SO_REUSEADDR | SO_REUSEPORT, &opt,
                    sizeof(opt))){
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
    
        // Forcefully attaching socket to the port 8080
        if(bind(server_fd, (struct sockaddr*)&address,
                sizeof(address))
            < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if(listen(server_fd, 3) < 0){
            perror("listen");
            exit(EXIT_FAILURE);
        }
        if((client_fd
            = accept(server_fd, (struct sockaddr*)&address,
                    &addrlen))
            < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        cout << "Client has connected.\n";
        while (isQuit == false){
            send(client_fd, WALL_HEADER, strlen(WALL_HEADER), 0);
            if(isClear){
                postQueue.clear();
                send(client_fd, EMPTY_MESSAGE, strlen(EMPTY_MESSAGE), 0);
            }
            else{
                for (size_t i = 0; i < postQueue.size() ; ++i) {
                // Send each string to the client socket
                    send(client_fd, postQueue[i].c_str(), postQueue[i].length(), 0);
                }
            }
            send(client_fd, COMMAND_PROMPT, strlen(COMMAND_PROMPT), 0);
            //TAKE COMMAND
            char command[1024];
            ssize_t bytes_read = read(client_fd, command, sizeof(command));
            command[bytes_read] = '\0';
            sscanf(command, "%[^\n]", command);

            cout << command << '\n';

            if(strcmp(command,"kill") == 0){
                send(client_fd, KILL_MESSAGE, strlen(KILL_MESSAGE), 0);
                close(client_fd);
                close(server_fd);
                isKill = true;
                isQuit = true;
            }            
            else if(strcmp(command,"quit") == 0){
                send(client_fd, QUIT_MESSAGE, strlen(QUIT_MESSAGE), 0);
                close(client_fd);
                isQuit = true;
            }
            else if(strcmp(command,"clear") == 0){
                send(client_fd, CLEAR_MESSAGE, strlen(CLEAR_MESSAGE), 0);
                isClear = true;
            }
            else if(strcmp(command,"post") == 0){
                //initialize name
                send(client_fd, NAME_PROMPT, strlen(NAME_PROMPT), 0);
                char name[MAX_POST_LENGTH];
                ssize_t bytes_read = read(client_fd, name, sizeof(name));
                name[bytes_read] = '\0';
                sscanf(name, "%[^\n]", name);
                // Calculate the size of the name string without newline characters
                int name_size = strlen(name);
                int remainingSpace = 80 - name_size - 2;
                string remainingSpaceStr = to_string(remainingSpace);
                send(client_fd, POST_PROMPT1, strlen(POST_PROMPT1), 0);
                send(client_fd, remainingSpaceStr.c_str(), remainingSpaceStr.length(), 0);
                send(client_fd, POST_PROMPT2, strlen(POST_PROMPT2), 0);
                char message[2*MAX_POST_LENGTH];
                bytes_read = read(client_fd, message, sizeof(message));
                message[bytes_read] = '\0';
                sscanf(message, "%[^\n]", message);
                // Calculate the size of the message string without newline characters
                int message_size = strlen(message);
                if(message_size > remainingSpace){
                    send(client_fd, ERROR_MESSAGE, strlen(ERROR_MESSAGE), 0);
                }
                else{
                    string post = string(name) + ": " + string(message) + '\n';
                    postQueue.push_back(post);
                    if ((int)postQueue.size() == queue_size){
                        //least recent dropped
                        postQueue.pop_front();
                    }
                    isClear = false;
                }
            }
            else{
                char tryAgain[] = "Please respond with a valid command. (post, clear, quit, kill)\n";
                send(client_fd, tryAgain, strlen(tryAgain), 0);
            }
        }
        isQuit = false;
    }
    return 0;
}
