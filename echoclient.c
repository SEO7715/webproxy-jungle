#include "csapp.h"

// echo client는 서버와의 연결 수립한 후, 클라이언트는 표준 입력에서 텍스트 줄을 반복해서 읽는 루프에 진입하고,
// 서버에 텍스트 줄을 전송, 서버에서 echo 줄을 읽어서 그 결과를 표준 출력으로 인쇄

int main(int argc, char **argv) {

    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    // 예외처리
    if(argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    host = argv[1]; //첫번째 인자 
    port = argv[2]; //두번째 인자

    clientfd = Open_clientfd(host, port); //host : port에 연결한 파일 clientfd 생성 (server의 connectfd와 연결됨)
    Rio_readinitb(&rio, clientfd);
    // rio(buffer)와 clientfd 연결
    // clientfd로 rio_t 구조체를 초기화

    while(Fgets(buf, MAXLINE, stdin) != NULL) { // Fgets(buf, MAXLINE, stdin)에서 입력받아 buf에 저장
        Rio_writen(clientfd, buf, strlen(buf)); // buf 내용을 clientfd에 write -> connectfd에도 전송되어 각각 동시에 write됨
        // rio -> clientfd <-> connectfd
        // strlen 문자열 길이
        Rio_readlineb(&rio, buf, MAXLINE); 
        // server에서 처리한 내용이 connectfd를 통해 clientfd에 기록된 상태
        // 텍스트 줄을 파일 rio에서 읽고, 이것을 메모리 위치 buf로 복사하고, 텍스트라인을 null문자로 종료시킴
        // read/write에서 fd를 인자로 받아 사용했다면, rio_t라는 별도의 구조체가 존재함. 
        // 이 구조체에는 fd 정보는 물론 내부적인 임시 버퍼와 관련된 정보도 포함되어 있음
        
        Fputs(buf, stdout); //rio를 읽어서 clientfd를 읽는 것과 동일 -> buf에 저장 
        // buf 내용을 stdout으로 출력
    }
    Close(clientfd); //
    exit(0);
}