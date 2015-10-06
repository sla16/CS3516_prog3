#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>      /* File Header */
#include "header.h"

int main(int argc, char *argv[])
{
    char *hostname;                  /* Machine Hostname */
    int i;                           /* Misc values needed for addrinfo and loop */
    int client_id;                   /* Unique Integer for identifying this client */
    int num_photos;                  /* Number of photos client wants to upload */
    char save_path[50];              /* File path to log to */

    if (argc != 4)    /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server Machine> <ID> <Number of Photos>\n", argv[0]);
       exit(1);
    }

    hostname = argv[1];                             /* First arg: machine hostname */
    client_id = atoi(argv[2]);                      /* Second arg: client id */
    num_photos = atoi(argv[3]);                     /* Third arg: number of photos */
    sprintf(save_path, "client_%d.log", client_id); /* Log path */

    ConnectToServer(hostname);                      /* Physical Layer connect */
    f = fopen(save_path, "w");

    for(i = 0; i < num_photos; i++)
    {
        ReadPhotoFile();
        //Close connection when all done
    }

    exit(0);
}

/*
 * Helper function to read in a photo file and split it up into chunks of 256 bytes.
 */
void ReadPhotoFile()
{
    int photo_size, photo_file;
    int i = 0;
    char photo_path[50];
    char photo_buffer[256];
    printf("Please enter a photo to upload: ");
    scanf("%s", &photo_path);

    if ((photo_file = open(photo_path, O_RDONLY)) < 0) {
        DieWithSystemMessage("open() failed");
        exit(1);
    }

    while((photo_size = read(photo_file, photo_buffer, 256)) > 0) {
        fprintf(f, "Sent packet #%d\n", i);
        CreateFrame(photo_buffer, photo_size);
        //Deposit into payload
        //Put EOP indicator
        //Send to DataLink layer
        //Wait for network layer ACK
        i++;
        break;
    }

    if(photo_size < 0) {
        DieWithSystemMessage("read() failed");
        exit(1);
    } else {
        printf("Reached the end of the file");
    }

}
