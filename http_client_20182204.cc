// selectclient.c -- socket client using select()
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#define MAXDATASIZE 1000 // max number of bytes we can get at once
#define MAXPORTSIZE 5

int main(int argc, char *argv[])
{
    int             sockfd, numbytes;
    char            buf[MAXDATASIZE] = "GET ";
    struct addrinfo hints, *servinfo;
    int             rv;
    char            s[INET_ADDRSTRLEN];
    fd_set          master, read_fds;
    char            url[MAXDATASIZE]; //url저장
    char            port[MAXPORTSIZE]="80"; //포트번호 저장
    char            path_to_file[MAXDATASIZE] = "/"; //파일 이름 저장
    char            *charpos; //그냥 위치 찾는 용도
    FILE            *fp;
    //내 ip번호 찾는 용도 지금은 사용 안함
    // struct ifreq    ifr;
    // char            ipstr[40];
    // int             ifs;

    if (argc != 2)
    {
        fprintf(stderr, "usage: http_client http://hostname[:port][/path/to/file]\n");
        exit(1);
    }

    if(strncmp(argv[1],"http://",7) != 0)
    {
        fprintf(stderr, "usage: http_client http://hostname[:port][/path/to/file]\n");
        exit(1);
    }else
    {
        charpos = argv[1]+7;
        for(int j=0;;++j,++charpos)
        {
            if(*charpos == '/' || *charpos == ':') 
            {
                port[j] = 0;
                break;
            }
            url[j] = *charpos;
        }
    }

    if((charpos = strchr(argv[1]+7, ':')) != 0)
    {
        charpos += 1;
        for(int j=0;;++j,++charpos)
        {
            if(*charpos == '/') 
            {
                port[j] = 0;
                break;
            }
            port[j] = *charpos;
        }
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    printf("|%s|%s|\n", url,port);
    if ((rv = getaddrinfo(url, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: Name or service not known\n");
        return (1);

    }
    if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
    {
        perror("client: socket");
        return (2);
    }
    if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        close(sockfd);
        perror("connect: Connection refused");
        exit(1);
    }
    freeaddrinfo(servinfo); // all done with this structure


    //find file
    if((charpos = strchr(argv[1]+7, '/')) != 0)
    {
        int j = 0;
        for(j = 0;*charpos;++j,++charpos)
        {
            if(*charpos == ' '){
                break;
            }
            path_to_file[j] = *charpos;
        }
    }

    /*//find my ip not used
    ifs = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, "eno1", IFNAMSIZ);

	if (ioctl(ifs, SIOCGIFADDR, &ifr) < 0) {
		printf("Error");
	} else {
		inet_ntop(AF_INET, ifr.ifr_addr.sa_data+2, ipstr,sizeof(struct sockaddr));
        //test
		printf("myOwn IP Address is %s\n", ipstr);
	}*/

    //copy GET, file, version, ip
    strncat(buf, path_to_file,sizeof path_to_file);
    strcat(buf, " HTTP/1.1\r\nHost: localhost:8080\r\n\r\n");
    //printf("%s\n", buf);

    if (send(sockfd, buf, strlen(buf), 0) == -1)
    {
        perror("send");
        close(sockfd);
        exit(1);
    }
    while(1){
        
    }
    if ((numbytes = recv(sockfd, buf, sizeof buf, 0)) == -1)
    {
        perror("recv");
        close(sockfd);
        exit(1);
    }
    printf("//////////// %d ////////////////",numbytes);
    buf[numbytes] = '\0';
    //첫번쨰 ip, version 정보 출력
    char *tmp = buf;
    while(*tmp != '\r')
    {
        printf("%c",*(tmp++));
    }
    printf("\n");
    //출력을 위한 소문자화
    tmp = buf;
    while(*tmp != '\0')
    {
        *tmp = tolower(*tmp);
        tmp++;
    }
    //printf("%s\n",buf);
    if((charpos = strstr(buf,"content-length: ")))
    {
        char    length[MAXDATASIZE];
        int     i;

        fp = fopen("20182204.out","w");
        charpos += 16;
        for(i = 0; *charpos != '\r'; ++charpos, ++i)
        {
            length[i] = *charpos;
        }
        length[i] = 0;
        fprintf(fp,"%s",length);
        printf("%s bytes written to 20182204.out\n",length);
    }else
    {
        printf("Content-Length not specified.\n");
    }
    close(sockfd);
    return 0;
}