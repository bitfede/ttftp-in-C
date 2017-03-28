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
#define MAXLENERR 128

#define TFTP_ERROR 5
#define TFTP_DATA 3

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
		char* filename;
		char* reqMode;
		FILE* fp;
		int filelen = 0;
		int readIndex = 0;
		int errorNum = -1;
		/*
		 * for each RRQ 
		 */

		//allocate memory to contain our RRQ packet
		size_t rrq_size = sizeof(struct TftpReq) + FILENAMEMAXLEN + strlen(MODE)+ 2*sizeof(char) ; 
		recvReq = malloc(rrq_size);
		
		//call method recvfrom, which expects a RRQ packet
		addr_len = sizeof(struct sockaddr);
		puts("Server is listening...");
		numbytes = recvfrom(sockfd_l, recvReq, rrq_size,0,
			(struct sockaddr *)&their_addr, &addr_len);		
		//check for recvfrom-related errors
		if (numbytes == -1) {
			perror("recvfrom");
			exit(1);
		}

		/*
		 * parse request and open file
		 */
		puts("RRQ Received!");
		//vars for file info
		filename = recvReq->filename_and_mode;
		reqMode = recvReq->filename_and_mode + strlen(filename) + 1;
	        //open file and get length
	        fp = fopen(filename, "rb");
		if (fp != NULL) {
			fseek(fp, 0, SEEK_END);
			filelen = ftell(fp);
			rewind(fp);
		}
		else {
			errorNum = 1;
		}

		//check for illegal tftp operation - [we only support octet]
		
		if ( strcmp(reqMode, "octet") ) {
			errorNum = 4;
		} 
		
		/*
		 * create a sock for the data packets
		 */ 
		sockfd_s = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd_s == -1 ) {
			perror("socket");
			exit(1);
		}

		my_addr.sin_family = AF_INET;
		my_addr.sin_port = htons(0);
		my_addr.sin_addr.s_addr = INADDR_ANY;
		memset(my_addr.sin_zero, '\0', 8);
		
		int sockfd_sbind = bind(sockfd_s, (struct sockaddr *)&my_addr, sizeof(struct sockaddr) );
		if ( sockfd_sbind == -1 ) {
			perror("bind");
			exit(1);
		}


		//check now for any errors
		if (errorNum > 0) {
			//we need to send an error packet!
			struct TftpError * errPack;
			int numbytes;
			short eOpcode = htons(TFTP_ERROR);
			size_t packetsize = sizeof(struct TftpError) + MAXLENERR;
			errPack = malloc(packetsize);

			//set opcode
			errPack->opcode[0] = (eOpcode >> 8) & 0xff;
			errPack->opcode[1] = eOpcode & 0xff;
		
			//set error code
			short netErrorCode = htons(errorNum);
			errPack->error_code[0] = (netErrorCode >> 8) & 0xff;
			errPack->error_code[1] = netErrorCode & 0xff;

			//error message
			if( errorNum == 1) {
				char* msg = "The Server could not find the File.";
				strcpy(errPack->error_msg, msg);
			}
			else if (errorNum == 4) {
				char* msg = "Illegal TFTP operation.";
				strcpy(errPack->error_msg, msg);
			}
			//send it now
			numbytes = sendto(sockfd_s, (void *)errPack, packetsize, 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr));
			if (numbytes == -1) {
				perror("sendto");
				exit(1);
			}
			puts("Error Packet Sent");
			//exit since error occurred
			free(errPack);
			exit(0);
		}
			//we need to send data!
			//open file and get lenght
			fp = fopen(filename, "rb");
			if (fp == NULL) {
				printf("Error opening file!!!\n");
				exit(1);
			}	

			fseek(fp, 0, SEEK_END);
			filelen = ftell(fp);
			rewind(fp);
				
			int is_mac = 0;
			int is_padded = 0;

		block_count = 1 ;
		while (block_count) { 
			//declare vars and structs
			struct TftpData* fileData;
			struct TftpAck* ack;
			short Opcode = htons(TFTP_DATA);
			size_t packetsize;
			int numbytes;
			int addr_len = sizeof(struct sockaddr);
			//increase block number
			block_count++;
			//allocate space
			packetsize = sizeof(struct TftpData) + MAXMSGLEN;
			fileData = malloc(packetsize);

			//save the opcode
			fileData->opcode[0] = (Opcode >> 8) & 0xff;
			fileData->opcode[1] = Opcode & 0xff;
			
			//save the block number
			fileData->block_num[0] = (htons(block_count) >> 8) & 0x77;
			fileData->block_num[1] = htons(block_count) & 0x77;

			/*
			 * read from file
		         */
		   	 if (readIndex+MAXMSGLEN <= filelen) {
				fread(fileData->data, MAXMSGLEN, 1, fp);
				readIndex += MAXMSGLEN;
				fseek(fp, readIndex, SEEK_SET);
				block_count++;
			 }
			 else {
				if (readIndex == filelen + MAXMSGLEN) {
					char* empty = "";
					strcpy(fileData->data, empty);
					packetsize = sizeof(struct TftpData); 
					fileData = realloc(fileData, packetsize);
				}
				else {
					printf("- Last packet!\n");
					int len  = filelen - readIndex;
					fread(fileData->data, len, 1, fp);
					packetsize = sizeof(struct TftpData) + len;
					fileData = realloc(fileData, packetsize);	
				}
				block_count = 0;
			}
			/*
			 * send data packet
			 */
			if (DEBUG)
				printf("DEBUG: packetsize=%i\n\tblock_count: %i\n", packetsize, block_count);
			numbytes = sendto(sockfd_s, (void*)fileData, packetsize, 0, (struct sockaddr*)&their_addr, sizeof(struct sockaddr));
			if (numbytes == -1) {
				perror("sendto");
				exit(1);
			}
			if (DEBUG)
				printf("Data Packet #%i Sent!\n", block_count);
			/*
			 * wait for acknowledgement
			 */
			ack = malloc(sizeof(struct TftpAck));
			numbytes = recvfrom(sockfd_s, (struct TftpAck *)ack, sizeof(struct TftpAck), 0, (struct sockaddr *)&their_addr, &addr_len);
			if (numbytes == -1) {
				perror("recvfrom");
				exit(1);
			}
			free(fileData);
			free(ack);
			if (DEBUG)
				puts("ACK Received from client!");	
		}
	} while (!is_noloop) ;
	return 0 ;
}

