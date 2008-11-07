#include "_MoteClass.h"


/*! Constructor making sure that camera is initially set to active. Birthtime
    and link to parent are set if provided.

    \param Parent Pointer to parent element.
    \param BirthTime Time at which the mote is created.
*/
MoteClass::MoteClass(MoteClass* Parent, int BirthTime)
{
  m_Active = 1;
  m_Child1 = 0;
  m_Child2 = 0;
  m_Parent = Parent;
  m_BirthTime = BirthTime;
  m_Det_State = 0;
  m_Det_StatePos = 0;
  memset(m_Det_StateHist,0,sizeof(xshort)*MC_SIZE_STATEHISTORY);
  memset(m_Det_StateTime,0,sizeof(int)*MC_SIZE_STATEHISTORY);
}


/*! It makes sure that the children are deleted.
*/
MoteClass::~MoteClass()
{
  if(m_Child1)
    delete m_Child1;
  if(m_Child2)
    delete m_Child2;
}


/*! This function bisects any active partition based on specifications
    for the bisecting line and a mask of points where the occlusion was
    detected.

    \param x0 x-coordinate of a point in the bisecting line.
    \param y0 y-coordinate of a point in the bisecting line.
    \param dx x-coordinate of a vector in the direction of the bisecting line.
      We assume that this vector is normalized to have length 1.
    \param dy y-coordinate of a vector in the direction of the bisecting line.
      We assume that this vector is normalized to have length 1.
    \param BisecTime time at which bisection occurs. Used to assign birthtime
      of children.
    \param dataMask Pointer to image marking in which pixels the occlusion
      was detected.
    \param wStep Width step for the image array. The amount of pixels needed
      to move to the next row.
    \param width Width of the image.
    \param height Height of the image.
*/
void MoteClass::Bisect(int x0, int y0, int dx, int dy, int BisectTime,
    xshort* dataMask, int wStep, int width, int height)
{
  if(m_Active)
  {
    // We assume that this function is not called if the mask is empty.

    // Storing bisecting line information
    m_Line_x0 = x0;
    m_Line_y0 = y0;
    int norm = max(abs(dx),abs(dy));
    m_Line_dx = (dx<<8)/norm;
    m_Line_dy = (dy<<8)/norm;

    _UpdateDistCoeff();

    // Creating children
    m_Child1 = new MoteClass(this,BisectTime);
    m_Child2 = new MoteClass(this,BisectTime);
    m_Active = 0;
  }
  else
  {
    // If this region is not active then we go iterate over all pixels and
    // children. Given that we are not dealing with the same bisecting line.

    int d2 = _CompLineDist(x0,y0,dx,dy);
    if( d2 > MC_THRESH_LINE_DISTANCE*(m_Line_dx*m_Line_dx+m_Line_dy*m_Line_dy) )
    {
      // If the condition above is satisfied then we have different bisecting
      // line.

      // Here we will use the dummy state as an indicator that a particular
      // bisecting line has already been updated.
      _ResetDummyState();

      // Iterating over all active pixels in the mask.
      for(int i = 0; i<height; i++)
        for(int j = 0; j<width; j++)
        {
          if( dataMask[i*wStep+j] )
          {
            // The inequality below determines in which child region it is. This
            // is the dot product between the vector (j,i) - (x0,y0) and a
            // vector normal to the line.
            if( (-(j-m_Line_x0)*m_Line_dy + (i-m_Line_y0)*m_Line_dx)>0 )
              m_Child1->_Bisect(x0,y0,dx,dy,j,i,BisectTime);
            else
              m_Child2->_Bisect(x0,y0,dx,dy,j,i,BisectTime);
          }
        }
    }
    else
    {
      // In this case we update our bisecting line by using the line provided
      // to us. Note that we assume that the vectors points in the direction
      // of the line are normalized.

      m_Line_x0 = (8*m_Line_x0 + 2*x0 + 5)/10;
      m_Line_y0 = (8*m_Line_y0 + 2*y0 + 5)/10;

      int norm = max(abs(dx),abs(dy));
      dx = (dx<<8)/norm;
      dy = (dy<<8)/norm;
      m_Line_dx = (8*m_Line_dx + 2*dx);
      m_Line_dy = (8*m_Line_dy + 2*dy);
      norm = max(abs(m_Line_dx),abs(m_Line_dy));
      m_Line_dx = ((m_Line_dx<<8)+norm/2)/norm;
      m_Line_dy = ((m_Line_dy<<8)+norm/2)/norm;

      _UpdateDistCoeff();
    }
  }

  return;
}

