#ifndef PRACTICAL_H_
#define PRACTICAL_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>

// Handle error with user msg
void DieWithUserMessage(const char *msg, const char *detail);
// Handle error with sys msg
void DieWithSystemMessage(const char *msg);
// Print socket address
void PrintSocketAddress(const struct sockaddr *address, FILE *stream);
// Test socket address equality
bool SockAddrsEqual(const struct sockaddr *addr1, const struct sockaddr *addr2);
// Create, bind, and listen a new TCP server socket
int SetupTCPServerSocket(const char *service);
// Accept a new TCP connection on a server socket
int AcceptTCPConnection(int servSock);
// Handle new TCP client
void HandleTCPClient(int clntSocket, int client_id);
// Create and connect a new TCP client socket
int SetupTCPClientSocket(const char *server, const char *service);


// Process Frame received
int processInput(char *input, int clntSock, int client_id);
// Send a frame to client
int sendFrame(uint16_t seq, char ft, int clntSock);

enum sizeConstants {
  MAXSTRINGLENGTH = 128,
  BUFSIZE = 512,
};


#define MAX_CLIENTS 200
#define MAX_PHOTOS 200

#endif // PRACTICAL_H_
