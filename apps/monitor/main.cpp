#include "libs/ui/ui.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

struct SystemStats {
  float cpuUsage;
  float memUsage;
  float memTotal;
  float memUsed;
  float diskTotal;
  float diskUsed;
  int processCount;
  std::string uptime;
};

SystemStats stats = {0};

void UpdateStats() {
  FILE *fp;
  char buffer[256];

  // CPU kullanımı
  fp = popen("top -bn1 | grep 'Cpu(s)' | awk '{print $2}'", "r");
  if (fp) {
    if (fgets(buffer, sizeof(buffer), fp)) {
      stats.cpuUsage = atof(buffer) / 100.0f;
    }
    pclose(fp);
  }

  // Bellek
  fp = popen("free -m | grep Mem | awk '{print $2, $3}'", "r");
  if (fp) {
    if (fgets(buffer, sizeof(buffer), fp)) {
      sscanf(buffer, "%f %f", &stats.memTotal, &stats.memUsed);
      stats.memUsage = stats.memUsed / stats.memTotal;
    }
    pclose(fp);
  }

  // Disk
  fp = popen("df -BG / | tail -1 | awk '{print $2, $3}'", "r");
  if (fp) {
    if (fgets(buffer, sizeof(buffer), fp)) {
      sscanf(buffer, "%fG %fG", &stats.diskTotal, &stats.diskUsed);
    }
    pclose(fp);
  }

  // Process sayısı
  fp = popen("ps aux | wc -l", "r");
  if (fp) {
    if (fgets(buffer, sizeof(buffer), fp)) {
      stats.processCount = atoi(buffer);
    }
    pclose(fp);
  }

  // Uptime
  fp = popen("uptime -p", "r");
  if (fp) {
    if (fgets(buffer, sizeof(buffer), fp)) {
      buffer[strcspn(buffer, "\n")] = 0;
      stats.uptime = buffer;
    }
    pclose(fp);
  }
}

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(500, 450, "Sistem Monitörü");
  SetTargetFPS(60);
  LUI::SetTheme(true);

  double lastUpdate = 0;
  UpdateStats();

  while (!WindowShouldClose()) {
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    // Her 2 saniyede güncelle
    if (GetTime() - lastUpdate > 2.0) {
      UpdateStats();
      lastUpdate = GetTime();
    }

    BeginDrawing();
    ClearBackground(LUI::currentTheme->windowBg);

    LUI::WindowResult winResult = LUI::DrawWindowFrame(
        {0, 0, (float)w, (float)h}, "Sistem Monitörü", true);
    if (winResult.closeClicked)
      break;

    float y = 50;
    float barW = w - 150;

    // CPU
    DrawText("İşlemci (CPU)", 20, y, 16, LUI::currentTheme->text);
    y += 25;
    LUI::ProgressBar({20, y, barW, 20}, stats.cpuUsage,
                     stats.cpuUsage > 0.8f ? LUI::currentTheme->error
                                           : LUI::currentTheme->accent);
    DrawText(TextFormat("%.1f%%", stats.cpuUsage * 100), barW + 35, y + 2, 14,
             LUI::currentTheme->text);
    y += 45;

    // Bellek
    DrawText("Bellek (RAM)", 20, y, 16, LUI::currentTheme->text);
    y += 25;
    LUI::ProgressBar({20, y, barW, 20}, stats.memUsage,
                     stats.memUsage > 0.8f ? LUI::currentTheme->warning
                                           : LUI::currentTheme->success);
    DrawText(TextFormat("%.0f / %.0f MB", stats.memUsed, stats.memTotal),
             barW + 35, y + 2, 12, LUI::currentTheme->text);
    y += 45;

    // Disk
    float diskUsage =
        stats.diskTotal > 0 ? stats.diskUsed / stats.diskTotal : 0;
    DrawText("Disk", 20, y, 16, LUI::currentTheme->text);
    y += 25;
    LUI::ProgressBar({20, y, barW, 20}, diskUsage,
                     diskUsage > 0.9f ? LUI::currentTheme->error
                                      : (Color){255, 149, 0, 255});
    DrawText(TextFormat("%.0f / %.0f GB", stats.diskUsed, stats.diskTotal),
             barW + 35, y + 2, 12, LUI::currentTheme->text);
    y += 55;

    LUI::Separator(20, y, w - 40);
    y += 20;

    // Bilgiler
    DrawText("Sistem Bilgileri", 20, y, 16, LUI::currentTheme->text);
    y += 30;

    DrawText("Çalışan İşlemler:", 30, y, 13, LUI::currentTheme->textDim);
    DrawText(TextFormat("%d", stats.processCount), 170, y, 13,
             LUI::currentTheme->text);
    y += 24;

    DrawText("Çalışma Süresi:", 30, y, 13, LUI::currentTheme->textDim);
    DrawText(stats.uptime.c_str(), 170, y, 13, LUI::currentTheme->text);
    y += 40;

    // Yenile butonu
    if (LUI::Button({20, y, 120, 30}, "Yenile")) {
      UpdateStats();
    }

    DrawText("Otomatik güncelleme: 2sn", 160, y + 8, 12,
             LUI::currentTheme->textDim);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
