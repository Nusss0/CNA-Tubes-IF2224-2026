#pragma once

#include <string>
#include <cstdint>

struct RuntimeValue {
    enum class Kind : uint8_t { NONE, INTEGER, REAL, BOOLEAN, CHAR, STRING };

    Kind kind = Kind::NONE;

    long long intVal  = 0;
    double    realVal = 0.0;
    bool      boolVal = false;
    std::string strVal;

    static RuntimeValue none();
    static RuntimeValue integer(long long v);
    static RuntimeValue real(double v);
    static RuntimeValue boolean(bool v);
    static RuntimeValue chr(char v);
    static RuntimeValue str(const std::string& v);

    bool isTruthy() const;
    std::string toString() const;
    std::string realToString() const;
    bool equals(const RuntimeValue& other) const;
    int compare(const RuntimeValue& other) const; // -1 less, 0 equal, 1 greater
};
