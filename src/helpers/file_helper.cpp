#include "file_helper.hpp"
#include "../lexical/token_processing.hpp"

string DFAFileReader(const string fileName){
    ifstream f(fileName);

    //error handling
    if(!f.is_open()) {
        cerr << "File not found !!!\n";
        return "";
    }

    string res, tmp;
    while(getline(f,tmp)){
        res += tmp + '\n';
    }

    return res;
}

void TokenPrinter(const vector<Token> &tokens){
    for(auto &t : tokens){
        if(t.value.empty()){
            cout << t.type << endl;
        }
        else{
            cout << t.type << "("<< t.value << ")" << endl;
        }
    }
}

void PrintTokenToFile(const string fileName, const vector<Token> &tokens){
    ofstream file(fileName);
    for(auto &t : tokens){
        if(t.value.empty()){
            file << t.type + "\n";
        }
        else{
            file << t.type +  "("<< t.value << ")\n";
        }
    }

    file.close();
}

bool IsFileExist(const string path){
    ifstream f(path);
    char op;
    bool restart = true;
    if(f.good()){ //jika file name ditemukan
        cout << "File Exist !!!\n";
        while (restart){ //selama tidak return, akan terjebak di loop ini
            cout << "Do you want to override it ? [Y/n]\n";
            cin >> op;
            if(op =='Y'||op=='y'){
                return false; //asumsi file tidak ada, jadi kena override
            }    
            else if(op =='N'||op=='n'){
                return true; //file ada jadi tidak boleh override
            }
            else{
                cout << "[ERROR] : Unknown Option !!\n";
            }
        }
    }  
    //jika file tidak ditemukan
    return false; //langsung return false
}