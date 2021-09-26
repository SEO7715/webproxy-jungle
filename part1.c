#include <stdio.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *prox_hdr = "Proxy-Connection: close\r\n";
static const char *host_hdr_format = "Host: %s\r\n";
static const char *requestlint_hdr_format = "GET %s HTTP/1.0\r\n";
static const char *endof_hdr = "\r\n";

static const char *connection_key = "Connection";
static const char *user_agent_key= "User-Agent";
static const char *proxy_connection_key = "Proxy-Connection";
static const char *host_key = "Host";

void doit(int connfd);
void parse_uri(char *uri,char *hostname,char *path,int *port);
void build_http_header(char *http_header,char *hostname,char *path,int port,rio_t *client_rio);
int connect_endServer(char *hostname,int port,char *http_header);

int main(int argc,char **argv)
{
    int listenfd, connfd;
    socklen_t  clientlen;
    char hostname[MAXLINE], port[MAXLINE];

    struct sockaddr_storage clientaddr; /* generic sockaddr struct which is 28 Bytes. The same use as sockaddr */

    if (argc != 2) {
        fprintf(stderr, "usage :%s <port> \n", argv[0]);
        exit(1);
    }

    /* open a listening socket */
    listenfd = Open_listenfd(argv[1]);

    while(1){
        clientlen = sizeof(clientaddr);
        /* connect to client */
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        /*print accepted message*/
        Getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s %s).\n", hostname, port);

        /*sequential handle the client transaction*/
        doit(connfd);
        Close(connfd);
    }
    return 0;
}

/* handle the client HTTP transaction */
void doit(int connfd)
{
    int end_serverfd; /* the end server file descriptor */

    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char endserver_http_header[MAXLINE];

    /* store the request line arguments */
    char hostname[MAXLINE], path[MAXLINE];
    int port;

    rio_t rio, server_rio; /* rio is client's rio, server_rio is endserver's rio*/

    /* connection to client */
    Rio_readinitb(&rio, connfd); // rio(buffer)와 connfd 연결 // connfd로 rio_t 구조체를 초기화
    Rio_readlineb(&rio, buf, MAXLINE); // rio(connfd)에 있는 내용을 buf에 라인별로 기록 
    sscanf(buf, "%s %s %s", method, uri, version); /* read the client request line */

    /* only accept GET method */
    if (strcasecmp(method, "GET")) {
        printf("Proxy does not implement the method");
        return;
    }
    /* parse the uri to get hostname, file path, port */ 
    // tiny에선 URI parse 해서 URI, filename, cgiargs로 나눔
    parse_uri(uri, hostname, path, &port);

    /* build the http header which will send to the end server */
    // end server에 보낼 http header
    build_http_header(endserver_http_header, hostname, path, port, &rio);

    /* connect to the end server */
    end_serverfd = connect_endServer(hostname, port, endserver_http_header);

    if (end_serverfd < 0) {
        printf("connection failed\n");
        return;
    }

    /* connection to end_server */
    Rio_readinitb(&server_rio, end_serverfd);
    /* write the http header to endserver */
    Rio_writen(end_serverfd, endserver_http_header, strlen(endserver_http_header)); //end_serverfd에 내용을 endserver_http_header에 기록해서 전송

    /* receive message from end server and send to the client */
    size_t n;
    while ((n = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0) {
        printf("proxy received %d bytes, then send\n", n);
        Rio_writen(connfd, buf, n);
    }
    Close(end_serverfd);
}

/* client request를 바탕으로 End server에 보낼 헤더 만드는 함수 */
// build_http_header(endserver_http_header, hostname, path, port, &rio);
void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *client_rio) {

    char buf[MAXLINE], request_hdr[MAXLINE], other_hdr[MAXLINE], host_hdr[MAXLINE];

    /* request line */
    sprintf(request_hdr, requestlint_hdr_format, path);
    // request_hdr에 "GET %s HTTP/1.0\r\n" 형식 중 s에 path 넣어서 출력하기

    /* get other request header for client rio and change it */
    while (Rio_readlineb(client_rio, buf, MAXLINE) > 0) {

        /* End of request header */
        if (strcmp(buf, endof_hdr) == 0)  break;

        /* Host */ 
        // HOST 가 있으면 host_hdr의 내용 buf에 기록 복사
        if (!strncasecmp(buf, host_key, strlen(host_key))){
            strcpy(host_hdr, buf);
            continue;
        }
        //strcpy : 문자열 복사 //strcat : 문자열 결합
        /* Other headers: Connection, Proxy-Connection, User-Agent */
        // connection_key, proxy_connection_key, user_agent_key 이 있으면 other_hdr와 buf 결합
        if (!strncasecmp(buf, connection_key, strlen(connection_key))
                &&!strncasecmp(buf, proxy_connection_key, strlen(proxy_connection_key))
                &&!strncasecmp(buf, user_agent_key, strlen(user_agent_key))) {
            strcat(other_hdr, buf);
        }
    }

    /* fill in Host-tag if it is empty */
    // host_hdr 내 "Host: %s\r\n" 중 s에 hostname 기재해서 출력
    if (strlen(host_hdr) == 0) {
        sprintf(host_hdr, host_hdr_format, hostname);
    }

    /* concatenate all header tags into http_header */
    sprintf(http_header,"%s%s%s%s%s%s%s",
            request_hdr,    // "GET file-path HTTP/1.0"
            host_hdr,       // "Host"
            conn_hdr,       // "Connection: close\r\n"
            prox_hdr,       // Proxy-Connection: close\r\n"
            user_agent_hdr, // "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n"
            other_hdr,      // "Connection; Proxy-Connection; User-Agent"
            endof_hdr);     // "\r\n"
    return;
}

