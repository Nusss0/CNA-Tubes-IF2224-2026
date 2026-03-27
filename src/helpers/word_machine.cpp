#include "word_machine.hpp"

string DFAFileReader(const string fileName){
    ifstream f(fileName);

    //error handling
    if(!f.is_open()) {
        cerr << "File not found !!!\n";
        return "";
    }

    string res, tmp;
    while(getline(f,tmp)){
        res += tmp;
    }

    return res;
}

void TokenPrinter(vector<Token> &tokens){
    for(auto &t : tokens){
        if(t.value.empty()){
            cout << t.type << endl;
        }
        else{
            cout << t.type << "("<< t.value << ")" << endl;
        }
    }
}
