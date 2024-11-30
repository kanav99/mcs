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
const int num_threads = 4;

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
void read_into(int connfd[num_threads], T& buffer)
{
    u64 start = 0;
    while (start < sizeof(buffer))
    {
        int n = read(connfd[0], (char *)(&buffer) + start, sizeof(buffer) - start);
        if (n < 0)
        {
            std::cerr << "[server] Error reading from socket" << std::endl;
            exit(1);
        }
        start += n;
    }
}

template <class T>
void write_into(int connfd[num_threads], const T& buffer)
{
    u64 start = 0;
    while (start < sizeof(buffer))
    {
        int n = write(connfd[0], (const char*)(&buffer) + start, sizeof(buffer) - start);
        if (n < 0)
        {
            std::cerr << "[server] Error writing to socket" << std::endl;
            exit(1);
        }
        start += n;
    }
}

void read_into(int connfd, char *data, u64 size)
{
    u64 start = 0;
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

void write_into(int connfd, const char *data, u64 size)
{
    u64 start = 0;
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

std::pair<u64, u64> chunk(u64 size, int i)
{
    u64 chunk = size / num_threads;
    u64 start = i * chunk;
    u64 end = (i == num_threads - 1) ? size : (i + 1) * chunk;
    u64 cs = end - start;
    return {start, cs};
}

template <class T>
void read_into(int connfd[num_threads], Mat<T> &buffer)
{
    u64 start = 0;
    char *data = (char *)buffer.data();
    auto size = buffer.size() * sizeof(T);

    // #pragma omp parallel for num_threads(num_threads)
    for (int i = 0; i < num_threads; i++)
    {
        auto [s, cs] = chunk(size, i);
        read_into(connfd[i], data + s, cs);
    } 
}

template <class T>
void write_into(int connfd[num_threads], const Mat<T> &buffer)
{
    u64 start = 0;
    char *data = (char *)buffer.data();
    auto size = buffer.size() * sizeof(T);

    // #pragma omp parallel for num_threads(num_threads)
    for (int i = 0; i < num_threads; i++)
    {
        auto [s, cs] = chunk(size, i);
        write_into(connfd[i], data + s, cs);
    } 
}

template <class T>
void loop(const Mat<T> &A, int port = DEFAULT_PORT) 
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
 
    struct sockaddr_in cliaddr, servaddr; 
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

    for (;;) { 
        socklen_t len = sizeof(cliaddr);
        int connfd[num_threads];
        for (int i = 0; i < num_threads; i++)
        {
            connfd[i] = accept(listenfd, (struct sockaddr*)&cliaddr, &len); 
        } 
        
        u64 d0, d1;
        read_into(connfd, d0);
        read_into(connfd, d1);
        Mat<T> inp(d0, d1); 
        read_into(connfd, inp);
        auto start = std::chrono::high_resolution_clock::now();
        Mat<T> res = A * inp;
        auto end = std::chrono::high_resolution_clock::now();
        write_into(connfd, res);

        for (int i = 0; i < num_threads; i++)
        {
            close(connfd[i]);
        } 
        std::cerr << "Time for computation: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
    } 
}

template <class T>
Mat<T> request(const Mat<T> &x, u64 out_d0, std::string ip = "127.0.0.1", int port = DEFAULT_PORT) 
{ 
    int sockfd[num_threads]; 
    struct sockaddr_in servaddr; 

    for (int i = 0; i < num_threads; i++)
    {
        if ((sockfd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
            printf("socket creation failed"); 
            exit(0); 
        }
    } 

    memset(&servaddr, 0, sizeof(servaddr)); 

    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port); 
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str()); 

    for (int i = 0; i < num_threads; i++)
    {
        if (connect(sockfd[i], (struct sockaddr*)&servaddr, 
                                sizeof(servaddr)) < 0) { 
            printf("\n Error : Connect Failed \n"); 
        } 
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
    for (int i = 0; i < num_threads; i++)
    {
        close(sockfd[i]);
    } 
    return y;
} 
