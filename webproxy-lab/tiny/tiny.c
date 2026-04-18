/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
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
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen); // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);  // line:netp:tiny:doit
    Close(connfd); // line:netp:tiny:close
  }
}


void doit(int fd)
{
  int is_static; /* 정적 파일 요청 여부 판단 */
  struct stat sbuf; /* 이건 아마 파일의 상태를 확인한 값을 넘겨받기 위한 거*/
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE]; /* TODO. cgiargs는 뭐지? */
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
  sscanf(buf, "%s %s %s", method, uri, version);
  if (strcasecmp(method, "GET") != 0) {
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");

    return;
  }
  read_requesthdrs(&rio);
  
  /* parse_uri로 uri를 넘겨주고 filename, cgiargs를 파싱하기. 결과는 static 여부를 반환*/
  is_static = parse_uri(uri, filename, cgiargs);
  printf("uri: %s, filename: %s, cgiargs: %s\n\n", uri, filename, cgiargs);
  /* 존재하지 않는 파일인 경우 404 에러 반환 후 종료 */
  if (stat(filename, &sbuf) < 0) {
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }

  /* static 인 경우 static content를 반환(serve_static 호출) */
  if (is_static) { /* Serve static content */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read this file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size);
  }
  /* dynamic인 경우 dynamic content를 반환(serve_dynamic 호출) */
  else {
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
      return;
    }
    serve_dynamic(fd, filename, cgiargs);
  }
}

void read_requesthdrs(rio_t *rp) {
  char buf[MAXLINE];

  
  while (1) {
    Rio_readlineb(rp, buf, MAXLINE);
    if (strcmp(buf, "\r\n") == 0) {
      break;
    }
    printf("%s", buf);
  }
  return;
}

int parse_uri(char *uri, char *filename, char *cgiargs) {
  char *ptr;

  if (strstr(uri, "cgi-bin") == NULL) {
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    if (strlen(uri) == 1 && uri[strlen(uri) - 1] == '/') {
      strcat(filename, "home.html");
    }
    return 1;
  } else {
    ptr = index(uri, '?');
    if (ptr) {
      strcpy(cgiargs, ptr+1);
      *ptr = '\0';
    } else {
      strcpy(cgiargs, "");
    }
    strcpy(filename, ".");
    strcat(filename, uri);
    return 0;
  }
}

void serve_static(int fd, char *filename, int filesize) {
}

void get_filetype(char *filename, char *filetype) {
}

void serve_dynamic(int fd, char *filename, char *cgiargs) {
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