#!/bin/bash
echo "Derleme başlatılıyor..."

# 1. Masaüstü Ortamı (Main System)
g++ desktop.cpp -o main_system -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
if [ $? -eq 0 ]; then echo "Desktop derlendi."; else echo "Desktop hatası!"; exit 1; fi

# 2. Dosya Yöneticisi
g++ file_manager.cpp -o file_manager -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
if [ $? -eq 0 ]; then echo "Dosya Yöneticisi derlendi."; else echo "Dosya Yöneticisi hatası!"; exit 1; fi

# 3. Ayarlar
g++ settings.cpp -o settings -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
if [ $? -eq 0 ]; then echo "Ayarlar derlendi."; else echo "Ayarlar hatası!"; exit 1; fi

echo "Tüm bileşenler hazır."
