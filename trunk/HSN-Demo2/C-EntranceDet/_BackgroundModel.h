#ifndef BACKGROUNDMODEL_H
#define BACKGROUNDMODEL_H


#include <iostream>
#include <fstream>
#include <cv.h>
#include "_Global.h"
using namespace std;


/*! Class storing background model. This class creates and stores a
    background model from images that are read from a specified image
    sequence. Some of the functionalities of this class include the
    generation of a foreground mask from a provided image by perfoming
    background subtraction.
*/
class BackgroundModel
{
public:
  int m_width;    //!< Width Of the images to be considered.
  int m_height;   //!< Height of the images to be considered.

  int* m_imBG;    //!< Array containing background model.
  int* m_imDx;  //!< x-coordinate of the gradient of the background.
  int* m_imDy;  //!< y-coordinate of the gradient of the background.
  xshort* m_imEdge;  //!< Mask containing edges in the background.

  ~BackgroundModel();
  
  void CreateModel(char* filePattern);
  int GetFGMask(IplImage* Dst, IplImage* Im);
};


#endif