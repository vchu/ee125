#include "..\Socket\_Socket.h"

#include <iostream>

using namespace std;

int main() {

  try {
    SocketClient s("localhost", 2000);
	     
    while (1) {
      string l ;//= s.ReceiveLine();
	  std::cin>>l;
	  s.SendLine(l);
     // if (l.empty()) break;
      //cout << l;
     // cout.flush();
    }

  } 
  catch (const char* s) {
    cerr << s << endl;
  } 
  catch (std::string s) {
    cerr << s << endl;
  } 
  catch (...) {
    cerr << "unhandled exception\n";
  }
  int blah;
  std::cin>> blah;
  return 0;
}
