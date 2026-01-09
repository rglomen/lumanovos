#include "raylib.h"
#include <vector>
#include <string>
#include <math.h>
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>

// Sistem komutlarını çalıştırıp çıktısını alan yardımcı fonksiyon
std::string Exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return "Hata!";
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    return result;
}

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Lumanovos OS - Full Pro Console");
    SetTargetFPS(60);

    // Menü Verileri
    std::vector<std::string> cats = {"OYUNLAR", "DOSYALAR", "AG", "AYARLAR", "GUC"};
    int currentCat = 0;
    int currentItem = 0;
    
    // Alt Menüler
    std::vector<std::vector<std::string>> items = {
        {"Retro Arch", "Lumanovos Arcade", "Cloud Gaming"},
        {"Dahili Depolama", "USB Sürücü", "Medya Klasörü"},
        {"Wi-Fi Tara", "IP Bilgisi", "Baglanti Kes"},
        {"Ses Seviyesi", "Mikrofon Durumu", "Sistem Bilgisi", "Cozunurluk"},
        {"Yeniden Baslat", "Sistemi Kapat"}
    };

    std::string statusMsg = "Sistem Hazır";
    float waveOffset = 0.0f;

    while (!WindowShouldClose()) {
        // --- INPUT ---
        if (IsKeyPressed(KEY_RIGHT)) { currentCat = (currentCat + 1) % cats.size(); currentItem = 0; }
        if (IsKeyPressed(KEY_LEFT)) { currentCat = (currentCat - 1 + cats.size()) % cats.size(); currentItem = 0; }
        if (IsKeyPressed(KEY_DOWN)) { currentItem = (currentItem + 1) % items[currentCat].size(); }
        if (IsKeyPressed(KEY_UP)) { currentItem = (currentItem - 1 + items[currentCat].size()) % items[currentCat].size(); }

        if (IsKeyPressed(KEY_ENTER)) {
            std::string selection = items[currentCat][currentItem];
            
            // DONANIM KONTROLLERİ
            if (selection == "Ses Seviyesi") {
                statusMsg = "Ses: " + Exec("amixer get Master | grep -o '[0-9]*%' | head -1");
            }
            else if (selection == "IP Bilgisi") {
                statusMsg = "IP: " + Exec("hostname -I | awk '{print $1}'");
            }
            else if (selection == "Dahili Depolama") {
                statusMsg = "Disk: " + Exec("df -h / | awk 'NR==2 {print $4}'") + " Bos";
            }
            else if (selection == "Sistem Bilgisi") {
                statusMsg = Exec("uname -sr");
            }
            else if (selection == "Sistemi Kapat") { system("sudo poweroff"); }
        }

        waveOffset += GetFrameTime() * 2.0f;

        // --- DRAW ---
        BeginDrawing();
            ClearBackground({18, 18, 28, 255});

            // Gelişmiş Arka Plan Dalgası
            for (int i = 0; i < screenWidth; i += 2) {
                float y1 = sin(waveOffset + i * 0.005f) * 40.0f + (screenHeight / 2.0f);
                float y2 = cos(waveOffset * 0.5f + i * 0.008f) * 20.0f + (screenHeight / 2.0f + 10);
                DrawPixel(i, (int)y1, {0, 150, 255, 100});
                DrawPixel(i, (int)y2, {0, 255, 200, 50});
            }

            // Kategoriler (Üst Menü)
            for (int i = 0; i < cats.size(); i++) {
                Color c = (i == currentCat) ? WHITE : DARKGRAY;
                DrawText(cats[i].c_str(), 100 + (i * 220), 80, 25, c);
                if (i == currentCat) DrawRectangle(100 + (i * 220), 115, 100, 3, {0, 255, 150, 255});
            }

            // Öğeler (Dikey Liste)
            for (int j = 0; j < items[currentCat].size(); j++) {
                Color c = (j == currentItem) ? {0, 255, 150, 255} : LIGHTGRAY;
                std::string label = (j == currentItem) ? "> " + items[currentCat][j] : "  " + items[currentCat][j];
                DrawText(label.c_str(), 120, 220 + (j * 50), 30, c);
            }

            // Alt Durum Çubuğu (PlayStation Tarzı Bildirim Alanı)
            DrawRectangle(0, screenHeight - 60, screenWidth, 60, {30, 30, 45, 255});
            DrawText(statusMsg.c_str(), 30, screenHeight - 40, 20, GREEN);
            DrawText(Exec("date +%H:%M").c_str(), screenWidth - 100, screenHeight - 40, 20, WHITE);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
