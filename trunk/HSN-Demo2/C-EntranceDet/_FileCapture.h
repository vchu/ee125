#ifndef FILECAPTURE_H
#define FILECAPTURE_H


#include <iostream>
#include <fstream>
using namespace std;

/*! Class containing parameters required for retreaving images from multiple
    views in a sequence over time. This class is intended as a simple
    structure containing parameters used for image retreavel. In particular,
    file-name patterns are stored in an array such that they can be
    evaluated by functions such as "printf" in C. A sample file-name pattern
    is: "C:/image%04d.jpg".
*/
class FileCapture
{
public:
  char** m_filePattern; //!< Array containing the file-name patterns.

  FileCapture();
  ~FileCapture();

  int getNoViews(){ return m_noViews; }; //!< Returns number of views.
  int getNoFrames(){ return m_noFrames; }; //!< Returns number of frames/

  void InitCapture(char folderName[]);
private:
  void setNoFrames(int val);
  void setNoViews(int val);

  int m_noFrames; //!< Number of frames in the sequence.
  int m_noViews;  //!< Number of views in the sequence.
};


#endif