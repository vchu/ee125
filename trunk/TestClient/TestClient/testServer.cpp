/* 
   EchoServer.cpp

   Copyright (C) 2002-2004 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch
*/


#include "_Socket.h"
#include <process.h>
#include <string>
#include <iostream>
#include <list>
typedef std::list<Socket*> socket_list;
socket_list g_connections;
unsigned __stdcall Answer(void* a) {
  Socket* s = (Socket*) a;
 g_connections.push_back(s);
 s->SendLine("Welcome to EECS125 Server test:");
 std::cout<< "So far, there are :"<< g_connections.size() << "Connections";

  while (1) {
    std::string r = s->ReceiveLine();
    if (r.empty()) break;
	std::cout<<r;
    s->SendLine(r);
  }
 g_connections.remove(s);

  delete s;

  return 0;
}

int main(int argc, char* argv[]) {
  SocketServer in(2000,5);
  std::cout << argc<<"\n";
  std::cout<< argv[0] << "\n";

  while (1) {
    Socket* s=in.Accept();

    unsigned ret;
	 //Answer((void*)s);
     _beginthreadex(0,0,Answer,(void*) s,0,&ret);
  }
 
  return 0;
}