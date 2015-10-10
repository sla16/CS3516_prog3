#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "data.h"
#include "Practical.h"


/*
 * XOR the buffer content to get error detection
 *
 * @param {char *} buffer The buffer to send to the server
 * @param {char *} error The error detection bytes to set
 */
void CalculateError(char *buffer, char *error)
{
    // LOOK INTO 135
    for (int i = 0; i < numBytesRcvd-2; i+=2)
        error[0] ^= buffer[i];

    for (int i = 1; i < numBytesRcvd-2; i+=2)
        error[1] ^= buffer[i];
}



//deals with interaction to network layer and packets
int toNetwork(frame_ptr frame_list, char eop, int clntSock, int client_id)
{
    int  ibuf = 0;
    frame_ptr current_frame = frame_list;
    packet_ptr packet = malloc(sizeof(packet));

    // take linked list of frames & put their data into a packet
    if(current_frame == NULL)
        printf("\n\n\n frame list is empty \n\n\n");
    while(current_frame)
    {
        memcpy(packet->data + ibuf, current_frame->data, current_frame->data_length);

        ibuf += current_frame->data_length;
        printf("iBuf = %d\n", ibuf);
        if(ibuf > 256)
            DieWithUserMessage("frame error", "packet has more than 256 bytes");

        current_frame = current_frame->next_frame;
    }


    uint16_t seq = networkPacket(packet, eop, client_id); // send packet to network
    sendFrame(seq, ACK_PACKET ,clntSock);// send packet ack to physical layer

    fprintf(log_file, "sent packet (%" PRIu16 ") to network layer\n", seq);


    return 0;
}



// SEQ, FT, EOP, DATA, ED
int processInput(char *input, int clntSock, int client_id)
{
    char error[2] = {0,0};
    frame_ptr frame = NULL;             // pointer to received frame
    static uint16_t frameExpect = 0;    // staticly stores frame expected
    static int bytesReceived = 0;       // uint16_t computedED;
    static frame_ptr frame_list = NULL; // pointer to list of frames

    frame = parseFrame(input); // parse frame

    printf("sequence number: %" PRIu16 "\n", frame->seq);
    printf("frame type:      %c\n", frame->ft);
    printf("end of packet:   %c\n", frame->eop);




    //if frame contains transmission errors
    CalculateError(input, error);
    if(frame->ed[0] != error[0] && frame->ed[1] != error[1])
    {
        printf("Detected Transmission Error\n");
        fprintf(log_file, "transmission error detected\n");
        return -1; //break
    }





    //if SEQ duplicate
    if(frame->seq == ((frameExpect - 1) % MAX_SEQ))
    {
        printf("frame expect - 1: %"PRIu16"\n", frameExpect);
        sendFrame(frameExpect-1, ACK_FRAME, clntSock); //send ACK
        fprintf(log_file, "duplicate frame receivde #(%" PRIu16 ")\n", frame->seq);
    }




    else if(frame->seq == frameExpect)//else if SEQ correct
    {
        fprintf(log_file, "frame #(%" PRIu16 ") received\n", frame->seq); // log it
        printf("saving frame\n");
        saveFrame(&frame_list, frame);                // save frame data
        printf("saved frame\n");
        bytesReceived += frame->data_length;         // track bytes received
        printf("bytes of data Received: %d\n\n\n", bytesReceived);

        if(bytesReceived >= 256 || frame->eop > '0')
        {   // received entire packet
            // send packet to network layer
            fflush(stdout);
            toNetwork(frame_list, frame->eop, clntSock, client_id);
            freeFrames(frame_list); // free used up frames

            frame = NULL;
            frame_list = NULL;
            bytesReceived = 0;
        }
        else
        {
            sendFrame(frameExpect, ACK_FRAME, clntSock); // send ACK
        }


        frameExpect = (frameExpect + 1);     // increment frame expected
        printf("frame expected %" PRIu16 "\n", frameExpect);
    }




    else
    {
        printf("expected: %"PRIu16"\n received: %"PRIu16"\n", frameExpect, frame->seq);
        fprintf(log_file, "frame received in error #(%" PRIu16 ")\n", frame->seq);
    }

    fflush(log_file);
    fflush(stdout);
    return 0;
}
