#include "_Socket.h"

#include <iostream>

using namespace std;

int main() {
 int blah;
  try {
    SocketClient s("127.0.0.1", 2000);

    

    while (1) {
      string l = s.ReceiveLine();
      if (l.empty()) break;
      cout << l;
      cout.flush();
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
cin >> blah;
  return 0;
}