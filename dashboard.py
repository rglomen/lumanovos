import pygame
import os
import subprocess
import sys
import time
import math

# --- KONFİGÜRASYON ---
BASE_PATH = os.path.dirname(os.path.abspath(__file__))
IMAGE_PATH = os.path.join(BASE_PATH, "images")
FPS = 60

# Renkler
WHITE = (255, 255, 255)
BLACK = (10, 10, 15)
BLUE  = (0, 120, 215)
GRAY  = (50, 50, 50)

class LumanovosOS:
    def __init__(self):
        pygame.init()
        self.info = pygame.display.Info()
        self.screen = pygame.display.set_mode((self.info.current_w, self.info.current_h), pygame.FULLSCREEN)
        pygame.mouse.set_visible(False)
        self.clock = pygame.time.Clock()
        
        # Fontlar
        self.font_main = pygame.font.SysFont("Sans", 55, bold=True)
        self.font_sub  = pygame.font.SysFont("Sans", 32)
        self.font_tiny = pygame.font.SysFont("Monospace", 22)

        # Menü Yapısı
        self.categories = ["OYUNLAR", "MEDYA", "AYARLAR", "GÜÇ"]
        self.menu_items = {
            "OYUNLAR": ["Retro Arch", "Doom (Native)", "Cloud Games", "Store"],
            "MEDYA":   ["YouTube", "Dosya Gezgini", "Resimler"],
            "AYARLAR": ["Ağ Ayarları", "Tema Değiştir", "Sistem Güncelle", "Ekran Çözünürlüğü"],
            "GÜÇ":     ["Yeniden Başlat", "Sistemi Kapat", "Uyku Modu"]
        }
        
        self.cat_idx = 0
        self.item_idx = 0
        self.bg_images = self.load_backgrounds()
        self.current_bg = 0
        self.anim_offset = 0

    def load_backgrounds(self):
        images = []
        if os.path.exists(IMAGE_PATH):
            for file in os.listdir(IMAGE_PATH):
                if file.endswith((".png", ".jpg", ".jpeg")):
                    img = pygame.image.load(os.path.join(IMAGE_PATH, file)).convert()
                    img = pygame.transform.smoothscale(img, (self.info.current_w, self.info.current_h))
                    images.append(img)
        return images

    def draw_animated_bg(self):
        if self.bg_images:
            self.screen.blit(self.bg_images[self.current_bg], (0, 0))
            # Hafif bir karartma katmanı (yazıların okunması için)
            overlay = pygame.Surface((self.info.current_w, self.info.current_h))
            overlay.set_alpha(150)
            overlay.fill((0, 0, 0))
            self.screen.blit(overlay, (0, 0))
        else:
            self.screen.fill(BLACK)
            # Hareketli dalga efekti (PS3 tarzı)
            for i in range(0, self.info.current_w, 5):
                y = math.sin((i + self.anim_offset) * 0.01) * 50 + (self.info.current_h / 2)
                pygame.draw.circle(self.screen, BLUE, (i, int(y)), 1)
        
        self.anim_offset += 2

    def render(self):
        self.draw_animated_bg()
        
        # 1. KATEGORİLER (Üst Menü)
        for i, cat in enumerate(self.categories):
            is_selected = (i == self.cat_idx)
            color = WHITE if is_selected else GRAY
            scale = 1.2 if is_selected else 1.0
            
            txt = self.font_main.render(cat, True, color)
            if is_selected: # Seçili olanın altına çizgi
                pygame.draw.rect(self.screen, BLUE, (100 + (i*250), 130, txt.get_width(), 5))
            
            self.screen.blit(txt, (100 + (i*250), 70))

        # 2. ALT ÖĞELER (Dikey Liste)
        curr_cat = self.categories[self.cat_idx]
        items = self.menu_items[curr_cat]
        
        for j, item in enumerate(items):
            is_sel = (j == self.item_idx)
            x_offset = 120 if is_sel else 100
            color = BLUE if is_sel else WHITE
            
            # Seçim oku animasyonu
            if is_sel:
                arr = self.font_sub.render(">", True, BLUE)
                self.screen.blit(arr, (x_offset - 30, 250 + (j*60)))

            txt = self.font_sub.render(item, True, color)
            self.screen.blit(txt, (x_offset, 250 + (j*60)))

        # 3. ALT BİLGİ PANELİ
        time_str = time.strftime("%H:%M:%S")
        status = self.font_tiny.render(f"Lumanovos OS v1.2 | {time_str} | IP: Connected", True, WHITE)
        self.screen.blit(status, (50, self.info.current_h - 50))

        pygame.display.flip()

    def run(self):
        running = True
        while running:
            self.render()
            for event in pygame.event.get():
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_RIGHT:
                        self.cat_idx = (self.cat_idx + 1) % len(self.categories)
                        self.item_idx = 0
                    elif event.key == pygame.K_LEFT:
                        self.cat_idx = (self.cat_idx - 1) % len(self.categories)
                        self.item_idx = 0
                    elif event.key == pygame.K_DOWN:
                        self.item_idx = (self.item_idx + 1) % len(self.menu_items[self.categories[self.cat_idx]])
                    elif event.key == pygame.K_UP:
                        self.item_idx = (self.item_idx - 1) % len(self.menu_items[self.categories[self.cat_idx]])
                    elif event.key == pygame.K_RETURN:
                        self.handle_selection()
                    elif event.key == pygame.K_t: # 'T' tuşu ile tema/arka plan değiştir
                        if self.bg_images:
                            self.current_bg = (self.current_bg + 1) % len(self.bg_images)
            
            self.clock.tick(FPS)

    def handle_selection(self):
        sel = self.menu_items[self.categories[self.cat_idx]][self.item_idx]
        if sel == "Sistemi Kapat":
            subprocess.run(["sudo", "poweroff"])
        elif sel == "Yeniden Başlat":
            subprocess.run(["sudo", "reboot"])
        elif sel == "Tema Değiştir":
            if self.bg_images:
                self.current_bg = (self.current_bg + 1) % len(self.bg_images)

if __name__ == "__main__":
    app = LumanovosOS()
    app.run()
