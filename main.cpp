#include "raylib.h"
#include <vector>
#include <string>
#include <math.h>
#include <iostream>
#include <cstdio>
#include <memory>
#include <dirent.h> // Dosya yöneticisi için

// Sistem komutu çıktı yakalayıcı
std::string Exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return "Hata!";
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) result += buffer;
    return result;
}

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Lumanovos Pro OS");
    SetTargetFPS(60);

    std::vector<std::string> cats = {"DOSYALAR", "AG AYARLARI", "DONANIM", "GUC"};
    int currentCat = 0;
    int currentItem = 0;
    std::string statusMsg = "Lumanovos OS Hazir";

    // Dosya listesi için değişkenler
    std::vector<std::string> fileList;

    while (!WindowShouldClose()) {
        // --- INPUT ---
        if (IsKeyPressed(KEY_RIGHT)) { currentCat = (currentCat + 1) % cats.size(); currentItem = 0; }
        if (IsKeyPressed(KEY_LEFT)) { currentCat = (currentCat - 1 + cats.size()) % cats.size(); currentItem = 0; }
        
        // --- AKSİYONLAR ---
        if (IsKeyPressed(KEY_ENTER)) {
            if (cats[currentCat] == "DOSYALAR") {
                fileList.clear();
                DIR *dir; struct dirent *ent;
                if ((dir = opendir("/home/lomen")) != NULL) {
                    while ((ent = readdir(dir)) != NULL) fileList.push_back(ent->d_name);
                    closedir(dir);
                }
                statusMsg = "Dosyalar Listelendi";
            }
            if (cats[currentCat] == "AG AYARLARI") {
                statusMsg = "Taraniyor... IP: " + Exec("hostname -I");
            }
            if (cats[currentCat] == "DONANIM") {
                statusMsg = "Disk: " + Exec("df -h / | awk 'NR==2 {print $4}'") + " Bos";
            }
            if (cats[currentCat] == "GUC") system("sudo reboot");
        }

        // --- DRAW ---
        BeginDrawing();
            ClearBackground({15, 15, 25, 255});
            
            // Üst Menü
            for (int i = 0; i < cats.size(); i++) {
                DrawText(cats[i].c_str(), 50 + (i * 250), 50, 25, (i == currentCat) ? GREEN : GRAY);
            }

            // İçerik Alanı
            DrawRectangleLines(50, 120, 1180, 500, DARKGRAY);
            
            if (cats[currentCat] == "DOSYALAR" && !fileList.empty()) {
                for(int i=0; i<fmin(fileList.size(), 10); i++) {
                    DrawText(fileList[i].c_str(), 80, 150 + (i*35), 20, LIGHTGRAY);
                }
            } else {
                DrawText("Secmek ve Calistirmak icin ENTER'a basin", 400, 300, 20, GRAY);
            }

            // Alt Bilgi Barı
            DrawRectangle(0, 660, 1280, 60, {30, 30, 45, 255});
            DrawText(statusMsg.c_str(), 20, 680, 20, GREEN);
            
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
