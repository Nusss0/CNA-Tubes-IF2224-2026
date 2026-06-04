#include "symbol_table.hpp"

#include <iomanip>
#include <stdexcept>

// helpers
static const char* objClassName(int obj) {
    switch (static_cast<ObjClass>(obj)) {
        case ObjClass::UNDEFINED: return "undef";
        case ObjClass::CONSTANT: return "const";
        case ObjClass::VARIABLE: return "var";
        case ObjClass::TYPE_DECL: return "type";
        case ObjClass::PROCEDURE: return "proc";
        case ObjClass::FUNCTION: return "func";
        case ObjClass::PROGRAM: return "program";
        default: return "?";
    }
}


static void printHorizontalRule(ostream& out, const vector<int>& widths) {
    for (size_t i = 0; i < widths.size(); ++i) {
        out << string(widths[i], '-');
        if (i + 1 < widths.size()) out << '-';
    }
    out << '\n';
}

// ctor
SymbolTable::SymbolTable() {
    tab.reserve(128);
    btab.reserve(32);
    atab.reserve(32);

    tab.push_back({"", 0, (int)ObjClass::UNDEFINED, TC_NOTYPE, 0, true, 0, 0});

    initReservedWords();
    initPredefined();
    predefinedEnd = static_cast<int>(tab.size());

    btab.push_back({0, 0, 0, 0});
    btab[0].last = static_cast<int>(tab.size()) - 1;

    // dummy atab[0]
    atab.push_back({0, 0, 0, 0, 0, 0, 0});

    display.push_back(0);
    level = 0;
}

// init reserved
void SymbolTable::initReservedWords() {
    struct RawReserved {
        const char* name;
        int obj;
        int type;
    };

    static const RawReserved kReserved[32] = {
        //  1‑ 5
        {"and", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"array", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"begin", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"case", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"const", (int)ObjClass::UNDEFINED, TC_NOTYPE},

        //  6‑10
        {"div", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"downto", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"do", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"else", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"end", (int)ObjClass::UNDEFINED, TC_NOTYPE},

        // 11‑15
        {"for", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"function", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"if", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"mod", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"not", (int)ObjClass::UNDEFINED, TC_NOTYPE},

        // 16‑20
        {"of", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"or", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"procedure", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"program", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"record", (int)ObjClass::UNDEFINED, TC_NOTYPE},

        // 21‑25
        {"repeat", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"integer", (int)ObjClass::TYPE_DECL, TC_INTEGER},
        {"real", (int)ObjClass::TYPE_DECL, TC_REAL},
        {"boolean", (int)ObjClass::TYPE_DECL, TC_BOOLEAN},
        {"char", (int)ObjClass::TYPE_DECL, TC_CHAR},

        // 26‑30
        {"string", (int)ObjClass::TYPE_DECL, TC_STRING },
        {"then", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"to", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"type", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"until", (int)ObjClass::UNDEFINED, TC_NOTYPE},

        // 31‑32
        {"var", (int)ObjClass::UNDEFINED, TC_NOTYPE},
        {"while", (int)ObjClass::UNDEFINED, TC_NOTYPE},
    };

    for (int i = 0; i < 32; ++i) {
        int prev = static_cast<int>(tab.size()) - 1;  
        TabEntry e;
        e.id   = kReserved[i].name;
        e.link = prev;
        e.obj  = kReserved[i].obj;
        e.type = kReserved[i].type;
        e.ref  = 0;
        e.nrm  = true;
        e.lev  = 0;
        e.adr  = 0;
        tab.push_back(e);
    }
}


