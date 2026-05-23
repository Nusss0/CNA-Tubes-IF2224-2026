#pragma once

#include "../std.hpp"
#include "ast_node.hpp"
#include "symbol_table.hpp"

// ast decorator
class AstDecorator {
public:
    explicit AstDecorator(const SymbolTable& sym);

    void decorate(const AstNodePtr& root);

private:
    const SymbolTable& symbols;
    int currBlock = 0;
    int currLev = 0;

    // tracking record context
    vector<int> recordBlocks;     // btab indices yg record (urut deklarasi)
    size_t recordBlockCursor = 0; // posisi RecordType berikutnya yg dikunjungi
    vector<int> recordContext;    // stack block record yg sedang aktif

    void visit(const AstNodePtr& node);

    // lookup helper
    void annotateFromTab(const AstNodePtr& node, const string& id);
    // lookup terbatas pada satu btab block (utk field record)
    int lookupInBlock(int blockIdx, const string& id) const;
    string typeNameFromCode(int code) const;
};
