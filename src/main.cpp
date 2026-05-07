#include "main.hpp"


int main() {
   // Choose input mode
   cout << "[1] Source code file (run lexer + parser)\n";
   cout << "[2] Token file       (skip lexer, run parser only)\n";
   int mode = 0;
   while (mode != 1 && mode != 2) {
      cout << "Choose input mode: ";
      cin >> mode;
      if (mode != 1 && mode != 2) {
         cout << "[ERROR] Unknown Options !!\n";
         cin.clear();
         cin.ignore(10000, '\n');
      }
   }

   string path, fileName;
   cout << "Enter file path: ";
   cin >> fileName;
   path = "test/" + fileName;

   vector<Token> tokens;

   if (mode == 1) {
      // Source-code mode: read raw -> tokenize -> print -> optional export
      string raw = DFAFileReader(path);
      tokens = Tokenizing(raw);

      // filter token komentar biar parser ga perlu pusing
      vector<Token> filtered;
      for (const auto& t : tokens) {
         if (t.type != "comment") filtered.push_back(t);
      }
      tokens.swap(filtered);

      TokenPrinter(tokens);

      char op;
      cout << "Export to file ? [N/y]\n";
      cin >> op;
      bool restart = true;
      while (restart){
         if(op == 'Y'||op == 'y'){
            while (restart){
               cout << "Input File Name : ";
               cin >> fileName;
               path = "test/" + fileName;
               if(!IsFileExist(path)){
                  PrintTokenToFile(path,tokens);
                  restart = false;
               }
               else{
                  restart = true;
               }
            }
         }
         else if(op == 'N'||op=='n'){
            restart = false;
         }
         else{
            cout << "[ERROR] Unknown Options !!\n";
            cin >> op;
         }
      }
   } else {
      // Token-file mode: skip lexer, load tokens directly
      tokens = TokenFileReader(path);
      cout << "[INFO] Loaded " << tokens.size() << " tokens from file.\n";
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
         path = "test/" + fileName;
         if (!IsFileExist(path)) {
            saveTreeToFile(root, path);
            retry = false;
         }
      }
   }
}