// init predefined
void SymbolTable::initPredefined() {
    /* true */
    {
        int prev = static_cast<int>(tab.size()) - 1;
        tab.push_back({"true", prev, (int)ObjClass::CONSTANT, TC_BOOLEAN, 0, true, 0, 1});
    }

    /* false */
    {
        int prev = static_cast<int>(tab.size()) - 1;
        tab.push_back({"false", prev, (int)ObjClass::CONSTANT, TC_BOOLEAN, 0, true, 0, 0});
    }

    /* readln */
    {
        int prev = static_cast<int>(tab.size()) - 1;
        tab.push_back({"readln", prev, (int)ObjClass::PROCEDURE, TC_NOTYPE, 0, true, 0, 0});
    }

    /* writeln */
    {
        int prev = static_cast<int>(tab.size()) - 1;
        tab.push_back({"writeln", prev, (int)ObjClass::PROCEDURE, TC_NOTYPE, 0, true, 0, 0});
    }

    /* write (tanpa newline) — OPR WRT (13) di spek */
    {
        int prev = static_cast<int>(tab.size()) - 1;
        tab.push_back({"write", prev, (int)ObjClass::PROCEDURE, TC_NOTYPE, 0, true, 0, 0});
    }
}

// add entry
int SymbolTable::addTabEntry(const TabEntry& entry) {
    tab.push_back(entry);
    return static_cast<int>(tab.size()) - 1;
}

// enter
int SymbolTable::enter(const string& id, int obj, int type, int ref, bool nrm, int lev, int adr) {
    int blockIdx = currentBlock();
    int prevLast = btab[blockIdx].last;

    TabEntry entry;
    entry.id = id;
    entry.link = prevLast;
    entry.obj = obj;
    entry.type = type;
    entry.ref = ref;
    entry.nrm = nrm;
    entry.lev = lev;
    entry.adr = adr;

    int idx = addTabEntry(entry);
    btab[blockIdx].last = idx;
    // track var size
    if (obj == (int)ObjClass::VARIABLE) {
        btab[blockIdx].vsze += 1;
    }
    return idx;
}

// case-insensitive
static bool sameId(const string& a, const string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (tolower(static_cast<unsigned char>(a[i])) != tolower(static_cast<unsigned char>(b[i]))) return false;
    }
    return true;
}

// link ref
void SymbolTable::setRef(int tabIdx, int ref) {
    if (tabIdx > 0 && tabIdx < (int)tab.size()) tab[tabIdx].ref = ref;
}

// mark params
void SymbolTable::markParamBoundary() {
    int blockIdx = currentBlock();
    btab[blockIdx].lpar = btab[blockIdx].last;
    btab[blockIdx].psze = btab[blockIdx].vsze;
}

// lookup
int SymbolTable::lookup(const string& id) const {
    for (int lev = level; lev >= 0; --lev) {
        int blockIdx = display[lev];
        int idx = btab[blockIdx].last;

        while (idx != 0) {
            if (sameId(tab[idx].id, id)) return idx;
            idx = tab[idx].link;
        }
    }
    return -1;
}

// enter array
int SymbolTable::enterArray(int xtyp, int etyp, int eref, int low, int high, int elsz, int size) {
    AtabEntry entry;
    entry.xtyp = xtyp;
    entry.etyp = etyp;
    entry.eref = eref;
    entry.low  = low;
    entry.high = high;
    entry.elsz = elsz;
    entry.size = size;
    atab.push_back(entry);
    return static_cast<int>(atab.size()) - 1;
}

// push block
void SymbolTable::pushBlock() {
    btab.push_back({0, 0, 0, 0});
    ++level;
    display.push_back(static_cast<int>(btab.size()) - 1);
}

// push record block (tanpa naikkan level — record bukan scope leksikal)
int SymbolTable::pushRecordBlock() {
    btab.push_back({0, 0, 0, 0});
    int newIdx = static_cast<int>(btab.size()) - 1;
    blockOverride.push_back(newIdx); // arahkan enter() ke block ini
    return newIdx;
}

void SymbolTable::popRecordBlock() {
    if (!blockOverride.empty()) blockOverride.pop_back();
}

// pop block 
void SymbolTable::popBlock() {
    if (level <= 0) return;         
    --level;
    display.pop_back();
}

