#!/bin/bash
# LumanovOS Build Script

echo "Bilesenler derleniyor..."
# Raylib baglantilari ile derleme
g++ desktop.cpp -o main_system -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
g++ file_manager.cpp -o file_manager -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
g++ settings.cpp -o settings -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

chmod +x main_system file_manager settings
echo "Derleme tamamlandi."
