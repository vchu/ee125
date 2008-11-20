#include "_MoteFncs.h"


/*! It is used for recursive update of the tree-structure when processing the
    Model package.
*/
void _ProcPkgModel1(xshort* buf, int& bufSz, MoteClass* mote, int& ct)
{
  if(ct%8)
    buf[bufSz] = (buf[bufSz]<<1);
  else
    bufSz++;

  ct++;
  if( buf[bufSz]>=(1<<7) )
  {
    mote->m_Active = 0;

    if(!mote->m_Child1)
      mote->m_Child1 = new MoteClass;
    if(!mote->m_Child2)
      mote->m_Child2 = new MoteClass;

    _ProcPkgModel1(buf,bufSz,mote->m_Child1,ct);
    _ProcPkgModel1(buf,bufSz,mote->m_Child2,ct);
  }
  else
  {
    mote->m_Active = 1;

    if(mote->m_Child1)
      delete mote->m_Child1;
    if(mote->m_Child2)
      delete mote->m_Child2;
  }

  return;
}


/*! It is used for recursive update of the birth-time when processing the
    Model Package.
*/
void _ProcPkgModel2(xshort* buf, int& bufSz, MoteClass* mote)
{
  if(mote->m_Active)
  {
    mote->m_BirthTime = char2int(buf+bufSz,4);
    bufSz+=4;
  }
  else
  {
    _ProcPkgModel2(buf,bufSz,mote->m_Child1);
    _ProcPkgModel2(buf,bufSz,mote->m_Child2);
  }

  return;
}


/*! It updates the mote structure by using the provide Model Package. It assumes
    the appropriate format for the package.

    \param buf Pointer to buffer containing Model Package.
    \param mote Pointer to the mote structure to be updated.
*/
void ProcPkgModel(xshort* buf, MoteClass* mote)
{
  int bufSz = 0;
  int ct = 1;

  // First we create the structure.
  if( buf[bufSz]>=(1<<7) )
  {
    mote->m_Active = 0;

    if(!mote->m_Child1)
      mote->m_Child1 = new MoteClass;
    if(!mote->m_Child2)
      mote->m_Child2 = new MoteClass;

    _ProcPkgModel1(buf,bufSz,mote->m_Child1,ct);
    _ProcPkgModel1(buf,bufSz,mote->m_Child2,ct);
  }
  else
  {
    mote->m_Active = 1;

    if(mote->m_Child1)
      delete mote->m_Child1;
    if(mote->m_Child2)
      delete mote->m_Child2;
  }
  bufSz++;


  // Now we recover the birth-times.
  if(mote->m_Active)
  {
    mote->m_BirthTime = char2int(buf+bufSz,4);
    bufSz+=4;
  }
  else
  {
    _ProcPkgModel2(buf,bufSz,mote->m_Child1);
    _ProcPkgModel2(buf,bufSz,mote->m_Child2);
  }

  return;
}


/*! It is used for recursive update of the line parameters when processing the
    Parameters Package.
*/
void _ProcPkgParameters(xshort* buf, int& bufSz, MoteClass* mote)
{
  if(!mote->m_Active)
  {
    mote->m_Line_x0 = int(char2int(buf+bufSz,1)*MC_IMAGEWIDTH/255.0); bufSz++;
    mote->m_Line_y0 = int(char2int(buf+bufSz,1)*MC_IMAGEHEIGHT/255.0); bufSz++;

    int dx = int(char2int(buf+bufSz,1) - 127.5); bufSz++;
    int dy = int(char2int(buf+bufSz,1) - 127.5); bufSz++;
    int norm = max(abs(dx),abs(dy));
    mote->m_Line_dx = (dx<<8)/norm;
    mote->m_Line_dy = (dy<<8)/norm;

    _ProcPkgParameters(buf,bufSz,mote->m_Child1);
    _ProcPkgParameters(buf,bufSz,mote->m_Child2);
  }

  return;
}


