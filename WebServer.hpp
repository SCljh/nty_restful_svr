//
// Created by acmery on 2020/7/16.
//

#ifndef SIMPLE_SERVER_WEBSERVER_HPP
#define SIMPLE_SERVER_WEBSERVER_HPP

#include <iostream>
#include <stdio.h>
#include <cstring>
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <boost/timer/timer.hpp>

#include "for_pcap.hpp"

class WebServer {

public :
    WebServer(char *ipaddr, int port, char* tor_port);
    int eventListen();
    int getfd();
    int getPort(){return m_port;}
    char* getIpv4Addr() {return m_ipv4_addr;}

    static void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
    int startCatch();

    int postReq(char *url, const char *text);

    boost::timer::cpu_timer start_time;

    char tor_port[6];
    bool is_tor_start;

private:
    char m_ipv4_addr[16];
    int m_port;
    int m_listenfd;
};


#endif //SIMPLE_SERVER_WEBSERVER_HPP