/*! This function bisects any active partition based on specifications
    for the bisecting line and a single point where the occlusion event was
    detected.

    \param x0 x-coordinate of a point in the bisecting line.
    \param y0 y-coordinate of a point in the bisecting line.
    \param dx x-coordinate of a vector in the direction of the bisecting line.
    \param dy y-coordinate of a vector in the direction of the bisecting line.
    \param x x-coordinate of point where occlusion was detected.
    \param y y-coordinate of point where occlusion was detected.
    \param BisecTime time at which bisection occurs. Used to assign birthtime
      of children.
*/
void MoteClass::_Bisect(int x0, int y0, int dx, int dy, int x, int y,
  int BisectTime)
{
  // If the birthtime is greater or equal to the bisection time it means that
  // that this region has already been bisected at this time. That is, we only
  // allow one bisection on each reagion per time step.
  if(m_BirthTime<BisectTime)
  {
    if(m_Active)
    {
      m_Line_x0 = x0;
      m_Line_y0 = y0;
      int norm = max(abs(dx),abs(dy));
      m_Line_dx = (dx<<8)/norm;
      m_Line_dy = (dy<<8)/norm;

      _UpdateDistCoeff();

      m_Child1 = new MoteClass(this,BisectTime);
      m_Child2 = new MoteClass(this,BisectTime);
      m_Active = 0;
    }
    else
    {
      int d2 = _CompLineDist(x0,y0,dx,dy);
      if( d2>MC_THRESH_LINE_DISTANCE*(m_Line_dx*m_Line_dx+m_Line_dy*m_Line_dy) )
      {
        // If region is not active we pass pass the fnc call to the children.
        if( (-(x-m_Line_x0)*m_Line_dy + (y-m_Line_y0)*m_Line_dx)>0 )
          m_Child1->_Bisect(x0,y0,dx,dy,x,y,BisectTime);
        else
          m_Child2->_Bisect(x0,y0,dx,dy,x,y,BisectTime);
      }
      else if(m_dummyState==0)
      {
        m_Line_x0 = (8*m_Line_x0 + 2*x0 + 5)/10;
        m_Line_y0 = (8*m_Line_y0 + 2*y0 + 5)/10;

        int norm = max(abs(dx),abs(dy));
        dx = (dx<<8)/norm;
        dy = (dy<<8)/norm;
        m_Line_dx = (8*m_Line_dx + 2*dx);
        m_Line_dy = (8*m_Line_dy + 2*dy);
        norm = max(abs(m_Line_dx),abs(m_Line_dy));
        m_Line_dx = ((m_Line_dx<<8)+norm/2)/norm;
        m_Line_dy = ((m_Line_dy<<8)+norm/2)/norm;

        _UpdateDistCoeff();

        m_dummyState = 1;
      }
    }
  }

  return;
}


/*! This function sets the states of the regions in the structure. It
    determines which regions are actually detecting something by using the
    provided mask.

    \param dataMask Pointer to image marking in which pixels the foreground
      was detected.
    \param wStep Width step for the image array. The amount of pixels needed
      to move to the next row.
    \param width Width of the image.
    \param height Height of the image.
    \param curTime Time at which this states are being observed.
*/
void MoteClass::SetState(xshort* dataMask, int wStep, int width, int height,
                         int curTime)
{
  // We start by resetting all states and then we assign values of '1' if
  // there were any detections.
  SetNoDetection();

  if(m_Active)
  {
    // We assume that this function is not called if the mask is empty.
    for(int i = MC_THRESH_STATE_DISTBND; i<height-MC_THRESH_STATE_DISTBND; i++)
      for(int j = MC_THRESH_STATE_DISTBND; j<width-MC_THRESH_STATE_DISTBND; j++)
      {
        if( dataMask[i*wStep+j] )
          m_Det_State = 1;
      }
  }
  else
  {
    // If region is not active we iterate over all pixels where there was
    // a detection and attemp to set the state for the children.
    for(int i = MC_THRESH_STATE_DISTBND; i<height-MC_THRESH_STATE_DISTBND; i++)
      for(int j = MC_THRESH_STATE_DISTBND; j<width-MC_THRESH_STATE_DISTBND; j++)
      {
        if( dataMask[i*wStep+j] )
        {
          if( abs((-(j-m_Line_x0)*m_Line_dy + (i-m_Line_y0)*m_Line_dx)) 
            > MC_THRESH_STATE_DISTLINE*max(abs(m_Line_dy),abs(m_Line_dx)) )
          {
            if( (-(j-m_Line_x0)*m_Line_dy + (i-m_Line_y0)*m_Line_dx)>0 )
              m_Child1->_SetState(j,i);
            else
              m_Child2->_SetState(j,i);
          }
        }
      }
  }

  // Storing any changes on the states.
  _UpdateStateHistory(curTime);

  return;
}


