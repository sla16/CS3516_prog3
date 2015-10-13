/*
 * This is the physical layer. It is basically in charge of the connect, send and recv.
 * The data link layer sends a frame down to this layer and it constructs that frame into
 * a sendable buffer to the server. Then it receives and ack and depending on the ack, it will
 * either send the next frame, resend the frame or proceed to the next packet.
 */

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>		/* File header */
#include <sys/time.h>
#include "header.h"

int sockfd;
uint16_t frame_seq_num = 0;
int total_frames_sent = 0;
int total_retransmitted_frames = 0;
int total_good_acks = 0;
int total_bad_acks = 0;

/*
 * @author Sam La
 *
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
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;

	// Tell the system what kind(s) of address info we want
    struct addrinfo addrCriteria;                   // Criteria for address match
    memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
    addrCriteria.ai_family = AF_UNSPEC;             // Any address family
    addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
    addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

    // Get address(es) associated with the specified name/service
    struct addrinfo *addrList;                      // Holder for list of addresses returned

    // Use the default port 5000 as a check if no port was actually given
    if ((rtnVal = getaddrinfo(hostname, "5000", &addrCriteria, &addrList)) != 0)
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
 * @author Sam La
 *
 * Sends the frame received from the data link layer to the server data link layer
 *
 * @param {struct frame} Frame The frame structure for our packet
 * @param {int} frame_num The current frame number being sent
 */
int SendFrame(struct frame Frame, int frame_num, int length)
{
    char buffer[136];               /* Buffer to send frame */
    char error[2] = {0, 0};         /* Error bytes */
    char tempError[1];              /* Temporary buffer for flipping error bytes */
    int bufferLen;

    /* Put frame into buffer to send over */
    *(uint16_t *) buffer = frame_seq_num;           /* Sequence number */
    buffer[2] = Frame.frame_type;                   /* Frame type */
    buffer[3] = Frame.eop;                          /* End of Photo */
    memcpy(buffer + 4, Frame.datafield, length);    /* Datafield */
    CalculateError(buffer, error, length + 4);
    memcpy(buffer + (length + 4), error, 2);        /* Error detection */

    /* Induce an error for every 6th frame by flipping bits */
    if(((total_frames_sent + 1) % 6) == 0) {
        error[0] = buffer[135];
        error[1] = buffer[134];
        memcpy(buffer + (length + 4), error, 2);
    }

    bufferLen = length + 6;

    if (send(sockfd, buffer, bufferLen, 0) != bufferLen)
        DieWithSystemMessage("send() sent a different number of bytes than expected");

    return ReceiveAck();
}

/*
 * @author Sam La
 *
 * This handles receiving and handling acks from the server
 */
int ReceiveAck()
{
    char recvBuffer[136];           /* Buffer to receive ack */
    uint16_t ack_seq_num;
    uint16_t error_num;
    int recvSize;
    static int x = 0;

    while(1){
        total_frames_sent++;
        /* Receive message from server */
        if ((recvSize = recv(sockfd, recvBuffer, RCVBUFSIZE, 0)) < 0) {
            fprintf(f, "Timed out, resending frame\n");
            total_retransmitted_frames++;
            return -1;
            // DieWithSystemMessage("recv() failed");
        } else {
            recvBuffer[recvSize] = '\0';
            ack_seq_num = *(uint16_t *) recvBuffer;
            error_num = *(uint16_t *) (recvBuffer + 3);
            // There is an ACK error
            if ((frame_seq_num != ack_seq_num && recvBuffer[2] != PACKET_ACK) || ack_seq_num != error_num) {
                x++;
                fprintf(f, "Ack error, resending frame\n");
                total_retransmitted_frames++;
                total_bad_acks++;
                if(x > 10)
                    exit(1);
                return -1;
            } else {
                x = 0;
                if(recvBuffer[2] == FRAME_ACK) {
                    /* Frame ack, send next frame */
                    total_good_acks++;
                    fprintf(f, "Frame Ack #%d received\n", ack_seq_num);
                    return 1;
                } else if (recvBuffer[2] == PACKET_ACK) {
                    /* Packet ack, send next packet */
                    total_good_acks++;
                    fprintf(f, "Packet Ack received\n\n");
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
 * @author Sam La
 *
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
