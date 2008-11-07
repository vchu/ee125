#include "_Funcs.h"


/*! It fits a bisecting line given a mask where occlusion occurs. The line is
    supposed to match the edge of an entrance in a 2 1/2 D environment. A
    return value of '1' means that an appropriate fit was obtained ('0' is
    returned otherwise).

    \param dataDet Pointer to the image that will hold a mask of the set of
      points where an occlusion was detected. This is an output.
    \param wStep Width step for advancing to the next row in the image arrays.
    \param x0 Pointer used to return the coefficient of the line fit
      'x = (dy/dx)*(y - y0) + x0'.
    \param y0 Pointer used to return the coefficient of the line fit
      'x = (dy/dx)*(y - y0) + x0'.
    \param dx Pointer used to return the coefficient of the line fit
      'x = (dy/dx)*(y - y0) + x0'. The vector (dy,dx) has norm 1.
    \param dy Pointer used to return the coefficient of the line fit
      'x = (dy/dx)*(y - y0) + x0'. The vector (dy,dx) has norm 1.
    \param BG Pointer to the provided background model.
    \param dataMask Mask used for determining occlusion set.
*/
int FindBisection(xshort* dataDet, int wStep, int* x0, int* y0,
                  int* dx, int* dy, BackgroundModel* BG, xshort* dataMask)
{
  // Defines the size of the window used for searching detections
  int winSz = 5;
  // Threshold for static part of the window
  int thStatic = ((2*winSz+1)*(2*winSz+1)*3)/8;
  // Threshold for dynamic part of the window
  int thDynamic = ((2*winSz+1)*(2*winSz+1)*3)/10;
  // Threshold on the number of points for fitting (i.e. at least this many)
  int thNoDetections = 10;

  int width = BG->m_width;
  int height = BG->m_height;
  memset(dataDet,0,wStep*height*sizeof(unsigned char));

  // Obtaining set of detection points by using the provided mask. Local
  // templates are matched using the orientation of the edges in the background
  // image.
  int ctDet = 0; // For counting the number of detections.
  int xMean = 0, yMean = 0; // For computing center of mass of the set.
  for(int i = winSz; i<height-winSz; i++)
  {
    for(int j = winSz; j<width-winSz; j++)
    {
      int idx = i*width+j;

      // We only check for edge points. I am sure we can do this more efficient
      // if we let the set of edges to be a list instead of a large array.
      if(BG->m_imEdge[idx])
      {
        // FIRST CASE:
        int vx = BG->m_imDx[idx];
        int vy = BG->m_imDy[idx];
        int ctStatic = 0;
        int ctDynamic = 0;
        for(int n = -winSz; n<=winSz; n++)
        {
          for(int m = -winSz; m<=winSz; m++)
          {
            if(m*vx+n*vy>0 && dataMask[(i+n)*wStep+j+m]==0)
              ctStatic++;
            if(m*vx+n*vy<0 && dataMask[(i+n)*wStep+j+m]==255)
              ctDynamic++;
          }
        }
        if(ctStatic>thStatic && ctDynamic>thDynamic)
          dataDet[idx] = 255;

        // SECOND CASE:
        ctStatic = 0;
        ctDynamic = 0;
        for(int n = -winSz; n<=winSz; n++)
        {
          for(int m = -winSz; m<=winSz; m++)
          {
            if(m*vx+n*vy<0 && dataMask[(i+n)*wStep+j+m]==0)
              ctStatic++;
            if(m*vx+n*vy>0 && dataMask[(i+n)*wStep+j+m]==255)
              ctDynamic++;
          }
        }
        if(ctStatic>thStatic && ctDynamic>thDynamic)
          dataDet[idx] = 255;

        // We keep accumulate some values and keep track of counts of detected
        // elements.
        if(dataDet[idx])
        {
          ctDet++;
          xMean+=j;
          yMean+=i;
        }
      }
    }
  }
  // If there are too few points, then we don't attempt line fitting.
  if(ctDet<thNoDetections)
    return 0;

  // Computing the centroid of the set of detection points.
  xMean /= ctDet;
  yMean /= ctDet;


  // Fitting a Line parameterized by the y-coordinates.
  int Syy = 0;
  int Sxy = 0;
  for(int i = winSz; i<height-winSz; i++)
  {
    for(int j = winSz; j<width-winSz; j++)
    {
      int idx = i*wStep + j;
      if(dataDet[idx])
      {
        Syy += (i-yMean)*(i-yMean);
        Sxy += (i-yMean)*(j-xMean);
      }
    }
  }
  (*x0) = xMean;
  (*y0) = yMean;
  (*dx) = Sxy;
  (*dy) = Syy;

  // Evaluating spatial fit of the line to the provided coordinate points.
  // We select points that are close enough to the line and make sure there
  // is enough of them.
  ctDet = 0;
  for(int i = 0; i<height; i++)
  {
    int x0 = (Sxy*(i-yMean))/Syy + xMean - 2;
    int x1 = (Sxy*(i-yMean))/Syy + xMean + 2;

    for(int j = x0; j<= x1; j++)
      if(!(x0>= 0 && x1<width))
        dataDet[i*wStep+j] = 0;
      else if( dataDet[i*wStep+j] )
        ctDet++;
  }
  // If there are too few points, then we reject the fit.
  if(ctDet<thNoDetections)
    return 0;

  return 1;
}