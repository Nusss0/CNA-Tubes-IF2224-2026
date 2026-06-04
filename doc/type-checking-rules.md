# Type Checking — Rule Table (Milestone 3)

Modul: `src/semantic/type_check.cpp` (+ `type_check.hpp`)
Acuan spec: Section II.D (hal. 14–17), Lampiran C.

Modul ini berisi **aturan tipe murni** (stateless): mengambil keputusan tipe
tetapi tidak melaporkan error. Perakitan pesan + `reportError()` dilakukan di
`SemanticAnalyzer`, sehingga modul aturan mudah diuji terpisah. Semua pesan
akhir mengikuti style guide global: `[ERROR] <pesan> !`.

## Tabel 1 — Klasifikasi Tipe

| Kategori   | Tipe       | Kode (`TypeCode`)        | Ordinal? |
| ---------- | ---------- | ------------------------ | -------- |
| Simple     | Integer    | `TC_INTEGER`             | ya       |
| Simple     | Real       | `TC_REAL`                | tidak    |
| Simple     | Char       | `TC_CHAR`                | ya       |
| Simple     | Boolean    | `TC_BOOLEAN`             | ya       |
| Simple     | String     | `TC_STRING`              | tidak    |
| Simple     | Subrange   | tipe basis (Integer/Char/Boolean) + `low`/`high` | ya |
| Simple     | Enumerated | tipe anggota (semua ident wajib sama) | ya |
| Structured | Array      | `TC_ARRAY` (index ordinal, ≠ Real) | – |
| Structured | Record     | `TC_RECORD` (named / anonymous)    | – |

## Tabel 2 — Type Compatibility (`compatible(A, B)`)

| Kondisi                                            | Hasil          |
| -------------------------------------------------- | -------------- |
| `A` atau `B` bertipe `NOTYPE` (error sebelumnya)   | compatible (supresi) |
| `A.type == B.type` dan nama sama                   | compatible     |
| `A` & `B` subrange dengan tipe basis sama (Int/Char/Bool) | compatible |
| Salah satu subrange, satunya tipe basisnya         | compatible     |
| `A` & `B` String dengan panjang sama               | compatible     |
| `A` & `B` numeric (integer/real/subrange)          | compatible     |
| Salah satu Record **anonymous**                    | **incompatible** |
| Lainnya                                            | incompatible   |

## Tabel 3 — Assignment Compatibility (`lhs := rhs`)

| LHS                  | RHS                                   | Hasil          |
| -------------------- | ------------------------------------- | -------------- |
| `T`                  | `T` (sameType)                        | OK             |
| Real                 | Integer / subrange integer            | OK (promosi)   |
| Integer              | Real                                  | ERROR          |
| Subrange `[l..h]`    | nilai/subrange dgn rentang ⊆ `[l..h]` | OK             |
| Subrange `[l..h]`    | nilai di luar `[l..h]`                | ERROR (out of range) |
| String(n)            | String(m), m ≠ n                      | ERROR          |
| Record anonymous     | apa pun                               | ERROR          |
| `NOTYPE`             | apa pun                               | OK (supresi error berantai) |

Catatan: subset value range dicek best-effort bila kedua sisi membawa rentang
yang diketahui (`hasRange`) — literal integer membawa nilainya, variabel
subrange membawa batas deklarasinya.

## Tabel 4 — Inferensi Tipe Operator (`resultBinary` / `resultRelational`)

| Operator            | Operand valid              | Tipe hasil                              | Error bila gagal      |
| ------------------- | -------------------------- | --------------------------------------- | --------------------- |
| `+` `-` `*`         | numeric × numeric          | Real bila ada Real, selain itu Integer  | `NonNumeric`          |
| `/` (rdiv)          | numeric × numeric          | Real                                    | `NonNumeric`          |
| `div` `mod`         | Integer × Integer          | Integer                                 | `NonInteger`          |
| `and` `or`          | Boolean × Boolean          | Boolean                                 | `NonBoolean`          |
| `= <> < <= > >=`    | operand compatible         | Boolean                                 | `IncompatibleOperand` |

## Tabel 5 — Validasi Konteks

| Konteks              | Aturan                       | Pesan error (tanpa wrapper)              |
| -------------------- | ---------------------------- | ---------------------------------------- |
| if / while condition | tipe = Boolean               | `if-condition must be boolean` / `while-condition must be boolean` |
| Range bound          | simple, non-Real             | `range bounds cannot be real`            |
| Range bound          | lower ≤ upper                | `range lower bound exceeds upper bound`  |
| Array index type     | ordinal, ≠ Real              | `array index type cannot be real`        |
| Enumerated members   | semua ident bertipe sama     | `enumerated members must have the same type` |
| for control / batas  | numeric                      | `for start/end expression must be numeric` |

Semua pesan di atas dibungkus menjadi `[ERROR] <pesan> !` oleh `reportError()`.

## Pemetaan ke API modul

| Fungsi                              | Aturan terkait                  |
| ----------------------------------- | ------------------------------- |
| `isSimple/isOrdinal/isStructured`   | Tabel 1                         |
| `sameType`                          | Tabel 2 (identik/subrange/string) |
| `compatible`                        | Tabel 2                         |
| `assignmentCompatible`              | Tabel 3                         |
| `resultBinary`                      | Tabel 4 (aritmetika/logika)     |
| `resultRelational`                  | Tabel 4 (relasional)            |
| `validateRangeBounds`               | Tabel 5 (range)                 |
| `validArrayIndex`                   | Tabel 5 (array index)           |