/* Connect to the end server */
// inline함수: 호출하지 않고, 함수의 코드를 그 자리에서 그대로 실행(컴파일러가 함수를 복제하여 넣어줌)
// 코드를 그대로 복제 --> 실행속도가 빠르나, 코드가 길어져 실행파일의 크기가 커진다,
inline int connect_endServer(char *hostname, int port, char *http_header){
    char portStr[100];
    sprintf(portStr, "%d", port); //portStr에 port번호 `저장하여 출력
    return Open_clientfd(hostname, portStr);    // end_server에게 proxy는 client 
    // port는 int형이므로 char형으로 바꿔서 넣어주기
}

/* parse the uri to get hostname, file path, port */
void parse_uri(char *uri, char *hostname, char *path, int *port) {
    // 일반적인 URI format: http://host:port/path?query_string

    *port = 80;
    char* pos = strstr(uri, "//");

    pos = pos != NULL ? pos+2 : uri;    // pos: uri에서 "http://" 바로 뒷부분

    char *pos2 = strstr(pos, ":");      // port번호 있는지 여부
    // port번호 있을때: "host:port/path?query_string"
    if (pos2 != NULL) {
        *pos2 = '\0';
        sscanf(pos, "%s", hostname);        // "host"
        sscanf(pos2+1, "%d%s", port, path); // "port/path?query_string"
        if (strcmp(path, "") == 0) // path 부분이 공백일 경우, "/"가 있는 것으로 인식하도록 설정
            strcpy(path, "/");
    }
    // port번호 없을때: "host/path?query_string"
    else {
        pos2 = strstr(pos, "/");
        // path 있을때
        if (pos2 != NULL) {
            *pos2 = '\0';       // 먼저 host알아내기 위해 문자열 분리
            sscanf(pos,"%s", hostname);
            *pos2 = '/';        // '/'복구 후 path기록
            sscanf(pos2,"%s", path);
        }
        // path 없을때: "host"
        else {
            sscanf(pos,"%s", hostname);
            strcpy(path, "/");
        }
    }
}

