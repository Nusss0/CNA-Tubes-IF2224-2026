#pragma once

#include "../std.hpp"

// === Enum == //

// enum untuk menjelaskan tipe dari Objek
enum class ObjClass {
    UNDEFINED = 0,
    CONSTANT = 1,
    VARIABLE = 2,
    TYPE_DECL = 3,
    PROCEDURE = 4,
    FUNCTION = 5,
    PROGRAM = 6
};

// enum untuk menentukan code dari tipe
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

// tab Index untuk reserved word
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

// == Struct == //

//  struktur entry suatu table
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

// struktur satu baris dari sebuah entry block table
struct BtabEntry {
    int last;   
    int lpar;   
    int psze;   
    int vsze;   
};

// struktur satu entry untuk satu entry array table
struct AtabEntry {
    int xtyp;   
    int etyp;   
    int eref;   
    int low;    
    int high;  
    int elsz;   
    int size;   
};

// == Class == //

// kelas untuk symbol table
class SymbolTable {
    public:
        SymbolTable();

        // regist sebuah identifier
        int enter(const string& id, int obj, int type, int ref, bool nrm, int lev, int adr);

        // lookup sebuah identifier
        int lookup(const string& id) const;

        // push dan pop sebuah block
        void pushBlock();
        void popBlock();

        // mengecek current level dan block
        int currentLevel() const { return level; }
        int currentBlock() const { return display[level]; }

        // getter untuk Tab, Btab, Atab, dan Display
        const vector<TabEntry>& getTab() const { return tab; }
        const vector<BtabEntry>& getBtab() const { return btab; }
        const vector<AtabEntry>& getAtab() const { return atab; }
        const vector<int>& getDisplay() const { return display; }

        // melakukan dumping table
        void dumpTab(std::ostream& out) const;
        void dumpBtab(std::ostream& out) const;
        void dumpAtab(std::ostream& out) const;
        void dumpAll(std::ostream& out) const;

    private:
        vector<TabEntry> tab;
        vector<BtabEntry> btab;
        vector<AtabEntry> atab;
        vector<int> display;   
        int level;     

        // membuat entry table baru
        int addTabEntry(const TabEntry& entry);

        // initializer
        void initReservedWords();
        void initPredefined();
};
