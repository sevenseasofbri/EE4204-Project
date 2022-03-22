/**
 * udp_ser4.c Source file for Ex 4 server
 **/
#include "headsock.h"
#include "errno.h"
void str_ser(int sockfd, struct sockaddr *addr, int addrlen);  

int main(void) {
	int sockfd;
	struct sockaddr_in my_addr;
	pid_t pid;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);          //create socket
	if (sockfd <0)
	{
		printf("error in socket!");
		exit(1);
	}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {           //bind socket
		printf("error in binding");
		exit(1);
	}
       	printf("start receiving\n");
	while(1) {
		printf("waiting for data\n");
		str_ser(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));                        // send and receive
	}
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd, struct sockaddr *addr, int addrlen) {
    char buf[BUFSIZE];
	FILE *fp;
	char recvs[DATALEN];
	struct ack_so ack;
	int end, n = 0;
	long lseek=0;
	end = 0;
    int du_recv = 0;

    while(!end) {
       // n = recvfrom(sockfd, &recvs, DATALEN, 0, addr, (socklen_t *)&addrlen);
       // printf(strerror(n));
	if ((n = recvfrom(sockfd, &recvs, DATALEN, 0, addr, (socklen_t *)&addrlen)) == -1) {
            printf("error receiving");
            exit(1);
        }
	//printf("Data unit received!");
        du_recv++;
        // Check for EOF
        if (recvs[n-1] == '\0')									//if it is the end of the file
		{
			end = 1;
			n --;
		}
        memcpy((buf+lseek), recvs, n);
		lseek += n;
        if (du_recv == WINSIZE || end == 1) {
            // Send ACK
            ack.num = 1;
            ack.len = 0;
            n = sendto(sockfd, &ack, 2, 0, addr, addrlen);
            if (n == -1) {
                printf("error sending ACK");
                exit(1);
            }
	    printf("ACK sent!");
            du_recv = 0;
        }
    }
    if ((fp = fopen ("myUDPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exist\n");
		exit(0);
	}
    fwrite (buf , 1 , lseek , fp);					//write data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}
