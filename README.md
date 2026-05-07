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

Program ini adalah implementasi **kompiler bahasa pemrograman Arion** yang terdiri dari dua tahap utama:

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

Menerima daftar token hasil lexer dan membangun **Parse Tree** menggunakan algoritma **Recursive Descent**. Parser memeriksa apakah urutan token sesuai dengan grammar bahasa Arion dan melaporkan syntax error secara informatif apabila ditemukan pelanggaran aturan grammar. Grammar yang disesdiakan dalamproses Parsing ini terbagi menjadi ke beberapa jenis dengan spesifiknya:

- **Program utama:** `<program>`, `<program-header>`
- **Declaration Part:** `<declaration-part>`
- **Const Declaration:** `<const-declaration>`, `<constant> `
- **Type Declaration:** `<type-declaration>`, `<type>`, `<array-type>`, `<range>`, `<enumerated>`, `<record-type>`, `<field-list>` 
- **Var Declaration:** `<var-declaration>`, `<identifier-list>`
- **Subprogram Declaration:** `<subprogram-declaration>`, `<procedure-declaration>`, `<function-declaration>`, `<block>`, `<formal-parameter-list>`, `<parameter-group>`
- **Compound Statement:** `<compound-statement>`, `<statement-list>`
- **Statement:** `<statement>`, `<case-statement>`, `<for-statement>`
- **Assignment & Variable :** `<assignment-statement>`, `<variable>`, `<component-variable>`, `<index-list> `
- **Control Flow:** `<if-statement>`, `<case-statement>`, `<case-block>`, `<while-statement>`, `<repeat-statement>`, `<for-statement>`
- **Procedure/Function Call:** `<procedure/function-call>`, `<parameter-list>`
- **Expression:** `<expression>`, `<simple-expression>`, `<term>`, `<factor>`
- **Operator:** `<relational-operator>`, `<additive-operator>`, `<multiplicative-operator> `

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

1. Program akan meminta nama file input (cukup nama file, tanpa path).
   - File input harus berada di direktori `test/M1/` untuk Milestone 1 atau `test/M2/` untuk Milestone 2.
   - Contoh: masukkan `input1.txt` untuk memproses file input yang sesuai.
2. Hasil tokenisasi atau Parse Tree akan ditampilkan di konsol.
3. Program akan menanyakan apakah hasil ingin diekspor ke file.
   - Jika ya, masukkan nama file output (akan disimpan di direktori test yang sesuai).

---

## Pembagian Tugas

| Nama                    | Tugas                                 |
| ----------------------- | ------------------------------------- |
| Stevanus Agustaf Wongso | Implementasi Parser dan debugging             |
| Renuno Yuqa Frinardi    | Implementasi StatementSubprogramParser dan penulisan laporan |
| Valentino Daniel Kusumo |  Implementasi DeclarationParser dan penulisan laporan |
| Michael James Liman     |  Implementasi ExpressionParser dan penulisan laporan |
