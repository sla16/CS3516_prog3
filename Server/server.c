#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Practical.h"


void processClient(int clntSock, int client_id);

int main(int argc, char *argv[]) {

    if (argc != 2) // Test for correct number of arguments
    DieWithUserMessage("Parameter(s)", "<Server Port/Service>");

    char *service = argv[1]; // First arg:  local port

    // Create socket for incoming connections
    int servSock = SetupTCPServerSocket(service);
    if (servSock < 0)
        DieWithUserMessage("SetupTCPServerSocket() failed", service);


    // Fork MAX_CLIENTS - 1 child processes
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        // Fork child process and report any errors
        int clntSock = AcceptTCPConnection(servSock);

        pid_t processID = fork();

        if (processID < 0)
            DieWithSystemMessage("fork() failed");
        else if (processID == 0) // If this is the child process
            processClient(clntSock, i);
    }

    // NOT REACHED
    close(servSock);
}




void processClient(int clntSock, int client_id)
{
    printf("with child process: %d\n", getpid());
    HandleTCPClient(clntSock, client_id);
    close(clntSock);
    printf("Child exiting\n");
}
