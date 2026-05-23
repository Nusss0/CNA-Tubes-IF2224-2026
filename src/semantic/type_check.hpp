#pragma once

#include "../std.hpp"
#include "symbol_table.hpp"

// semantic type
struct SemanticType {
    int type = TC_NOTYPE;   // kode tipe (lihat enum TypeCode)
    string name;            // nama tipe / penanda kategori ("subrange", "enumerated", ...)
    bool anonymous = false; // true utk record/tipe anonim (tidak kompatibel)
    int ref = 0;            // ref ke entry tipe lain (atab/tab)
    int low = 0;            // batas bawah (subrange / enumerated)
    int high = 0;           // batas atas (subrange / enumerated)
    int size = 1;           // ukuran tipe
    bool isSubrange = false;// true bila tipe ini subrange
    bool hasRange = false;  // true bila low/high bermakna
    int len = 0;            // panjang string literal
    bool hasLen = false;    // true bila len bermakna (string literal)
};

// type check
namespace TypeCheck {


enum class TypeError {
    None,             // tidak ada error
    IncompatibleOperand, // operand operator tak cocok
    NonNumeric,       // butuh numeric (integer/real/subrange)
    NonInteger,       // butuh integer (div, mod)
    NonBoolean        // butuh boolean (and, or, not)
};

bool isError(const SemanticType& t);
bool isSimple(const SemanticType& t);
bool isOrdinal(const SemanticType& t);
bool isStructured(const SemanticType& t);
bool isNumeric(const SemanticType& t);
bool isBoolean(const SemanticType& t);

string typeCodeName(int type);
string describe(const SemanticType& t);

// type relation
bool sameType(const SemanticType& a, const SemanticType& b);
bool compatible(const SemanticType& a, const SemanticType& b);
bool assignmentCompatible(const SemanticType& lhs, const SemanticType& rhs);

SemanticType resultBinary(const string& op, const SemanticType& a,
                          const SemanticType& b, TypeError& err);
SemanticType resultRelational(const SemanticType& a, const SemanticType& b,
                              TypeError& err);

void validateRangeBounds(const SemanticType& lo, const SemanticType& hi,
                         bool& realBound, bool& orderBad);
bool validArrayIndex(const SemanticType& idx);

SemanticType makeBasic(const string& name);
SemanticType makeCode(int type);

} // namespace TypeCheck
