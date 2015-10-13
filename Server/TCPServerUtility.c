#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "data.h"
#include "Practical.h"

static const int MAXPENDING = 5; // Maximum outstanding connection requests

int SetupTCPServerSocket(const char *service) {
  // Construct the server address structure
  struct addrinfo addrCriteria;                   // Criteria for address match
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
  addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
  addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol



  struct addrinfo *servAddr; // List of server addresses
  int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));



  int servSock = -1;
  for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next) {
    // Create a TCP socket
    servSock = socket(addr->ai_family, addr->ai_socktype,
        addr->ai_protocol);
    if (servSock < 0)
      continue;       // Socket creation failed; try next address



    // Bind to the local address and set socket to listen
    if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) &&
        (listen(servSock, MAXPENDING) == 0))
    {
      // Print local address of socket
      struct sockaddr_storage localAddr;
      socklen_t addrSize = sizeof(localAddr);


      if (getsockname(servSock, (struct sockaddr *) &localAddr, &addrSize) < 0)
        DieWithSystemMessage("getsockname() failed");


      fputs("Binding to ", stdout);
      PrintSocketAddress((struct sockaddr *) &localAddr, stdout);
      fputc('\n', stdout);

      break;       // Bind and listen successful
    }

    close(servSock);  // Close and try again
    servSock = -1;
  }

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);

  return servSock;
}






int AcceptTCPConnection(int servSock) {
  struct sockaddr_storage clntAddr; // Client address
  // Set length of client address structure (in-out parameter)
  socklen_t clntAddrLen = sizeof(clntAddr);

  // Wait for a client to connect
  int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
  if (clntSock < 0)
    DieWithSystemMessage("accept() failed");

  // clntSock is connected to a client!

  fputs("Handling client ", stdout);
  PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
  fputc('\n', stdout);

  return clntSock;
}




void HandleTCPClient(int clntSocket, int client_id)
{
    char buffer[BUFSIZE]; // Buffer for echo string

    char logname[16];
    sprintf(logname, "log_file%03d.log", client_id);
    log_file = fopen(logname, "w"); //open log file

    numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0); // receive first frame

    while (numBytesRcvd > 0)
    {
        // Use this format to extract the frame content
        printf("%u", *(uint16_t *)buffer);
        printf("%c", buffer[2]);
        printf("%c", buffer[3]);
        printf("%s\n", buffer + 4);
        printf("%ld\n", numBytesRcvd);
        fflush(stdout);


        buffer[numBytesRcvd] = '\0';
        processInput(buffer, clntSocket, client_id);
        printf("process input returns\n\n\n\n");

        numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
        if (numBytesRcvd < 0)
            DieWithSystemMessage("recv() failed");
    }

    fclose(log_file);
}



int sendFrame(uint16_t seq, char ft, int clntSock)
{
    char buffer[10];
    ssize_t numBytesSent;
    static int frame_num = 0;

    // format message
    *(uint16_t *) buffer = seq;    /* Sequence number */
    buffer[2] = ft;               /* Frame type */
    *(uint16_t *)(buffer + 3) = seq;
    buffer[5] = '\0';


    if(ft == ACK_FRAME)
    {
        if(((frame_num + 1) % 11) == 0)
        {
            buffer[3] ^= 0x7;
        }
        frame_num++;
    }


    // send message
    numBytesSent = send(clntSock, buffer, 5, 0);

    // check for send errors
    if (numBytesSent < 0)
      DieWithSystemMessage("send() failed");
    else if (numBytesSent != 5)
      DieWithUserMessage("send()", "sent unexpected number of bytes");

    // print event to log file
    if(ft == ACK_PACKET)
    {
        printf("sending packet ack\n");
        fprintf(log_file, "sent packet ACK,(%" PRIu16 ")\n", seq);
    }
    else if(ft == ACK_FRAME)
    {
        printf("sending frame ack\n");
        fprintf(log_file, "sent frame ACK, (%" PRIu16 ")\n", seq);
    }

    fflush(log_file);

    return 0;
}
