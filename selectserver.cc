#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#define MAXDATASIZE 1000
#define BACKLOG 10

int main(int argc, char *argv[])
{
    int sockfd;
    int newfd;
    struct addrinfo hints, *servinfo;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET_ADDRSTRLEN];
    int rv;
    fd_set master;
    fd_set read_fds;
    int fdmax;
    char buf[MAXDATASIZE];

    int numbytes;
    if (argc != 2)
    {
        printf("usage: server portnum\n");
        exit(1);
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo - > ai_protocol)) == -1)
    {
        perror("server: socket");
        exit(1);
    }
    if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        close(sockfd);
        perror("server: bind");
        exit(1);
    }
    freeaddrinfo(servinfo);
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }
    FD_SET(sockfd, &master);
    fdmax = sockfd;
    while (1)
    {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(1);
        }
        for (int i = 0; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (i == sockfd)
                {
                    sin_size = sizeof their_addr;
                    newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
                    if (newfd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        FD_SET(newfd, &master);
                        if (newfd > fdmax)
                        {
                        }
                    }
                    printf("selectserver: new connection from %s on socket %d\n",
                           inet_ntop(their_addr.ss_family, &((struct sockaddr_in *)&their_addr)->sin_addr, s, sizeof s), newfd);
                }
            }
            else
            {
                if ((numbytes = recv(i, buf, sizeof buf, 0)) <= 0)
                {
                    if (numbytes == 0)
                    {
                        printf("selectserver: socket %d hung up\n", i);
                    }
                    else
                    {
                        perror("recv");
                    }
                    close(i);
                    FD_CLR(i, &master);
                }
                else
                {
                    buf[numbytes] = '\0';
                    printf("server received: %s\n", buf);
                    if (send(i, buf, strlen(buf), 0) == -1)
                    {
                        perror("send");
                    }
                }
            }
        }
    }
}
