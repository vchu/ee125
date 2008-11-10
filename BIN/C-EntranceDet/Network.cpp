#include "..\..\HSN-Demo2\C-Server\_Socket.h"
#include <iostream>

void sendTo(int ID, unsigned char * data, int dataSize, SocketClient s)
{
	char* sendbuffer; // Buffer for package
	sendbuffer = (char *) malloc((dataSize+2)*3);
	sprintf(sendbuffer,"ID%d ", ID);
	char* a = (char *) malloc(3);
	for(int i=0; i<dataSize; i++)
    {
	  sprintf(a, "%d ", (int)data[i]);
	  strcat(sendbuffer,a);
    } 
    s.SendLine(sendbuffer);
	delete(sendbuffer);
	delete(a);
}