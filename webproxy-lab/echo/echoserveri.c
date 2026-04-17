#include "csapp.h"

void echo(int connfd);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    // 포트 번호를 인자로 받아 특정 호스트로 들어오는 모든 요청을 처리한다.
    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        // listenfd를 Accept가 지켜보다가 요청이 들어오면 connfd를 반환한다.
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        // 해당 요청이 어떤 client에게로부터 온 것인지 파악하기 위해 Getnameinfo를 수행한다.
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        // connfd로 echo를 실행한다.
        echo(connfd);
        // 해당 connfd를 닫는다.
        Close(connfd);
    }
    exit(0);
}
