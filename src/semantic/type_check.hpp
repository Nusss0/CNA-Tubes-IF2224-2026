#pragma once

#include "../std.hpp"
#include "symbol_table.hpp"

// === Representasi tipe hasil inferensi semantic === //
// Dipakai oleh SemanticAnalyzer (analyzer.hpp) dan modul aturan TypeCheck.
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

// === Modul aturan pemeriksaan tipe ===
// Semua fungsi murni (stateless): mengambil keputusan tipe, tidak melaporkan
// error. Perakitan pesan + reportError() tetap di SemanticAnalyzer agar modul
// ini mudah diuji dan dipakai ulang.
namespace TypeCheck {

// jenis kesalahan tipe yang dideteksi modul (analyzer merakit pesannya)
enum class TypeError {
    None,             // tidak ada error
    IncompatibleOperand, // operand operator tak cocok
    NonNumeric,       // butuh numeric (integer/real/subrange)
    NonInteger,       // butuh integer (div, mod)
    NonBoolean        // butuh boolean (and, or, not)
};

// --- klasifikasi tipe ---
bool isError(const SemanticType& t);    // tipe NOTYPE (supresi error berantai)
bool isSimple(const SemanticType& t);   // integer/real/char/boolean/string/subrange/enum
bool isOrdinal(const SemanticType& t);  // integer/char/boolean/subrange/enumerated
bool isStructured(const SemanticType& t); // array/record
bool isNumeric(const SemanticType& t);  // integer/real/subrange numerik
bool isBoolean(const SemanticType& t);

// --- penamaan untuk pesan / anotasi ---
string typeCodeName(int type);          // kode tipe -> nama
string describe(const SemanticType& t); // nama human-readable

// --- relasi tipe (spec hal. 15) ---
// identik / subrange dgn basis sama / string panjang sama
bool sameType(const SemanticType& a, const SemanticType& b);
// kompatibilitas umum (operand relasional, dll.)
bool compatible(const SemanticType& a, const SemanticType& b);
// kompatibilitas assignment: Real<-Integer, subset value range, dst.
bool assignmentCompatible(const SemanticType& lhs, const SemanticType& rhs);

// --- inferensi tipe operator biner ---
// op = label token (plus/minus/times/rdiv/idiv/imod/andsy/orsy)
SemanticType resultBinary(const string& op, const SemanticType& a,
                          const SemanticType& b, TypeError& err);
// op relasional (eql/neq/lss/leq/gtr/geq) -> Boolean
SemanticType resultRelational(const SemanticType& a, const SemanticType& b,
                              TypeError& err);

// --- validasi konteks ---
// range bound: simple non-real, lower <= upper
void validateRangeBounds(const SemanticType& lo, const SemanticType& hi,
                         bool& realBound, bool& orderBad);
// index array tidak boleh real
bool validArrayIndex(const SemanticType& idx);

// --- factory ---
SemanticType makeBasic(const string& name); // "integer"/"real"/...
SemanticType makeCode(int type);

} // namespace TypeCheck
