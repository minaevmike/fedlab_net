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
#include <sys/ioctl.h>

#define PORT 110

int get_ip_by_hostname(char *ip, char *server) {
	struct hostent *he;
	struct in_addr **addr_list;
	if ((he = gethostbyname(server)) == NULL) {
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
void handle_socket(int sockfd) {
    int len;
    int n;
    char buf[1024];
    n = recv(sockfd, buf, sizeof(buf) - 1, 0);
    while (n > 0) {
        buf[n] = '\0';
        printf("%s", buf);
        if (strstr(buf, "\r\n.\r\n") != NULL) {
            break;
        }
        n = recv(sockfd, buf, sizeof(buf) - 1, 0);
    }
    printf("Exiting\n");
}
void print_list(int sockfd) {
    write(sockfd, "LIST\n", 5);
}

void disconnect(int sockfd) {
    write(sockfd, "QUIT\n", 5);
}

void print_mail(int sockfd, char *num) {
    write(sockfd, "RETR ", 5);
    write(sockfd, num, strlen(num) - 1);
    write(sockfd, "\n", 1);
}

void print_help(char *msg) {
    printf("%s\n", msg);
    printf("./pop3c <username> <password> <server>\n");
    exit(0);
}

int main(int argc, char **argv) {
    if (argc != 4) 
        print_help("Wrong number of arguments");
	struct sockaddr_in serv_addr;
	int sockfd = 0;
	char buf[1024];
    char cmd[1024];
	char ip[32];
    int n, k;
	if (get_ip_by_hostname(ip, argv[3]) != 0) {
		perror("Error by getting ip");
        print_help("Bad server");
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
        return -1;
	}

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("Connect error");
		return -1;
	}
    //wrte(sockfd, "\n", 1);
    int state = 0;
    char *token;
    while ((n = read(sockfd, buf, sizeof(buf))) > 0) {
        if (strstr(buf, "+OK") != NULL) {
                state++;
            
        } else{
            if (state == 1) {
                print_help("Bad username or password");
            }
            if (state == 2) {
                print_help("Bad username or password");
            }
        }
        if (state == 1) {
            write(sockfd, "USER ", 5);
            write(sockfd, argv[1], strlen(argv[1]));
            write(sockfd, "\n", 1);
            //Write password
            continue;
        }
        if (state == 2){
            write(sockfd, "PASS ",5);
            write(sockfd, argv[2], strlen(argv[2]));
            write(sockfd, "\n", 1);
            continue;
        }
        if (state == 3)
            break;
    }
    
    printf("Success login\n\t<list> - to print list\n\t<msg> <n> - get <n>th message\n\t<exit> - exit\n");
    while ((k = read(0, cmd, sizeof(cmd))) > 0) {
        if (strstr(cmd, "list") != NULL) {
            print_list(sockfd);
            handle_socket(sockfd);
        } else if (strstr(cmd, "msg") != NULL) {
            token = strtok(cmd, " \t");
            char* msg = strtok(NULL," \t");
            print_mail(sockfd, msg);
            handle_socket(sockfd);
        } else if (strstr(cmd, "exit") != NULL) {
            disconnect(sockfd);
            break;
        }

    }
    close(sockfd);
}