/*! It updates the bisecting line parameters for the tree structure. It assumes
    the provided package has the appropriate format.

    \param buf Pointer to buffer containing Model Package.
    \param mote Pointer to the mote structure to be updated.
*/
void ProcPkgParameters(xshort* buf, MoteClass* mote)
{
  int bufSz = 0;

  if(!mote->m_Active)
  {
    mote->m_Line_x0 = (char2int(buf+bufSz,1)*MC_IMAGEWIDTH)/255; bufSz++;
    mote->m_Line_y0 = (char2int(buf+bufSz,1)*MC_IMAGEHEIGHT)/255; bufSz++;

    int dx = int(char2int(buf+bufSz,1) - 127.5); bufSz++;
    int dy = int(char2int(buf+bufSz,1) - 127.5); bufSz++;
    int norm = max(abs(dx),abs(dy));
    mote->m_Line_dx = (dx<<8)/norm;
    mote->m_Line_dy = (dy<<8)/norm;

    _ProcPkgParameters(buf,bufSz,mote->m_Child1);
    _ProcPkgParameters(buf,bufSz,mote->m_Child2);
  }

  return;
}


/*! It determines the number of regions for which the observations are
    specified in the provided package. This function can be used to determine
    whether the number of observations received agree with the number of
    partitions in the tree structure.

    \param buf Pointer to the buffer containing Observations Package.
    \param bufSz Size of the buffer.
*/
int ProcPkgObs_noReg(xshort* buf, int bufSz)
{
  int noReg = 0;

  int idx = 0;
  while(bufSz>idx)
  {
    noReg++;
    idx += 3*buf[0]+2;
    buf += (3*buf[0]+2);
  }

  return noReg;
}


/*! It is used for recursive update the observations when processing the
    Observations Package.
*/
void _ProcPkgObservations(int nStart,xshort* buf,int bufSz, MoteClass* mote,
                         int& idxPos, int& idxMote)
{
  if(mote->m_Active)
  {
    idxMote++;

    if(idxMote>=nStart && bufSz>idxPos)
    {
      int noObs = buf[idxPos];
      idxPos++;

      // There are actual observations that we process them.
      if(noObs > 0)
      {
        // Keeps track of which state entry we are updating.
        int idx = mote->m_Det_StatePos+1;
        int ctObs = 1; // Counts the number of processed observations.

        // We obtain the initial state
        xshort curState = buf[idxPos]; idxPos++;
        int curTime = char2int(buf+idxPos,3); idxPos+=3;
        
        // We take care of hitting time 0, i.e. null time when observations
        // get started. We simply remove the observations reseting the state
        // to zero.
        while(ctObs<noObs && curTime==0)
        {
          curState = 1;
          curTime = char2int(buf+idxPos,3); idxPos+=3;

          ctObs++;
        }

        // We also take care of not including observations that are not in
        // increasing order of time.
        while( curTime < mote->m_Det_StateTime[mod(idx-2,MC_SIZE_STATEHISTORY)]
          && ctObs<noObs )
        {
          curState = !curState;
          curTime = char2int(buf+idxPos,3); idxPos+=3;

          ctObs++;
        }

        if(ctObs==noObs)
          curState = 0;

        // Now we assing our value to the first entry in the state arrays.
        mote->m_Det_StateHist[idx] = curState;
        mote->m_Det_StateTime[idx] = curTime;
        idx = mod(idx+1,MC_SIZE_STATEHISTORY);

        // We get all of the other states.
        while(ctObs<noObs)
        {
          curState = !curState;
          curTime = char2int(buf+idxPos,3); idxPos+=3;

          mote->m_Det_StateHist[idx] = curState;
          mote->m_Det_StateTime[idx] = curTime;
          idx = mod(idx+1,MC_SIZE_STATEHISTORY);

          ctObs++;
        }

        // Assigning current state.
        mote->m_Det_State = curState;

        // Now we add a set of zeros to mark the end of this sequence.
        mote->m_Det_StateHist[idx] = 0;
        mote->m_Det_StatePos = idx;

				// We take care of the case when we have repeated records.
				if(mote->m_Det_StateTime[mod(idx-1,MC_SIZE_STATEHISTORY)]==
					mote->m_Det_StateTime[mod(idx-3,MC_SIZE_STATEHISTORY)])
				{
					mote->m_Det_StateHist[idx-1] = 0;
					mote->m_Det_StateTime[idx-1] = 0;
					mote->m_Det_StatePos = idx-2;
					mote->m_Det_State = mote->m_Det_StateHist[idx-3];
				}
      }
    }
  }
  else
  {
    _ProcPkgObservations(nStart,buf,bufSz,mote->m_Child1,idxPos,idxMote);
    _ProcPkgObservations(nStart,buf,bufSz,mote->m_Child2,idxPos,idxMote);
  }

  return;
}


