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
    string currWord; // helper var
    while (idx <= size){
        // sentinel guard
        char c = (idx < size ? raw[idx] : '\0');

        switch (state)
        {
        // idle state
        case State::IDLE :
            currWord = ""; // reset
            // skip whitespace
            if(isspace(c) || c =='\r' || c=='\n' || c=='\0'){
                idx++;
                break;
            }

            // number
            else if(isdigit(c)){
                currWord = c; // init
                state = State::NUM; // number state
                idx++; 
                break;
            }

            // string start
            else if(c == '\''){
                // skip quote
                state = State::STRING; // string state
                idx++;
                break;
            }

            // identifier
            else if(isalpha(c) ){
                currWord = c; // init 
                state = State::WORD; // word state
                idx++;
                break;
            }
            // plus
            else if(c == '+'){
                tokens.push_back({"plus",""});
                idx++;
                break;
            }

            // minus
            else if(c == '-'){
                state = State::NEGATIVE;
                currWord+=c;
                idx++;
                break;
            }

            // times
            else if(c == '*'){
                tokens.push_back({"times",""});
                idx++;
                break;
            }

            // rdiv
            else if(c == '/'){
                tokens.push_back({"rdiv",""});
                idx++;
                break;
            }

            // equals
            else if(c == '='){
                currWord+=c;
                state = State::EQ;
                idx++;
                break;
            }

            // less 
            else if(c == '<'){
            // peek next
                state = State::LE;
                idx++;
                break;
            }

            // greater
            else if(c == '>'){
            // peek next
                state = State::GTR;
                idx++;
                break;
            }

            else if(c == '('){
                // open paren
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
                // curly comment
                state = State::CURLCMNT;
                idx++;
                break;
            }
            else {
                tokens.push_back({"unknown", string(1,c)});
                idx++;
                break;
            }
        
// number state
        case State::NUM :
            // digit check
            if(isdigit(c)){
                currWord+=c; // accumulate
                idx++;
                break; // next char
            }
            else{ // number end  
                // real start
                if(c == '.'){
                    state= State::REAL; // real state
                    currWord+=c;
                    idx++; // advance
                    break;
                }
                else{ // int token
                    tokens.push_back({"intcon",currWord});
                    state = State::IDLE;
                    // reprocess char
                    break;
                }
            }

        // real state
        case State::REAL : 
            if(isdigit(c)){ // real digit
                currWord+=c;
                idx++;
                break;
            }
            else if (!isdigit(raw[idx-1])){ // bare dot
                currWord.pop_back(); // drop dot
                tokens.push_back({"intcon", currWord});
                tokens.push_back({"period", ""});
                currWord = "";
                state = State::IDLE;
                // reprocess char
                break;
            }
            else{ // real end
                tokens.push_back({"realcon",currWord});
                state = State::IDLE;
                // reprocess char
                break;
            }
        
        // string state
        case State::STRING :
            // expect quote
            if (c == '\''){
                // skip quote
                idx++;
                state = State::ESCAPE;
                break;
            }
            else if(c == '\n' || c == '\0'){ // multiline guard
                tokens.push_back({"unknown", "'" + currWord});// prepend quote
                currWord = "";
                state = State::IDLE;
                break;
            }
            else{ // accumulate
                currWord += c;
                idx++;
                break;
            }
        // escape state
        case State::ESCAPE : 
        if (c == '\''){// escaped quote
            currWord+='\'';
            idx++;
            state = State::STRING;
            break;
        }
        else { // push token
            // char check
            if(currWord.size() == 1){
                tokens.push_back({"charcon",currWord});
            }
            else{ //string
                tokens.push_back({"string",currWord});
            }
            state = State::IDLE; // back to idle
            // reprocess char
            break;
        }

        // word state
        case State::WORD :
            // check char
            if(isalpha(c) || isdigit(c)){
                currWord += c; // accumulate 
                idx++;
                break; // next char
            }
            else{ // keyword check

                // uppercase
                string upper = currWord;
                for (char &ch : upper) ch = toupper(ch);

                if (keyword.count(upper)) { // keyword found
                    tokens.push_back({keyword[upper], ""});
                } 
                else{
                    tokens.push_back({"ident",currWord});
                }
                // reprocess char
                state = State::IDLE;
                break;
            }

        // eq state
        case State::EQ :
            // pascal equal
            // legacy note
            tokens.push_back({"eql",""});
            state = State::IDLE;
            break;

        // le state
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
                // reprocess char
            }
            state = State::IDLE;
            break;
        
        // gtr state
        case State::GTR :
            if(c=='='){
                tokens.push_back({"geq",""});
                idx++;
            }
            else{
                tokens.push_back({"gtr",""});
                // reprocess char
            }
            state = State::IDLE;
            break;

            
            // colon state
            case State::COLON :
                if(c == '='){
                    tokens.push_back({"becomes",""});
                    idx++;
                }
                else{
                    tokens.push_back({"colon",""});
                    // reprocess char
                }
                state = State::IDLE;
                break;

            // curly comment
            case State::CURLCMNT :
                if(c == '}'){ // comment end
                    // skip brace
                    tokens.push_back({"comment",currWord});
                    state = State::IDLE;
                }
                else if(c == '*'){ // star close
                    // skip star
                    state = State::CLSCMT;
                }
                else{ // accumulate
                    currWord+=c;
                }
                idx++;
                break;

            // star comment
            case State::COMMENTS :
                if(c == '}'){ // brace close
                    // skip brace
                    tokens.push_back({"comment",currWord});
                    state = State::IDLE;
                }
                else if(c == '*'){// star check
                    // skip star
                    state = State::CLSCMT;
                }
                else{
                    currWord+=c;
                }
                idx++;
                break;

            // close comment
            case State::CLSCMT :
                if(c == ')'){ // comment end
                    // skip paren
                    tokens.push_back({"comment",currWord});
                    state=State::IDLE;
                    idx++;
                    break;
                }
                else{ // not close
                    currWord += '*';// restore star
                    state = State::COMMENTS;
                    // reprocess char
                    break;
                }

            // open comment
            case State::OPCMT :
                if(c == '*'){
                    // skip star
                    state = State::COMMENTS; // comment state
                    idx++; // advance
                }
                else{
                    // left paren
                    currWord += '(';// redundant
                    tokens.push_back({"lparent",""}); // lparent token
                    state = State::IDLE;
                }
                // no advance
                break;

            // negative state
            case State::NEGATIVE :
                if(isdigit(c)){// negative number
                    state = State::NUM;
                    // no advance
                    break;
                }
                else{// minus sign
                    tokens.push_back({"minus",""});
                    state = State::IDLE;
                    // no advance
                    break;
                }   
            default :
                tokens.push_back({"unknown", string(1,c)});
                idx++;
        }
    }
    // unfinished token
    if(state != State::IDLE){
        tokens.push_back({"unknown", currWord});
    }

    return tokens;
}
