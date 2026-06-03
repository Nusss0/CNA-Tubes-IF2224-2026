#include "runtime_value.hpp"
#include <stdexcept>
#include <sstream>

using namespace std;

RuntimeValue RuntimeValue::none() {
    RuntimeValue v;
    v.kind = Kind::NONE;
    return v;
}

RuntimeValue RuntimeValue::integer(long long v) {
    RuntimeValue rv;
    rv.kind = Kind::INTEGER;
    rv.intVal = v;
    return rv;
}

RuntimeValue RuntimeValue::real(double v) {
    RuntimeValue rv;
    rv.kind = Kind::REAL;
    rv.realVal = v;
    return rv;
}

RuntimeValue RuntimeValue::boolean(bool v) {
    RuntimeValue rv;
    rv.kind = Kind::BOOLEAN;
    rv.boolVal = v;
    return rv;
}

RuntimeValue RuntimeValue::chr(char v) {
    RuntimeValue rv;
    rv.kind = Kind::CHAR;
    rv.intVal = static_cast<long long>(v);
    return rv;
}

RuntimeValue RuntimeValue::str(const string& v) {
    RuntimeValue rv;
    rv.kind = Kind::STRING;
    rv.strVal = v;
    return rv;
}

bool RuntimeValue::isTruthy() const {
    switch (kind) {
        case Kind::NONE:    return false;
        case Kind::INTEGER: return intVal != 0;
        case Kind::REAL:    return realVal != 0.0;
        case Kind::BOOLEAN: return boolVal;
        case Kind::CHAR:    return intVal != 0;
        case Kind::STRING:  return !strVal.empty();
    }
    return false;
}

string RuntimeValue::toString() const {
    switch (kind) {
        case Kind::NONE:    return "none";
        case Kind::INTEGER: return to_string(intVal);
        case Kind::REAL:    return to_string(realVal);
        case Kind::BOOLEAN: return boolVal ? "true" : "false";
        case Kind::CHAR:    return string(1, static_cast<char>(intVal));
        case Kind::STRING:  return strVal;
    }
    return "";
}

bool RuntimeValue::equals(const RuntimeValue& other) const {
    if (kind != other.kind) return false;
    switch (kind) {
        case Kind::NONE:    return true;
        case Kind::INTEGER: return intVal == other.intVal;
        case Kind::REAL:    return realVal == other.realVal;
        case Kind::BOOLEAN: return boolVal == other.boolVal;
        case Kind::CHAR:    return intVal == other.intVal;
        case Kind::STRING:  return strVal == other.strVal;
    }
    return false;
}

int RuntimeValue::compare(const RuntimeValue& other) const {
    double a = 0.0, b = 0.0;
    switch (kind) {
        case Kind::INTEGER: a = static_cast<double>(intVal); break;
        case Kind::REAL:    a = realVal; break;
        case Kind::BOOLEAN: a = boolVal ? 1.0 : 0.0; break;
        case Kind::CHAR:    a = static_cast<double>(intVal); break;
        default: return 0;
    }
    switch (other.kind) {
        case Kind::INTEGER: b = static_cast<double>(other.intVal); break;
        case Kind::REAL:    b = other.realVal; break;
        case Kind::BOOLEAN: b = other.boolVal ? 1.0 : 0.0; break;
        case Kind::CHAR:    b = static_cast<double>(other.intVal); break;
        default: return 0;
    }
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}