/*! It updates the observations for each region in the tree structure. It does
    it by getting rid of any observations that are not in increasing order, i.e.
    it does not allow for duplication in the time information.

    \param nStart Index of first region for which the data should be used.
    \param buf Pointer to buffer containing Observations Package.
    \param bufSz Size of the buffer.
    \param mote Pointer to the mote structure to be updated.
*/
void ProcPkgObservations(int nStart,xshort* buf,int bufSz,MoteClass* mote)
{
  int idxPos = 0; // Keeps track of position in buffer for data extraction.
  int idxMote = 0; // Keeps track of the mote that is being updated.

  if(mote->m_Active)
  {
    if(idxMote>=nStart && bufSz>idxPos)
    {
      // First we get the number of observations
      int noObs = buf[idxPos];
      idxPos++;

      // There are actual observations that we process them.
      if(noObs > 0)
      {
        // Keeps track of which state entry we are updating.
        int idx = mote->m_Det_StatePos+1;
        int ctObs = 1; // Counts the number of processed observations.

        // We obtain the initial state
        xshort curState = buf[idxPos]; idxPos++;
        int curTime = char2int(buf+idxPos,3); idxPos+=3;
        
        // We take care of hitting time 0, i.e. null time when observations
        // get started. We simply remove the observations reseting the state
        // to zero.
        while(ctObs<noObs && curTime==0)
        {
          curState = 1;
          curTime = char2int(buf+idxPos,3); idxPos+=3;

          ctObs++;
        }

        // We also take care of not including observations that are not in
        // increasing order of time.
        while( curTime < mote->m_Det_StateHist[mod(idx-2,MC_SIZE_STATEHISTORY)]
          && ctObs<noObs )
        {
          curState = !curState;
          curTime = char2int(buf+idxPos,3); idxPos+=3;

          ctObs++;
        }

        if(ctObs==noObs)
          curState = 0;

        // Now we assing our value to the first entry in the state arrays.
        mote->m_Det_StateHist[idx] = curState;
        mote->m_Det_StateTime[idx] = curTime;
        idx = mod(idx+1,MC_SIZE_STATEHISTORY);

        // We get all of the other states.
        while(ctObs<noObs)
        {
          curState = !curState;
          curTime = char2int(buf+idxPos,3); idxPos+=3;

          mote->m_Det_StateHist[idx] = curState;
          mote->m_Det_StateTime[idx] = curTime;
          idx = mod(idx+1,MC_SIZE_STATEHISTORY);

          ctObs++;
        }

        // Assigning current state.
        mote->m_Det_State = curState;

        // Now we add a set of zeros to mark the end of this sequence.
        mote->m_Det_StateHist[idx] = 0;
        mote->m_Det_StatePos = idx;

				// We take care of the case when we have repeated records.
				if(mote->m_Det_StateTime[mod(idx-1,MC_SIZE_STATEHISTORY)]==
					mote->m_Det_StateTime[mod(idx-3,MC_SIZE_STATEHISTORY)])
				{
					mote->m_Det_StateHist[idx-1] = 0;
					mote->m_Det_StateTime[idx-1] = 0;
					mote->m_Det_StatePos = idx-2;
					mote->m_Det_State = mote->m_Det_StateHist[idx-3];
				}
      }
    }
  }
  else
  {
    _ProcPkgObservations(nStart,buf,bufSz,mote->m_Child1,idxPos,idxMote);
    _ProcPkgObservations(nStart,buf,bufSz,mote->m_Child2,idxPos,idxMote);
  }

  return;
}