/*! This function sets the states of the regions in the structure. It
    determines which regions are actually detecting something based on the
    provided coordinates for a point of detection.

    \param x x-coordinate of point of detection.
    \param y y-coordinate of point of detection.
*/
void MoteClass::_SetState(int x, int y)
{
  if(m_Active)
    m_Det_State = 1;
  else
  {
    if( abs((-(x-m_Line_x0)*m_Line_dy + (y-m_Line_y0)*m_Line_dx))
      > MC_THRESH_STATE_DISTLINE*max(abs(m_Line_dy),abs(m_Line_dx)) )
    {
      if( (-(x-m_Line_x0)*m_Line_dy + (y-m_Line_y0)*m_Line_dx)>0 )
        m_Child1->_SetState(x,y);
      else
        m_Child2->_SetState(x,y);
    }
  }

  return; 
}


/*! This function sets all detection states to 0, i.e. no detections.
    \param curTime Time at which we are setting the states, a value of -1
      tells the program to not update the history when clearing the states.
*/
void MoteClass::SetNoDetection(int curTime)
{
  if(m_Active)
    m_Det_State = 0;
  else
  {
    m_Child1->SetNoDetection();
    m_Child2->SetNoDetection();
  }

  if(curTime>=0)
    _UpdateStateHistory(curTime);

  return;
}


/*! This function resets the values of the dummy state to 0 to current region
    and all children.
*/
void MoteClass::_ResetDummyState()
{
  m_dummyState = 0;
  if(!m_Active)
  {
    m_Child1->_ResetDummyState();
    m_Child2->_ResetDummyState();
  }

  return;
}


/*! It updates the coefficients required for computing the scaled distance
    between the base point of two lines. It scales down the distance in the
    direction of the line that is currently used for bisection.
*/
void MoteClass::_UpdateDistCoeff()
{
  m_Line_c1 = m_Line_dx*m_Line_dx/MC_INVFAC_DISTANCE_SCALE + m_Line_dy*m_Line_dy;
  m_Line_c2 = 2*(1-MC_INVFAC_DISTANCE_SCALE)*m_Line_dx*m_Line_dy
    /MC_INVFAC_DISTANCE_SCALE;
  m_Line_c3 = m_Line_dy*m_Line_dy/MC_INVFAC_DISTANCE_SCALE + m_Line_dx*m_Line_dx;

  return;
}


/*! Computes the distance of a line to the current bisecting line in the
    region under consideration. This score is used to determine if two lines
    are to be identified or not.

    \param x0 x-coord. of base point of line.
    \param y0 y-coord. of base point of line.
    \param dx x-coord. of vector in direction of line.
    \param dy y-coord. of vector in direction of line.
*/
int MoteClass::_CompLineDist(int x0, int y0, int dx, int dy)
{
  // NOTE: Currently we are only considering scaled distance between points.
  //       Make sure to include slope distance in the future. This is good
  //       enough for the demo.
  int d2 = m_Line_c1*(x0-m_Line_x0)*(x0-m_Line_x0)
    + m_Line_c2*(x0-m_Line_x0)*(y0-m_Line_y0)
    + m_Line_c3*(y0-m_Line_y0)*(y0-m_Line_y0);
  
  return d2;
}


