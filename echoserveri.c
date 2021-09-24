#include "csapp.h"

void echo(int connfd);

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; //enough space for any address
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if(argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]); // 주어진 port 로 listenfd 생성
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);

        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); 
        //client에서 connect request를 받고, client 정보를 clientaddr 구조체에 저장(clientfd ~ connectfd 연결된 상태)
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        // client 정보가 담긴 clientaddr 구조체에서 정보를 빼내서 client_hostname, client_port에 각각 client ip주소, client port 번호 저장
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd); // read and echo input lines from client til EOF
        Close(connfd); // close the connection
    }
    exit(0);
}

void echo(int connfd) {
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd); //rio buf와 connectfd 파일 연결
    
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        // (n = Rio_readlineb(&rio, buf, MAXLINE) 
        // rio를 읽는다 <-> connectfd를 읽는다
        printf("server received %d bytes\n", (int)n);
        Rio_writen(connfd, buf, n);
        // buf 내용을 connectfd에 쓰기 -> connectfd에 연결된 clientfd에도 전송됨
    }
}