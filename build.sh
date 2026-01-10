#!/bin/bash
# LumanovOS Build Script
# Tüm bileşenleri derler

set -e

echo "======================================"
echo "  LumanovOS Build System"
echo "======================================"

RAYLIB_FLAGS="-lraylib -lGL -lm -lpthread -ldl -lrt -lX11"
CXX="g++"
CXXFLAGS="-O2 -Wall"

# Dizin oluştur
mkdir -p bin

echo ""
echo "[1/3] UI kütüphanesi derleniyor..."
$CXX $CXXFLAGS -c libs/ui/ui.cpp -o libs/ui/ui.o
echo "      ✓ libs/ui/ui.o"

echo ""
echo "[2/3] Shell derleniyor..."
$CXX $CXXFLAGS core/shell.cpp libs/ui/ui.o -o bin/lumanovos-shell $RAYLIB_FLAGS
echo "      ✓ bin/lumanovos-shell"

echo ""
echo "[3/3] Uygulamalar derleniyor..."

for app in files settings terminal notepad browser monitor; do
    if [ -f "apps/$app/main.cpp" ]; then
        $CXX $CXXFLAGS apps/$app/main.cpp libs/ui/ui.o -o apps/$app/$app $RAYLIB_FLAGS
        echo "      ✓ apps/$app/$app"
    else
        echo "      - apps/$app/main.cpp bulunamadı, atlanıyor"
    fi
done

echo ""
echo "======================================"
echo "  Build tamamlandı!"
echo "======================================"
echo ""
echo "Çalıştırmak için: ./bin/lumanovos-shell"
