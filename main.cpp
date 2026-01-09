#include "raylib.h"
#include <vector>
#include <string>
#include <math.h>
#include <iostream>

// Donanım komutlarını çalıştırmak için yardımcı (Ses kontrolü vb.)
void RunSystemCommand(std::string cmd) {
    system(cmd.c_str());
}

int main() {
    // 1. AYARLAR & BAŞLATMA
    const int screenWidth = 1280;
    const int screenHeight = 720;

    // Sanal makine uyumluluğu için bayraklar
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Lumanovos OS - C++ Native Core");
    
    // Mouse'u aktif et
    ShowCursor();
    SetTargetFPS(60);

    // Menü Verileri
    std::vector<std::string> categories = {"OYUNLAR", "MEDYA", "AYARLAR", "GUC"};
    int currentCat = 0;
    
    // Animasyon Değişkenleri
    float selectorX = 100.0f;
    float targetSelectorX = 100.0f;
    float waveOffset = 0.0f;

    // 2. ANA DÖNGÜ
    while (!WindowShouldClose()) {
        // --- GİRDİ KONTROLÜ (INPUT) ---
        if (IsKeyPressed(KEY_RIGHT)) {
            currentCat = (currentCat + 1) % categories.size();
        }
        if (IsKeyPressed(KEY_LEFT)) {
            currentCat = (currentCat - 1 + categories.size()) % categories.size();
        }

        // Ses Kontrolü (Yukarı/Aşağı Ok Tuşları)
        if (IsKeyPressed(KEY_UP)) RunSystemCommand("amixer -q set Master 5%+");
        if (IsKeyPressed(KEY_DOWN)) RunSystemCommand("amixer -q set Master 5%-");

        // --- GÜNCELLEME (UPDATE) ---
        targetSelectorX = 100.0f + (currentCat * 250.0f);
        // Yumuşak geçiş animasyonu (Lerp)
        selectorX += (targetSelectorX - selectorX) * 0.15f;
        waveOffset += 0.05f;

        // --- ÇİZİM (DRAW) ---
        BeginDrawing();
            ClearBackground({15, 15, 25, 255}); // Koyu Modern Arka Plan

            // A. Dinamik Arka Plan Animasyonu (C++ ile %0 CPU yükü)
            for (int i = 0; i < screenWidth; i += 4) {
                float y = sin(waveOffset + i * 0.005f) * 60.0f + (screenHeight / 2.0f);
                DrawCircleGradient(i, (int)y, 2, {0, 200, 255, 150}, {0, 0, 0, 0});
            }

            // B. Üst Menü Kategorileri
            for (int i = 0; i < categories.size(); i++) {
                Color textColor = (i == currentCat) ? WHITE : GRAY;
                DrawText(categories[i].c_str(), 100 + (i * 250), 80, 35, textColor);
            }

            // C. Seçim Çizgisi (Alt Çizgi Animasyonu)
            DrawRectangleV({selectorX, 130}, {150, 5}, {0, 255, 200, 255});

            // D. Orta Panel Bilgisi
            DrawText("LUMANOVOS NATIVE CORE", 450, 350, 30, {255, 255, 255, 50});
            DrawRectangleLinesEx({50, 200, screenWidth - 100, 400}, 2, {255, 255, 255, 20});

            // E. Alt Bilgi Çubuğu
            DrawRectangle(0, screenHeight - 40, screenWidth, 40, {0, 0, 0, 150});
            DrawText("OK Tuslari: Gezinti | ENTER: Sec | YUKARI/ASAGI: Ses", 20, screenHeight - 30, 18, LIGHTGRAY);
            
            // Performans Göstergesi (FPS)
            DrawFPS(screenWidth - 80, 10);

        EndDrawing();
    }

    // 3. KAPANIŞ
    CloseWindow();
    return 0;
}