void _ProcPkgObs(int regID_Start, xshort* buf, int bufSz, MoteClass* mote,
                 int& bufPos, int& regID)
{
	regID++;

  if(mote->m_Active)
  {
    if(regID>=regID_Start && bufSz>bufPos)
    {
      // First we get the number of observations
      int noObs = buf[bufPos++];
			//printf("Number of Observations: %d | %d | %d %d %d %d %d\n",noObs,
			//	bufPos,buf[bufPos-2],buf[bufPos-1],buf[bufPos],buf[bufPos+1],
			//	buf[bufPos+2]);

			int bufPos_Original = bufPos-1;
			int noObs_Original = noObs;

      // There are actual observations that we process them.
      if(noObs > 0)
      {
        // Keeps track of which state entry we are updating.
        int statePos = mote->m_Det_StatePos;
        int lastTime = mote->m_Det_StateTime[statePos];

        // We obtain the initial state
        xshort curState = buf[bufPos++];
        int curTime = char2int(buf+bufPos,3); bufPos+=3;
        
        // REMOVING ZERO OBSERVATIONS. WE SET THE NEXT NON-ZERO OBSERVATION
        // TO A VISIBLE DETECTION STATE
        while( curTime==0 )
        {
          curState = 1;
          curTime = char2int(buf+bufPos,3); bufPos+=3;

          noObs--;
          if(noObs<=0)
					{
  					bufPos = bufPos_Original + 2 + 3*noObs_Original;
            return;
					}
        }

        // REMOVING PREVIOUS OBSERVATIONS.
        while( curTime < lastTime)
        {
          curState = !curState;
          curTime = char2int(buf+bufPos,3); bufPos+=3;

          noObs--;
          if(noObs<=0)
					{
						bufPos = bufPos_Original + 2 + 3*noObs_Original;
            return;
					}
        }

        // In this case we don't have continuity on the chains of observations.
        if( curTime!=lastTime )
        {
          statePos = mod(statePos+1,MC_SIZE_STATEHISTORY);
          mote->m_Det_StateHist[statePos] = 0;
          mote->m_Det_StateTime[statePos] = 0;
        }
				else
				{
          curState = !curState;
          curTime = char2int(buf+bufPos,3); bufPos+=3;

					noObs--;
				}

        // Now we continue with the rest of the values that are in the list.
        while(noObs>0)
        {
          statePos = mod(statePos+1,MC_SIZE_STATEHISTORY);
          mote->m_Det_StateHist[statePos] = curState;
          mote->m_Det_StateTime[statePos] = curTime;
          curState = !curState;
          curTime = char2int(buf+bufPos,3); bufPos+=3;

					noObs--;
        }
				mote->m_Det_StatePos = statePos;
				bufPos = bufPos_Original + 2 + 3*noObs_Original;
				//printf("%d %d %d\n",bufPos_Original,noObs_Original,bufPos);
      }
    }
  }
  else
  {
    _ProcPkgObs(regID_Start,buf,bufSz,mote->m_Child1,bufPos,regID);
    _ProcPkgObs(regID_Start,buf,bufSz,mote->m_Child2,bufPos,regID);
  }

  return;
}


void ProcPkgObs(int regID_Start, xshort* buf, int bufSz, MoteClass* mote)
{
  int bufPos = 0; // Keeps track of position in buffer for data extraction.
  int regID = 0; // Keeps track of the ID of the region been updated

  if(mote->m_Active)
  {
    if(regID>=regID_Start && bufSz>bufPos)
    {
      // First we get the number of observations
      int noObs = buf[bufPos++];

      // There are actual observations that we process them.
      if(noObs > 0)
      {
        // Keeps track of which state entry we are updating.
        int statePos = mote->m_Det_StatePos;
        int lastTime = mote->m_Det_StateTime[statePos];

        // We obtain the initial state
        xshort curState = buf[bufPos++];
        int curTime = char2int(buf+bufPos,3); bufPos+=3;
        
        // REMOVING ZERO OBSERVATIONS. WE SET THE NEXT NON-ZERO OBSERVATION
        // TO A VISIBLE DETECTION STATE
        while( curTime==0 )
        {
          curState = 1;
          curTime = char2int(buf+bufPos,3); bufPos+=3;

          noObs--;
          if(noObs<=0)
            return;
        }

        // REMOVING PREVIOUS OBSERVATIONS.
        while( curTime < lastTime)
        {
          curState = !curState;
          curTime = char2int(buf+bufPos,3); bufPos+=3;

          noObs--;
          if(noObs<=0)
            return;
        }

        // In this case we don't have continuity on the chains of observations.
        if( curTime!=lastTime )
        {
          statePos = mod(statePos+1,MC_SIZE_STATEHISTORY);
          mote->m_Det_StateHist[statePos] = 0;
          mote->m_Det_StateTime[statePos] = 0;
        }
				else
				{
          curState = !curState;
          curTime = char2int(buf+bufPos,3); bufPos+=3;

					noObs--;
				}

        // Now we continue with the rest of the values that are in the list.
        while(noObs>0)
        {
          statePos = mod(statePos+1,MC_SIZE_STATEHISTORY);
          mote->m_Det_StateHist[statePos] = curState;
          mote->m_Det_StateTime[statePos] = curTime;
          curState = !curState;
          curTime = char2int(buf+bufPos,3); bufPos+=3;

					noObs--;
        }
				mote->m_Det_StatePos = statePos;
      }
    }
  }
  else
  {
		//printf("SPLITTING Obs ...\n");
    _ProcPkgObs(regID_Start,buf,bufSz,mote->m_Child1,bufPos,regID);
    _ProcPkgObs(regID_Start,buf,bufSz,mote->m_Child2,bufPos,regID);
  }

  return;
}


