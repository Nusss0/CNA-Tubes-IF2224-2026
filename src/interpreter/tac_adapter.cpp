#include "tac_adapter.hpp"

using namespace std;

namespace TacAdapter {

RuntimeInstruction convert(const TacInstr& src) {
    RuntimeInstruction ri;
    ri.op     = src.op;
    ri.args   = src.args;
    ri.label  = src.label;
    ri.target = src.target;
    return ri;
}

vector<RuntimeInstruction> convertAll(const vector<TacInstr>& src) {
    vector<RuntimeInstruction> out;
    out.reserve(src.size());
    for (const auto& instr : src) {
        out.push_back(convert(instr));
    }
    return out;
}

}  // namespace TacAdapter
