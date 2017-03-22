# TTFTP (Truly Trivial File Transfer Protocol) Project

Professor: Burt Rosenberg

## Project Overview

The Truly Trivial File Transfer Protocol is a simplification of the Internet standard Trivial File Transfer Protocol (TFTP), defined in 1981 by RFC 783, and updated in RFC 1350.

### Specific Objectives

* To introduce protocols in levels 4 and 5 of the networking stack.
* To further refine understanding of UDP and the use of ports to create sessions.
* To introduce the use of positive acknowledgements to obtain communication reliability.
* To understand how data is represented and manipulated.
* To be aware of security concerns in network applications.

#### Man Page:


`
NAME
    ttftp
    
SYNOPSIS
    ttftp [-vL] [-h host -f filename] port
    
    
DESCRIPTION
    Implements a client and a server for the tftp protocol. If called without -h and -f
    options, the programs implements the server listening on port port. If called with
    both -h and -f options, the program uses the tftp protocol to read file filename
    from host host connecting to the server listening on port port.
    
    The client writes the received bytes to standard out.

    The server implements basic security by requiring that filename is in the current
    directory and is world readable. Else an error is returned.
    
    On reading of files in mode OCTET is supported. All over requests generate 
    error responses.
    
OPTIONS
    -f File to read.
    -h Host to contact for the file.
    -L If invoked as server, do not loop. Service one read request then exit.
    -v Verbose. Helpful debugging output to stdout. 
 
NOTES
    Implement only octet and only read. If requested return an error packet with 
    message "mode not supported". 
    
    Data provider (in this case the server) to resend DATA on acknowledgement timeout.
    
    Acknowledgement timeout, and maximum acknowledgement retries to be a define in the
    code, and can be changed by recompiling with the -D compiler option.
    
    Default acknowledgement timeout is 4 seconds, and 4 retries.
    
    Maximum filename length is 256 characters, and cannot contain a pathname. 
    
    Server can be single-threaded.
    
HISTORY
    First introduced in Fall 2003. Made Truly Trivial in Spring 2015.

LAST UPDATED 
    February 14, 2016
`	

### Detailed description

Please read the man page for ttftp. and the TFTP RFC 1350.

Create a single program, ttftp, that functions as client and server, according to command line options. Truly Trivial is a client server program supporting only reads. This means a file on the server will be sent to the client. The client initiates with a RRQ (read request) to a listen port number on the server machine, from a freshly chosen ephemeral port pc.

The server responds either with an error packet or with data block number 1, of up to 512 bytes. This data packet is sent not from the listen port but from a freshly chosen ephemeral port ps. All subsequence communication between client and server will be between ports pc and ps. This creates a session.

In response to the data packet, the client sends an acknowledgement packet with matching block number. This protocol continues alternating data packets from the server with acknowledgement packets to the server until the read file has no further bytes. The end of file is signaled by the server sending a data packet of less than 512 bytes. Note that this includes the possibility of the last data packet containing 0 bytes of data.

After the acknowledgement to the last data packet, the server closes the socket bound to ps. After acknowledging the last data packet, and the client lingers for linger-time in case the server retransmits the data packet, then closes the socket bound to pc. The protocol is then done.

### Implementation notes and restrictions

Note the restrictions mentioned in the manpage: only RRQ for OCTET data is supported; and the read file must be in the current server directory and world-readable.

Do not implement timeouts and retransmission. Do not worry about serving multiple clients simultaneously. These features may be assigned as a future project.

The network is big endian. Intel is little endian. Program accordingly.

Do not assume the file transfered is text; do not assume nulls terminate byte buffers; do not assume just because it is written to standard out the characters are printable.

Do not crash because of bad inputs. Never trust inputs from the network. It is not excusable that an error surprises your code, and your code crashes. If you want to bail out on bad data, in this class you are free to use an assertion, although user friendly code might handle the error more gracefully.

Be consistent with the RFC in handling of all errrors. Check the TID of the sender and send a non-terminating error, are directed in the RFC. All other errors send a terminating error with correct error number. Refer to the statment "receiving a packet which cannot be explained by a delay or duplication in the network" for the definition of what is an error.

Read about the Sorcerer's Apprentice Syndrome. You will not do timeouts or retransmissions in this version of the project, but be familiar with the issue as you code.

### Protocol trace

`Matawan:proj4 burt$ sudo tcpdump -i lo0 -X port 4434 or portrange 50000-65535`

```
	22:43:34.415269 IP localhost.64881 > localhost.4434: UDP, length 19
		0x0000:  4500 002f 9eb1 0000 4011 0000 7f00 0001  E../....@.......
		0x0010:  7f00 0001 fd71 1152 001b fe2e 0001 6d79  .....q.R......my
		0x0020:  6669 6c65 2e74 7874 006f 6374 6574 00    file.txt.octet.
	22:43:34.415368 IP localhost.60537 > localhost.64881: UDP, length 278
		0x0000:  4500 0132 c984 0000 4011 0000 7f00 0001  E..2....@.......
		0x0010:  7f00 0001 ec79 fd71 011e ff31 0003 0001  .....y.q...1....
		0x0020:  616e 796f 6e65 206c 6976 6564 2069 6e20  anyone.lived.in.
		0x0030:  6120 7072 6574 7479 2068 6f77 2074 6f77  a.pretty.how.tow
		0x0040:  6e0a 2877 6974 6820 7570 2073 6f20 666c  n.(with.up.so.fl
		0x0050:  6f61 7469 6e67 206d 616e 7920 6265 6c6c  oating.many.bell
		0x0060:  7320 646f 776e 290a 7370 7269 6e67 2073  s.down).spring.s
		0x0070:  756d 6d65 7220 6175 7475 6d6e 2077 696e  ummer.autumn.win
		0x0080:  7465 720a 6865 2073 616e 6720 6869 7320  ter.he.sang.his.
		0x0090:  6469 646e e280 9974 2068 6520 6461 6e63  didn...t.he.danc
		0x00a0:  6564 2068 6973 2064 6964 2e0a 0a57 6f6d  ed.his.did...Wom
		0x00b0:  656e 2061 6e64 206d 656e 2862 6f74 6820  en.and.men(both.
		0x00c0:  6c69 7474 6c65 2061 6e64 2073 6d61 6c6c  little.and.small
		0x00d0:  290a 6361 7265 6420 666f 7220 616e 796f  ).cared.for.anyo
		0x00e0:  6e65 206e 6f74 2061 7420 616c 6c0a 7468  ne.not.at.all.th
		0x00f0:  6579 2073 6f77 6564 2074 6865 6972 2069  ey.sowed.their.i
		0x0100:  736e e280 9974 2074 6865 7920 7265 6170  sn...t.they.reap
		0x0110:  6564 2074 6865 6972 2073 616d 650a 7375  ed.their.same.su
		0x0120:  6e20 6d6f 6f6e 2073 7461 7273 2072 6169  n.moon.stars.rai
		0x0130:  6e0a                                     n.
	22:43:34.415429 IP localhost.64881 > localhost.60537: UDP, length 4
		0x0000:  4500 0020 9609 0000 4011 0000 7f00 0001  E.......@.......
		0x0010:  7f00 0001 fd71 ec79 000c fe1f 0004 0001  .....q.y........ 
```

### License

This work is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.

### Credits

Author: Burton Rosenberg 
Created: January 18, 2014 as mytftp 
Last Update: February 20, 2017