/*! It updates the history of the states observed for each region. It looks
    adds the current state value and time if there is a change on the state.
    \param curTime Time at which the current states were updated.
*/
void MoteClass::_UpdateStateHistory(int curTime)
{
  if(m_Active)
  {
    if(m_Det_StateHist[m_Det_StatePos]!=m_Det_State)
    {
      m_Det_StatePos = (m_Det_StatePos+1)%MC_SIZE_STATEHISTORY;
      m_Det_StateHist[m_Det_StatePos] = m_Det_State;
      m_Det_StateTime[m_Det_StatePos] = curTime;
    }
  }
  else
  {
    m_Child1->_UpdateStateHistory(curTime);
    m_Child2->_UpdateStateHistory(curTime);
  }
}


/*! This function prints the states for a tree structure. It iterates
    looking for active regions.
    \param mote Pointer to mote structure.
*/
void PrintState(MoteClass* mote)
{
  if(mote->m_Active)
    printf("%d",(int)(mote->m_Det_State));
  else
  {
    printf(" [");
    PrintState(mote->m_Child1);
    PrintState(mote->m_Child2);
    printf("] ");
  }

  return;
}


/*! This function prints the equations of the bisecting lines found.
    \param mote Pointer to mote structure.
*/
void PrintLine(MoteClass* mote, char* arrOut, int* arrSz)
{
  if(!mote->m_Active)
  {
    float beta1 = ((float)mote->m_Line_dx)/mote->m_Line_dy;
    float beta0 = mote->m_Line_x0 - beta1*mote->m_Line_y0;

    if(arrOut==0)
      printf("      x = %f y + %f\n",beta1,beta0);
    else
      (*arrSz) += sprintf(arrOut+(*arrSz)," x = %f*y + %f\n",beta1,beta0);
    PrintLine(mote->m_Child1,arrOut,arrSz);
    PrintLine(mote->m_Child2,arrOut,arrSz);
  }

  return;
}


/*! This function prints the states observed for each bisected region.
    \param mote Pointer to mote structure.
*/
void PrintStateHistory(MoteClass* mote)
{
  if(mote->m_Active)
  {
    for(int i = 0; i<=mote->m_Det_StatePos; i++)
      printf("(%d: %d) ",mote->m_Det_StateTime[i],mote->m_Det_StateHist[i]);
    printf("\n");
  }
  else
  {
    PrintStateHistory(mote->m_Child1);
    PrintStateHistory(mote->m_Child2);
  }

  return;
}


/*! This functions takes and integer value and writes it as characters into
    the provided buffer.

    \param buf Pointer to the buffer where the data will be written.
    \param noBytes Number of bytes to express the integer with.
    \param val Integer value to write into buffer.
*/
void int2char(xshort* buf, int noBytes, int val)
{
  int n = 8*(noBytes-1);
  for(int i = 0; i<noBytes; i++)
  {
    buf[i] = val >> n;
    val = val - ((val>>n)<<n);
    n -= 8;
  }

  return;
}


/*! This functions returns and integer value created from values in the 
    provided buffer.

    \param buf Pointer to the buffer containing data.
    \param noBytes Number of bytes from the buffer to be used to obtain the
      integer.
*/
int char2int(xshort* buf, int noBytes)
{
  int val = 0;

  for(int i = 0; i<noBytes; i++)
    val = (val<<8) + (int)buf[i];
  
  return val;
}


/*! It returns "<val> modulo <base>".

    \param val Value for which the modulo will be computed.
    \param base Base used for computing the modulo.
*/
int mod(int val, int base)
{
  while(val<0)
    val += base;
  return (val%base);
}


/*! This function adds instructions recursively for recreating the binary tree
    model. This is a subfunction called when creating the model package.
*/
void _GetPkgModel1(xshort* buf, int* bufSz, MoteClass* mote, int& ct)
{
  if(ct%8)
    buf[(*bufSz)-1] = (buf[(*bufSz)-1]<<1);
  else
  {
    (*bufSz)++;
    buf[(*bufSz)-1] = 0;
  }

  ct++;
  if(!mote->m_Active)
  {
    buf[(*bufSz)-1] += 1;
    _GetPkgModel1(buf,bufSz,mote->m_Child1,ct);
    _GetPkgModel1(buf,bufSz,mote->m_Child2,ct);
  }

  return;
}


