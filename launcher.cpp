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
std::string status = "Sistem Kontrol Ediliyor...";

bool HasConnection() {
  return (system("ping -c 1 8.8.8.8 > /dev/null 2>&1") == 0);
}

void LoadLocal() {
  std::ifstream f(".version");
  if (f.is_open()) {
    f >> localV;
    f.close();
  } else
    localV = "Yok";
}

void Scan() {
  status = "Aglar Taraniyor...";
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
  status = "Guncelleme Denetleniyor...";
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
  // MiniOS/Trixy icinde daha temiz goruntu icin
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(GetScreenWidth(), GetScreenHeight(), "LumanovOS Setup");
  SetTargetFPS(60);
  LoadLocal();

  // Font Ayari - Eger sistemde varsa yukle, yoksa default kullan
  Font mainFont = GetFontDefault();
  if (access("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
             F_OK) == 0) {
    mainFont = LoadFontEx(
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 32,
        0, 250);
  }

  int timer = 0;
  while (!WindowShouldClose()) {
    timer++;
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
      status = "Sistem Guncelleniyor...";
      // Dizin kontrolu ve indirme
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
      status = "LumanovOS Baslatilmaya Hazir";
      break;
    }

    BeginDrawing();
    ClearBackground((Color){245, 245, 247, 255});
    float cx = GetScreenWidth() / 2.0f;
    float cy = GetScreenHeight() / 2.0f;

    // Arka Plan Karti
    DrawRectangleRounded((Rectangle){cx - 300, cy - 180, 600, 360}, 0.05f, 20,
                         WHITE);
    DrawRectangleRoundedLines((Rectangle){cx - 300, cy - 180, 600, 360}, 0.05f,
                              20, (Color){220, 220, 220, 255});

    // Logo
    DrawCircle(cx, cy - 80, 35, (Color){0, 122, 255, 255});
    DrawTextEx(mainFont, "L", (Vector2){cx - 12, cy - 105}, 50, 2, WHITE);

    // Metinler
    Vector2 msgSize = MeasureTextEx(mainFont, status.c_str(), 24, 1);
    DrawTextEx(mainFont, status.c_str(), (Vector2){cx - msgSize.x / 2, cy + 10},
               24, 1, BLACK);

    // Versiyon Alani
    DrawRectangle(cx - 300, cy + 100, 600, 1, (Color){235, 235, 235, 255});
    DrawTextEx(mainFont, "Yerel:", (Vector2){cx - 270, cy + 120}, 16, 1, GRAY);
    DrawTextEx(mainFont, localV.c_str(), (Vector2){cx - 270, cy + 140}, 20, 1,
               BLACK);
    DrawTextEx(mainFont, "Sunucu:", (Vector2){cx + 120, cy + 120}, 16, 1, GRAY);
    DrawTextEx(mainFont, remoteV.c_str(), (Vector2){cx + 120, cy + 140}, 20, 1,
               (Color){0, 122, 255, 255});

    if (state == READY) {
      Rectangle btn = {cx - 120, cy + 45, 240, 45};
      DrawRectangleRounded(btn, 0.5f, 20, (Color){0, 122, 255, 255});
      DrawTextEx(mainFont, "SISTEMI BASLAT", (Vector2){cx - 85, cy + 58}, 18, 1,
                 WHITE);
      if (IsKeyPressed(KEY_ENTER)) {
        system("./main_system &");
        CloseWindow();
      }
    }
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
