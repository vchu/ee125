#include "..\..\..\HSN-Demo2\C-Server\_Socket.h"
#include <process.h>
#include <cv.h>
#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <cstdlib>
#include <direct.h>
#include "..\..\..\HSN-Demo2/C-EntranceDet/_Global.h"
#include "..\..\..\HSN-Demo2/C-EntranceDet/_FileCapture.h"
#include "..\..\..\HSN-Demo2/C-EntranceDet/_BackgroundModel.h"
#include "..\..\..\HSN-Demo2/C-EntranceDet/_Funcs.h"
#include "..\..\..\HSN-Demo2/C-EntranceDet/_MoteClass.h"
#include "_MoteFncs.h"
#include "_MoteComm.h"

int CURTIME = 0;

const int NO_VIEWS = 6;
MoteClass MOTE[NO_VIEWS];
char MCOMM_PORT[100] = "COM6";
int MCOMM_DISPLAY = 0;

using namespace std;
queue<char *> dataQueue;

bool PathExists(char* pathtocheck)
{
int ret = _chdir(pathtocheck) ;
return (ret == 0) ;
}

unsigned __stdcall Answer(void* a) {
    
	Socket* s = (Socket*) a;
	unsigned char r[1024] ;
	int len=0;

  while (1) {
  
	s->ReceiveBytes(len, r);
	
    if (len==0 || len==-1) break;
	std::cout<<"\nClient sent:";
	char* temp = (char *) malloc(len);
	for (int i = 0 ; i<len; i++){
		printf("%d ",r[i]);
		temp[i] = r[i];
	}
	dataQueue.push(temp);
	char* parseData = dataQueue.front();
	int ID = parseData[0];
	char* IDstr= (char *) malloc(4);
	char* folderpath= (char *) malloc(256);
	sprintf(IDstr, "%d", ID);
	sprintf(folderpath, "C:\\%s", IDstr);
	
	if(!PathExists(folderpath)){
		mkdir(folderpath);
	}
	char * temp1 = (char *) malloc(256);
	sprintf(temp1, "%d", parseData[1]);
	char * filename = (char *) malloc(256);
	sprintf(filename, "%s\\%s", folderpath, temp1);
	std::ofstream myFile (filename, ios::out | ios::binary);
	std::string blah= parseData;
    myFile.write (parseData, len);
	myFile.close();

	dataQueue.pop();	// takes out packet from queue
	delete parseData;	// clear memory from malloc
  }

  delete s;
  

  return 0;
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

int main(int argc, char* argv[]) {
   std::cout<< "Initializing server at port 2000";
  SocketServer in(2000,5);

  while (1) {
	  printf("HeLLo thread");
    Socket* s=in.Accept();

    unsigned ret;
	printf("Good bye thread"); 
    _beginthreadex(0,0,Answer,(void*) s,0,&ret);
    

  }
 
  return 0;
}
