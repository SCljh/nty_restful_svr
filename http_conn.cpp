//
// Created by acmery on 2020/7/16.
//

#include <signal.h>
#include <jsoncpp/json/json.h>

#include "http_conn.hpp"

http_conn::http_conn(WebServer *s) : m_server(s){

    m_url = 0;
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_version = 0;
    strcpy(m_header_buf, "HTTP/1.1 200 ok\r\nconnection: close\r\n\r\n");

    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
}

char* http_conn::getUrl() {
    return m_url;
}

int http_conn::parseRequestLine(char *text) {
    //格式：请求类型 \t要访问的资源 \tHTTP版本
    //获得请求行中最先含有空格和\t的人以字符的位置并返回
    m_url = strpbrk(text, " \t");
    if (!m_url)
        return 0;

    //将该位置改为\0，将m_url记录为\0下一个位置(要访问的资源的第一个字符)并将前面数据输出
    *m_url++ = '\0';

    //让m_url的位置跳过潜在的 \t
    m_url += strspn(m_url, " \t");

    //使用与判断请求方式相同的方法潘对HTTP版本号
    m_version = strpbrk(m_url, " \t");
    if (!m_version)
        return 0;
    *m_version++ = '\0';
    m_version += strspn(m_version, " \t");
    //当前服务器仅支持HTTP/1.1
    if (strcasecmp(m_version, "HTTP/1.1") != 0)
        return 0;

    //对资源请求的前7个字符进行判断（单独处理http://的情况）
    if (strncasecmp(m_url, "http://", 7) == 0){
        m_url += 7;
        m_url = strchr(m_url, '/');
    }

    //同样https的情况
    if (strncasecmp(m_url, "https://", 8) == 0){
        m_url += 7;
        m_url = strchr(m_url, '/');
    }

    //一般不会带有上述两种符号，直接时单独的/或/后面带访问资源
    if (!m_url || m_url[0] != '/')
        return 0;
    return 1;
}

int http_conn::parseLine() {
    char temp;
    for (; m_checked_idx < m_read_idx; ++m_checked_idx){
        //将要分析的那个字节
        temp = m_read_buf[m_checked_idx];

        //如果当前字符为'\r'
        if (temp == '\r'){
            //如果下一个字符到达了m_read_buf末尾，则接受不完整，需要继续接受
            if ((m_checked_idx + 1) == m_read_idx)
                return 0;
                //下一个字符为'\n'，将\r\n改为\0\0，返回行处理完毕
            else if (m_read_buf[m_checked_idx+1] == '\n'){
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return 1;
            }
            //如果都不是，语法错误
            return 0;
        }

            //如果当前字符为\n，也有可能读取到完整行了
            //一般是赏赐读取到\r的时候没有接受完整，再次接受将出现此情况
        else if (temp == '\n'){
            //前一个字符是\r，表示一切正常
            if (m_checked_idx > 1 && m_read_buf[m_checked_idx - 1] == '\r'){
                m_read_buf[m_checked_idx-1] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return 1;
            }
            return 0;
        }
    }
    //到了末尾没有\r\n，则继续接受
    return 1;
}

bool http_conn::readOnce(int fd) {
    if (m_read_idx >= READ_BUFFER_SIZE){
        return false;
    }
    //接受到的数据的大小
    int bytes_read = 0;
//    while (true){
    //接受数据并存储在m_read_buf中
    bytes_read = recv(fd, m_read_buf + m_read_idx, READ_BUFFER_SIZE-m_read_idx, 0);
    if (bytes_read == -1){
        //当前没数据可读了(非阻塞ET)
        if (errno==EAGAIN||errno==EWOULDBLOCK)
            //break;
            return false;
    }
    else if (bytes_read == 0){
        return false;
    }
    //将m_read_idx更新
    m_read_idx += bytes_read;
//    }
    return true;
}

void http_conn::do_response(int connfd) {
    //解析m_url
    if (strcmp(m_url, "a") == 0){
        stopTor(connfd);
    }
    else if (strcmp(m_url, "b") == 0){
        startTor(connfd);
    }
    else if (strcmp(m_url, "/GetServerInfo") == 0){
        //sendRes(connfd, "gettingbb info...");
        getServerInfo(connfd);
    }
    else {
        sendRes(connfd, "404 Not Found...");
    }
}

int http_conn::sendRes(int connfd,const char *text){
    char *buf = "HTTP/1.1 200 ok\r\nconnection: close\r\n\r\n";
    int s = send(connfd, buf, strlen(buf), 0);
    std::cout << text << std::endl;
    send(connfd, text, strlen(text), 0);
}

void http_conn::a(int connfd) {
    
}

void http_conn::b(int connfd) {
    
}

void http_conn::getServerInfo(int connfd) {
    std::string text;
    Json::Value root;
    Json::FastWriter fast_writer;
    int runningStatus = 1;
    if (!m_server->is_tor_start) runningStatus = 2;
    root["ip"] = m_server->getIpv4Addr();
    root["local"] = util.getCityByIp();
    root["platform"] = util.getOS();
    std::string s_time = "0";
    root["onlineTime"] = s_time;
    root["port"] = m_server->getPort();
    text = fast_writer.write(root);
    sendRes(connfd, text.c_str());
}
