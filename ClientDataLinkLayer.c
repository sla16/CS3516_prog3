/*
 * This is the data link layer. This part takes in the 256 byte chunk (or less) from the
 * network layer and puts it into a frame.
 * After putting it into a frame, it sends it down to the physical layer to be sent over
 * to the server.
 */

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>		/* File header */
#include "header.h"

extern uint16_t frame_seq_num;

/*
 * @author Sam La
 *
 * Constructs the frame from the packet
 *
 * @param {char *} photo_buffer The 1-256 byte chunk of the photo (from the Network Layer)
 * @param {int} photo_size The amount of bytes in this chunk
 * @param {int} packet_num The current packet
 */
void CreateFrame(char *photo_buffer, int photo_size, int packet_num)
{
	int bytes_used = 0, resend_frame = 0, length = 0;
	int i = -1;
	int isEOPhoto = -1;
	struct frame Frame;
	memset(&Frame, 0, sizeof(Frame)); /* Zero out structure */

	if (photo_size == 0)
		isEOPhoto = 1;

	/* Loop logic to put into frames and send to physical layer */
	while(photo_size >= 0 || resend_frame == 1) {
		/* Construct the frame for transmission 
			logic to resend the frame if an ack is bad */
		if(resend_frame != 1) {
			i++;
			Frame.frame_type = DATA_FRAME;
			memset(Frame.datafield, 0, sizeof(Frame.datafield));
			if (photo_size > 130) {
				memcpy(Frame.datafield, photo_buffer + bytes_used, 130);
				length = 130;
			} else {
				memcpy(Frame.datafield, photo_buffer + bytes_used, photo_size);
				length = photo_size;
			}
			bytes_used = bytes_used + 130;
			photo_size = photo_size - 130;
			if (photo_size <= 0)
				Frame.eop = EOPacket;
			else
				Frame.eop = '0';
			if (isEOPhoto == 1) {
				printf("end of photo\n");
				Frame.eop = EOPhoto;
				length = 0;
			}
		}
		fprintf(f, "Frame #%d of packet #%d sent\n", frame_seq_num, packet_num);
		/* To physical layer */
		if ((resend_frame = SendFrame(Frame, i, length)) == 1) {	
			/* Frame successfully sent, send next frame */			
			frame_seq_num += 1;
			resend_frame = 0;
		} else if (resend_frame == 2) {
			/* Received network ack, proceed to next packet */
			frame_seq_num += 1;
			return;
		} else {
			resend_frame = 1;
		}
	if (isEOPhoto == 1)
		break;
	}
}
