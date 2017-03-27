/*
** name: ttftp-client.c
**
** author: Federico G. De Faveri
** created: 31 jan 2015 by bjr
**
** from template created 31 jan 2015 by bjr
**
*/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<assert.h>
#include<unistd.h>


#include "ttftp.h"

#define MAXFILENAMELEN 256

#define TFTP_RRQ 1


int ttftp_client( char * to_host, int to_port, char * file ) {
	int block_count ; 
	
	//sanity check
	if (strlen(file) > MAXFILENAMELEN || strchr(file, '/') != NULL) {
		printf("Invalid filename: too long or includes a path\nThis version of TTFTP does not accept file paths");
		exit(2);
	}
	
	//declare some variables
	struct sockaddr_in their_addr;
	struct sockaddr_in my_addr_c;
	int sockfd;
	int numbytes;
	short rrOpcode = TFTP_RRQ;
	struct hostent* he;
	int sockbind;
	int getsname;
	/*
	 * create a socket to send
	 */
	 
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket");
		exit(2);
	}
	
	he = gethostbyname(to_host);
	if (he == NULL) {
		perror("gethostbyname");
		exit(2);
	}


	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(to_port);
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);

	//bind the client socket to a  port so it can listen
	my_addr_c.sin_family = AF_INET;
	my_addr_c.sin_port = 0;
	my_addr_c.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr_c.sin_zero),'\0', 8);
	
	sockbind = bind(sockfd, (struct sockaddr *)&my_addr_c,sizeof(struct sockaddr));
	if (sockbind == -1) {
		perror("bind");
		exit(2);
	}

	//figure out the client port
	socklen_t length = sizeof(my_addr_c);
	getsname = getsockname(sockfd, (struct sockaddr *)&my_addr_c , &length);
	if (getsname == -1) {
		perror("getsockname");
		exit(2);
	}

	/*
	 * send RRQ
	 */
        
	struct TftpReq* readReq;
	//format the datagram
	size_t packetsize = sizeof(struct TftpReq ) + strlen(file) + strlen("octet") + 2*sizeof(char);
	readReq = malloc(packetsize);
	readReq->opcode[0] = (rrOpcode >> 8) & 0xff;
	readReq->opcode[1] = rrOpcode & 0xff;
	strcpy(readReq->filename_and_mode, file);
	strcpy(readReq->filename_and_mode + strlen(file)+1, "octet");

	//send the datagram
	numbytes = sendto(sockfd, (void*)readReq, packetsize, 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr));	
	if (numbytes == -1) {
		perror("sendto");
		exit(2);
	}
	
	//free memory we malloc-ed before
	free(readReq);
	printf("RRQ sent by client\n");

	block_count = 1 ; /* value expected */
	while (block_count ) {
	
		/*
		 * read at DAT packet
		 */
		 
		/*
		 * write bytes to stdout
		 */
		 
		/*
		 * send an ACK
		 */


		block_count ++ ;
		
		/* check if more blocks expected, else 
		 * set block_count = 0 ;
		 */
	}
	return 0 ;
}
