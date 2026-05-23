# Tugas Besar IF2224 — Kompiler Bahasa Arion

## Identitas Kelompok

| Nama                    | NIM      |
| ----------------------- | -------- |
| Stevanus Agustaf Wongso | 13524020 |
| Renuno Yuqa Frinardi    | 13524080 |
| Valentino Daniel Kusumo | 13524104 |
| Michael James Liman     | 13524106 |

---

## Deskripsi Program

Program ini adalah implementasi **kompiler bahasa pemrograman Arion** yang terdiri dari tiga tahap utama:

**Milestone 1 — Analisis Leksikal (Lexer/Tokenizer)**

Membaca kode sumber dan memecahnya menjadi urutan token menggunakan **Deterministic Finite Automaton (DFA)**. Token yang dikenali meliputi:

- **Kata kunci (Keywords):** `PROGRAM`, `VAR`, `BEGIN`, `END`, `IF`, `WHILE`, `FOR`, `FUNCTION`, `PROCEDURE`, dan lainnya (32 kata kunci total)
- **Identifier:** nama variabel dan fungsi buatan pengguna
- **Konstanta:** bilangan bulat, bilangan real, karakter, dan string
- **Operator:** aritmatika (`+`, `-`, `*`, `/`), relasional (`<`, `>`, `<=`, `>=`, `<>`, `==`), dan penugasan (`:=`)
- **Delimiter:** tanda kurung, kurung siku, koma, titik koma, titik dua, dan titik
- **Komentar:** format `{...}` dan `(*...*)`
- **Token tidak dikenal:** karakter yang tidak valid akan ditandai sebagai `unknown`

**Milestone 2 — Analisis Sintaksis (Parser)**

Menerima daftar token hasil lexer dan membangun **Parse Tree** menggunakan algoritma **Recursive Descent**. Parser memeriksa apakah urutan token sesuai dengan grammar bahasa Arion dan melaporkan syntax error secara informatif apabila ditemukan pelanggaran aturan grammar. Grammar yang disediakan dalam proses Parsing ini terbagi menjadi ke beberapa jenis dengan spesifiknya:

- **Program utama:** `<program>`, `<program-header>`
- **Declaration Part:** `<declaration-part>`
- **Const Declaration:** `<const-declaration>`, `<constant>`
- **Type Declaration:** `<type-declaration>`, `<type>`, `<array-type>`, `<range>`, `<enumerated>`, `<record-type>`, `<field-list>`
- **Var Declaration:** `<var-declaration>`, `<identifier-list>`
- **Subprogram Declaration:** `<subprogram-declaration>`, `<procedure-declaration>`, `<function-declaration>`, `<block>`, `<formal-parameter-list>`, `<parameter-group>`
- **Compound Statement:** `<compound-statement>`, `<statement-list>`
- **Statement:** `<statement>`, `<case-statement>`, `<for-statement>`
- **Assignment & Variable:** `<assignment-statement>`, `<variable>`, `<component-variable>`, `<index-list>`
- **Control Flow:** `<if-statement>`, `<case-statement>`, `<case-block>`, `<while-statement>`, `<repeat-statement>`, `<for-statement>`
- **Procedure/Function Call:** `<procedure/function-call>`, `<parameter-list>`
- **Expression:** `<expression>`, `<simple-expression>`, `<term>`, `<factor>`
- **Operator:** `<relational-operator>`, `<additive-operator>`, `<multiplicative-operator>`

**Milestone 3 — Analisis Semantik**

Menerima Parse Tree hasil parser dan melakukan **analisis semantik** menggunakan visitor pattern. Parse Tree diubah menjadi **Abstract Syntax Tree (AST)** yang lebih ringkas, lalu dianalisis untuk memeriksa kebenaran semantik program. Hasil analisis mencakup Decorated AST (AST beranotasi tipe) serta dump tiga tabel simbol: `tab` (identifier), `btab` (blok/scope), dan `atab` (tipe array). Pengecekan semantik yang dilakukan meliputi:

