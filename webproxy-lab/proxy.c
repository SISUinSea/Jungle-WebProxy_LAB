#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";


void doit(int fd);
void read_requesthdrs(rio_t *rp);

int parse_uri(char* request, char* host, char* port, char *path_and_query);
int send_request_to_server(int fd, char* method, char* path_and_query, char* http_version, char* host, rio_t* rp);
int send_response_to_client(int fd, int clientfd);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
  char *longmsg);




int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = accept(listenfd, (SA *)&clientaddr, &clientlen); // line:netp:tiny:accept

    if (connfd < 3) {
      printf("Accept fail\n");
      continue;
    }

    getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);  // line:netp:tiny:doit
    Close(connfd); // line:netp:tiny:close
  }
}


void doit(int fd)
{
  int clientfd; 
  struct stat sbuf; /* 이건 아마 파일의 상태를 확인한 값을 넘겨받기 위한 거*/
  char buf[MAXLINE], method[MAXLINE], request[MAXLINE], version[MAXLINE];
  char host[MAXLINE], port[MAXLINE], path_and_query[MAXLINE];
  rio_t rio;

  /*   첫 번째 헤더(GET / HTTP/1.0)만 받아서 처리
      첫 번째 헤더의 method가 GET이 아니면 
          clienterror
                      501 Not implemented, Tiny does not implement this method
      첫 번째 헤더 이외의 헤더들은 모두 무시(read_requesthdrs)
  */
  rio_readinitb(&rio, fd);
  rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, request, version);
  strcpy(version, "HTTP/1.0");
  if (strcasecmp(method, "GET") != 0 && strcasecmp(method, "HEAD") != 0) {
    clienterror(fd, method, "501", "Not implemented", "Proxy does not implement this method");

    return;
  }
  // read_requesthdrs(&rio);
  
  /* parse_uri로 uri를 넘겨주고 filename, cgiargs를 파싱하기. 결과는 static 여부를 반환*/
  if (parse_uri(request, host, port, path_and_query) == -1) {
    clienterror(fd, method, "400", "Bad Request", "Cannot parse request.");

    return;
  }
  printf("host: %s, port: %s, path_and_query: %s\n\n", host, port, path_and_query);

  clientfd = open_clientfd(host, port);

  if (clientfd < 3) {
    clienterror(fd, method, "500", "Server error", "Cannot connect with requested server.");
    return;
  }
  
  /* 서버에 요청 보내기 */
  send_request_to_server(clientfd, method, path_and_query, version, host, &rio);
  send_response_to_client(fd, clientfd);

  Close(clientfd);

}

// void read_requesthdrs(rio_t *rp) {
//   char buf[MAXLINE];

  
//   while (1) {
//     Rio_readlineb(rp, buf, MAXLINE);
//     if (strcmp(buf, "\r\n") == 0) {
//       break;
//     }
//     printf("%s", buf);
//   }
//   return;
// }


/*
@return 0 on success, -1 on fail
*/
int parse_uri(char *request, char *host, char *port, char* path_and_query) {
  int index;
  char *ptr = request, *temp;
  char *colon = NULL, *slash = NULL;

  /*
    request는 다음과 같은 형태를 띔.
    "http://localhost:8080/filename.type?argument=1&argument2=2"
    "http://localhost:8080"
    

    위와 같은 request가 나왔을 때 파싱 결과는 다음과 같아야 함.
      host-> "localhost"
      port-> "80"
      path_and_query-> "/filname.type?argument=1&argument2=2"(아무런 값도 주어지지 않은 경우에는 "/"이어야 함.)
  */

  strcpy(path_and_query, "/\0");
  strcpy(port, "80\0");

  /* http:// 가 없다면 fail */
  if (strncmp(ptr, "http://", 7) != 0) {
    return -1;
  }

  /* http:// 건너뛰기 */
  ptr += 7;

  colon = strchr(ptr, ':');
  slash = strchr(ptr, '/');

  /* domain */
  if (colon == NULL && slash == NULL) {
    strcpy(host, ptr);
  } else if (colon == NULL && slash != NULL) { /* domain + /uri */
    strncpy(host, ptr, slash - ptr);
    host[slash - ptr] = '\0';
    strcpy(path_and_query, slash);
  } else if (colon != NULL && slash == NULL) { /* domain:1234 */
    strncpy(host, ptr, colon - ptr);
    host[colon - ptr] = '\0';
    strcpy(port, colon + 1);
  } else { /* domain:1234/uri */
    strncpy(host, ptr, colon - ptr);
    host[colon - ptr] = '\0';
    strncpy(port, colon + 1, slash - (colon + 1));
    port[slash - (colon + 1)] = '\0';
    strcpy(path_and_query, slash);
  }

  return 0;
}


