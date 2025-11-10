# Panduan Install & Uninstall LEX Editor

## 🔧 Persiapan

Pastikan file `cmake/uninstall.cmake.in` sudah ada di folder project Anda.

## 📦 Build & Install

```bash
# 1. Buat direktori build
mkdir -p build
cd build

# 2. Generate build files dengan CMake
cmake ..

# 3. Compile project
make -j$(nproc) 

# 4. Install ke sistem (memerlukan sudo)
sudo make install
```

Binary akan terinstall di `/usr/local/bin/lex` secara default.

## 🔍 Cek Instalasi

Untuk mengecek dimana binary terinstall:

```bash
# cek binary lex
which lex
```



# Cek lokasi spesifik
ls -lh /usr/local/bin/lex
ls -lh /usr/bin/lex
```

## 🗑️ Uninstall (3 Metode)

### Metode 1: Uninstall Normal (Recommended)

Menggunakan file `install_manifest.txt` yang dibuat saat install:

```bash
cd build
sudo make uninstall
```

**Catatan:** Metode ini hanya bekerja jika:
- Anda masih punya folder `build/` 
- File `install_manifest.txt` masih ada
- Anda belum menghapus folder build

### Metode 2: Force Uninstall

Jika `install_manifest.txt` hilang atau metode 1 gagal:

```bash
cd build
sudo make force-uninstall
```

Metode ini akan mencoba menghapus binary dari semua lokasi umum:
- `/usr/local/bin/lex`
- `/usr/bin/lex`
- `${CMAKE_INSTALL_PREFIX}/bin/lex`

### Metode 3: Manual Uninstall

Jika kedua metode di atas gagal:

```bash
# Hapus secara manual
sudo rm -f /usr/local/bin/lex
sudo rm -f /usr/bin/lex

# Verifikasi sudah terhapus
which lex
# Output: (tidak ada output jika berhasil)
```

## 🧹 Purge (Uninstall + Hapus Build)

Untuk uninstall dan sekaligus menghapus folder build:

```bash
cd build
sudo make purge
```

**Peringatan:** Perintah ini akan:
1. Menjalankan uninstall
2. Menghapus seluruh folder `build/`

## 📍 Mengubah Lokasi Install

Jika ingin install ke lokasi berbeda (contoh: `/usr/bin`):

```bash
# Saat generate CMake
cmake -DCMAKE_INSTALL_PREFIX=/usr ..

# Install
sudo make install

# Binary akan ada di: /usr/bin/lex
```

## ❓ Troubleshooting

### Problem: "Binary masih ada setelah uninstall"

**Solusi:**

```bash
# 1. Cari semua instance lex
sudo find / -name "lex" -type f 2>/dev/null

# 2. Cek lokasi mana yang executable
which lex

# 3. Hapus manual
sudo rm -f /path/to/lex
```

### Problem: "Permission denied saat uninstall"

**Solusi:** Pastikan menggunakan `sudo`

```bash
sudo make uninstall
# atau
sudo make force-uninstall
```

### Problem: "install_manifest.txt tidak ditemukan"

**Solusi:** Gunakan force-uninstall

```bash
cd build
sudo make force-uninstall
```

### Problem: "Conflict dengan program 'lex' yang sudah ada"

Sistem Unix/Linux biasanya sudah memiliki program `lex` (flex lexical analyzer).

**Solusi 1:** Rename binary Anda

Edit `CMakeLists.txt`:
```cmake
project(mylex VERSION 0.5.2 LANGUAGES C)  # Ganti dari 'lex' ke 'mylex'
```

**Solusi 2:** Install ke lokasi lokal

```bash
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make
make install

# Binary akan ada di: ~/.local/bin/mylex
# Tambahkan ke PATH jika perlu
export PATH="$HOME/.local/bin:$PATH"
```

## 📝 Workflow Lengkap

```bash
# Build pertama kali
mkdir build && cd build
cmake ..
make
sudo make install

# Cek instalasi
make check-install
which lex

# Test program
lex --version
lex myfile.txt

# Uninstall
sudo make uninstall

# Atau force uninstall jika perlu
sudo make force-uninstall

# Rebuild setelah perubahan kode
cd build
make
sudo make install

# Clean total (uninstall + hapus build)
sudo make purge
```

## ✅ Verifikasi Uninstall Berhasil

Setelah uninstall, pastikan binary benar-benar terhapus:

```bash
# Test 1: which seharusnya tidak menemukan
which lex
# Output: (kosong atau "lex not found")

# Test 2: Coba jalankan
lex
# Output: "bash: lex: command not found"

# Test 3: Cek manual di lokasi install
ls /usr/local/bin/lex
# Output: "No such file or directory"
```

## 🎯 Target CMake yang Tersedia

| Target | Perintah | Deskripsi |
|--------|----------|-----------|
| Build | `make` | Compile project |
| Install | `sudo make install` | Install binary ke sistem |
| Check | `make check-install` | Cek lokasi instalasi |
| Uninstall | `sudo make uninstall` | Hapus instalasi (butuh manifest) |
| Force Uninstall | `sudo make force-uninstall` | Paksa hapus dari lokasi umum |
| Purge | `sudo make purge` | Uninstall + hapus build directory |

## 🔐 Catatan Keamanan

- Selalu gunakan `sudo` untuk install/uninstall ke `/usr/local/bin` atau `/usr/bin`
- Untuk instalasi tanpa sudo, gunakan prefix lokal:
  ```bash
  cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
  ```
- Verifikasi binary sebelum install:
  ```bash
  # Cek binary yang akan diinstall
  file ./lex
  ./lex --version
  ```