#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define PORT 110
#define SERVER "pop.qip.ru"
#define USER ""
#define PASS ""

int get_ip_by_hostname(char *ip) {
	struct hostent *he;
	struct in_addr **addr_list;
	if ((he = gethostbyname(SERVER)) == NULL) {
		perror("GETHOSTBYNAME");
		return 1;
	}
	addr_list = (struct in_addr **) he->h_addr_list;
	if (addr_list[0] == NULL) {
		perror("Addr list empty");
		return 1;
	}
	else { 
		strcpy(ip, inet_ntoa(*addr_list[0]));
		printf("Succsessfully get ip %s\n", ip);
		return 0;
	}
}


int main(int argc, char **argv) {
	struct sockaddr_in serv_addr;
	int sockfd = 0;
	char buf[1024];
	char ip[32];
    int n;
	if (get_ip_by_hostname(ip) != 0) {
		perror("Error by getting ip");
		return -1;
	}
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Can't create socket");
		return -1;
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		perror("Inet_Pton error");
	}

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("Connect error");
		return -1;
	}
    //wrte(sockfd, "\n", 1);
    write(sockfd, "USER ", 5);
    write(sockfd, USER, strlen(USER));
    write(sockfd, "\n", 1);
    int state = 0;
    while ((n = read(sockfd, buf, sizeof(buf))) > 0) {
        if (!strncmp(buf, "OK", sizeof(buf)))
                state++;

        printf("%s!!\n",buf;
    }
    close(sockfd);
}

