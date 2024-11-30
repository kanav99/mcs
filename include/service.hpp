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
#include <chrono>

const int DEFAULT_PORT = 5065;

template <class T>
using Mat = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;
using u64 = unsigned long long;

template <class T>
void print(const Mat<T>& mat)
{
    int D0 = mat.rows();
    int D1 = mat.cols();
    for (int i = 0; i < D0; i++)
    {
        for (int j = 0; j < D1; j++)
        {
            std::cout << mat(i, j) << " ";
        }
        std::cout << std::endl;
    }
}

template <class T>
void print(const T& x)
{
    std::cout << x << std::endl;
}

template <class T>
void read_into(int connfd, T& buffer)
{
    u64 start = 0;
    while (start < sizeof(buffer))
    {
        int n = read(connfd, (char *)(&buffer) + start, sizeof(buffer) - start);
        if (n < 0)
        {
            std::cerr << "[server] Error reading from socket" << std::endl;
            exit(1);
        }
        start += n;
    }
}

template <class T>
void read_into(int connfd, Mat<T> &buffer)
{
    u64 start = 0;
    char *data = (char *)buffer.data();
    auto size = buffer.size() * sizeof(T);
    while (start < size)
    {
        int n = read(connfd, data + start, size - start);
        if (n < 0)
        {
            std::cerr << "[server] Error reading from socket" << std::endl;
            exit(1);
        }
        start += n;
    } 
}

template <class T>
void write_into(int connfd, const T& buffer)
{
    u64 start = 0;
    while (start < sizeof(buffer))
    {
        int n = write(connfd, (const char*)(&buffer) + start, sizeof(buffer) - start);
        if (n < 0)
        {
            std::cerr << "[server] Error writing to socket" << std::endl;
            exit(1);
        }
        start += n;
    }
}

template <class T>
void write_into(int connfd, const Mat<T> &buffer)
{
    u64 start = 0;
    char *data = (char *)buffer.data();
    auto size = buffer.size() * sizeof(T);
    while (start < size)
    {
        int n = write(connfd, data + start, size - start);
        if (n < 0)
        {
            std::cerr << "[server] Error writing to socket" << std::endl;
            exit(1);
        }
        start += n;
    } 
}

template <class T>
void loop(const Mat<T> &A, int port = DEFAULT_PORT) 
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
                
                u64 d0, d1;
                read_into(connfd, d0);
                read_into(connfd, d1);
                // std::cerr << "[server] Connection from " << inet_ntoa(cliaddr.sin_addr) << " with a matrix of dimensions " << d0 << "x" << d1 << std::endl;
                Mat<T> inp(d0, d1); 
                // printf("Message From TCP client: "); 
                read_into(connfd, inp);
                // std::cerr << inp(0, 0) << " " << inp(d0 - 1, d1 - 1) << std::endl;
                // print(buffer);
                // std::cout << buffer << std::endl;
                Mat<T> res = A * inp;
                // std::cerr << "[server] Sending back a matrix of dimensions " << res.rows() << "x" << res.cols() << std::endl;
                // std::cerr << res(0, 0) << " " << res(A.rows() - 1, d1 - 1) << std::endl;
                write_into(connfd, res);
                close(connfd); 
                // exit(0); 
            } 
            close(connfd); 
        }
    } 
}

template <class T>
Mat<T> request(const Mat<T> &x, u64 out_d0, std::string ip = "127.0.0.1", int port = DEFAULT_PORT) 
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
    u64 d0 = x.rows();
    u64 d1 = x.cols();
    Mat<T> y(out_d0, d1);
    auto start = std::chrono::high_resolution_clock::now();
    write_into(sockfd, d0);
    write_into(sockfd, d1);
    write_into(sockfd, x);
    // printf("Message from server: "); 
    read_into(sockfd, y);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time for communication: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
    // std::cout << y << std::endl;
    close(sockfd); 
    return y;
} 
