//
// Created by acmery on 2020/7/16.
//

#ifndef SIMPLE_SERVER_HTTP_CONN_HPP
#define SIMPLE_SERVER_HTTP_CONN_HPP

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ctime>
#include <cstdlib>

#include "WebServer.hpp"
#include "Utils.hpp"

class http_conn {
public:
    static const int READ_BUFFER_SIZE = 2048;

    explicit http_conn(WebServer *s);

    char *getUrl();

    int parseLine();
    bool readOnce(int fd);
    int parseRequestLine(char *text);
    char* getLine() {return m_read_buf + m_start_line; }

    void do_response(int fd);

    void a(int connfd);
    void b(int connfd);
    void getServerInfo(int connfd);
    static int sendRes(int connfd,const char *text);

    char *m_url;

private:

    Utils util;
    char m_read_buf[READ_BUFFER_SIZE];
    int m_read_idx;
    int m_checked_idx;
    int m_start_line;
    char *m_version;
    char m_header_buf[50];//HTTP响应

    WebServer *m_server;

};


#endif //SIMPLE_SERVER_HTTP_CONN_HPP
