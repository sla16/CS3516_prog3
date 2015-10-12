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
            current_frame = current_frame->next_frame;
        }

        current_frame->next_frame = new_frame;
    }

    return 0;
}



// Extract frame headers and data from received frame
    //SEQ, FT, EOP, DATA, ED
frame_ptr parseFrame(char *input)
{
    char *i = input;
    frame_ptr new_frame = malloc(sizeof(frame));
    // memset(new_frame, 0, sizeof(frame));

    if(numBytesRcvd < 7)
        return NULL;



    // extract sequence number string and convert to int
    memcpy(&new_frame->seq, i, 2);
    i += 2;

    printf("%u\n", *(uint16_t *)input);
    fflush(stdout);

    memcpy(&new_frame->ft,  i++, 1); // frame type byte
    memcpy(&new_frame->eop, i++, 1); // eop byte


    //extract data from frame
    int bytesLeft = numBytesRcvd - (i - input);
    memcpy(&new_frame->data, i, bytesLeft - 2);

    i += bytesLeft - 2; // find beginning of ed

    // extract ed from frame and convert to int
    memcpy(new_frame->ed, i, 2);


    new_frame->next_frame = NULL;
    new_frame->data_length = numBytesRcvd - 6;
    printf("numBytesRcvd: %ld\n", numBytesRcvd);

    return new_frame;
}


// Accepts linked list of frames and
// recursively frees the frames
void freeFrames(frame_ptr frames)
{
    frame_ptr next;

    while(frames)
    {
        next = frames;
        frames = frames->next_frame;
        free(next);
    }
    // if(frames->next_frame == NULL)
    // {
    //     free(frames);
    //     printf("freed end frame\n");
    // }
    // else
    // {
    //     printf("calling freeFrames\n");
    //     freeFrames(frames->next_frame);
    //     printf("freeFrames returned\n");
    //     free(frames);
    //     printf("freed begin frame\n");
    // }
}
