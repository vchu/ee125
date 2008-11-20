#include "..\..\HSN-Demo2\C-Server\_Socket.h"
#include <process.h>
#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <cstdlib>
#include <direct.h>
using namespace std;
queue<char *> dataQueue;

bool PathExists(char* pathtocheck)
{
int ret = _chdir(pathtocheck) ;
return (ret == 0) ;
}

unsigned __stdcall Answer(void* a) {
    Socket* s = (Socket*) a;
	unsigned char r[1024] ;
	int len=0;
  while (1) {
  
	s->ReceiveBytes(len, r);
	printf("\n Len=%d\n",len);
	
    if (len==0 || len==-1) break;
	std::cout<<"\nClient sent:";
	char* temp = (char *) malloc(len);
	for (int i = 0 ; i<len; i++){
		printf("%d ",r[i]);
		temp[i] = r[i];
	}
	dataQueue.push(temp);
	char* parseData = dataQueue.front();
	int ID = parseData[0];
	char* IDstr= (char *) malloc(4);
	char* folderpath= (char *) malloc(256);
	sprintf(IDstr, "%d", ID);
	sprintf(folderpath, "C:\\%s", IDstr);
	
	if(!PathExists(folderpath)){
		mkdir(folderpath);
	}
	char * temp1 = (char *) malloc(256);
	sprintf(temp1, "%d", parseData[1]);
	char * filename = (char *) malloc(256);
	sprintf(filename, "%s\\%s", folderpath, temp1);
	printf("\n filename is: %s", filename);
	std::ofstream myFile (filename, ios::out | ios::binary);
	std::string blah= parseData;
    myFile.write (parseData, len);
	myFile.close();

	//for (int j=0; j< len; j++){
	//	printf("%d ", test[j]);
	//}
	dataQueue.pop();
	delete parseData;
  }

  delete s;
  

  return 0;
}

int main(int argc, char* argv[]) {
   std::cout<< "Initializing server at port 2000";
  SocketServer in(2000,5);

  while (1) {
	  printf("HeLLo thread");
    Socket* s=in.Accept();

    unsigned ret;
	printf("Good bye thread"); 
    _beginthreadex(0,0,Answer,(void*) s,0,&ret);
    

  }
 
  return 0;
}
