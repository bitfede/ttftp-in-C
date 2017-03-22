/*
** name: ttftp-server.c
**
** author: Federico G. De Faveri
** created: 14 feb 2016 by bjr
**
** from extracted from ttftp.c
*/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<assert.h>
#include<unistd.h>

#include "ttftp.h"

#define FILENAMEMAXLEN 256
#define MODE "octet"



int  ttftp_server( int listen_port, int is_noloop ) {

	int sockfd_l;
	int sockfd_s ;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int block_count ;
	
	/*
	 * create a socket to listen for RRQ
	 */
	
	sockfd_l = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd_l == -1) {
		perror("socket");
		exit(1);
        }
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(listen_port);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), '\0', 8) ;
	
	int sockbind = bind(sockfd_l, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));
	if (sockbind == -1) {
		perror("bind");
		exit(1);
	}

	do {
		//declaring variables
		struct TftpReq *recvReq;
		int numbytes;
		int addr_len;
		
		/*
		 * for each RRQ 
		 */

		//allocate memory to contain our RRQ packet
		size_t rrq_size = sizeof(struct TftpReq) + FILENAMEMAXLEN + strlen(MODE)+ 2*sizeof(char) ; 
		recvReq = malloc(rrq_size);
		
		//call method recvfrom, which expects a RRQ packet
		addr_len = sizeof(struct sockaddr);
		numbytes = recvfrom(sockfd_l, recvReq, rrq_size,0,
			(struct sockaddr *)&their_addr, &addr_len);		
		//check for recvfrom-related errors
		if (numbytes == -1) {
			perror("recvfrom");
			exit(1);
		}

		puts("all good until here --- no errors");
		/*
		 * parse request and open file
		 */
		 
		/*
		 * create a sock for the data packets
		 */	 
		
		block_count = 1 ;
		while (block_count) { 

			/*
			 * read from file
			 */
			 
			/*
			 * send data packet
			 */
			 
			/*
			 * wait for acknowledgement
			 */
			block_count++ ;
		}
	
	} while (!is_noloop) ;
	return 0 ;
}

