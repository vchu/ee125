#include "..\Socket\_Socket.h"
#include <process.h>
#include <string>
#include <iostream>

unsigned __stdcall Answer(void* a) {
  Socket* s = (Socket*) a;

  while (1) {
    std::string r = s->ReceiveLine();
    if (r.empty()) break;
	std::cout<<"Client sent:"<<r<<"\n";
	//blah
  
   // s->SendLine(r);
  }

  delete s;

  return 0;
}

int main(int argc, char* argv[]) {
   std::cout<< "Initializing server at port 2000";
  SocketServer in(2000,5);

  while (1) {
    Socket* s=in.Accept();

    unsigned ret;
    _beginthreadex(0,0,Answer,(void*) s,0,&ret);
  }
 
  return 0;
}