/*! This functions add birth-time recursively. This is a subfunction called when
    creating the model package.
*/
void _GetPkgModel2(xshort* buf, int* bufSz, MoteClass* mote)
{
  if(mote->m_Active)
  {
    int2char(buf+(*bufSz),4,mote->m_BirthTime);
    (*bufSz)+=4;
  }
  else
  {
    _GetPkgModel2(buf,bufSz,mote->m_Child1);
    _GetPkgModel2(buf,bufSz,mote->m_Child2);
  }

  return;
}


/*! It creates a package containing instructions for recreating the binary
    tree structure in the mote. The data is stored into the provided buffer
    and the size of the package is returned. This function is meant to generate
    a single package of size less than 100 bytes, which requires that the
    number of partitions is less than 20.

    \param buf Pointer to the buffer where data will be written.
    \param mote Pointer to the mote structure to be used for creating the
      package.
*/
int GetPkgModel(xshort* buf, MoteClass* mote)
{
  int bufSz = 1;
  buf[0] = 0;

  // The following counter is used as an indicator for when to switch to the
  // next byte. We store instructions for recreating the tree one bit at the
  // time.
  int ct = 1;

  // Adding instructions for constructing tree.
  if(!mote->m_Active)
  {
    buf[0] += 1;
    _GetPkgModel1(buf,&bufSz,mote->m_Child1,ct);
    _GetPkgModel1(buf,&bufSz,mote->m_Child2,ct);

    // Shifting the last entry if necessary.
    if(ct%8)
    {
      buf[bufSz-1] = buf[bufSz-1]<<(8-(ct%8));
    }
  }

  // Adding Birth times.
  if(mote->m_Active)
  {
    int2char(buf+bufSz,4,mote->m_BirthTime);
    bufSz+=4;
  }
  else
  {
    _GetPkgModel2(buf,&bufSz,mote->m_Child1);
    _GetPkgModel2(buf,&bufSz,mote->m_Child2);
  }

  return bufSz;
}


/*! This functions add line parameters recursively. This is a subfunction called
    when creating the parameter package.
*/
void _GetPkgParameters(xshort* buf, int* bufSz, MoteClass* mote)
{
  if(!mote->m_Active)
  {
    int2char(buf+(*bufSz),1,(255*mote->m_Line_x0)/MC_IMAGEWIDTH); (*bufSz)++;
    int2char(buf+(*bufSz),1,(255*mote->m_Line_y0)/MC_IMAGEHEIGHT); (*bufSz)++;
    int2char(buf+(*bufSz),1,(255*(mote->m_Line_dx+256))/512); (*bufSz)++;
    int2char(buf+(*bufSz),1,(255*(mote->m_Line_dy+256))/512); (*bufSz)++;

    _GetPkgParameters(buf,bufSz,mote->m_Child1);
    _GetPkgParameters(buf,bufSz,mote->m_Child2);
  }

  return;
}


/*! It creates the package specifying all bisecting line parameters in the
    tree structure. It assigns 1 byte for each parameter. In order to keep the
    package less than 100 bytes, we should not have more than 20 partitions.

    \param buf Pointer to the buffer where data will be written.
    \param mote Pointer to the mote structure to be used for creating the
      package.
*/
int GetPkgParameters(xshort* buf, MoteClass* mote)
{
  int bufSz = 0;

  if(!mote->m_Active)
  {
    // We are assigning one byte per coordinate after we normalize such that
    // values are between 0 and 255.
    int2char(buf+bufSz,1,(255*mote->m_Line_x0)/MC_IMAGEWIDTH); bufSz++;
    int2char(buf+bufSz,1,(255*mote->m_Line_y0)/MC_IMAGEHEIGHT); bufSz++;
    
    // We are assigning one byte per coordinate after we normalize such that
    // values are between 0 and 255. Note that we are assuming that norm of the
    // vector is 1, so the coordinates are between -1 and 1 before
    // normalization.
    int2char(buf+bufSz,1,(255*(mote->m_Line_dx+256))/512); bufSz++;
    int2char(buf+bufSz,1,(255*(mote->m_Line_dy+256))/512); bufSz++;

    _GetPkgParameters(buf,&bufSz,mote->m_Child1);
    _GetPkgParameters(buf,&bufSz,mote->m_Child2);
  }

  return bufSz;
}