// dump tab 
void SymbolTable::dumpTab(ostream& out) const {
    out << "\ntab:\n";

    const vector<int> widths = {4, 20, 8, 8, 4, 4, 4, 4, 5};
    auto printRow = [&](int i, const string& id, const string& obj, int type, int ref, bool nrm, int lev, int adr, int link) {
        out << right
            << setw(widths[0]) << i << ' '
            << setw(widths[1]) << id << ' '
            << setw(widths[2]) << obj << ' '
            << setw(widths[3]) << type << ' '
            << setw(widths[4]) << ref << ' '
            << setw(widths[5]) << (nrm ? 1 : 0) << ' '
            << setw(widths[6]) << lev << ' '
            << setw(widths[7]) << adr << ' '
            << setw(widths[8]) << link << '\n';
    };

    out << setw(widths[0]) << "idx" << ' '
        << setw(widths[1]) << "id" << ' '
        << setw(widths[2]) << "obj" << ' '
        << setw(widths[3]) << "type" << ' '
        << setw(widths[4]) << "ref" << ' '
        << setw(widths[5]) << "nrm" << ' '
        << setw(widths[6]) << "lev" << ' '
        << setw(widths[7]) << "adr" << ' '
        << setw(widths[8]) << "link" << '\n';

    printHorizontalRule(out, widths);

    for (size_t i = 0; i < tab.size(); ++i) {
        string dispId;
        if (tab[i].id.empty()) dispId = "(empty)";
        else dispId = tab[i].id;

        printRow(static_cast<int>(i), dispId, objClassName(tab[i].obj), tab[i].type, tab[i].ref, tab[i].nrm, tab[i].lev, tab[i].adr, tab[i].link);
    }
}

// dump btab
void SymbolTable::dumpBtab(ostream& out) const {
    out << "\nbtab:\n";

    const vector<int> widths = {4, 6, 6, 6, 6};
    out << setw(widths[0]) << "idx"  << ' '
        << setw(widths[1]) << "last" << ' '
        << setw(widths[2]) << "lpar" << ' '
        << setw(widths[3]) << "psze" << ' '
        << setw(widths[4]) << "vsze" << '\n';

    out << string(widths[0]+widths[1]+widths[2]+widths[3]+widths[4]+4, '-') << '\n';

    for (size_t i = 0; i < btab.size(); ++i) {
        out << right
            << setw(widths[0]) << i << ' '
            << setw(widths[1]) << btab[i].last << ' '
            << setw(widths[2]) << btab[i].lpar << ' '
            << setw(widths[3]) << btab[i].psze << ' '
            << setw(widths[4]) << btab[i].vsze << '\n';
    }
}

// dump atab
void SymbolTable::dumpAtab(ostream& out) const {
    out << "\natab:\n";

    // dummy atab[0]
    if (atab.size() <= 1) {
        out << "(empty)\n";
        return;
    }

    const vector<int> widths = {4, 7, 7, 7, 5, 5, 5, 5};
    out << setw(widths[0]) << "idx" << ' '
        << setw(widths[1]) << "xtyp" << ' '
        << setw(widths[2]) << "etyp" << ' '
        << setw(widths[3]) << "eref" << ' '
        << setw(widths[4]) << "low" << ' '
        << setw(widths[5]) << "high" << ' '
        << setw(widths[6]) << "elsz" << ' '
        << setw(widths[7]) << "size" << '\n';

    out << string(52, '-') << '\n';

    // skip dummy
    for (size_t i = 1; i < atab.size(); ++i) {
        out << right
            << setw(widths[0]) << i << ' '
            << setw(widths[1]) << atab[i].xtyp << ' '
            << setw(widths[2]) << atab[i].etyp << ' '
            << setw(widths[3]) << atab[i].eref << ' '
            << setw(widths[4]) << atab[i].low << ' '
            << setw(widths[5]) << atab[i].high << ' '
            << setw(widths[6]) << atab[i].elsz << ' '
            << setw(widths[7]) << atab[i].size << '\n';
    }
}

// dump all 
void SymbolTable::dumpAll(ostream& out) const {
    dumpTab(out);
    dumpBtab(out);
    dumpAtab(out);
}
