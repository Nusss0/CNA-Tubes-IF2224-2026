#include "token_processing.hpp"
#include "../helpers/file_helper.hpp"

map<string,string> keyword = {
    {"NOT", "notsy"},
    {"DIV", "idiv"},
    {"MOD", "imod"},
    {"AND", "andsy"},
    {"OR", "orsy"},
    {"CONST", "constsy"},
    {"TYPE", "typesy"},
    {"VAR", "varsy"},
    {"FUNCTION", "functionsy"},
    {"PROCEDURE", "proceduresy"},
    {"ARRAY", "arraysy"},
    {"RECORD", "recordsy"},
    {"PROGRAM", "programsy"},
    {"BEGIN", "beginsy"},
    {"IF", "ifsy"},
    {"CASE", "casesy"},
    {"REPEAT", "repeatsy"},
    {"WHILE", "whilesy"},
    {"FOR", "forsy"},
    {"END", "endsy"},
    {"ELSE", "elsesy"},
    {"UNTIL", "untilsy"},
    {"OF", "ofsy"},
    {"DO", "dosy"},
    {"TO", "tosy"},
    {"DOWNTO", "downtosy"},
    {"THEN", "thensy"},
};

enum class State{
    IDLE,
    NUM,
    REAL,
    STRING,
    ESCAPE,
    WORD,
    EQ,
    LE,
    GTR,
    COLON,
    CURLCMNT,
    COMMENTS,
    OPCMT,
    CLSCMT,
    NEGATIVE,
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
            currWord = ""; //jaga"
            //kalau masih space kosong, skip. State tetap IDLE
            if(isspace(c) || c =='\r' || c=='\n' || c=='\0'){
                idx++;
                break;
            }

            //kalau c berupa angka
            else if(isdigit(c)){
                currWord = c; //inisialisasi karena state masih IDLE
                state = State::NUM; //lanjut proses di state NUM
                idx++; 
                break;
            }

            //kalau c berupa petik tunggal (')
            else if(c == '\''){
                //skip isi petiknya
                state = State::STRING; //ekspetasi awal ke Char dulu
                idx++;
                break;
            }

            //kalau c itu karakter a-z / A-Z / _ maka statenya itu word
            else if(isalpha(c) ){
                currWord = c; //inisialisasi 
                state = State::WORD; //lanjut proses di state WORD
                idx++;
                break;
            }
            //Kalau +, maka masuk ke state PLUS
            else if(c == '+'){
                tokens.push_back({"plus",""});
                idx++;
                break;
            }

            //kalau -, masuk ke state MIN
            else if(c == '-'){
                state = State::NEGATIVE;
                currWord+=c;
                idx++;
                break;
            }

            //kalau *, masuk ke state TIMES
            else if(c == '*'){
                tokens.push_back({"times",""});
                idx++;
                break;
            }

            //kalau /, masuk ke state RDIV
            else if(c == '/'){
                tokens.push_back({"rdiv",""});
                idx++;
                break;
            }

            //kalau =, masuk ke state EQ dan expect = lagi
            else if(c == '='){
                currWord+=c;
                state = State::EQ;
                idx++;
                break;
            }

            //kalau <, masuk ke state LE 
            else if(c == '<'){
            //ga perlu save ke currword, cukup perhatiin karakter selanjutnya
                state = State::LE;
                idx++;
                break;
            }

            //kalau >, masuk ke state GTR
            else if(c == '>'){
            //ga perlu save ke currword, cukup perhatiin karakter selanjutnya
                state = State::GTR;
                idx++;
                break;
            }

            else if(c == '('){
                //skip inclue ( dulu
                state = State::OPCMT;
                idx++;
                break;
            }
            else if(c == ')'){
                tokens.push_back({"rparent",""});
                idx++;
                break;
            }
            else if(c == '['){
                tokens.push_back({"lbrack",""});
                idx++;
                break;
            }
            else if(c == ']'){
                tokens.push_back({"rbrack",""});
                idx++;
                break;
            }
            else if(c == ','){
                tokens.push_back({"comma",""});
                idx++;
                break;
            }
            else if(c == '.'){
                tokens.push_back({"period",""});
                idx++;
                break;
            }
            else if(c == ';'){
                tokens.push_back({"semicolon",""});
                idx++;
                break;
            }
            else if(c == ':'){
                state = State::COLON;
                idx++;
                break;
            }
            else if(c == '{'){
                //skip include { di currWord
                state = State::CURLCMNT;
                idx++;
                break;
            }
            else {
                tokens.push_back({"unknown", string(1,c)});
                idx++;
                break;
            }
        
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
                    // tidak idx++, karakter saat ini belum diproses
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
            else if (!isdigit(raw[idx-1])){ //titik tidak diikuti angka, pisah jadi intcon + period
                currWord.pop_back(); //hapus '.' dari currWord
                tokens.push_back({"intcon", currWord});
                tokens.push_back({"period", ""});
                currWord = "";
                state = State::IDLE;
                // tidak idx++, karakter saat ini belum diproses
                break;
            }
            else{ //angka setelah titik sudah habis, tokenisasi sebagai realcon
                tokens.push_back({"realcon",currWord});
                state = State::IDLE;
                // tidak idx++, karakter saat ini belum diproses
                break;
            }
        
