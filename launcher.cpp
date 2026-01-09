#include "raylib.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

enum SystemState {
  CHECKING_INTERNET,
  WIFI_SETUP,
  CHECKING_UPDATES,
  DOWNLOADING,
  READY_TO_LAUNCH
};

struct WifiNetwork {
  std::string ssid;
  std::string signal;
};

SystemState currentState = CHECKING_INTERNET;
std::string localVersion = "0.0.0";
std::string remoteVersion = "---";
std::vector<WifiNetwork> networks;
std::string statusMessage = "Sistem kontrol ediliyor...";

bool HasInternet() {
  return (system("ping -c 1 8.8.8.8 > /dev/null 2>&1") == 0);
}

void LoadLocalVersion() {
  std::ifstream vFile(".version");
  if (vFile.is_open()) {
    vFile >> localVersion;
    vFile.close();
  } else
    localVersion = "Kurulu Degil";
}

void ScanWifi() {
  statusMessage = "Aglar taraniyor...";
  networks.clear();
  system("nmcli -t -f SSID,SIGNAL dev wifi > wifi_list.txt");
  std::ifstream f("wifi_list.txt");
  std::string line;
  while (std::getline(f, line)) {
    size_t pos = line.find(':');
    if (pos != std::string::npos) {
      std::string ssid = line.substr(0, pos);
      if (!ssid.empty())
        networks.push_back({ssid, line.substr(pos + 1)});
    }
  }
}

void CheckRemoteVersion() {
  statusMessage = "Sunucuya baglaniliyor...";
  system("curl -s --max-time 5 "
         "https://raw.githubusercontent.com/rglomen/lumanovos/main/version.txt "
         "> remote_version.txt");
  std::ifstream f("remote_version.txt");
  if (f.is_open()) {
    f >> remoteVersion;
    f.close();
    if (remoteVersion.find("404") != std::string::npos || remoteVersion.empty())
      remoteVersion = "Hata";
  } else
    remoteVersion = "Hata";
}

int main() {
  // MiniOS/Trixy icinde fontlari yumusatmak icin MSAA aciyoruz
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(GetScreenWidth(), GetScreenHeight(), "LumanovOS Setup");
  SetTargetFPS(60);
  LoadLocalVersion();

  Color appleBg = (Color){245, 245, 247, 255};
  Color appleBlue = (Color){0, 122, 255, 255};
  int frameCounter = 0;

  while (!WindowShouldClose()) {
    frameCounter++;
    switch (currentState) {
    case CHECKING_INTERNET:
      if (frameCounter > 60) {
        if (HasInternet())
          currentState = CHECKING_UPDATES;
        else {
          ScanWifi();
          currentState = WIFI_SETUP;
        }
      }
      break;
    case WIFI_SETUP:
      if (IsKeyPressed(KEY_F1))
        ScanWifi();
      break;
    case CHECKING_UPDATES:
      CheckRemoteVersion();
      if (remoteVersion != "Hata" && remoteVersion != localVersion)
        currentState = DOWNLOADING;
      else
        currentState = READY_TO_LAUNCH;
      break;
    case DOWNLOADING: {
      statusMessage = "Dosyalar indiriliyor ve derleniyor...";
      // Repo yoksa clone, varsa pull yap
      if (system("ls .git > /dev/null 2>&1") != 0)
        system("git clone https://github.com/rglomen/lumanovos.git .");
      else
        system("git pull origin main");

      system("chmod +x build_all.sh && ./build_all.sh");
      std::ofstream vOut(".version");
      vOut << remoteVersion;
      vOut.close();
      localVersion = remoteVersion;
      currentState = READY_TO_LAUNCH;
      break;
    }
    case READY_TO_LAUNCH:
      statusMessage = "Sistem baslatilmaya hazir.";
      if (IsKeyPressed(KEY_ENTER)) {
        system("./main_system &");
        CloseWindow();
      }
      break;
    }

    BeginDrawing();
    ClearBackground(appleBg);
    float cx = GetScreenWidth() / 2.0f;
    float cy = GetScreenHeight() / 2.0f;

    DrawRectangleRounded((Rectangle){cx - 300, cy - 180, 600, 360}, 0.05f, 20,
                         WHITE);
    DrawRectangleRoundedLines((Rectangle){cx - 300, cy - 180, 600, 360}, 0.05f,
                              20, (Color){220, 220, 220, 255});

    DrawCircle(cx, cy - 80, 35, appleBlue);
    DrawText("L", cx - 10, cy - 100, 40, WHITE);

    // Turkce karakter sorununu gecici olarak ASCII ile cozuyoruz (font
    // yuklenene kadar)
    DrawText(statusMessage.c_str(),
             cx - MeasureText(statusMessage.c_str(), 20) / 2, cy + 10, 20,
             BLACK);

    DrawRectangle(cx - 300, cy + 100, 600, 1, (Color){235, 235, 235, 255});
    DrawText("Cihaz:", cx - 270, cy + 120, 16, GRAY);
    DrawText(localVersion.c_str(), cx - 270, cy + 140, 18, BLACK);
    DrawText("Sunucu:", cx + 120, cy + 120, 16, GRAY);
    DrawText(remoteVersion.c_str(), cx + 120, cy + 140, 18, appleBlue);

    if (currentState == READY_TO_LAUNCH) {
      DrawRectangleRounded((Rectangle){cx - 100, cy + 40, 200, 40}, 0.5f, 20,
                           appleBlue);
      DrawText("SISTEMI BASLAT", cx - MeasureText("SISTEMI BASLAT", 16) / 2,
               cy + 52, 16, WHITE);
    }
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
