#ifndef MOTECOMM_H
#define MOTECOMM_H

#include <WinSock2.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "../../HSN-Demo2/C-EntranceDet/_MoteClass.h"

using namespace std;

// Maximum size of packet received from mote.
#define MAX_PACKET_LEN 200

extern char MCOMM_PORT[];
extern int MCOMM_DISPLAY;

void ReceiveFromMote(void* parm);
void ParseMotePacket(xshort* msg, xshort* buf, int& bufSz, int& moteID, int& pktType);
void ProcessPacket(int moteID,int pktType, xshort* buf, int bufSz);
void SendStopSync();

#endif