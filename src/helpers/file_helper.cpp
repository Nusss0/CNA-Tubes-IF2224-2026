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

vector<Token> TokenFileReader(const string& path){
    ifstream f(path);
    vector<Token> tokens;

    if(!f.is_open()){
        cerr << "File not found !!!\n";
        return tokens;
    }

    string line;
    while(getline(f, line)){
        if(!line.empty() && line.back() == '\r') line.pop_back();
        if(line.empty()) continue;

        Token t;
        size_t lp = line.find('(');
        size_t rp = line.rfind(')');
        if(lp != string::npos && rp != string::npos && lp < rp){
            t.type = line.substr(0, lp);
            t.value = line.substr(lp + 1, rp - lp - 1);
        } else {
            t.type = line;
            t.value = "";
        }
        tokens.push_back(t);
    }

    return tokens;
}

vector<Token> LoadTokens(const string& path){
    ifstream f(path);
    vector<Token> tokens;

    if(!f.is_open()){
        cerr << "File not found !!!\n";
        return tokens;
    }

    string line;
    int checked = 0;
    int tokenLike = 0;
    
    // cek 8 baris aja
    while(getline(f, line) && checked < 8){
        if(!line.empty() && line.back() == '\r') line.pop_back();
        if(line.empty()) continue;
        checked++;

        size_t lp = line.find('(');
        size_t rp = line.rfind(')');
        bool isTypeOnly = (lp == string::npos && rp == string::npos && line.find(' ') == string::npos && line.find('\t') == string::npos);
        bool isTypeValue = (lp != string::npos && rp == line.size() - 1 && lp < rp && line.substr(0, lp).find(' ') == string::npos &&
                            line.substr(0, lp).find('\t') == string::npos);

        if(isTypeOnly || isTypeValue){
            tokenLike++;
        }
    }

    bool TokenFile = (checked > 0 && tokenLike == checked);
    if(TokenFile){
        return TokenFileReader(path);
    }

    // run lexer and remove comment tokens before parser (raw file)
    string raw = DFAFileReader(path);
    tokens = Tokenizing(raw);

    vector<Token> filtered;
    for(const auto& t : tokens){
        if(t.type != "comment") filtered.push_back(t);
    }
    return filtered;
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