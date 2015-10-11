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
}

/*
 * Sends the frame received from the data link layer to the server data link layer
 *
 * @param {struct frame} Frame The frame structure for our packet
 * @param {int} frame_num The current frame number being sent
 */
int SendFrame(struct frame Frame, int frame_num, int length)
{
    char buffer[136];               /* Buffer to send frame */
    char recvBuffer[136];           /* Buffer to receive ack */
    char error[2] = {0, 0};         /* Error bytes */
    char tempError[1];              /* Temporary buffer for flipping error bytes */
    static int resendBreak = 0;     /* Used for debugging, if frame resent twice, something must have broke */
    int bufferLen, recvSize;
    uint16_t ack_seq_num;

    /* Put frame into buffer to send over */
    *(uint16_t *) buffer = frame_seq_num;           /* Sequence number */
    buffer[2] = Frame.frame_type;                   /* Frame type */
    buffer[3] = Frame.eop;                          /* End of Photo? */
    memcpy(buffer + 4, Frame.datafield, length);    /* Datafield */
    CalculateError(buffer, error, length + 4);      
    memcpy(buffer + (length + 4), error, 2);        /* Error detection */

    /* Induce an error for every 6th frame */
    // if((total_frames_sent + 1) % 6 == 0) {
    //     tempError[0] = buffer[134];
    //     buffer[134] = buffer[135];
    //     buffer[135] = tempError[0];
    // }
    bufferLen = length + 6;

    // printf("----%d---\n", bufferLen);
    // Use this format to extract the frame content
    // printf("%u", *(uint16_t *)buffer);
    // printf("%c", buffer[2]);
    // printf("%c", buffer[3]);
    // printf("%s", buffer + 4);
    // printf("%c - %c", error[0], error[1]);
    // fflush(stdout);

    if (send(sockfd, buffer, bufferLen, 0) != bufferLen)
        DieWithSystemMessage("send() sent a different number of bytes than expected");

    while(1){
        /* Receive message from server */
        if ((recvSize = recv(sockfd, recvBuffer, RCVBUFSIZE, 0)) < 0) {
            /* Timeout, resend the same frame */
            resendBreak++;
            if (resendBreak > 1)
                exit(0);
            fprintf(f, "Timed out, resending frame\n");
            total_retransmitted_frames++;
            return -1;
            // DieWithSystemMessage("recv() failed");
        } else {
            recvBuffer[recvSize] = '\0';
            ack_seq_num = *(uint16_t *) recvBuffer;
            // There is an ACK error
            if (frame_seq_num != ack_seq_num && recvBuffer[2] != PACKET_ACK) {
                fprintf(f, "Ack error, resending frame\n");
                if (resendBreak > 1)
                    exit(0);
                total_bad_acks++;
                return -1;
            } else {
                if(recvBuffer[2] == FRAME_ACK) {
                    /* Frame ack, send next frame */
                    total_frames_sent++;
                    total_good_acks++;
                    resendBreak = 0;
                    fprintf(f, "Frame Ack #%d received\n", ack_seq_num);
                    return 1;
                } else if (recvBuffer[2] == PACKET_ACK) {
                    /* Packet ack, send next packet */
                    total_frames_sent++;
                    total_good_acks++;
                    resendBreak = 0;
                    fprintf(f, "Packet Ack received\n");
                    return 2;           
                } else
                    /* Unknown error */
                    fprintf(f, "FT Ack error, resending frame\n");
                    total_bad_acks++;
                    return -1;          
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
void CalculateError(char *buffer, char *error, int length)
{
    int i;
    for (i = 0; i < length; i+=2)
    {
        error[0] ^= buffer[i];
    }
    for (i = 1; i < length; i+=2)
    {
        error[1] ^= buffer[i];
    }
}
