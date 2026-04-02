# Tugas Besar IF2224 — Analisis Leksikal (Lexer/Tokenizer)

## Identitas Kelompok

| Nama                    | NIM      |
| ----------------------- | -------- |
| Stevanus Agustaf Wongso | 13524020 |
| Renuno Yuqa Frinardi    | 13524080 |
| Valentino Daniel Kusumo | 13524104 |
| Michael James Liman     | 13524106 |

---

## Deskripsi Program

Program ini adalah sebuah **Analisis Leksikal (Lexer/Tokenizer)** untuk bahasa pemrograman Arion. Program membaca kode sumber, lalu memecahnya menjadi urutan token menggunakan **Deterministic Finite Automaton (DFA)**.

Token yang dikenali meliputi:

- **Kata kunci (Keywords):** `PROGRAM`, `VAR`, `BEGIN`, `END`, `IF`, `WHILE`, `FOR`, `FUNCTION`, `PROCEDURE`, dan lainnya (32 kata kunci total)
- **Identifier:** nama variabel dan fungsi buatan pengguna
- **Konstanta:** bilangan bulat, bilangan real, karakter, dan string
- **Operator:** aritmatika (`+`, `-`, `*`, `/`), relasional (`<`, `>`, `<=`, `>=`, `<>`, `==`), dan penugasan (`:=`)
- **Delimiter:** tanda kurung, kurung siku, koma, titik koma, titik dua, dan titik
- **Komentar:** format `{...}` dan `(*...*)`
- **Token tidak dikenal:** karakter yang tidak valid akan ditandai sebagai `unknown`

Hasil tokenisasi dapat ditampilkan di konsol maupun diekspor ke file teks.

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
   - File input harus berada di direktori `test/M1/`.
   - Contoh: masukkan `input1.txt` untuk memproses `test/M1/input1.txt`.
2. Hasil tokenisasi akan ditampilkan di konsol.
3. Program akan menanyakan apakah hasil ingin diekspor ke file.
   - Jika ya, masukkan nama file output (akan disimpan di `test/M1/`).

---

## Pembagian Tugas

| Nama                    | Tugas                                 |
| ----------------------- | ------------------------------------- |
| Stevanus Agustaf Wongso | Implementasi kode program             |
| Renuno Yuqa Frinardi    | Perancangan DFA dan penulisan laporan |
| Valentino Daniel Kusumo | Perancangan DFA dan penulisan laporan |
| Michael James Liman     | Perancangan DFA dan penulisan laporan |