/*! This function returns the state at a specified time for a given mote. In
    order to determine the state, the values stores in the state history array
    are used. The function returns (-1) to indicate that no much was found for
    the specified time. Otherwise, a value of 0 or 1 is returned.

    \param time Time corresponding to the state requested.
    \param mote Pointer to the bisecting tree structure.
*/
int GetState(int time, MoteClass* mote)
{
  // If the time requested is before the birth time of the mote, then there is
  // no point on proceeding with the request.
  if(time<=mote->m_BirthTime)
    return 0;

  // We take the time at the end of the state-history array. We take care of
  // this case separately since we are considering a circular buffer.
  int t0 = mote->m_Det_StateTime[MC_SIZE_STATEHISTORY-1];
  int t1 = mote->m_Det_StateTime[0];

  // We check that the time is within the interval and both times are greater
  // than zero.
  if(t0<=time && time<t1 && t0>0 && t1>0)
  {
    // If this condition is satisfied, then the state correspond to the state
    // specified at the lower time.
		if(mote->m_Det_StateHist[MC_SIZE_STATEHISTORY-1])
		{
			// We return a value of 2 if there is a 3-time slot overlap.
			if(t0<=time && time<t1-3)
				return 2;
			else
				return 1;
		}
		else
			return 0;
  }
  else
  {
    // We repeat the previous process over all other entries of the state
    // history.
    for(int i = 1; i<MC_SIZE_STATEHISTORY; i++)
    {
      t0 = mote->m_Det_StateTime[i-1];
      t1 = mote->m_Det_StateTime[i];
      if(t0<=time && time<t1 && t0>0 && t1>0)
			{
				if(mote->m_Det_StateHist[i-1])
				{
					if(t0<=time && time<t1-3)
						return 2;
					else
						return 1;
				}
				else
					return 0;
			}
    }
  }

  // If no interval is found, then we return -1.
  return 0;
}


/*! It is used to compute simplices from the provided mote structure. This
    function is meant to be called recursively and iteratively for all time.
    It returns '1' if a change in the states is detected from a previous
    iteration.
*/
int _GetSimplex(int time, MoteClass* mote, int* offset, int* moteList,
                int* moteListSz)
{
  // This flagged is used to determine if changes have been made to the
  // states. We are looking for changes in which the target become visible
  // in a new region.
  int flagChange = 0;
  xshort state; // Use to store the state at the given time.
  
  if(mote->m_Active)
  {
    (*offset)++;

    // If we request a time before birth time there is no point on continuing
    // computations.
    if(time<mote->m_BirthTime)
      return 0;

    // If mote is active we look at the current state and compare it to the 
    // state stored in the mote. We update the state in the mote and change the
    // flag if an "appearence" event is detected.
    state = (xshort)GetState(time,mote);
    if((state>0)!=mote->m_Det_State)
    {
      mote->m_Det_State = (state>0);
      if(state>0)
        flagChange = 1;
    }

    // If the state is 1, we add it to the list of motes that are detecting
    // something.
    if(state==2)
    {
      moteList[(*moteListSz)] = (*offset);
      (*moteListSz)++;
    }
  }
  else
  {
    // If not active, we call recursively over the children.
    flagChange += _GetSimplex(time,mote->m_Child1,offset,moteList,moteListSz);
    flagChange += _GetSimplex(time,mote->m_Child2,offset,moteList,moteListSz);
  }

  // We are just interested in returning 1 or 0.
  return (flagChange>0);
}


