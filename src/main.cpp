#include "main.hpp"


int main(int argc, char* argv[]) {

   //Error handling kalau parameter run < 1
   if (argc != 2){
      cerr << "Usage : make run \"File Name\" \n";
      return 1;
   }
   cout << DFAFileReader(argv[1]) << endl;
}