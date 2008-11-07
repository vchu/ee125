#ifndef FUNCS_H
#define FUNCS_H


#include <iostream>
#include <fstream>
#include "_Global.h"
#include "_BackgroundModel.h"
using namespace std;

int FindBisection(xshort* dataDet, int wStep, int* x0, int* y0,
                  int* dx,int* dy, BackgroundModel* BG,
                  xshort* dataMask);


#endif