/*! It builds a simplex from the provided mote structure considering all active
    regions in the mote.
*/
void _GetMoteSimplex(MoteClass* mote, int* offset, int* moteList,
                    int* moteListSz)
{
  if(mote->m_Active)
  {
    (*offset)++;

    moteList[(*moteListSz)] = (*offset);
    (*moteListSz)++;
  }
  else
  {
    _GetMoteSimplex(mote->m_Child1,offset,moteList,moteListSz);
    _GetMoteSimplex(mote->m_Child2,offset,moteList,moteListSz);
  }

  return;
}


/*! It writes the complex. If a file name is specified then the complex is
    written to that file, otherwise the complex is displayed in the screen.

    \param curTime It is the final time to be used when computing the simplices
      over the state history.
    \param mote Pointer to mote array. These are the motes to be used for
      computing the complex.
    \param noMotes Number of motes in the array.
    \param fileName File to which the complex will be written. If not provided,
      the simplices will be printed in the screen.
*/
void WriteComplex(int curTime, MoteClass* mote, int noMotes, char* fileName)
{
  int offset = 0; // Offset used for counting nodes.
  int moteList[100]; // Array used for storing motes.
  int moteListSz = 0; // Size of the complex.

  // If file name is not provided we print to the screen.
  if(fileName==0)
  {
    // First we print the local mote structure.
    for(int n = 0; n<noMotes; n++)
    {
      moteListSz = 0;
      _GetMoteSimplex(mote+n,&offset,moteList,&moteListSz);

      printf("t = %04d : [ ",0);
      for(int i = 0; i<moteListSz; i++)
        printf("%d ",moteList[i]);
      printf("]\n");
    }

    // Now we built simplices based on time observations
    for(int t = 0; t<=curTime; t++)
    {
      // Resetting offset and size of simplex.
      offset = 0;
      moteListSz = 0;

      // We use this variable to determine if any changes have been detected
      // in the states. We only care about events that are "appearence".
      int flagUpdate = 0;

      // Computing simplex and determining if an update has been done.
      for(int n = 0; n<noMotes; n++)
        flagUpdate += _GetSimplex(t,mote+n,&offset,moteList,&moteListSz);

      // If simplex has been updated then we print.
      if(flagUpdate)
      {
        printf("t = %04d : [ ",t);
        for(int i = 0; i<moteListSz; i++)
          printf("%d ",moteList[i]);
        printf("]\n");
      }
    }
  }
  else
  {
    ofstream out;
    out.open(fileName);

    for(int n = 0; n<noMotes; n++)
    {
      moteListSz = 0;
      _GetMoteSimplex(mote+n,&offset,moteList,&moteListSz);

      for(int i = 0; i<moteListSz; i++)
        out<<moteList[i]<<" ";
      out<<endl;
    }

    for(int t = 0; t<=curTime; t++)
    {
      offset = 0;
      moteListSz = 0;

      int flagUpdate = 0;
      for(int n = 0; n<noMotes; n++)
        flagUpdate += _GetSimplex(t,mote+n,&offset,moteList,&moteListSz);

      if(flagUpdate)
      {
        for(int i = 0; i<moteListSz; i++)
          out<<moteList[i]<<" ";
        out<<endl;
      }
    }
  }

  return;
}


int GetCurrTime(MoteClass* mote)
{
	int val = 0;

	if(mote->m_Active)
		val = mote->m_Det_StateTime[mote->m_Det_StatePos];
	else
		val = max(GetCurrTime(mote->m_Child1),GetCurrTime(mote->m_Child2));

	return val;
}


int GetCntRegions(MoteClass* mote)
{
	int cnt = 0;
	if(mote->m_Active)
		cnt = 1;
	else
	{
		cnt += GetCntRegions(mote->m_Child1);
		cnt += GetCntRegions(mote->m_Child2);
	}

	return cnt;
}