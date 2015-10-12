/* @author Sam La 
 * A header file containing definitions and struct information as well as function prototypes.
 */


#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <stdio.h>
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */

#define RCVBUFSIZE 200  /* Size of receive buffer */
#define EOPacket '1'
#define EOPhoto '2'
#define DATA_FRAME '0'
#define FRAME_ACK '1'
#define PACKET_ACK '2'

/*
 * This struct is to hold all the information
 * from a getaddrinfo call when using the hostname
 */
struct addrinfo {
    int              ai_flags;
    int              ai_family;
    int              ai_socktype;
    int              ai_protocol;
    socklen_t        ai_addrlen;
    struct sockaddr *ai_addr;
    char            *ai_canonname;
    struct addrinfo *ai_next;
};

struct frame {
	uint16_t    seq_num;
	char        frame_type;
    char        eop;
    char        datafield[130];
    char        ed[2];          

};

FILE *f;

void DieWithSystemMessage(char *errorMessage);  /* Error handling function */
void ReadPhotoFile(int, int);                   /* Reads in photos and splits into 256 byte chunk */
void ConnectToServer(char *);                   /* Connects to the server */
void CreateFrame(char*, int, int);              /* Constructs a frame from the 256 byte chunk */
int SendFrame(struct frame, int, int);          /* Handles communication with the server */
void CalculateError(char *, char *, int);       /* Handles calculating for error detection through XOR */
