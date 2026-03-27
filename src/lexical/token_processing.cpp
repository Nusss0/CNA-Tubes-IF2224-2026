
#include "token_processing.hpp"
#include "../helpers/word_machine.hpp"

map<string,string> keyword = {

};

enum class State{
    IDLE,
    NUM,
    REAL,
    STRING,
    ESCAPE,
    WORD,

};


vector<Token> Tokenizing(const string &raw){
    vector<Token> tokens;
    State state = State::IDLE;
    int idx = 0;
    int size = raw.size();
    string currWord; //var pembantu untuk wording nantinya
    while (idx <= size){
        // Use sentinel '\0' when idx == size to avoid out-of-bounds access
        char c = (idx < size ? raw[idx] : '\0');

        switch (state)
        {
        //kalau state IDLE / belum nerima apapun maka cek karakter pertama
        case State::IDLE :
            
            //kalau masih space kosong, skip. State tetap IDLE
            if(isspace(c)){
                idx++;
                break;
            }

            //kalau c berupa angka
            if(isdigit(c)){
                currWord = c; //inisialisasi karena state masih IDLE
                state = State::NUM; //lanjut proses di state NUM
                idx++; 
                break;
            }

            //kalau c berupa petik tunggal (')
            if(c == '\''){
                currWord = c; //inisialisasi
                state = State::STRING; //ekspetasi awal ke Char dulu
                idx++;
                break;
            }
            //kalau c itu karakter a-z / A-Z / _ maka statenya itu word
            // if(isalpha(c) || c == '_'){
            //     currWord = c; //inisialisasi 
            //     state = State::WORD; //lanjut proses di state WORD
            //     idx++;
            //     break;
            // }
            idx++;
            break;
        
//-------------------------------------- STATE NUMBER ----------------------------------------
        case State::NUM :
            //cek number
            if(isdigit(c)){
                currWord+=c; //lanjut proses number
                idx++;
                break; //next char
            }
            else{ //number sudah habis, tokenisasi  
                //kalau berupa titik, lanjut ke state Real
                if(c == '.'){
                    state= State::REAL; //masuk ke state Real
                    currWord+=c;
                    idx++; //skip dlu
                    break;
                }
                else{ //sisanya tokenisasi INTCON(currWord) ke token
                    tokens.push_back({"intcon",currWord});
                    state = State::IDLE;
                    idx++;
                    break;
                }
            }

        //------------------------------------------ REAL STATE ----------------------------------------
        case State::REAL : 
            if(isdigit(c)){ //setelah titik, harus masih berupa angka
                currWord+=c;
                idx++;
                break;
            }
            else if (!isdigit(raw[idx-1])){
                ErrorTokenMessage(c,currWord);
                return tokens;
            }
            else{ //jika bukan angka, maka unidentified Tokens.
                tokens.push_back({"realcon",currWord});
                state = State::IDLE;
                idx++;
                break;
            }
        
        //---------------------------------------- STRING STATE ---------------------------------------------
        case State::STRING : 
            //Expect alpha atau ' , otherwise error
            if (c == '\''){
                currWord += c; //tambahin petik
                idx++;
                state = State::ESCAPE;
                break;
            }
            else{ //karakter selain ' akan dianggap sebagai lanjutan
                currWord += c;
                idx++;
                break;
            }
        //----------------------------------------- ESCAPE STATE ------------------------------------------
        case State::ESCAPE : 
        //Disini kondisi setelah char/string, jadi setelah menemukan ' kedua kalinya, akan masuk ke kondisi escape. Jika ditemukan ' lagi, maka akan dianggap sebagai 1 buah petik saja (escape character). Jika ditemukan selain ', lgsg EOF.
        if (c == '\''){
            idx++;
            state = State::STRING;
            break;
        }
        else { //kalau karakternya bukan ', langsung push 
            //If currWord berupa 3 karakter (2 petik dan 1 alpha) berarti itu char
            if(currWord.size() == 3){
                tokens.push_back({"charcon",currWord});
            }
            else{ //string
                tokens.push_back({"string",currWord});
            }
            state = State::IDLE; //balik ke state idle
            //tidak perlu idx++, karena current karakter belum diproses
            break;
        }

        //state WORD
        // case State::WORD :
        //     //cek karakter
        //     if(isalpha(c) || c=='_'){
        //         currWord += c; //lanjut proses kata 
        //         idx++;
        //         break; //next char
        //     }
        //     else{ //proses kata selesai, cek keyword
        //         continue;
        //     }


        default:
            idx++;
            break;
        }
    }
    // kalau input selesai dalam kondisi state selain IDLE, maka token tidak dikenali
    if(state != State::IDLE){
        cerr << "[ERROR] EOP while Tokenizing \""<<currWord<<"\"\n";
    }
    return tokens;
}

//STRING COLLECTOR
//  tmp = raw[idx];
//         //kalau berupa karakter A-Z atau a-z
//         while((raw[idx+1]>=65 && raw[idx+1]<=90) || (raw[idx+1]>=97 && raw[idx+1]<=122)){
//             //kumpulin sampai bukan karakter
//             tmp += raw[idx+1];
//             idx++; //lanjut next char
//         }
