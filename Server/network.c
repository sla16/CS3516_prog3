#include <stdio.h>
#include <string.h>
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
void savePacket(packet_ptr *packet_list, packet_ptr new_packet)
{
    if(*packet_list == NULL)
    {
        *packet_list = new_packet;
    }
    else
    {
        packet_ptr current_packet = *packet_list;

        while(current_packet->next_packet != NULL)
            current_packet = current_packet->next_packet;

        current_packet->next_packet = new_packet;
    }
}





// accepts a packet returns a (frame_ptr)packet ACK frame
uint16_t networkPacket(packet_ptr new_packet, char eop, int client_id)
{
    uint16_t return_seq;
    static int photo_num = 0;
    static uint16_t packet_seq = 0;
    static packet_ptr packet_list = NULL;

    // save packet data to static packet list
    printf("saving packet\n");
    savePacket(&packet_list, new_packet);
    printf("packet saved\n");
    // if(eop) write photo out to FILE
    printf("EOP %c\n", eop);


    if(eop == EO_PHOTO)
    {
        char filename[16]; // filename buffer for log file
        sprintf(filename, "photo%03d%03d.jpg", client_id, photo_num);
        printf("filename: %s\n", filename);
        FILE *fp;
        if((fp = fopen(filename, "w")) == NULL) // open file for writing
            DieWithUserMessage("fopen()", "filed to open file for writing");

        // write out packet data to file
        packet_ptr packets = packet_list;
        while(packets)
        {
            // char buffer[257];
            // memcpy(buffer, packets->data, 260);
            // buffer[267] = '\0';
            //
            // printf("%s\n", buffer);

            fwrite(packets->data, 1, packets->data_length, fp);
            packets = packets->next_packet;
        }
        // freePackets(packet_list); // free linked list of packets
        fflush(fp);
        fclose(fp);
        photo_num++;
    }

    // return frame
    return_seq = packet_seq;
    packet_seq = packet_seq + 1;
    return return_seq;
}