/*! This functions add observations recursively. This is a subfunction called
    when creating the observations package.
*/
void _GetPkgObservations(xshort* buf, int* bufSz, MoteClass* mote, int noObs)
{
  if(mote->m_Active)
  {
    int idxInit = mote->m_Det_StatePos - noObs + 1;
    buf[*bufSz] = noObs;
    (*bufSz)++;
    buf[*bufSz] = mote->m_Det_StateHist[mod(idxInit,MC_SIZE_STATEHISTORY)];
    (*bufSz)++;

    for(int i = 0; i<noObs; i++)
    {
      int idx = mod(idxInit+i,MC_SIZE_STATEHISTORY);
      int2char(buf+(*bufSz),3,mote->m_Det_StateTime[idx]);
      (*bufSz)+=3;
    }
  }
  else
  {
    _GetPkgObservations(buf,bufSz,mote->m_Child1,noObs);
    _GetPkgObservations(buf,bufSz,mote->m_Child2,noObs);
  }

  return;
}


/*! It creates the package specifying all observations made by the mote.
    mote. It assigns 1 byte for each parameter. It stores the number of
    observations transmitted, the current state, and the time stamp when
    transitions were detected per region.

    \param buf Pointer to the buffer where data will be written.
    \param mote Pointer to the mote structure to be used for creating the
      package.
    \param noObs Number of observations to be transmitted for each region.
      This number should be less that 100 bytes to makes sure agreement with
      the mote package size.
*/
int GetPkgObservations(xshort* buf, MoteClass* mote, int noObs)
{
  int bufSz = 0;

  if(mote->m_Active)
  {
    // Adding Number of Observations and Current State
    int idxInit = mote->m_Det_StatePos - noObs + 1;
    buf[bufSz] = noObs;
    bufSz++;
    buf[bufSz] = mote->m_Det_StateHist[mod(idxInit,MC_SIZE_STATEHISTORY)];
    bufSz++;

    // Adding State Time History
    for(int i = 0; i<noObs; i++)
    {
      int idx = mod(idxInit+i,MC_SIZE_STATEHISTORY);
      int2char(buf+bufSz,3,mote->m_Det_StateTime[idx]);
      bufSz+=3;
    }
  }
  else
  {
    _GetPkgObservations(buf,&bufSz,mote->m_Child1,noObs);
    _GetPkgObservations(buf,&bufSz,mote->m_Child2,noObs);
  }

  return bufSz;
}


/*! This function split the Observations packet into segments of bounded
    length. The user must provide pointers to the arrays where everything will
    be stored.

    \param segPtr Pointer to the 2D array that will store the split segments.
      The rows correspond to the segments.
    \param segPtrSz Pointer to sizes of the corresponding segments.
    \param buf Pointer to the packet to be split.
    \param bufSz Size of the packet to be split.
    \param bndSz Upper bound on the size of the segments.
*/
int SplitPkgObservations(xshort** segPtr, int* segPtrSz,
                          xshort* buf, int bufSz, int bndSz)
{
  // This index will keep track of the location of the data to be extracted
  // in the original packet.
  int idx = 0;

  // This variable will store the size of the next block to be added to the
  // current packet.
  int segLength;

  // Keeps track of the block that is added to the segment.
  int blockIdx = 0;

  // Count of the number of segments that are created.
  int ctSegments = 0;
  for(int i = 0; i<MC_MAX_REGIONS; i++)
  {
    segPtr[i][0] = blockIdx;
    segPtrSz[i] = 1;

    segLength = 3*buf[idx] + 2;
    while(segPtrSz[i]+segLength < bndSz && idx < bufSz)
    {
      // Copying the block, updating the size of the segment and block index.
      memcpy(segPtr[i]+segPtrSz[i],buf+idx,segLength);
      segPtrSz[i] += segLength;
      blockIdx++;

      // Updating data index and getting length of next segment.
      idx += segLength;
      segLength = 3*buf[idx] + 2;
    }
    // We increase the number of segments if we have actually added something
    // to the segment.
    if(segPtrSz[i]>1)
      ctSegments++;

    // If we have reached the end of the packet, we exit the loop.
    if(idx>=bufSz)
      break;
  }

  // Returning the number of segments created.
  return ctSegments;
}
