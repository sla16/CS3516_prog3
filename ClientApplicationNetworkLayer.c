#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>      /* File Header */
#include <sys/time.h>
#include "header.h"

extern int total_frames_sent;           /* Count of total frames sent */
extern int total_retransmitted_frames;  /* Count of total frames retransmitted */
extern int total_good_acks;             /* Count of total good acks */
extern int total_bad_acks;              /* Count of total bad acks */
extern int sockfd;                      /* The socket */

/*
 * This main function begins by validating the correct input from the command line
 * and then proceeds to run the client/server interaction.
 */
int main(int argc, char *argv[])
{
    char *hostname;                  /* Machine Hostname */
    int i;                           /* Misc values needed for addrinfo and loop */
    int client_id;                   /* Unique Integer for identifying this client */
    int num_photos;                  /* Number of photos client wants to upload */
    char save_path[50];              /* File path to log to */
    struct timeval start, end;       /* Used for timing the photo transfer */

    if (argc != 4)    /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server Machine> <ID> <Number of Photos>\n", argv[0]);
       exit(1);
    }

    hostname = argv[1];                             /* First arg: machine hostname */
    client_id = atoi(argv[2]);                      /* Second arg: client id */
    num_photos = atoi(argv[3]);                     /* Third arg: number of photos */
    sprintf(save_path, "client_%d.log", client_id); /* Log path */

    gettimeofday(&start, NULL);                     /* Start timer */

    ConnectToServer(hostname);                      /* Physical Layer connect */
    f = fopen(save_path, "w");                      /* Log file for client */

    /* Read the photo file and send to server */
    for(i = 0; i < num_photos; i++)
    {
        ReadPhotoFile(client_id, i);
    }

    gettimeofday(&end, NULL);                       /* End timer */
    fprintf(f, "Total transfer time: %d ms.\n\n", (end.tv_sec * 1000 + end.tv_usec / 1000) - 
                                                 (start.tv_sec * 1000 + start.tv_usec / 1000));
    fprintf(f, "Total frames sent: %d\nTotal number of frames retransmitted: %d\nTotal number of good ACKs: %d\nTotal number of ACKs with errors: %d\n", total_frames_sent, total_retransmitted_frames, total_good_acks, total_bad_acks);
    fclose(f);
    close(sockfd);
    exit(0);
}

/*
 * Helper function to read in a photo file and split it up into chunks of 256 bytes.
 *
 * @param {int} client_id The ID of the client
 * @param {int} num_photo The current i-th photo from the client
 */
void ReadPhotoFile(int client_id, int num_photo)
{
    int photo_size, photo_file;
    int i = 0;
    char photo_path[50];
    char photo_buffer[256];

    sprintf(photo_path, "photo%d%d.jpg", client_id, num_photo);
    // printf("Please enter a photo to upload: ");
    // scanf("%s", &photo_path);

    if ((photo_file = open(photo_path, O_RDONLY)) < 0) {
        DieWithSystemMessage("open() failed");
        exit(1);
    }

    /* Read the photo in 256 byte chunks, send to datalink layer to put into frames */
    while((photo_size = read(photo_file, photo_buffer, 256)) > 0) {
        fprintf(f, "Packet #%d sent\n", i);
        CreateFrame(photo_buffer, photo_size, i);
        i++;
    }

    if(photo_size < 0) {
        DieWithSystemMessage("read() failed");
        exit(1);
    } else {
        printf("Reached the end of the file\n");
    }
}
