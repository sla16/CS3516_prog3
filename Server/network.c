#include <stdio.h>
#include <stdlib.h>
#include "data.h"
#include "Practical.h"


void freePackets(packet_ptr packet)
{
    if(packet->next_packet == NULL)
        free(packet);
    else
        freePackets(packet->next_packet);
}





// Save a frame to a linked list of frames
void savePacket(packet_ptr packet_list, packet_ptr new_packet)
{
    if(packet_list == NULL)
    {
        packet_list = new_packet;
    }
    else
    {
        packet_ptr current_packet = packet_list;

        while(current_packet->next_packet != NULL)
            current_packet = current_packet->next_packet;

        current_packet->next_packet = new_packet;
    }
}





// accepts a packet returns a (frame_ptr)packet ACK frame
uint16_t networkPacket(packet_ptr new_packet, int eop, int client_id)
{
    uint16_t return_seq;
    static uint16_t packet_seq = 0;
    static packet_ptr packet_list = NULL;

    // save packet data to static packet list
    savePacket(packet_list, new_packet);

    // if(eop) write photo out to FILE
    if(eop == EO_PHOTO)
    {
        char filename[16]; // filename buffer for log file
        sprintf(filename, "photo%03d%03d.jpg", client_id, (unsigned int)packet_seq);

        FILE *fp;
        if((fp = fopen(filename, "w")) == NULL) // open file for writing
            DieWithUserMessage("fopen()", "filed to open file for writing");

        // write out packet data to file
        packet_ptr packets = packet_list;
        while(packets)
        {
            fwrite(packets->data, 1, 256, fp);
            packets = packets->next_packet;
        }
        freePackets(packet_list); // free linked list of packets
    }

    // return frame
    return_seq = packet_seq;
    packet_seq = (packet_seq+1) % MAX_SEQ;
    return return_seq;
}
