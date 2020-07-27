//
// Created by acmery on 2020/7/16.
//

#include "WebServer.hpp"
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <pthread.h>

WebServer::WebServer(char *ipaddr, int port, char* tor){
    m_port = port;
    is_tor_start = false;
    strcpy(m_ipv4_addr, ipaddr);
    strcpy(tor_port, tor);
    start_time.stop();
}

int WebServer::eventListen() {
    m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(m_listenfd >= 0);

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl((INADDR_ANY));
    address.sin_port = htons(m_port);

    int flag = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(m_listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(m_listenfd, 5);
    assert(ret >= 0);
}

int WebServer::getfd() {
    return m_listenfd;
}

int WebServer::postReq(char *url, const char *text) {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    struct  curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type:application/json");
    if (curl){
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, text);
        res = curl_easy_perform(curl);
        std::cout << "res:" << res << std::endl;
        curl_easy_cleanup(curl);
        return -1;
    }
    return res;
}

void* catchCap(void* arg) {
    std::cout << "into catchTorCap thread..." << std::endl;
    char filter_port[] = "port ";
    char filter_middle[] = " and ip host ";
    char filter[128] = "\0";
    char *dev, errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    struct bpf_program fp;
    bpf_u_int32 net;

    WebServer *server = (WebServer *)arg;

    strcat(filter, filter_port);
    strcat(filter, server->tor_port);
    strcat(filter, filter_middle);
    strcat(filter, server->getIpv4Addr());
    std::cout << filter << std::endl;

    const struct sniff_ethernet *ethernet; /* The ethernet header */
    const struct sniff_ip *ip; /* The IP header */
    const struct sniff_tcp *tcp; /* The TCP header */
    const char *payload; /* Packet payload */

    u_int size_ip;
    u_int size_tcp;

    //
    dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
        fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
    }

    std::cout << "device: " << dev << std::endl;

    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
    }
    if (pcap_compile(handle, &fp, filter, 0, net) == -1) {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter, pcap_geterr(handle));
    }
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter, pcap_geterr(handle));
    }

    while (server->is_tor_start){
        pcap_loop(handle, 1, WebServer::got_packet, reinterpret_cast<u_char *>(server));
    }

}

void WebServer::got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    const struct sniff_ethernet *ethernet; /* The ethernet header */
    const struct sniff_ip *ip; /* The IP header */
    const struct sniff_tcp *tcp; /* The TCP header */
    const char *payload; /* Packet payload */
    WebServer *server = (WebServer *)args;

    struct ip_data *ip_d;
    struct tcp_data *tcp_d;
    struct ethernet_data *ether_d;

    u_int size_ip;
    u_int size_tcp;

    ip_d = (struct ip_data*)malloc(sizeof(struct ip_data));
    tcp_d = (struct tcp_data*)malloc(sizeof(struct tcp_data));
    ether_d = (struct ethernet_data*)malloc(sizeof(struct ethernet_data));

    ethernet = (struct sniff_ethernet*)(packet);
    for (int i = 0; i < ETHER_ADDR_LEN; i++){
        ether_d->mac_src[i] = ethernet->ether_shost[i];
        ether_d->mac_dst[i] = ethernet->ether_dhost[i];
    }

#ifdef DEBUG
    //printf("ethernet src:%x");
    //printf("ethernet dst:%x");
#endif

    ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
    size_ip = IP_HL(ip)*4;
    if (size_ip < 20) {
        printf("   * Invalid IP header length: %u bytes\n", size_ip);
        return;
    }
    ip_d->ip_src = ip->ip_dst;
    ip_d->ip_dst = ip->ip_src;

}

int WebServer::startCatch() {
    pthread_t th;
    int ret = pthread_create(&th, NULL, catchTorCap, this);
    if (ret != 0){
        std::cout << "create thread error!" << std::endl;
        return -1;
    }
    return 0;
}
