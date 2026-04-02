#include "main.hpp"


int main() {
   //file name input 
   string path, fileName;
   cout << "[NOTES] : Just use filename without path\n";
   cout << "Enter file path: ";
   cin >> fileName;
   path = "test/M1/" + fileName;

   //file processing
   string raw;
   raw = DFAFileReader(path);
   vector<Token> tokens = Tokenizing(raw);
   TokenPrinter(tokens);
   PrintTokenToFile("test/M1/output.txt",tokens);
}