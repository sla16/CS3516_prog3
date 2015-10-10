#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "data.h"

// Save a frame to a linked list of frames
int saveFrame(frame_ptr *frame_list, frame_ptr new_frame)
{
    if(*frame_list == NULL)
    {
        *frame_list = new_frame;
    }
    else
    {
        frame_ptr current_frame = *frame_list;
        while(current_frame->next_frame != NULL)
        {
            printf("seq %"PRIu16"\n", current_frame->seq);
            current_frame = current_frame->next_frame;
            printf("loop after\n");
            printf("seq %"PRIu16"\n", current_frame->seq);
            fflush(stdout);
        }

        printf("saving frame\n");
        fflush(stdout);
        current_frame->next_frame = new_frame;
        printf("saved frame\n");
    }
    printf("returning\n");
    fflush(stdout);
    return 0;
}



// Extract frame headers and data from received frame
    //SEQ, FT, EOP, DATA, ED
frame_ptr parseFrame(char *input)
{
    char *i = input;
    frame_ptr frame = malloc(sizeof(frame));

    if(numBytesRcvd < 7)
        return NULL;



    // extract sequence number string and convert to int
    memcpy(&frame->seq, i, 2);
    i += 2;

    printf("%u\n", *(uint16_t *)input);
    fflush(stdout);

    memcpy(&frame->ft,  i++, 1); // frame type byte
    memcpy(&frame->eop, i++, 1); // eop byte


    //extract data from frame
    int bytesLeft = numBytesRcvd - (i - input);
    memcpy(&frame->data, i, bytesLeft - 2);

    i += bytesLeft - 2; // find beginning of ed

    // extract ed from frame and convert to int
    memcpy(frame->ed, i, 2);


    frame->next_frame = NULL;
    frame->data_length = bytesLeft - 2;

    return frame;
}


// Accepts linked list of frames and
// recursively frees the frames
void freeFrames(frame_ptr frames)
{
    frame_ptr current_frame = frames;

    if(current_frame->next_frame == NULL)
        free(current_frame);
    else
        freeFrames(current_frame->next_frame);
}
