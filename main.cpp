#include "WebServer.hpp"
#include "http_conn.hpp"
#include <vector>
#include <sys/param.h>


using namespace std;
#define MYPORT 37824

void init_daemon(void)
{
    int pid;
    int i;
    if(pid=fork())
        exit(0);
    else if(pid< 0)
        exit(1);

    setsid();

    if(pid=fork())
        exit(0);
    else if(pid< 0)
        exit(1);

    for(i=0;i< NOFILE;++i)
        close(i);
    umask(0);
    return;
}

int main(int argc, char* argv[]){

    if (argc != 3){
        cout << "use: {" << argv[0] << " ipaddr tor_port} to start server." << endl;
        return 1;
    }
    init_daemon();

    WebServer *server = new WebServer(argv[1], MYPORT, argv[2]);
    server->eventListen();
    int listenfd = server->getfd();
    char *text = 0;
    cout << "listen successful" << endl;

    while (1) {   //不断执行
        http_conn *h = new http_conn(server);
        struct sockaddr_in client;
        socklen_t client_addrlength = sizeof(client);
        int connfd = accept(listenfd, (struct sockaddr*)&client, &client_addrlength);

        if (connfd < 0){
            printf("errno\n");
        }
        else{
            h->readOnce(connfd);
            h->parseLine();
            text = h->getLine();
            h->parseRequestLine(text);
            cout << "url: " << h->getUrl() << endl;

            h->do_response(connfd);

            close(connfd);
        }
        delete(h);
    }
    return 0;
}
