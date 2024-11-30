#pragma once

#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <iostream> 
#include <Eigen/Dense>

const int DEFAULT_PORT = 5065;

template <class T, int D0, int D1>
void print(const Eigen::Matrix<T, D0, D1>& mat)
{
    for (int i = 0; i < D0; i++)
    {
        for (int j = 0; j < D1; j++)
        {
            std::cout << mat(i, j) << " ";
        }
        std::cout << std::endl;
    }
}

template <class ArgType, class RetType, RetType (*F)(const ArgType&)>
void loop(int port = DEFAULT_PORT) 
{ 
    // pid_t childpid;  
    struct sockaddr_in cliaddr, servaddr; 
    // char* message = "Hello Client"; 
    // void sig_chld(int); 

    /* create listening TCP socket */
    int listenfd = socket(AF_INET, SOCK_STREAM, 0); 
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port); 

    // binding server addr structure to listenfd 
    int bindstatus = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)); 
    if (bindstatus != 0)
    {
        printf("port already used\n");
        exit(1);
    }
    listen(listenfd, 10); 
    
    // clear the descriptor set 
    fd_set rset; 
    FD_ZERO(&rset); 

    for (;;) { 

        // set listenfd and udpfd in readset 
        FD_SET(listenfd, &rset); 

        // select the ready descriptor 
        int nready = select(listenfd + 1, &rset, NULL, NULL, NULL); 

        // if tcp socket is readable then handle 
        // it by accepting the connection 
        if (FD_ISSET(listenfd, &rset)) { 
            socklen_t len = sizeof(cliaddr); 
            int connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len); 
            // if ((childpid = fork()) == 0) 
            { 
                // close(listenfd); 
                std::cerr << "[server] Connection from " << inet_ntoa(cliaddr.sin_addr) << std::endl;
                ArgType buffer;
                bzero((char *)(&buffer), sizeof(buffer)); 
                // printf("Message From TCP client: "); 
                int start = 0;
                while (start < sizeof(buffer))
                {
                    int n = read(connfd, (void *)(&buffer) + start, sizeof(buffer) - start);
                    if (n < 0)
                    {
                        std::cerr << "[server] Error reading from socket" << std::endl;
                        exit(1);
                    }
                    start += n;
                }
                // print(buffer);
                // std::cout << buffer << std::endl;
                RetType res = F(buffer);
                start = 0;
                while (start < sizeof(res))
                {
                    int n = write(connfd, (const void*)(&res) + start, sizeof(res) - start);
                    if (n < 0)
                    {
                        std::cerr << "[server] Error writing to socket" << std::endl;
                        exit(1);
                    }
                    start += n;
                }
                close(connfd); 
                // exit(0); 
            } 
            close(connfd); 
        }
    } 
}

template <class ArgType, class RetType>
RetType request(ArgType x, std::string ip = "127.0.0.1", int port = DEFAULT_PORT) 
{ 
    int sockfd; 
    struct sockaddr_in servaddr; 

    // int n, len; 
    // Creating socket file descriptor 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("socket creation failed"); 
        exit(0); 
    } 

    memset(&servaddr, 0, sizeof(servaddr)); 

    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port); 
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str()); 

    if (connect(sockfd, (struct sockaddr*)&servaddr, 
                            sizeof(servaddr)) < 0) { 
        printf("\n Error : Connect Failed \n"); 
    } 

    // memset(buffer, 0, sizeof(buffer)); 
    // strcpy(buffer, "Hello Server"); 
    write(sockfd, (const void *)(&x), sizeof(x)); 
    // printf("Message from server: "); 
    RetType y;
    read(sockfd, (void *)(&y), sizeof(y)); 
    // std::cout << y << std::endl;
    close(sockfd); 
    return y;
} 