- **Deklarasi:** redeclaration variabel, konstanta, tipe, prosedur, fungsi, dan field record
- **Identifier:** penggunaan identifier yang belum dideklarasikan
- **Tipe Assignment:** kompatibilitas tipe pada operator `:=`, termasuk widening `integer → real`
- **Konstanta:** assignment ke konstanta dan penggunaan konstanta sebagai variabel for-loop dicegah
- **Kondisi Boolean:** kondisi `if`, `while`, dan `repeat-until` harus bertipe boolean
- **For-loop:** variabel loop harus ordinal; nilai awal/akhir harus assignment-compatible dengan variabel loop
- **Indeks Array:** tipe indeks harus ordinal (bukan real, array, atau record); mendukung ekspresi penuh sebagai indeks
- **Record:** akses field yang tidak terdefinisi dilaporkan sebagai error
- **Operasi Aritmatika:** `div`/`mod` butuh integer; `/` menghasilkan real; `+`/`-`/`*` butuh numerik
- **Operasi Boolean:** `and`/`or`/`not` butuh operand boolean; unary `+`/`-` butuh operand numerik
- **String:** operator `+` pada dua string menghasilkan konkatenasi string
- **Case:** label duplikat dan ketidaksesuaian tipe label dengan ekspresi selector
- **Subrange:** batas tidak boleh real; batas bawah tidak boleh melebihi batas atas
- **Prosedur/Fungsi:** jumlah argumen dan kompatibilitas tipe parameter pada pemanggilan
- **Enumerated:** identifier enum didaftarkan sebagai konstanta ordinal di tabel simbol

---

## Requirements

- Sistem operasi Linux/Unix untuk MakeFile
- Compiler `g++` dengan dukungan C++17
- `GNU Make`

---

## Cara Penggunaan Program

### 1. Build Program

```bash
make all
```

Binary hasil kompilasi akan tersimpan di `bin/compiler`.

### 2. Menjalankan Program

```bash
make run
```

atau jalankan langsung:

```bash
./bin/compiler
```

### 3. Membersihkan Build

```bash
make clean
```

### 4. Alur Penggunaan

1. Program akan meminta mode input:
   - **[1] Source code file** — jalankan lexer + parser + semantic (file `.txt` berisi kode sumber Arion)
   - **[2] Token file** — lewati lexer, langsung jalankan parser + semantic (file berisi token)
2. Masukkan nama file input dengan path relatif dari direktori `test/`:
   - Milestone 1 : `M1/input1.txt`
   - Milestone 2 : `M2/input-1.txt`
   - Milestone 3 : `M3/comprehensive.txt`
3. Program menampilkan hasil secara berurutan:
   - **Token list** (mode 1) — hasil tokenisasi
   - **Parse Tree** — struktur sintaksis
   - **Abstract Syntax Tree (AST)** — representasi semantik ringkas
   - **Decorated AST** — AST yang dianotasi tipe dan informasi tabel simbol
   - **Symbol Table / Block Table / Array Table** — dump tabel simbol lengkap
   - **Daftar error semantik** (jika ada)
4. Di setiap tahap, program menanyakan apakah hasil ingin diekspor ke file.
   - Jika ya, masukkan nama file output (akan disimpan di direktori `test/` sesuai path yang dimasukkan).

---

## Pembagian Tugas

### Milestone 1 & 2

| Nama                    | Tugas                                                        |
| ----------------------- | ------------------------------------------------------------ |
| Stevanus Agustaf Wongso | Implementasi Parser dan debugging                            |
| Renuno Yuqa Frinardi    | Implementasi StatementSubprogramParser dan penulisan laporan |
| Valentino Daniel Kusumo | Implementasi DeclarationParser dan penulisan laporan         |
| Michael James Liman     | Implementasi ExpressionParser dan penulisan laporan          |

### Milestone 3

| Nama                    | Tugas                                                                   |
| ----------------------- | ----------------------------------------------------------------------- |
| Stevanus Agustaf Wongso | Implementasi SemanticAnalyzer, SymbolTable, AstBuilder, dan debugging   |
| Renuno Yuqa Frinardi    | Implementasi pengecekan control flow dan penulisan laporan              |
| Valentino Daniel Kusumo | Implementasi pengecekan tipe dan deklarasi, penulisan laporan           |
| Michael James Liman     | Implementasi TypeCheck, AstDecorator, AstPrinter, dan penulisan laporan |
