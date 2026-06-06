#pragma once

#include "runtime_value.hpp"
#include "runtime_instruction.hpp"
#include <vector>
#include <sstream>
#include <string>
#include <cstddef>

// catatan per-frame untuk deteksi stack corruption & smashing
struct FrameGuard {
    int basePtr;      // bp frame ini
    int expectedTop;  // posisi stack pointer yg benar saat keluar (bp + size)
    int canaryRa;     // salinan return address saat masuk — deteksi smashing
};

struct RuntimeContext {
    std::vector<RuntimeValue> stack;
    int bp = 0;   // base pointer  (stack index of current frame start)
    int ra = -1;  // return address (1‑based instruction number, -1 = none)
    int dl = -1;  // dynamic link  (previous bp value)
    int frameDepth = 0;  // jumlah stack frame aktif — cegah overflow rekursi
    std::vector<FrameGuard> frames;  // bookkeeping deteksi corruption/smashing

    // instruction pointer (index into the program vector, 0‑based)
    size_t ip = 0;

    // program loaded
    std::vector<RuntimeInstruction> program;

    // output buffer
    std::ostringstream output;

    bool halted = false;

    void reset();
};
