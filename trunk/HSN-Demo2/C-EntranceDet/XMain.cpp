#include <iostream>
#include <fstream>
#include <cv.h>
#include <highgui.h>
#include "_Global.h"
#include "_FileCapture.h"
#include "_BackgroundModel.h"
#include "_Funcs.h"
#include "_MoteClass.h"
#include "..\..\HSN-Demo2\C-Server\_Socket.h"
//#include "..\..\BIN\C-EntranceDet\Network.h"

using namespace std;

int main()
{
  // Initialize Socket Port
  SocketClient s("192.168.1.2", 2000);  

  // Initializing File Capture
  FileCapture FileData;
  FileData.InitCapture("C:/Pictures/Seq_Car01/");

  // Creating Background Model
  BackgroundModel *BG = new BackgroundModel[FileData.getNoViews()];
  for(int n = 0; n<FileData.getNoViews(); n++)
    BG[n].CreateModel(FileData.m_filePattern[n]);

  // Reading first frame to get corresponding sizes of images and then
  // create other necessary arrays.
  IplImage* im = 0;
  char imName[1000];
  sprintf(imName,FileData.m_filePattern[0],1);
  im = cvLoadImage(imName,0);

  // Generating foreground masks.
  IplImage** Mask  = new IplImagep[FileData.getNoViews()];
  IplImagep* Mask0 = new IplImagep[FileData.getNoViews()];
  for(int n = 0; n<FileData.getNoViews(); n++)
  {
    Mask0[n] = cvCreateImage(cvSize(im->width,im->height),8,1);
    memset(Mask0[n]->imageData,0,Mask0[n]->height*Mask0[n]->widthStep);
  }
  IplImagep* Mask1 = new IplImagep[FileData.getNoViews()];
  for(int n = 0; n<FileData.getNoViews(); n++)
  {
    Mask1[n] = cvCreateImage(cvSize(im->width,im->height),8,1);
  }
  IplImagep Detection = cvCreateImage(cvSize(im->width,im->height),8,1);

  // Creating mote structure.
  MoteClass* mote = new MoteClass[FileData.getNoViews()];
  for(int n = 0; n<FileData.getNoViews(); n++)
    mote[n].SetNoDetection();

  // Opening some windows for display.
  for(int n = 0; n < FileData.getNoViews(); n++)
  {
    sprintf(imName,"Image%02d",n); cvNamedWindow(imName,1);
    sprintf(imName,"FGMask%02d",n); cvNamedWindow(imName,1);
    sprintf(imName,"Detection%02d",n); cvNamedWindow(imName,1);
  }

  // Coefficient for line fitting 'x = (dx/dy)*(y - y0) + x0'.
  int x0, y0, dx, dy;

  // Arrays for the observations packet.
  int segPtrSz[MC_MAX_REGIONS];
  xshort* segPtr[MC_MAX_REGIONS];
  segPtr[0] = new xshort[MC_MAX_REGIONS*200];
  for(int i = 1; i < MC_MAX_REGIONS; i++)
    segPtr[i] = segPtr[i-1]+200;

  // Variables used for occlusion detection
  int* boolDetect0 = new int[FileData.getNoViews()];
  int* boolDetect1 = new int[FileData.getNoViews()];
  for(int n = 0; n<FileData.getNoViews(); n++)
  {
    boolDetect0[n] = 0;
    boolDetect1[n] = 0;
  }
  int delay; // Delay used for stopping when occlusions are detected.

  // Now we iterate over all frames.
  cvWaitKey(0);
  for(int i = 5; i<=FileData.getNoFrames(); i++)
  {
    delay = 10;

    for(int n = 0; n < FileData.getNoViews(); n++)
    {
      memset(Detection->imageData,0,Detection->height*Detection->widthStep);

      // Loading Image
      sprintf(imName,FileData.m_filePattern[n],i);
      printf("%s\n",imName);
      im = cvLoadImage(imName,0);

      // Getting Foreground Mask. Return 1 if there was a detection.
      boolDetect1[n] = BG[n].GetFGMask(Mask1[n],im);

      // Updating states in mote
      if(boolDetect1[n])
        mote[n].SetState((xshort*)Mask1[n]->imageData,Mask1[n]->widthStep,
          Mask1[n]->width,Mask1[n]->height,i);
      else
        mote[n].SetNoDetection(i);
      printf("  [-] STATUS: ");
      PrintState(&(mote[n]));
      printf("\n");
      
      // If there was a change on detection states then there was an occlusion.
      if(boolDetect0[n]!=boolDetect1[n])
      {
        // Displaying type of occlusion. We also determine if we want to use
        // the current foreground mask or the one from the previous time step.
        if(boolDetect0[n]==0)
        {
          printf("  [-] Appear Event\n");
          Mask[n] = Mask1[n];
        }
        else
        {
          printf("  [-] Disappear Event\n");
          Mask[n] = Mask0[n];
        }

        // Obtaining line fitting for occlusion. We display results if fit is
        // returned.
        if(FindBisection((xshort*)Detection->imageData,Detection->widthStep,
          &x0,&y0,&dx,&dy,&(BG[n]),(xshort*)Mask[n]->imageData))
        {
          // Bisecting the field of view of the camera
          mote[n].Bisect(x0,y0,dx,dy,i,(xshort*)Mask[n]->imageData,
            Mask[n]->widthStep,Mask[n]->width,Mask[n]->height);

          // Displaying results
          unsigned char* dataDet = (unsigned char*)(Detection->imageData);
          for(int i = 0; i<Detection->height; i++)
          {
            int idx = (dx*(i-y0))/dy + x0;
            if( idx>= 0 && idx<Detection->width &&
              dataDet[idx + i*Detection->widthStep]==0)
              dataDet[idx + i*Detection->widthStep] = 100;
          }
          float beta1 = ((float)dx)/dy;
          float beta0 = x0 - beta1*y0;
          printf("  [-] Found Line: x = %f y + %f\n",beta1,beta0);
          printf("  [-] Lines Found so far:\n");
          PrintLine(&(mote[n]));
        }

        // We want to pause if there was an occlusion detection.
        delay = min(delay,0);
      }
      else
      {
        // If no detection, then we will display current mask and have a short
        // delay for visualization purposes.
        Mask[n] = Mask1[n];
      }

      // Displaying some results.
      sprintf(imName,"Image%02d",n); cvShowImage(imName,im);
      sprintf(imName,"FGMask%02d",n); cvShowImage(imName,Mask[n]);
      sprintf(imName,"Detection%02d",n); cvShowImage(imName,Detection);

      // Swapping some values (New values become Old ones)
      boolDetect0[n] = boolDetect1[n];
      Mask[n] = Mask1[n];
      Mask1[n] = Mask0[n];
      Mask0[n] = Mask[n];

      cvReleaseImage(&im);
    }

    cvWaitKey(delay);
  }

  for(int n = 0; n < FileData.getNoViews(); n++)
  {
    printf("\n\n--------------------------------------- VIEW %02d\n",n+1);
    printf("  [-] Lines Found so far:\n");
    PrintLine(&(mote[n]));

    ofstream out;

    // Obtaining and Displaying Model Package.
    xshort buf[1000];
    int bufSz = GetPkgModel(buf,&(mote[n]));
    printf("\nMODEL PACKAGE: \n    ");
    sprintf(imName,"../../Pkg_Model_%02d.dta",n);
    out.open(imName,ios::binary);
    for(int i=0; i<bufSz; i++)
    {
      printf("%d ",(int)buf[i]);
    }
    out.write((char*)buf,bufSz);
    out.close();
    printf("\n");

    // Obtaining and Displaying Parameters Package.
    bufSz = GetPkgParameters(buf,&(mote[n]));
    printf("\nPARAMETERS PACKAGE: \n    ");
    sprintf(imName,"../../Pkg_Parameters_%02d.dta",n);
    out.open(imName,ios::binary);
    for(int i=0; i<bufSz; i++)
    {
      printf("%d ",(int)buf[i]);
    }
    out.write((char*)buf,bufSz);
    out.close();
    printf("\n");

    // Obtaining and Displaying Observations Package.
    buf[0] = 0; // Because observations start at mote 0.
    bufSz = GetPkgObservations(buf+1,&(mote[n]),30)+1;
    printf("\nOBSERVATIONS PACKAGE: \n    ");
    sprintf(imName,"../../Pkg_Observations_%02d.dta",n);
    out.open(imName,ios::binary);
    for(int i=0; i<bufSz; i++)
    {
      printf("%d ",(int)buf[i]);
    }
	s.sendTo(4,(const char *)buf,bufSz);
    out.write((char*)buf,bufSz);
    out.close();
    printf("\n");

    // Obtaining split Obsevations packet.
    int noSeg = SplitPkgObservations(segPtr,segPtrSz,buf+1,bufSz-1,100);
    for(int j = 0; j<noSeg; j++)
    {
      printf("Segment - %d\n",j+1);
      for(int i = 0; i<segPtrSz[j]; i++)
        printf("%d ",(int)segPtr[j][i]);
      printf("\n");
    }

    printf("\nSTATE HISTORY:\n");
    PrintStateHistory(&(mote[n]));
  }

  // Releasing Images and Windows
  cvWaitKey(0);

  delete BG;
  delete segPtr[0];
  for(int n = 0; n<FileData.getNoViews(); n++)
  {
    sprintf(imName,"Image%02d",n); cvDestroyWindow(imName);
    sprintf(imName,"FGMask%02d",n); cvDestroyWindow(imName);
    sprintf(imName,"Detection%02d",n); cvDestroyWindow(imName);
    cvReleaseImage(Mask0+n);
    cvReleaseImage(Mask1+n);
  }
  delete Mask0;
  delete Mask1;
  delete Mask;
  cvReleaseImage(&Detection);
  return 0;
}
