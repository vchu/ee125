#include "_FileCapture.h"


/*! Initializing values for members in the class.
*/
FileCapture::FileCapture()
{
    m_noFrames = 0;
    m_noViews = 0;
    m_filePattern = 0;
}


/*! Deleting generated dynamic arrays.
*/
FileCapture::~FileCapture()
{
  for(int i = 0; i<m_noViews; i++)
    delete m_filePattern[i];
  delete m_filePattern;
}


/*! Setting number of frames.
    \param val Value to be assigned.
*/
void FileCapture::setNoFrames(int val)
{
  m_noFrames = val;
}


/*! Setting number of views. This function automatically generates dynamic
    arrays based on the provided number of views.
    \param val Value to be assigned
*/
void FileCapture::setNoViews(int val)
{
  // Deleting any previously generated arrays.
  for(int i = 0; i<m_noViews; i++)
    delete m_filePattern[i];
  delete m_filePattern;

  // Generating new dynamic arrays.
  m_noViews = val;
  m_filePattern = new char*[m_noViews];
  for(int i = 0; i<m_noViews; i++)
    m_filePattern[i] = new char[100];
}


/*! This function reads parameters from the provided folder and generates
    file-name patterns that can be used for reading images from the
    sequences.
    \param folderName Specifies the folder where the sequence of images is
      found.
*/
void FileCapture::InitCapture(char folderName[])
{
  char dummy[2000];
  int tmp;

  sprintf(dummy,"%s_readme.txt",folderName);
  ifstream in;
  in.open(dummy);

  // Reading number of frames
  in>>tmp;
  setNoFrames(tmp);
  in.getline(dummy,100);
  
  // Reading number of views
  in>>tmp;
  setNoViews(tmp);
  in.getline(dummy,100);

  // Reading patterns from file
  for(int i = 0; i<getNoViews(); i++)
  {
    in.get(dummy,100,' ');
    sprintf(m_filePattern[i],"%s%s",folderName,dummy);
    in.getline(dummy,100);
  }

  in.close();
  return;
}
