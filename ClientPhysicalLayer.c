// 0 for data frame
// 1 for frame ack
// 2 for packet ack

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>      /* File header */
#include <sys/time.h>
#include "header.h"

int sockfd;

/*
 * Connects the client to the server.
 *
 * @param {char*} hostname The machine name to connect to
 */
void ConnectToServer(char *hostname)
{
    int sock, flags;                        /* Socket descriptor */
    struct addrinfo *p;              /* Struct of the addrinfo for hostname */
    struct timeval timeout;
    int rtnVal;

    /* Sets 1 second timeout */
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    // Tell the system what kind(s) of address info we want
    struct addrinfo addrCriteria;                   // Criteria for address match
    memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
    addrCriteria.ai_family = AF_UNSPEC;             // Any address family
    addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
    addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

    // Get address(es) associated with the specified name/service
    struct addrinfo *addrList;                      // Holder for list of addresses returned

    // Use the default port 5000 as a check if no port was actually given
    if ((rtnVal = getaddrinfo(hostname, "5001", &addrCriteria, &addrList)) != 0)
    {
        DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));
    }

    /* Establish the connection to the echo server */
    // loop through all the results and connect to the first we can
    // This part came from GetAddrInfo.c from the DC textbook
    for(p = addrList; p != NULL; p = p->ai_next) {
        /* Create a reliable, stream socket using TCP */
        if ((sock = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
            DieWithSystemMessage("socket() failed");
            continue;
        }

        if (connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            DieWithSystemMessage("connect() failed");
            continue;
        }

        break; // if we get here, we must have connected successfully
    }

    if (p == NULL) {
        // looped off the end of the list with no connection
        fprintf(stderr, "failed to connect\n");
        exit(1);
    }

    sockfd = sock;

    /* Timeout set on recv */
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // freeaddrinfo(addrList); // all done with this structure
    // printf("\n");
    // close(sock);
}

/*
 * Sends the frame received from the data link layer to the server data link layer
 *
 * @param {struct frame} Frame The frame structure for our packet
 */
int SendFrame(struct frame Frame, int frame_num)
{
    unsigned char buffer[136];
    unsigned char buffer2[136];
    char seq_num[2];
    char error[2] = {0, 0};
    char temp[2];
    int i = 0;
    int bufferLen, recvSize;
    printf("--%X--", Frame.seq_num);
    memcpy(seq_num, &Frame.seq_num, sizeof(short));
    memcpy(buffer2, seq_num, sizeof(seq_num));
    sprintf(buffer2+2, "%c%c%s", Frame.frame_type, Frame.eop, Frame.datafield);
    CalculateError(buffer2, error);
    sprintf(buffer+134, "%c%c", error[0], error[1]);

    bufferLen = strlen(buffer2);

    printf("%s", buffer2);
    fflush(stdout);
    
    if (send(sockfd, buffer2, bufferLen, 0) != bufferLen)
        DieWithSystemMessage("send() sent a different number of bytes than expected");

    while(1){
        //     /* Receive message from server */
        if ((recvSize = recv(sockfd, buffer, RCVBUFSIZE, 0)) < 0) {
            /* Timeout, resend the same frame */
            printf("Timed out\n");
            printf("Resending frame\n");
            fprintf(f, "Timed out, frame resent\n");
            return -1;
            // DieWithSystemMessage("recv() failed");
        } else {
            printf("Received something\n");
            //CHECK FOR ACK ERROR//
            return 1;           /* frame ack */
            // return 2;        /* network ack */
        }
    }
}

/*
 * XOR the buffer content to get error detection
 */
void CalculateError(char *buffer, char *error)
{
    // LOOK INTO 135
    int i;
    for (i = 0; i < 135; i+=2)
    {
        error[0] ^= buffer[i];
    }
    for (i = 1; i < 135; i+=2)
    {
        error[1] ^= buffer[i];
    }
}
