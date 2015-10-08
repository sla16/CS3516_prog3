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
uint16_t frame_seq_num = 0;
int total_frames_sent = 0;
int total_retransmitted_frames = 0;
int total_good_acks = 0;
int total_bad_acks = 0;

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
 * @param {int} frame_num The current frame number being sent
 */
int SendFrame(struct frame Frame, int frame_num)
{
    char buffer[136];
    char tempBuffer[136];
    char tempBuffer2[134];
    char error[2] = {0, 0};
    char errorCheck[2] = {0, 0};
    int i = 0;
    int bufferLen, recvSize;
    uint16_t ack_seq_num;

    *(uint16_t *) buffer = frame_seq_num;       /* Sequence number */
    buffer[2] = Frame.frame_type;               /* Frame type */
    buffer[3] = Frame.eop;                      /* End of Photo? */
    memcpy(buffer + 4, Frame.datafield, 130);   /* Datafield */
    CalculateError(buffer, error);
    memcpy(buffer + 134, error, 2);             /* Error detection */

    bufferLen = sizeof(buffer);
    // Use this format to extract the frame content
    // printf("%u", *(uint16_t *)buffer);
    // printf("%c", buffer[2]);
    // printf("%c", buffer[3]);
    // printf("%s", buffer + 4);
    
    if (send(sockfd, buffer, bufferLen, 0) != bufferLen)
        DieWithSystemMessage("send() sent a different number of bytes than expected");

    while(1){
        //     /* Receive message from server */
        if ((recvSize = recv(sockfd, tempBuffer, RCVBUFSIZE, 0)) < 0) {
            /* Timeout, resend the same frame */
            printf("Timed out\n");
            printf("Resending frame\n");
            fprintf(f, "Timed out, resending frame\n");
            total_retransmitted_frames++;
            return -1;
            // DieWithSystemMessage("recv() failed");
        } else {
            // printf("Received something\n");
            memcpy(tempBuffer2, tempBuffer, 5);
            CalculateError(tempBuffer2, errorCheck);
            // There is an ACK error
            if (!(error[0] == errorCheck[0] && error[1] == errorCheck[1])) {
                fprintf(f, "Ack error\n");
                total_bad_acks++;
                return -1;
            }
            ack_seq_num = *(uint16_t *) tempBuffer;
            if(ack_seq_num == frame_seq_num) {
                if(tempBuffer[2] == '1') {
                    total_frames_sent++;
                    total_good_acks++;
                    fprintf(f, "Ack #%d received\n", ack_seq_num);
                    return 1;           /* frame ack */
                } else if (tempBuffer[2] == '2') {
                    total_frames_sent++;
                    total_good_acks++;
                    fprintf(f, "Network Ack received\n");
                    return 2;           /* network ack */
                } else
                    total_bad_acks++;
                    return -1;          /* unknown error */
            }
        }
    }
}

/*
 * XOR the buffer content to get error detection
 *
 * @param {char *} buffer The buffer to send to the server
 * @param {char *} error The error detection bytes to set
 */
void CalculateError(char *buffer, char *error)
{
    // LOOK INTO 135
    int i;
    for (i = 0; i < sizeof(buffer); i+=2)
    {
        error[0] ^= buffer[i];
    }
    for (i = 1; i < sizeof(buffer); i+=2)
    {
        error[1] ^= buffer[i];
    }
}