int send_request_to_server(int fd, char* method, char* path_and_query, char* http_version, char* host, rio_t* rp)
{
  char buf[MAXLINE];
  int has_host_hdr = 0;
  sprintf(buf, "%s %s %s\r\n", method, path_and_query, http_version);
  printf("buf: %s", buf);
  Rio_writen(fd, buf, strlen(buf));

  sprintf(buf, "Connection: close\r\n");
  printf("buf: %s", buf);
  Rio_writen(fd, buf, strlen(buf));

  sprintf(buf, "Proxy-Connection: close\r\n");
  printf("buf: %s", buf);
  Rio_writen(fd, buf, strlen(buf));

  while (1) {
    Rio_readlineb(rp, buf, MAXLINE);
    if (strncmp(buf, "Proxy-Connection", 16) == 0 || 
        strncmp(buf, "User-Agent", 10) == 0 || 
        strncmp(buf, "Connection", 10) == 0 
      ) {
      continue;
    }
    if (strncmp(buf, "Host", 4) == 0) {
      has_host_hdr = 1;
    }
    if (strcmp(buf, "\r\n") == 0) {
      break;
    }
    printf("buf: %s", buf);
    Rio_writen(fd, buf, strlen(buf));
  }

  if (has_host_hdr == 0) {
    sprintf(buf, "Host: %s\r\n", host);
    printf("buf: %s", buf);
    Rio_writen(fd, buf, strlen(buf));
  }
  
  sprintf(buf, "%s\r\n", user_agent_hdr);
  Rio_writen(fd, buf, strlen(buf));
  printf("buf: %s", buf);

  return 0;
}


int send_response_to_client(int fd, int clientfd){
  rio_t rio;
  int readn;

  char buf[MAXLINE];

  rio_readinitb(&rio, clientfd);
  
  while (1) {
    readn = rio_readnb(&rio, buf, MAXLINE);
    if (readn <= 0) {
      break;
    }
    rio_writen(fd, buf, readn);
    printf("%s", buf);
  }
  
  return 0;
}



void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
  char buf[MAXLINE], body[MAXBUF];

  body[0] = '\0';

  sprintf(body + strlen(body), "<html><title>Tiny Error</title>");
  sprintf(body + strlen(body), "<body bgcolor=\"ffffff\">\r\n");
  sprintf(body + strlen(body), "%s: %s\r\n", errnum, shortmsg);
  sprintf(body + strlen(body), "<p>%s: %s\r\n", longmsg, cause);
  sprintf(body + strlen(body), "<hr><em>The Tiny Web Server</em>\r\n");

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));

  Rio_writen(fd, body, strlen(body));
}


/**
 * 1. 모든 ip addr, port에 대해서 요청을 받는 listen port를 생성한다.
 * 2. accept으로 listenfd를 지켜보다가 새로운 요청이 들어오면 
 *  2.1. 해당 요청을 실제 서버로 보낸다.
 *  2.2. 응답을 받았을 때, connfd로 해당 연결을 넘겨준다.
 * 
*/