#include "csapp.h"

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    // 연결할 주소 정보를 인자로 받는다.
    host = argv[1];
    port = argv[2];


    // 주소 정보를 넘겨주고 client socket을 생성한다.
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    // 사용자가 입력을 종료하기 전까지
    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        // 사용자가 입력한 값을 socket을 통해 서버로 보낸다.
        Rio_writen(clientfd, buf, strlen(buf));
        // 서버가 응답한 값을 socket을 통해 읽어온다.
        Rio_readlineb(&rio, buf, MAXLINE);
        // 해당 서버의 응답 값을 출력한다.
        Fputs(buf, stdout);
    }

    // 종료하기 전에 소켓을 닫는다.
    Close(clientfd);
    exit(0);
}
