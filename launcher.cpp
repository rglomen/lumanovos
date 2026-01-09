#include "raylib.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

enum SystemState {
  CHECK_INT,
  WIFI_SET,
  CHECK_UPD,
  SYNCING,
  READY,
  ERROR_STATE
};

struct Wifi {
  std::string ssid;
  std::string pwr;
};

SystemState state = CHECK_INT;
std::string localV = "0.0.0";
std::string remoteV = "---";
std::string status = "LumanovOS Başlatılıyor...";
std::string lastError = "";

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

void CheckRemote() {
  status = "Güncelleme Denetleniyor...";
  system("curl -s --max-time 5 "
         "https://raw.githubusercontent.com/rglomen/lumanovos/main/version.txt "
         "> rv.txt 2> err.txt");
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
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(GetScreenWidth(), GetScreenHeight(), "LumanovOS Welcome");
  SetTargetFPS(60);
  LoadLocal();

  Color bgDark = (Color){10, 10, 12, 255};
  Color cardDark = (Color){25, 25, 30, 255};
  Color accentBlue = (Color){0, 122, 255, 255};
  Color accentRed = (Color){255, 59, 48, 255};

  int timer = 0;
  while (!WindowShouldClose()) {
    timer++;

    switch (state) {
    case CHECK_INT:
      if (timer > 60) {
        if (HasConnection())
          state = CHECK_UPD;
        else
          state = WIFI_SET;
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
      status = "GitHub'dan Dosyalar Çekiliyor...";
      // Temizlik ve Yeni Clone
      system("rm -rf lumanovos_new");
      if (system("git clone https://github.com/rglomen/lumanovos.git "
                 "lumanovos_new 2> build_err.txt") == 0) {
        system("rm -rf lumanovos && mv lumanovos_new lumanovos");
        status = "Sistem Bileşenleri Derleniyor...";
        // Klasörün içine girip derle
        int res = system("cd lumanovos && chmod +x build_all.sh && "
                         "./build_all.sh >> ../build_err.txt 2>&1");
        if (res == 0) {
          std::ofstream o(".version");
          o << remoteV;
          o.close();
          localV = remoteV;
          state = READY;
        } else {
          state = ERROR_STATE;
          lastError = "Derleme Hatası! (build_err.txt'ye bakın)";
        }
      } else {
        state = ERROR_STATE;
        lastError = "Git Clone Hatası! (İnterneti kontrol edin)";
      }
      break;
    case READY:
      status = "LumanovOS Başlatılmaya Hazır";
      break;
    case WIFI_SET:
      status = "Bağlantı Yok! WiFi Ayarlayın.";
      break;
    }

    BeginDrawing();
    ClearBackground(bgDark);
    float cx = GetScreenWidth() / 2.0f;
    float cy = GetScreenHeight() / 2.0f;

    DrawRectangleRounded((Rectangle){cx - 320, cy - 200, 640, 400}, 0.05f, 20,
                         cardDark);

    if (state == ERROR_STATE) {
      DrawText("SISTEM HATASI", cx - MeasureText("SISTEM HATASI", 24) / 2,
               cy - 80, 24, RED);
      DrawText(lastError.c_str(), cx - MeasureText(lastError.c_str(), 18) / 2,
               cy - 20, 18, LIGHTGRAY);
      DrawText("Tekrar denemek için 'R' tuşuna basın.",
               cx -
                   MeasureText("Tekrar denemek için 'R' tuşuna basın.", 16) / 2,
               cy + 30, 16, GRAY);
      if (IsKeyPressed(KEY_R))
        state = CHECK_INT;
    } else {
      DrawCircleGradient(cx, cy - 100, 40, accentBlue,
                         (Color){0, 80, 200, 255});
      DrawText("L", cx - 10, cy - 120, 45, WHITE);
      DrawText(status.c_str(), cx - MeasureText(status.c_str(), 22) / 2,
               cy - 20, 22, WHITE);
    }

    // Butonlar
    if (state == READY) {
      Rectangle bEnter = {cx - 210, cy + 40, 190, 50};
      Rectangle bPower = {cx + 20, cy + 40, 190, 50};
      bool hE = CheckCollisionPointRec(GetMousePosition(), bEnter);
      bool hP = CheckCollisionPointRec(GetMousePosition(), bPower);

      DrawRectangleRounded(bEnter, 0.5f, 20,
                           hE ? (Color){0, 150, 255, 255} : accentBlue);
      DrawText("SISTEME GIR",
               bEnter.x + 95 - MeasureText("SISTEME GIR", 18) / 2,
               bEnter.y + 16, 18, WHITE);

      DrawRectangleRounded(bPower, 0.5f, 20,
                           hP ? (Color){255, 80, 70, 255} : accentRed);
      DrawText("KAPAT", bPower.x + 95 - MeasureText("KAPAT", 18) / 2,
               bPower.y + 16, 18, WHITE);

      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (hE) {
          // lumanovos klasorunun icindeki binary'yi calistir
          if (access("lumanovos/main_system", F_OK) == 0) {
            CloseWindow();
            // chdir ile klasöre gir ve execl ile process'i değiştir
            // Bu sayede X11 oturumu kapanmaz, launcher yerine main_system
            // çalışır
            if (chdir("lumanovos") == 0) {
              execl("./main_system", "./main_system", (char *)NULL);
            }
            // execl başarısız olursa buraya ulaşılır
            return 1;
          } else
            status = "HATA: main_system bulunamadı!";
        }
        if (hP) {
          system("echo 120820 | sudo -S poweroff"); // Sudo ile kapatmayı dene
          CloseWindow();
          return 0;
        }
      }
    }

    // Sürüm Bilgisi
    DrawText(TextFormat("v%s", localV.c_str()), 40, GetScreenHeight() - 40, 16,
             GRAY);
    // Mouse imleci
    DrawCircle(GetMouseX(), GetMouseY(), 4, WHITE);
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
