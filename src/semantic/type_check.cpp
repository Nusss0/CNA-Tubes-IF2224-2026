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

// classify

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

// naming

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

// relation

bool sameType(const SemanticType& a, const SemanticType& b) {
    if (isError(a) || isError(b)) return false;

    if (a.type == TC_STRING && b.type == TC_STRING) {
        if (a.hasLen && b.hasLen) return a.len == b.len;
        return true;
    }

    if (a.type == TC_RECORD && b.type == TC_RECORD) {
        if (a.anonymous || b.anonymous) return false;
        return !a.name.empty() && a.name == b.name;
    }

    if (a.type == b.type) return true;

    return false;
}

bool compatible(const SemanticType& a, const SemanticType& b) {
    if (isError(a) || isError(b)) return true;

    if ((a.type == TC_RECORD && a.anonymous) ||
        (b.type == TC_RECORD && b.anonymous)) {
        return false;
    }

    if (sameType(a, b)) return true;

    if ((a.name == "enumerated" || b.name == "enumerated") && a.type == b.type)
        return true;

    return false;
}

bool assignmentCompatible(const SemanticType& lhs, const SemanticType& rhs) {
    if (isError(lhs) || isError(rhs)) return true;

    if ((lhs.type == TC_RECORD && lhs.anonymous) ||
        (rhs.type == TC_RECORD && rhs.anonymous)) {
        return false;
    }

    if (sameType(lhs, rhs)) {
        if (lhs.hasRange && rhs.hasRange) {
            return rhs.low >= lhs.low && rhs.high <= lhs.high;
        }
        return true;
    }

    if (lhs.type == TC_REAL && (rhs.type == TC_INTEGER || rhs.isSubrange)) {
        return true;
    }

    return false;
}

// binary op

SemanticType resultBinary(const string& op, const SemanticType& a,
                          const SemanticType& b, TypeError& err) {
    err = TypeError::None;

    if (isError(a) || isError(b)) {
        err = TypeError::None;
        return SemanticType{};
    }

    if (op == "andsy" || op == "orsy") {
        if (!isBoolean(a) || !isBoolean(b)) err = TypeError::NonBoolean;
        return makeBasic("boolean");
    }

    if (op == "idiv" || op == "imod") {
        bool aInt = a.type == TC_INTEGER || a.isSubrange;
        bool bInt = b.type == TC_INTEGER || b.isSubrange;
        if (!aInt || !bInt) err = TypeError::NonInteger;
        return makeBasic("integer");
    }

    if (op == "rdiv") {
        if (!isNumeric(a) || !isNumeric(b)) err = TypeError::NonNumeric;
        return makeBasic("real");
    }

    if (op == "plus" && a.type == TC_STRING && b.type == TC_STRING) {
        SemanticType result = makeBasic("string");
        if (a.hasLen && b.hasLen) {
            result.len    = a.len + b.len;
            result.hasLen = true;
        }
        return result;
    }

    if (op == "plus" || op == "minus" || op == "times") {
        if (!isNumeric(a) || !isNumeric(b)) {
            err = TypeError::NonNumeric;
            return makeBasic("integer");
        }
        if (a.type == TC_REAL || b.type == TC_REAL) return makeBasic("real");
        return makeBasic("integer");
    }

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

// validation

void validateRangeBounds(const SemanticType& lo, const SemanticType& hi,
                         bool& realBound, bool& orderBad) {
    realBound = (lo.type == TC_REAL || hi.type == TC_REAL);
    orderBad = false;
    if (!realBound && lo.hasRange && hi.hasRange) {
        orderBad = lo.low > hi.high;
    }
}

bool validArrayIndex(const SemanticType& idx) {
    if (idx.type == TC_NOTYPE) return true;
    if (idx.type == TC_REAL)   return false;
    if (idx.type == TC_ARRAY)  return false;
    if (idx.type == TC_RECORD) return false;
    return true;
}

// factory

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