        //---------------------------------------- STRING STATE ---------------------------------------------
        case State::STRING :
            //Expect alpha atau ' , otherwise error
            if (c == '\''){
                //skip petik
                idx++;
                state = State::ESCAPE;
                break;
            }
            else if(c == '\n' || c == '\0'){ //string tidak boleh multiline, langsung unknown
                tokens.push_back({"unknown", "'" + currWord});//menambahkan ' diawal sebagai bagian dari unknown (karena di State string, petik di skip unutk)
                currWord = "";
                state = State::IDLE;
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
        if (c == '\''){//secondary ' (berarti sebelumnya esc)
            currWord+='\'';
            idx++;
            state = State::STRING;
            break;
        }
        else { //kalau karakternya bukan ', langsung push 
            //If currWord berupa 3 karakter (2 petik dan 1 alpha) berarti itu char
            if(currWord.size() == 1){
                tokens.push_back({"charcon",currWord});
            }
            else{ //string
                tokens.push_back({"string",currWord});
            }
            state = State::IDLE; //balik ke state idle
            //tidak perlu idx++, karena current karakter belum diproses
            break;
        }

        //---------------------------------------------- STATE WORD ------------------------------------------
        case State::WORD :
            //cek karakter
            if(isalpha(c) || isdigit(c)){
                currWord += c; //lanjut proses kata 
                idx++;
                break; //next char
            }
            else{ //proses kata selesai, cek keyword

                // uppercase currWord untuk case-insensitive lookup
                string upper = currWord;
                for (char &ch : upper) ch = toupper(ch);

                if (keyword.count(upper)) { //Kalau berada di keyword, maka langsung push dan mapping
                    tokens.push_back({keyword[upper], ""});
                } 
                else{
                    tokens.push_back({"ident",currWord});
                }
                //tidak perlu idx++ karena karakter sekarang blm diproses
                state = State::IDLE;
                break;
            }

        //-------------------------------------------- STATE EQ -------------------------------------------
        case State::EQ :
            //Pascal pakai single '=' utk eql. token udah di-emit saat masuk state ini,
            //jadi langsung selesai tanpa consume karakter berikutnya.
            //(legacy: dulu expect '==' style C, sekarang dibalik)
            tokens.push_back({"eql",""});
            state = State::IDLE;
            break;

        //-------------------------------------------- STATE LE -------------------------------------------
        case State::LE :
            if(c=='>'){
                tokens.push_back({"neq",""});
                idx++;
            }
            else if(c == '='){
                tokens.push_back({"leq",""});
                idx++;
            }
            else{
                tokens.push_back({"lss",""});
                // tidak idx++, karakter saat ini belum diproses
            }
            state = State::IDLE;
            break;
        
        //-------------------------------------------- STATE LE -------------------------------------------
        case State::GTR :
            if(c=='='){
                tokens.push_back({"geq",""});
                idx++;
            }
            else{
                tokens.push_back({"gtr",""});
                // tidak idx++, karakter saat ini belum diproses
            }
            state = State::IDLE;
            break;

            
            //-------------------------------------------- STATE COLON -------------------------------------------
            case State::COLON :
                if(c == '='){
                    tokens.push_back({"becomes",""});
                    idx++;
                }
                else{
                    tokens.push_back({"colon",""});
                    // tidak idx++, karakter saat ini belum diproses
                }
                state = State::IDLE;
                break;

            //-------------------------------------------- STATE CURLY COMMENTS -------------------------------------------
            case State::CURLCMNT :
                if(c == '}'){ //comment slesai dengan }
                    //skip }
                    tokens.push_back({"comment",currWord});
                    state = State::IDLE;
                }
                else if(c == '*'){ //bisa juga ditutup dengan *)
                    //skip include *
                    state = State::CLSCMT;
                }
                else{ //selainnya ditambahkan sebagai bagian komentar
                    currWord+=c;
                }
                idx++;
                break;

            //-------------------------------------------- STATE Star COMMENTS -------------------------------------------
            case State::COMMENTS :
                if(c == '}'){ //bisa juga ditutup dengan }
                    //skip include }
                    tokens.push_back({"comment",currWord});
                    state = State::IDLE;
                }
                else if(c == '*'){//kalau ada bintang
                    //skip inlcude *
                    state = State::CLSCMT;
                }
                else{
                    currWord+=c;
                }
                idx++;
                break;

            //-------------------------------------------- STATE CLOSE COMMENT FOR STAR CMT -------------------------------------------
            case State::CLSCMT :
                if(c == ')'){ //comment slesai dengan *)
                    //skip include )
                    tokens.push_back({"comment",currWord});
                    state=State::IDLE;
                    idx++;
                    break;
                }
                else{ //bukan penutup, kembali baca sebagai isi komentar
                    currWord += '*';//bukan penutup jadi include * yang dari sebelumya
                    state = State::COMMENTS;
                    // tidak idx++, biarkan karakter ini diproses ulang di COMMENTS
                    break;
                }

            //-------------------------------------------- STATE OPEN COMMENTS -------------------------------------------
            case State::OPCMT :
                if(c == '*'){
                    //skip include *
                    state = State::COMMENTS; //state komentar
                    idx++; //hanya ini yg butuh next char untuk lanjut proses
                }
                else{
                    //hanya lparent biasa jadi
                    currWord += '(';//meski redundant
                    tokens.push_back({"lparent",""}); //else tokenize jadi lparent
                    state = State::IDLE;
                }
                //no need next char
                break;

            //-------------------------------------------- STATE OPEN COMMENTS -------------------------------------------
            case State::NEGATIVE :
                if(isdigit(c)){//kalau berupa digit setelah -, maka akan dialihkan ke Number
                    state = State::NUM;
                    //idx tidak perlu di incr
                    break;
                }
                else{//kalau bukan digit, maka dianggap sebagai minus sign biasa
                    tokens.push_back({"minus",""});
                    state = State::IDLE;
                    //tidak perlu idx++
                    break;
                }   
            default :
                tokens.push_back({"unknown", string(1,c)});
                idx++;
        }
    }
    // kalau input selesai dalam kondisi state selain IDLE, maka token tidak dikenali
    if(state != State::IDLE){
        tokens.push_back({"unknown", currWord});
    }

    return tokens;
}
