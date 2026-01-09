#include "raylib.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>


// Sistem Durumlari
enum SystemState { CHECK_INT, WIFI_SET, CHECK_UPD, SYNCING, READY };

struct Wifi {
  std::string ssid;
  std::string pwr;
};

// Global degiskenler
SystemState state = CHECK_INT;
std::string localV = "0.0.0";
std::string remoteV = "---";
std::vector<Wifi> networks;
std::string status = "Sistem Hazırlanıyor...";

bool HasConnection() {
  return (system("ping -c 1 8.8.8.8 > /dev/null 2>&1") == 0);
}

void LoadLocal() {
  std::ifstream f(".version");
  if (f.is_open()) {
    f >> localV;
    f.close();
  } else
    localV = "Bulunmadı";
}

void Scan() {
  status = "Ağlar Taranıyor...";
  networks.clear();
  system("nmcli -t -f SSID,SIGNAL dev wifi > wl.txt");
  std::ifstream f("wl.txt");
  std::string line;
  while (std::getline(f, line)) {
    size_t p = line.find(':');
    if (p != std::string::npos) {
      std::string s = line.substr(0, p);
      if (!s.empty())
        networks.push_back({s, line.substr(p + 1)});
    }
  }
}

void CheckRemote() {
  status = "Güncelleme Denetleniyor...";
  system("curl -s --max-time 5 "
         "https://raw.githubusercontent.com/rglomen/lumanovos/main/version.txt "
         "> rv.txt");
  std::ifstream f("rv.txt");
  if (f.is_open()) {
    f >> remoteV;
    f.close();
    if (remoteV.find("404") != std::string::npos || remoteV.empty())
      remoteV = "Hata";
  } else
    remoteV = "Hata";
}

int main() {
  // Dark Mode ve Premium Gorunum Ayarlari
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(GetScreenWidth(), GetScreenHeight(), "LumanovOS Welcome");
  SetTargetFPS(60);
  LoadLocal();

  // Renk Paleti (Dark Mode)
  Color bgDark = (Color){10, 10, 12, 255};      // Çok koyu arka plan
  Color cardDark = (Color){25, 25, 30, 255};    // Kart rengi
  Color accentBlue = (Color){0, 122, 255, 255}; // Apple Blue
  Color accentRed = (Color){255, 59, 48, 255};  // Apple Red
  Color textMain = WHITE;
  Color textGray = (Color){142, 142, 147, 255};

  int timer = 0;
  while (!WindowShouldClose()) {
    timer++;

    // --- LOGIC ---
    switch (state) {
    case CHECK_INT:
      if (timer > 60) {
        if (HasConnection())
          state = CHECK_UPD;
        else {
          Scan();
          state = WIFI_SET;
        }
      }
      break;
    case CHECK_UPD:
      CheckRemote();
      if (remoteV != "Hata" && remoteV != localV)
        state = SYNCING;
      else
        state = READY;
      break;
    case SYNCING:
      status = "LumanovOS Dosyaları Senkronize Ediliyor...";
      system("rm -rf lumanovos_tmp && git clone "
             "https://github.com/rglomen/lumanovos.git lumanovos_tmp");
      system("cp -r lumanovos_tmp/* . && rm -rf lumanovos_tmp");
      system("chmod +x build_all.sh && ./build_all.sh");
      {
        std::ofstream o(".version");
        o << remoteV;
        o.close();
      }
      localV = remoteV;
      state = READY;
      break;
    case WIFI_SET:
      if (IsKeyPressed(KEY_F1))
        Scan();
      break;
    case READY:
      status = "LumanovOS Kullanıma Hazır";
      break;
    }

    // --- DRAW ---
    BeginDrawing();
    ClearBackground(bgDark);
    float cx = GetScreenWidth() / 2.0f;
    float cy = GetScreenHeight() / 2.0f;

    // Arka Plan Kartı
    DrawRectangleRounded((Rectangle){cx - 320, cy - 200, 640, 400}, 0.05f, 20,
                         cardDark);
    DrawRectangleRoundedLines((Rectangle){cx - 320, cy - 200, 640, 400}, 0.05f,
                              20, (Color){60, 60, 65, 255});

    // Logo Üstte
    DrawCircleGradient(cx, cy - 100, 45, accentBlue, (Color){0, 80, 200, 255});
    DrawText("L", cx - 12, cy - 122, 50, WHITE);

    // Durum Mesajı
    DrawText(status.c_str(), cx - MeasureText(status.c_str(), 22) / 2, cy - 20,
             22, textMain);

    // Alt Bilgi Alanı
    DrawRectangle(cx - 320, cy + 100, 640, 1, (Color){50, 50, 55, 255});
    DrawText("Sürüm:", cx - 290, cy + 120, 16, textGray);
    DrawText(localV.c_str(), cx - 290, cy + 140, 18, textMain);
    DrawText("Sunucu:", cx + 180, cy + 120, 16, textGray);
    DrawText(remoteV.c_str(), cx + 180, cy + 140, 18, accentBlue);

    // Butonlar (FAZ 2: Mouse Desteği)
    if (state == READY) {
      // 1. Sisteme Gir Butonu
      Rectangle btnEnter = {cx - 220, cy + 30, 200, 45};
      bool hoverEnter = CheckCollisionPointRec(GetMousePosition(), btnEnter);
      DrawRectangleRounded(btnEnter, 0.5f, 20,
                           hoverEnter ? (Color){0, 150, 255, 255} : accentBlue);
      DrawText("SISTEME GIR",
               btnEnter.x + 100 - MeasureText("SISTEME GIR", 18) / 2,
               btnEnter.y + 13, 18, WHITE);

      // 2. Sistemi Kapat Butonu
      Rectangle btnPower = {cx + 20, cy + 30, 200, 45};
      bool hoverPower = CheckCollisionPointRec(GetMousePosition(), btnPower);
      DrawRectangleRounded(btnPower, 0.5f, 20,
                           hoverPower ? (Color){255, 80, 70, 255} : accentRed);
      DrawText("KAPAT", btnPower.x + 100 - MeasureText("KAPAT", 18) / 2,
               btnPower.y + 13, 18, WHITE);

      // Tıklama Kontrolü
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (hoverEnter) {
          // Sisteme girerken binary kontrolü yap
          if (access("./main_system", F_OK) == 0) {
            system("./main_system &");
            CloseWindow();
            return 0; // Uygulamadan temiz cikis
          } else {
            status = "HATA: main_system dosyası bulunamadı!";
          }
        }
        if (hoverPower) {
          system("poweroff"); // Sistemi kapat (sudo gerektirebilir)
          CloseWindow();
          return 0;
        }
      }
    }

    // Mouse Cursor (Sistem masusu gorunmuyor olabilir)
    DrawCircle(GetMouseX(), GetMouseY(), 5, WHITE);

    EndDrawing();
  }
  CloseWindow();
  return 0;
}
