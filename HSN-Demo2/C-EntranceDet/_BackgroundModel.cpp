#include "_BackgroundModel.h"
#include <highgui.h>

/*! Here we generate the background model. The model is generated from multiple
    images extracted from the provided file-name pattern. The file-name must be
    in a format that can be interpreted by "printf" in C after evaluation of a
    frame number. A simpel example is: "C:/image%04d.jpg". This function creates
    a mean background image from a number of frames. The gradient and edge mask
    for this image are also computed.

    \param filePattern Pattern for the names of files corresponding to a
      sequence of images.
*/
void BackgroundModel::CreateModel(char* filePattern)
{
  int thGrad = 400; // ... Threshold for Edge Detection

  IplImage* im;
  char imName[1000];

  // Getting Dimensions of the images
  sprintf(imName,filePattern,1);
  im = cvLoadImage(imName,0);
  m_width = im->width;
  m_height = im->height;
  cvReleaseImage(&im);

  // Creating necessary arrays
  m_imBG = new int[m_width*m_height];
  memset(m_imBG,0,m_width*m_height*sizeof(int));
  m_imDx = new int[m_width*m_height];
  memset(m_imDx,0,m_width*m_height*sizeof(int));
  m_imDy = new int[m_width*m_height];
  memset(m_imDy,0,m_width*m_height*sizeof(int));
  m_imEdge = new xshort[m_width*m_height];
  memset(m_imEdge,0,m_width*m_height*sizeof(xshort));

  // Obtaining Mean Background Image
  const int NImages = 14; // Number of Images to process.
  for(int n = 2; n<=NImages+1; n++)
  {
    sprintf(imName,filePattern,n);
    im = cvLoadImage(imName,0);
    
    unsigned char* data = (unsigned char*)(im->imageData);
    for(int i = 0; i<m_height; i++)
      for(int j = 0; j<m_width; j++)
        m_imBG[i*m_width+j] += data[i*im->widthStep+j];

    cvReleaseImage(&im);
  }
  for(int i = 0; i<(m_height*m_width); i++)
      m_imBG[i] /= NImages;

  // Computing Vertical Gradient
  for(int i = 1; i<(m_height-1); i++)
    for(int j = 0; j<m_width; j++)
      m_imDy[i*m_width+j] = m_imBG[(i+1)*m_width+j]-m_imBG[(i-1)*m_width+j];
  for(int i = 1; i<(m_height-1); i++)
    for(int j = 1; j<(m_width-1); j++)
      m_imDy[i*m_width+j] = (2*m_imDy[i*m_width+j] + m_imDy[i*m_width+j+1]
        + m_imDy[i*m_width+j-1]) >> 3;

  // Computing Horizontal Gradient
  for(int i = 0; i<m_height; i++)
    for(int j = 1; j<(m_width-1); j++)
      m_imDx[i*m_width+j] = m_imBG[i*m_width+j+1]-m_imBG[i*m_width+j-1];
  for(int i = 1; i<(m_height-1); i++)
    for(int j = 1; j<(m_width-1); j++)
      m_imDx[i*m_width+j] = (2*m_imDx[i*m_width+j] + m_imDx[(i+1)*m_width+j]
        + m_imDx[(i-1)*m_width+j]) >> 3;

  // Computing Edge Mask
  for(int i = 0; i<(m_height*m_width); i++)
    if(m_imDx[i]*m_imDx[i]+m_imDy[i]*m_imDy[i]>thGrad)
      m_imEdge[i] = 255;

  return;
}


/*! Deleting all dynamic arrays.
*/
BackgroundModel::~BackgroundModel()
{
  if(m_imBG){ delete m_imBG; }
  if(m_imDx){ delete m_imDx; }
  if(m_imDy){ delete m_imDy; }
  if(m_imEdge){ delete m_imEdge; }
}


/*! Function computing foreground masks. It takes a current image and returns
    a '1' if there is a foreground object in the image or '0' otherwise.
    
    \param Dst Pointer to the foreground mask to be returned. A value of 255
      is assigned at the pixels where a detection is made.
    \param im Input image to be used for detection of foreground.
*/
int BackgroundModel::GetFGMask(IplImage *Dst, IplImage *im)
{
  int thDiff = 20; // ... Threshold for grayscale difference

  unsigned char* dataIm  = (unsigned char*)(im->imageData);
  unsigned char* dataDst = (unsigned char*)(Dst->imageData);

  // Performing Simple Background/Image thresholding
  for(int i = 0; i<m_height; i++)
    for(int j = 0; j<m_width; j++)
    {
      if(abs(m_imBG[i*m_width+j]-(int)dataIm[i*im->widthStep+j])>thDiff)
        dataDst[i*Dst->widthStep+j] = 255;
      else
        dataDst[i*Dst->widthStep+j] = 0;
    }

  // Applying a median filter to get rid of noise
  cvSmooth(Dst,Dst,CV_MEDIAN,5);

  // Determining if there is anything visible in the region.
  int sum = 0;
  for(int i = 0; i<m_height; i++)
    for(int j = 0; j<m_width; j++)
      sum += dataDst[i*Dst->widthStep+j];

  return (sum>0);
}
