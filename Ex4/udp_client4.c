/**
 * udp_client4.c Source file for Ex 4 client
 **/
#include "headsock.h"

// Protoypes
float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len); 
void tv_sub(struct timeval *out, struct timeval *in);

int main(int argc, char **argv) {
    int sockfd;
    float ti, rt, throughput;
    long len;
    struct sockaddr_in ser_addr;
    char ** pptr;
    struct hostent *sh;
    struct in_addr **addrs;
    FILE *fp;

    if (argc != 2) {
		printf("parameters not match");
	}

    sh = gethostbyname(argv[1]);	                                       //get host's information
	if (sh == NULL) {
		printf("error when gethostby name");
		exit(0);
	}

    // Print remote host information
    printf("canonical name: %s\n", sh->h_name);					//print the remote host's information
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}
    addrs = (struct in_addr **)sh->h_addr_list;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
	    printf("error in socket");
	    exit(1);
    }

    ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);

    // Attempt to open file
    if((fp = fopen ("myfile.txt","r+t")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}

    ti = str_cli(fp, sockfd,(struct sockaddr*)&ser_addr, sizeof(struct sockaddr_in),&len);                       //perform the transmission and receiving
	rt = (len/(float)ti);                                 //caculate the average transmission rate
    throughput = (8*rt)/1000;                             //throughput in bps       
	printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\nThroughput: %f (bits/s)\n", ti, (int)len, rt, throughput);

	close(sockfd);
	fclose(fp);
	exit(0);

}

float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len) {
    char *buf;
    long lsize, ci;
    char sends[DATALEN];
    struct ack_so ack;
	int n, slen;
	float time_inv = 0.0;
	struct timeval sendt, recvt;
	ci = 0;

	int du_sent = 0;

	fseek(fp , 0 , SEEK_END);
	lsize = ftell(fp);
	rewind (fp);
	printf("The file length is %d bytes\n", (int)lsize);
	printf("the packet length is %d bytes\n",DATALEN);

    // allocate memory to contain the whole file.
	buf = (char *) malloc (lsize);
	if (buf == NULL) exit (2);

    // copy the file into the buffer.
	fread (buf,1,lsize,fp);

	/*** the whole file is loaded in the buffer. ***/
	buf[lsize] ='\0';									//append the end byte
	gettimeofday(&sendt, NULL);							//get the current time

	while (ci <= lsize) {
		if ((lsize + 1 - ci) <= DATALEN) {
			slen = lsize + 1 - ci;
		} else {
			slen = DATALEN;
		}
		memcpy(sends, (buf+ci), slen);
		// Send the DU
		//printf("Data unit sent. Waiting for ACK");
		n = sendto(sockfd, &sends, slen, 0, addr, addrlen);
		if (n == -1) {
			printf("send error!");
			exit(1);
		}
		du_sent++;
		if (du_sent == WINSIZE) {
			// Wait for receiving ACK
			n = recvfrom(sockfd, &ack, 2, 0, addr, (socklen_t*)&addrlen);
			if (n == -1) {
				printf("error when receiving ACK\n");
				exit(1);
			}
			if (ack.num != 1 || ack.len != 0) {
				printf("error in transmission of ACK\n");
			} else {
				printf("ACK received\n");
				ci += slen; // ACK received, move onto next DU
			}
			du_sent = 0;
		} else {
			ci += slen; // If window is not full yet
		}
	}
	// Full transmission time
	gettimeofday(&recvt, NULL);
	*len= ci;                                                         //get current time
	tv_sub(&recvt, &sendt);                                                                 // get the whole trans time
	time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return(time_inv);
}

/**
 * Calculates time interval between send and receive.
 * Taken from Example 3.
 **/
void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

