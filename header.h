#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <stdio.h>
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */

#define RCVBUFSIZE 200  /* Size of receive buffer */

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
    short       seq_num;
    char        frame_type;
    char        eop;
    char        datafield[130];
    char        ed[2];          

};

FILE *f;

void DieWithSystemMessage(char *errorMessage);  /* Error handling function */
void ReadPhotoFile();
void ConnectToServer(char *);
void CreateFrame(char[], int);
int SendFrame(struct frame, int);
void CalculateError(char *, char *);
