#include "type_check.hpp"

namespace {
string toLowerCopy(string text) {
    for (char& c : text) {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    return text;
}
} // namespace

namespace TypeCheck {

// === Klasifikasi === //

bool isError(const SemanticType& t) {
    return t.type == TC_NOTYPE;
}

bool isSimple(const SemanticType& t) {
    switch (t.type) {
        case TC_INTEGER:
        case TC_REAL:
        case TC_CHAR:
        case TC_BOOLEAN:
        case TC_STRING:
            return true;
        default:
            return false;
    }
}

bool isOrdinal(const SemanticType& t) {
    if (t.isSubrange || t.name == "enumerated") return true;
    switch (t.type) {
        case TC_INTEGER:
        case TC_CHAR:
        case TC_BOOLEAN:
            return true;
        default:
            return false;
    }
}

bool isStructured(const SemanticType& t) {
    return t.type == TC_ARRAY || t.type == TC_RECORD;
}

bool isNumeric(const SemanticType& t) {
    return t.type == TC_INTEGER || t.type == TC_REAL || t.isSubrange ||
           t.name == "subrange";
}

bool isBoolean(const SemanticType& t) {
    return t.type == TC_BOOLEAN;
}

// === Penamaan === //

string typeCodeName(int type) {
    switch (type) {
        case TC_INTEGER: return "integer";
        case TC_REAL:    return "real";
        case TC_BOOLEAN: return "boolean";
        case TC_CHAR:    return "char";
        case TC_STRING:  return "string";
        case TC_ARRAY:   return "array";
        case TC_RECORD:  return "record";
        default:         return "unknown";
    }
}

string describe(const SemanticType& t) {
    if (!t.name.empty()) return t.name;
    return typeCodeName(t.type);
}

// === Relasi tipe === //

bool sameType(const SemanticType& a, const SemanticType& b) {
    if (isError(a) || isError(b)) return false;

    // string: identik bila panjang sama (bila panjang diketahui)
    if (a.type == TC_STRING && b.type == TC_STRING) {
        if (a.hasLen && b.hasLen) return a.len == b.len;
        return true; // panjang salah satu tak diketahui -> anggap cocok
    }

    // record: hanya named record dengan nama sama yang identik.
    // record anonim ditolak (lihat juga compatible/assignmentCompatible).
    if (a.type == TC_RECORD && b.type == TC_RECORD) {
        if (a.anonymous || b.anonymous) return false;
        return !a.name.empty() && a.name == b.name;
    }

    // tipe simple & subrange: cukup bandingkan kode tipe (basis subrange ada di
    // field type). Nama tidak dibandingkan karena lookup variabel mengosongkan
    // name sedangkan literal mengisinya (mis. integer dari var vs intcon).
    // - subrange vs tipe basisnya     (mis. [1..10] dengan Integer)
    // - dua subrange dari basis sama   (mis. [1..10] dengan [50..100])
    if (a.type == b.type) return true;

    return false;
}

bool compatible(const SemanticType& a, const SemanticType& b) {
    if (isError(a) || isError(b)) return true;

    // record anonim tidak pernah kompatibel
    if ((a.type == TC_RECORD && a.anonymous) ||
        (b.type == TC_RECORD && b.anonymous)) {
        return false;
    }

    if (sameType(a, b)) return true;

    // numeric saling kompatibel (integer/real/subrange)
    if (isNumeric(a) && isNumeric(b)) return true;

    return false;
}

bool assignmentCompatible(const SemanticType& lhs, const SemanticType& rhs) {
    if (isError(lhs) || isError(rhs)) return true;

    // record anonim tidak dapat di-assign
    if ((lhs.type == TC_RECORD && lhs.anonymous) ||
        (rhs.type == TC_RECORD && rhs.anonymous)) {
        return false;
    }

    if (sameType(lhs, rhs)) {
        // subset value range: rhs harus berada dalam rentang lhs (best-effort)
        if (lhs.hasRange && rhs.hasRange) {
            return rhs.low >= lhs.low && rhs.high <= lhs.high;
        }
        return true;
    }

    // Real <- Integer (promosi)
    if (lhs.type == TC_REAL && (rhs.type == TC_INTEGER || rhs.isSubrange)) {
        return true;
    }

    return false;
}

// === Inferensi operator biner === //

SemanticType resultBinary(const string& op, const SemanticType& a,
                          const SemanticType& b, TypeError& err) {
    err = TypeError::None;

    // operand error
    if (isError(a) || isError(b)) {
        err = TypeError::None;
        return SemanticType{};
    }

    // logika: and / or
    if (op == "andsy" || op == "orsy") {
        if (!isBoolean(a) || !isBoolean(b)) err = TypeError::NonBoolean;
        return makeBasic("boolean");
    }

    // div / mod: integer saja
    if (op == "idiv" || op == "imod") {
        bool aInt = a.type == TC_INTEGER || a.isSubrange;
        bool bInt = b.type == TC_INTEGER || b.isSubrange;
        if (!aInt || !bInt) err = TypeError::NonInteger;
        return makeBasic("integer");
    }

    // pembagian real '/': selalu real
    if (op == "rdiv") {
        if (!isNumeric(a) || !isNumeric(b)) err = TypeError::NonNumeric;
        return makeBasic("real");
    }

    // aritmetika + - *
    if (op == "plus" || op == "minus" || op == "times") {
        if (!isNumeric(a) || !isNumeric(b)) {
            err = TypeError::NonNumeric;
            return makeBasic("integer");
        }
        if (a.type == TC_REAL || b.type == TC_REAL) return makeBasic("real");
        return makeBasic("integer");
    }

    // operator tak dikenal -> kembalikan operand kiri
    return a;
}

SemanticType resultRelational(const SemanticType& a, const SemanticType& b,
                              TypeError& err) {
    err = TypeError::None;
    if (!isError(a) && !isError(b) && !compatible(a, b)) {
        err = TypeError::IncompatibleOperand;
    }
    return makeBasic("boolean");
}

// === Validasi konteks === //

void validateRangeBounds(const SemanticType& lo, const SemanticType& hi,
                         bool& realBound, bool& orderBad) {
    realBound = (lo.type == TC_REAL || hi.type == TC_REAL);
    orderBad = false;
    if (!realBound && lo.hasRange && hi.hasRange) {
        orderBad = lo.low > hi.high;
    }
}

bool validArrayIndex(const SemanticType& idx) {
    return idx.type != TC_REAL;
}

// === Factory === //

SemanticType makeBasic(const string& name) {
    SemanticType t;
    string lowered = toLowerCopy(name);
    if (lowered == "integer") t.type = TC_INTEGER;
    else if (lowered == "real") t.type = TC_REAL;
    else if (lowered == "char") t.type = TC_CHAR;
    else if (lowered == "boolean") t.type = TC_BOOLEAN;
    else if (lowered == "string") t.type = TC_STRING;
    else if (lowered == "void") t.type = TC_NOTYPE;
    else t.type = TC_NOTYPE;
    t.name = lowered;
    return t;
}

SemanticType makeCode(int type) {
    SemanticType t;
    t.type = type;
    return t;
}

}
