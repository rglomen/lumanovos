#include "raylib.h"

int main() {
    // 1. GPU Sürücülerini zorla tetikle
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    
    // 2. Ekran boyutunu otomatik al
    InitWindow(1280, 720, "Lumanovos Pro Core");
    
    if (!IsWindowReady()) return 1; // Başlatılamazsa çık
    
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // --- DRAW ---
        BeginDrawing();
            // Ekranın dolduğunu kanıtlamak için parlak bir temizlik yapalım
            ClearBackground({20, 20, 40, 255}); 

            // Orta kısma büyük bir parlayan logo
            DrawCircleGradient(640, 360, 300, {0, 255, 200, 100}, {0, 0, 0, 0});
            DrawText("LUMANOVOS OS", 400, 330, 60, WHITE);
            
            // Alt kısma hareketli bir bar
            float time = GetTime();
            DrawRectangle(0, 680, (int)(1280 * (sin(time) * 0.5 + 0.5)), 40, GREEN);
            
            DrawText("PERFORMANS MODU: AKTIF (C++/RAYLIB)", 20, 20, 20, LIME);
            DrawFPS(1200, 20);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
