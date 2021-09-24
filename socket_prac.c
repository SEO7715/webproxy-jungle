#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>

#define MAXLINE 100

// struct addrinfo {
//     int     ai_flags;           // hints argument flags
//     int     ai_family;          // first arg to socket function
//     int     ai_socktype;        // second arg to socket function
//     int     ai_protocol;        // third arg to socket function
//     char    *ai_cannoname;      // canonical hostname
//     size_t  ai_addrlen;         // size of ai_addr struct
//     struct sockaddr     *ai_addr; //prt to socket address structure
//     struct addrinfo     *ai_next; // ptr to next item in linked list
// };

// int getnameinfo (const struct sockaddr *sa, socklen_t salen, char *host, size_t hostlen, char *service, size_t servlen, int flags);

int main(int argc, char **argv) {
    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE];
    int rc, flags;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
        exit(0);
    }
    // Get a list of addrinfo records
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; //IPv4 only
    hints.ai_socktype = SOCK_STREAM; // Connections only
    if ((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
        exit(1);
    }

    //Walk the list and display each IP address
    flags = NI_NUMERICHOST; //display address string instead of domain name
    for (p= listp; p; p = p -> ai_next) {
        getnameinfo(p-> ai_addr, p -> ai_addrlen, buf, MAXLINE, NULL, 0, flags);
        printf("%s\n", buf);
    }

    // clean up
    freeaddrinfo(listp);

    exit(0);
}