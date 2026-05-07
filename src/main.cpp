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
   raw = DFAFileReader(path); //masukan seluruh code ke 1 line string

   vector<Token> tokens = Tokenizing(raw); //tokenisasi ke tokens

   // filter token komentar biar parser ga perlu pusing
   vector<Token> filtered;
   for (const auto& t : tokens) {
      if (t.type != "comment") filtered.push_back(t);
   }
   tokens.swap(filtered);

   TokenPrinter(tokens); //output ke CLI dlu

   //file output process
   char op;
   cout << "Export to file ? [N/y]\n";
   cin >> op;
   bool restart = true;
   while (restart){
      if(op == 'Y'||op == 'y'){
         while (restart){
            cout << "Input File Name : ";
            cin >> fileName;
            path = "test/M1/" + fileName;
            if(!IsFileExist(path)){ //file harus tidak exist baru bisa dilanjutkan 
               PrintTokenToFile(path,tokens);   
               restart = false; //tidak perlu restart
            }
            else{ //tidak ingin override, berarti harus ulang input file name
               restart = true; //input ulang file name
            }
         }
      }
      else if(op == 'N'||op=='n'){
         restart = false; //end of program
      }
      else{
         cout << "[ERROR] Unknown Options !!\n"; //restart masih true, akan minta keputusan export file
      }
   }

   // ---------------- Syntax Analysis (Recursive Descent) ----------------
   cout << "\n=== Parse Tree ===\n";
   Parser parser(tokens);
   NodePtr root = parser.parseProgram();
   if (root) printTree(root);
   if (parser.hasError()) {
      cout << "\n[INFO] Parsing finished with " << parser.getErrors().size()
           << " syntax error(s).\n";
   } else {
      cout << "\n[INFO] Parsing finished successfully.\n";
   }

   cout << "Export parse tree to file ? [N/y]\n";
   char op2;
   cin >> op2;
   if (op2 == 'Y' || op2 == 'y') {
      bool retry = true;
      while (retry) {
         cout << "Input File Name : ";
         cin >> fileName;
         path = "test/M1/" + fileName;
         if (!IsFileExist(path)) {
            saveTreeToFile(root, path);
            retry = false;
         }
      }
   }
}