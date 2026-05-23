#pragma once

#include "../std.hpp"

// enums
enum class ObjClass {
    UNDEFINED = 0,
    CONSTANT = 1,
    VARIABLE = 2,
    TYPE_DECL = 3,
    PROCEDURE = 4,
    FUNCTION = 5,
    PROGRAM = 6
};


enum TypeCode {
    TC_NOTYPE = 0,
    TC_INTEGER = 1,
    TC_REAL = 2,
    TC_BOOLEAN = 3,
    TC_CHAR = 4,
    TC_STRING = 5,
    TC_ARRAY = 6,
    TC_RECORD = 7
};

// reserved index
namespace ReservedIdx {
    constexpr int PROGRAM = 19;
    constexpr int FUNCTION = 12;
    constexpr int PROCEDURE = 18;
    constexpr int INTEGER = 22;
    constexpr int REAL = 23;
    constexpr int BOOLEAN = 24;
    constexpr int CHAR = 25;
    constexpr int STRING = 26;
}

// structs
struct TabEntry {
    string id;    
    int link;   
    int obj;    
    int type;   
    int ref;    
    bool nrm;    
    int lev;    
    int adr;    
};


struct BtabEntry {
    int last;   
    int lpar;   
    int psze;   
    int vsze;   
};


struct AtabEntry {
    int xtyp;   
    int etyp;   
    int eref;   
    int low;    
    int high;  
    int elsz;   
    int size;   
};

// class
class SymbolTable {
    public:
        SymbolTable();

        // enter identifier
        int enter(const string& id, int obj, int type, int ref, bool nrm, int lev, int adr);

        // link ref
        void setRef(int tabIdx, int ref);

        // mark param boundary
        void markParamBoundary();

        // lookup
        int lookup(const string& id) const;

        // scope
        void pushBlock();
        void popBlock();

        // push/pop record block: btab dibuat & diarahkan sebagai target enter(),
        // TAPI level tidak naik (record bukan scope leksikal terpisah).
        // return: indeks btab block record (utk disimpan di result.ref).
        int pushRecordBlock();
        void popRecordBlock();

        // array entry
        int enterArray(int xtyp, int etyp, int eref, int low, int high, int elsz, int size);

        // current scope
        int currentLevel() const { return level; }
        int currentBlock() const {
            return blockOverride.empty() ? display[level] : blockOverride.back();
        }

        // predefined count
        int predefinedCutoff() const { return predefinedEnd; }

        // getters
        const vector<TabEntry>& getTab() const { return tab; }
        const vector<BtabEntry>& getBtab() const { return btab; }
        const vector<AtabEntry>& getAtab() const { return atab; }
        const vector<int>& getDisplay() const { return display; }

        // dump
        void dumpTab(std::ostream& out) const;
        void dumpBtab(std::ostream& out) const;
        void dumpAtab(std::ostream& out) const;
        void dumpAll(std::ostream& out) const;

    private:
        vector<TabEntry> tab;
        vector<BtabEntry> btab;
        vector<AtabEntry> atab;
        vector<int> display;
        vector<int> blockOverride; // stack: target block utk enter() saat masuk record
        int level;
        int predefinedEnd = 0; //index terakhir + 1 utk reserved+predefined entries

        // add entry
        int addTabEntry(const TabEntry& entry);

        // init
        void initReservedWords();
        void initPredefined();
};
