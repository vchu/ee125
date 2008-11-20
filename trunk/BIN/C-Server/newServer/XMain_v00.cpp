#include <iostream>
#include <fstream>
#include <cv.h>
#include <process.h>
#include <string>
#include "../../SRC/C-EntranceDet/_Global.h"
#include "../../SRC/C-EntranceDet/_FileCapture.h"
#include "../../SRC/C-EntranceDet/_BackgroundModel.h"
#include "../../SRC/C-EntranceDet/_Funcs.h"
#include "../../SRC/C-EntranceDet/_MoteClass.h"
#include "_MoteFncs.h"
#include "_MoteComm.h"
#include "_Socket.h"
using namespace std;


MoteClass* MOTE = 0;
Socket* SERVER = 0;
int NO_VIEWS = 0;


// This function takes care of writing the complex if a message is received.
unsigned __stdcall WriteC(void* a) {
  static int ct = 1;
  int curTime = *((int*)a);

  while (1) {
    std::string r = SERVER->ReceiveLine();

    if (r.empty()) break;

    if(MOTE)
    {
      printf("  [-] Writing Complex (%d)\n",ct++);
      WriteComplex(curTime,MOTE,NO_VIEWS,"../../Complex.dta");
    }
  }

  delete SERVER;

  return 0;
}


int main()
{
  xshort buf[1000];
  int bufSz;
  ifstream in;

  // Reading the number of views to be considered.
  in.open("C:/Pictures/HSN/Seq03/_readme.txt");
  in.getline((char*)buf,1000);
  in>>NO_VIEWS;
  in.close();

  // Creating bisecting tree structure.
  MOTE = new MoteClass[NO_VIEWS];

  // We will update the current time based on the latest observation recorded
  // by all motes.
  int curTime = 0;
  for(int n = 0; n < NO_VIEWS; n++)
  {
    printf("\n\n--------------------------------------- VIEW %02d\n",n+1);

    // Processing Model Package
    sprintf((char*)buf,"../../Pkg_Model_%02d.dta",n);
    in.open((char*)buf,ios::binary);
    bufSz = 0;
    buf[bufSz] = in.get();
    while(!in.eof())
    {
      bufSz++;
      buf[bufSz] = in.get();
    }
    in.clear();
    in.close();
    ProcPkgModel(buf,MOTE+n);

    // Processing Parameter Package
    sprintf((char*)buf,"../../Pkg_Parameters_%02d.dta",n);
    in.open((char*)buf,ios::binary);
    bufSz = 0;
    buf[bufSz] = in.get();
    while(!in.eof())
    {
      bufSz++;
      buf[bufSz] = in.get();
    }
    in.clear();
    in.close();
    ProcPkgParameters(buf,MOTE+n);

    // Processing Parameter Package
    //MOTE[n].m_Child1->m_Det_StateTime[MC_SIZE_STATEHISTORY-1] = 266;
    //MOTE[n].m_Child1->m_Det_StateHist[MC_SIZE_STATEHISTORY-1] = 1;
    //MOTE[n].m_Child1->m_Det_StateTime[0] = 300;
    //MOTE[n].m_Child1->m_Det_StateHist[0] = 0;
    //MOTE[n].m_Child1->m_Det_StatePos=1;

    sprintf((char*)buf,"../../Pkg_Observations_%02d.dta",n);
    in.open((char*)buf,ios::binary);
    bufSz = 0;
    buf[bufSz] = in.get();
    while(!in.eof())
    {
      bufSz++;
      buf[bufSz] = in.get();
    }
    in.clear();
    in.close();
    printf("No. of Regions in Obs Pkg:   %d\n",ProcPkgObs_noReg(buf+1,bufSz-1));
    ProcPkgObservations(buf[0],buf+1,bufSz-1,MOTE+n);

    // Displaying Recovered Data
    printf("Current States:   ");
    PrintState(MOTE+n); printf("\n");
    printf("History of States: \n");
    PrintStateHistory(MOTE+n);

    printf("Line Equations:\n");
    sprintf((char*) buf,"../../Mote%02d_Bisection.dta",n);
    ofstream out;
    out.open((char*)buf);
    bufSz = 0; PrintLine(MOTE+n,(char*)buf,&bufSz); printf((char*)buf);
    out<<buf;
    out.close();

    // Updating current time
    curTime = max(curTime,MOTE[n].m_Child1->m_Det_StateTime[
      MOTE[n].m_Child1->m_Det_StatePos-1]);
  }

  // Testing to function to return state
  //int val = 1;
  //while(val>0)
  //{
  //  printf("\nEnter a time: ");
  //  scanf("%d",&val);
  //  
  //  printf("State for first region at time %d is (%d)\n",val,
  //    GetState(val,MOTE[0].m_Child1));
  //}

  // Writing Complex
  WriteComplex(curTime,MOTE,NO_VIEWS,"../../Complex.dta");

  // Getting Packets from mote
  //ReceiveFromMote();

  // Starting Server Connection
  printf("\n\n--------------------------------------- CONNECTION\n");
  SocketServer inSocket(4000,5);
  while (1) {
    SERVER = inSocket.Accept();
    printf("[-] Connection Established ...\n");
    WriteC(&curTime);
    printf("[-] Connection Lost ... \n");
  }

  // Getting rid of some arrays
  delete MOTE;
  return 0;
}
