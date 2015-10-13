#ifndef DATA_H_
#define DATA_H_

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define EO_PHOTO '2'
#define EO_PACKET '1'

#define DATA_FRAME 0
#define ACK_FRAME  '1'
#define ACK_PACKET '2'

#define SEQ_SIZE 16
#define MAX_SEQ  65536 - 1

struct Frame {
    uint16_t seq;      //2-Bytes:   sequence number
    char     ft;       //1-Bytes:   frame type
    char     eop;      //1-Bytes:   end of packet indicator
    int      data_length; // Amount of bytes data frame actually contains
    char     data[130];//130-Bytes: data
    char     ed[2];       //2-Bytes:   error detection code
    struct Frame *next_frame; // pointer to next frame
};

struct Packet {
    char data[256];
    int  data_length;
    struct Packet *next_packet;
};

typedef struct Packet packet;
typedef packet *packet_ptr;

typedef struct Frame frame;
typedef frame *frame_ptr;


FILE *log_file;
ssize_t numBytesRcvd;


// Save a frame to a linked list of frames
int saveFrame(frame_ptr *frame_list, frame_ptr new_frame);
// Extract frame headers and data from received frame
frame_ptr parseFrame(char *input);
// recursively frees frames
void freeFrames(frame_ptr frames);
// network layer: accepts packet and EOP indicator
uint16_t networkPacket(packet_ptr new_packet, char eop, int client_id);


#endif
