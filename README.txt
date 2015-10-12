Heric Flores, Sam La
CS 3516
Program 3

How to compile:
	There will be two folders included, one called Client and the other called Server.
	For the client:
		Run the makefile and you should get an executable called 'client'
	For the server:
		Run the makefile and you should get an executable called 'runServer'

How to run:
	Start the server first. Usage is ./runServer <Port Number>

	Example:
		./runServer 5000

	Run the client. Usage is ./client <hostname> <client id> <num photos>. NOTE: Currently, the client will connect to a default port of 5000. If you want to change the port, please look into the ClientPhysicalLayer.c file and look for '5000' to change it.

	Example:
		./client CCCWORK4.WPI.edu 1 2

How it works:
	Client:
	The client is split into 4 files, ClientApplicationNetworkLayer.c, ClientDataLinkLayer.c, ClientPhyiscalLayer and a header.h. This is to simulate the different layers.
	ClientApplicationNetworkLayer
		> Validates the input from the command line. This part keeps the useful information such as the client id and how many photos are being sent.
		> Connects to the server with the hostname and port number by calling ConnectToServer, which is located in the ClientDataLinkLayer.
		> Opens all the files to start the logging process, as well as starting timers to keep track of the sending.
		> Reads in the photo file as a photoij.jpg where i is the client id and j is the current number photo being sent. This is done in a loop until it finishes. At any point if there is an error, the program will exit.
		> If the photo is valid, this part will read the photo in 256 byte chunks and send it 'down to the data link layer', located in the ClientDataLinkLayer.c through CreateFrame(). This repeats until the photo is done being read.
	ClientDataLinkLayer
		> Takes the 256 byte chunk from the above layer and store its information into a frame. The frame wll be in the format of a [2] byte sequence number, a [1] byte frame type, a [1] byte EOP indicator, [130] bytes of data, and   [2] bytes for error detection. It will be [2][1][1][130][2] in the frame.
		> There is logic in here to resend the frame if the send fails over the server. Otherwise, it will send this constructed frame down to the physical layer, located in ClientPhysicalLayer.c through SendFrame().
	ClientPhysicalLayer
		> Takes the frame from the above layer and stores it into a sendable buffer.
		> The error detection bytes are constructed in here by XORing all the bytes.
		> There is error logic in this code where every 6th frame sent will have error detection flipped.
		> Depending on what ack is received:
			frame ack - send next frame by going back to data  link layer.
			packet ack (network ack) - send next packet by going up to data link then back up to network to get next chunk
			error - will time out and resend the frame
		> NOTE: select() wasn't used because could not get it working. Instead of using select() as the timeout tool, we set a timeout on recv using setsockopt() with a timeout of 1 second.

	Server:

Notes and other comments:
	