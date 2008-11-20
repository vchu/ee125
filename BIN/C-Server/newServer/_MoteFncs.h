#ifndef MOTEFNCS_H
#define MOTEFNCS_H


#include <iostream>
#include <fstream>
#include <math.h>
#include "../../../HSN-Demo2/C-EntranceDet/_MoteClass.h"
using namespace std;

void ProcPkgModel(xshort* buf, MoteClass* mote);
void ProcPkgParameters(xshort* buf, MoteClass* mote);
void ProcPkgObservations(int nStart,xshort* buf,int bufSz,MoteClass* mote);
void ProcPkgObs(int regID_Start, xshort* buf, int bufSz, MoteClass* mote);
int ProcPkgObs_noReg(xshort* buf, int bufSz);
int GetState(int time, MoteClass* mote);
void WriteComplex(int curTime, MoteClass* mote, int noMotes,
                  char* fileName = 0);
int GetCurrTime(MoteClass* mote);
int GetCntRegions(MoteClass* mote);

#endif