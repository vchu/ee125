#include "..\..\HSN-Demo2\C-Server\_Socket.h"
#include <iostream>

void sendTo(int ID, unsigned char * data, int dataSize, Socket s)
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
	//send(s, data, dataSize, 0);
    //s.SendLine(sendbuffer);
	delete(sendbuffer);
	delete(a);
}