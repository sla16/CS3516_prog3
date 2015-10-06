all: client
client: ClientApplicationNetworkLayer.c ClientDataLinkLayer.c ClientPhysicalLayer.c DieWithMessage.c header.h
	gcc -o client ClientApplicationNetworkLayer.c ClientDataLinkLayer.c ClientPhysicalLayer.c DieWithMessage.c

clean:
	rm client
