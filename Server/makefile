CC = gcc
CFLAGS = -g -Wall -std=gnu99

default: runServer

runServer: server.o TCPServerUtility.o DieWithMessage.o AddressUtility.o datalink.o frame.o network.o
	$(CC) $(CFLAGS) -o runServer server.o TCPServerUtility.o DieWithMessage.o AddressUtility.o datalink.o frame.o network.o
server.o: server.c Practical.h
	$(CC) $(CFLAGS) -c server.c



TCPServerUtility.o: TCPServerUtility.c Practical.h
	$(CC) $(CFLAGS) -c TCPServerUtility.c
AddressUtility.o: AddressUtility.c
	$(CC) $(CFLGAS) -c AddressUtility.c


network.o: network.c data.h
	$(CC) $(CFLAGS) -c network.c


datalink.o: datalink.c Practical.h data.h
	$(CC) $(CFLAGS) -c datalink.c
frame.o: frame.c data.h
	$(CC) $(CFLAGS) -c frame.c



DieWithMessage.o: DieWithMessage.c
	$(CC) $(CFLAGS) -c DieWithMessage.c

clean:
	$(RM) runClient runServer *.o *~
