#ifndef MOTECLASS_H
#define MOTECLASS_H


#include <iostream>
#include <math.h>
#include "_Global.h"
using namespace std;


#define MC_THRESH_LINE_DISTANCE 400
//! Threshold on distance from line to set states to zero if within value.
#define MC_THRESH_STATE_DISTLINE 10
//! Threshold on distance from bnd to set states to zero if within value.
#define MC_THRESH_STATE_DISTBND 10
#define MC_INVFAC_DISTANCE_SCALE 10
#define MC_SIZE_STATEHISTORY 1000
#define MC_IMAGEWIDTH 200
#define MC_IMAGEHEIGHT 150
#define MC_MAX_REGIONS 20

/*! This class takes care of keeping track of the bisection tree structure
    associated to each camera field of view. It also keeps track of the
    states based on the foreground masks.
*/
class MoteClass
{
public:
  MoteClass(MoteClass* Parent = 0,int BirthTime = 0);
  ~MoteClass();

  MoteClass* m_Child1; //!< Pointer to first child
  MoteClass* m_Child2; //!< Pointer to second child
  MoteClass* m_Parent; //!< Pointer to the parent
  int m_BirthTime; //!< Time of birth of the camera region
  xshort m_Active; //!< Flag to determine if it is an active partition
  xshort m_dummyState; //!< Dummy state used when updating tree structure
  int m_Line_x0; //!< x-coordinate of a point in line bisecting domain
  int m_Line_y0; //!< y-coordinate of a point in line bisecting domain
  int m_Line_dx; //!< Coord. of vector in the direction of bisecting line
  int m_Line_dy; //!< Coord. of vector in the direction of bisecting line
  int m_Line_c1; //!< First coeff. for inter-line distance measure
  int m_Line_c2; //!< Second coeff. for inter-line distance measure
  int m_Line_c3; //!< Third coeff. for inter-line distance measure

  xshort m_Det_State; //!< Current state of detection in the current region
  //! History of the detection states.
  xshort m_Det_StateHist[MC_SIZE_STATEHISTORY];
  //! Time for each detection state.
  int m_Det_StateTime[MC_SIZE_STATEHISTORY];
  int m_Det_StatePos; //!< Current position on the states history arrays.

  void Bisect(int x0, int y0, int dx, int dy, int BisectTime,
    xshort* dataMask, int wStep, int width, int height);
  void SetState(xshort* dataMask, int wStep, int width, int height,
    int curTime);
  void SetNoDetection(int curTime = -1);
protected:
  void _ResetDummyState();
  void _SetState(int x, int y);
  void _Bisect(int x0, int y0, int dx, int dy, int x, int y,
    int BisectTime);
  void _UpdateDistCoeff();
  int _CompLineDist(int x0, int y0, int dx, int dy);
  void _UpdateStateHistory(int curTime);
};

void PrintState(MoteClass* mote);
void PrintLine(MoteClass* mote, char* arrOut = 0, int* arrSz = 0);
void PrintStateHistory(MoteClass* mote);

void int2char(xshort* buf,int noBytes,int val);
int char2int(xshort* buf,int noBytes);
int mod(int val,int base);

int GetPkgModel(xshort* buf, MoteClass* mote);
int GetPkgParameters(xshort* buf, MoteClass* mote);
int GetPkgObservations(xshort* buf, MoteClass* mote, int noObs);
int SplitPkgObservations(xshort** segPtr, int* segPtrSz,
                         xshort* buf, int bufSz, int bndSz);

#endif