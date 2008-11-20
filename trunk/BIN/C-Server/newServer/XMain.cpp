#include <iostream>
#include <fstream>
#include <cv.h>
#include <process.h>
#include <string>
#include "../../../HSN-Demo2/C-EntranceDet/_Global.h"
#include "../../../HSN-Demo2/C-EntranceDet/_FileCapture.h"
#include "../../../HSN-Demo2/C-EntranceDet/_BackgroundModel.h"
#include "../../../HSN-Demo2/C-EntranceDet/_Funcs.h"
#include "../../../HSN-Demo2/C-EntranceDet/_MoteClass.h"
#include "_MoteFncs.h"
#include "_MoteComm.h"
#include "..\..\..\HSN-Demo2\C-Server\_Socket.h"
using namespace std;


int CURTIME = 0;

const int NO_VIEWS = 6;
MoteClass MOTE[NO_VIEWS];
char MCOMM_PORT[100] = "COM6";
int MCOMM_DISPLAY = 0;

// This function takes care of writing the complex if a message is received.
void WriteC(void* parm)
{
  int ct = 1;

	SocketServer inSocket(4000,5);

	while(1)
	{
		Socket* server = inSocket.Accept();
		printf("\n[-] Connection Established ... \n");

		while(1)
		{
			std::string r = server->ReceiveLine();
			if(r.empty()) break;

			//printf("  [-] Writing Complex (%d)\n",ct++);
			WriteComplex(CURTIME,MOTE,NO_VIEWS,"Complex.dta");
		}

		printf("\n[-] Connection Lost ...\n");
		delete server;
	}

  return;
}

// This function processes the packet and it MUST be created if using the
// "MoteComm" functions.
void ProcessPacket(int moteID, int pktType, xshort* buf, int bufSz)
{
	switch(pktType)
	{
	case 11:
		ProcPkgModel(buf,&(MOTE[0])+moteID);
		break;
	case 12:
    ProcPkgParameters(buf,&(MOTE[0])+moteID);
		break;
	case 13:
		ProcPkgObs(buf[0],buf+1,bufSz-1,&(MOTE[0])+moteID);
		break;
	}

	if(pktType==13)
	{
		//printf("  [-] Number of regions: ");
		for(int n = 0; n< NO_VIEWS; n++)
		{
			// Getting current time
			CURTIME = max(CURTIME,GetCurrTime(MOTE+n));

			// Displaying Number of Regions:
			//printf(" %d ",GetCntRegions(MOTE+n));
		}
		//printf("\n");
	}

	return;
}


int main(int argCt, char* argv[])
{
  char buf[1000];

	// Getting Port
	for(int i = 1; i<(argCt-1); i++)
		if(argv[i][0]=='-' && argv[i][1]=='p')
			strcpy(MCOMM_PORT,argv[i+1]);
	printf("Using Port: %s\n\n",MCOMM_PORT);
	SendStopSync();

  // Starting Server Connection
	_beginthread(WriteC,0,NULL);

	// Getting Packets from mote
  _beginthread(ReceiveFromMote,0,NULL);

	// Entering main Menu
	int ch = -1;
	while(ch!=0)
	{
		printf("\n\n\n");
		printf("-----------------------------------\n");
		printf("| [1] Send Stop Sync Signal       |\n");
		printf("| [2] Display Packets             |\n");
		printf("| [3] Show Observations           |\n");
		printf("| [4] Display Saved Complex       |\n");
		printf("| [0] EXIT                        |\n");
		printf("-----------------------------------\n");
		printf(">> ");
		cin>>ch;
		printf("\n");

		switch(ch)
		{
		case 1:	// Sending Stop Sync Message
			printf("[-] Sending Stop Synchronization signal ...\n");
			SendStopSync();
			break;
		case 2: // Setting Display flag to active.
			MCOMM_DISPLAY = 1;
			cin.getline(buf,1000);
			cin.getline(buf,1000);
			MCOMM_DISPLAY = 0;
			break;
		case 3: // Displaying Observations
			for(int n = 0; n<NO_VIEWS; n++)
			{
				printf("History of Mote-%d\n",n+1);
				PrintStateHistory(MOTE+n);
			}
			break;
		case 4:
			ifstream in;
			in.open("Complex.dta");

			if(!in.fail())
			{
				in.getline(buf,1000);
				while(!in.eof())
				{
					printf("  [%s]\n",buf);
					in.getline(buf,1000);
				}
			}

			in.close();
			break;
		}
	}

  // Getting rid of some arrays
  delete MOTE;
  return 0;
}
