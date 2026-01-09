import pygame
import os
import subprocess
import sys
import time
import math

# --- SİSTEM BİLGİLERİ ---
def get_sys_info():
    try:
        cpu = subprocess.check_output("top -bn1 | grep 'Cpu(s)'", shell=True).decode().split()[1]
        ram = subprocess.check_output("free -m | awk 'NR==2{printf \"%.2f%%\", $3*100/$2 }'", shell=True).decode()
        return f"CPU: %{cpu} | RAM: {ram}"
    except:
        return "Sistem bilgisi alınamadı"

BASE_PATH = os.path.dirname(os.path.abspath(__file__))
IMAGE_PATH = os.path.join(BASE_PATH, "images")

class LumanovosOS:
    def __init__(self):
        pygame.init()
        self.info = pygame.display.Info()
        self.screen = pygame.display.set_mode((self.info.current_w, self.info.current_h), pygame.FULLSCREEN)
        pygame.mouse.set_visible(False)
        self.clock = pygame.time.Clock()
        
        self.font_main = pygame.font.SysFont("Sans", 50, bold=True)
        self.font_sub  = pygame.font.SysFont("Sans", 30)
        self.font_tiny = pygame.font.SysFont("Monospace", 20)

        self.categories = ["OYUNLAR", "AYARLAR", "GÜÇ"]
        self.menu_items = {
            "OYUNLAR": ["Terminali Aç", "Dosya Gezgini", "Demo Oyun"],
            "AYARLAR": ["Tema Değiştir", "Sistem Bilgisi", "Ağ Durumu", "Sistemi Güncelle"],
            "GÜÇ":     ["Yeniden Başlat", "Sistemi Kapat"]
        }
        
        self.cat_idx = 0
        self.item_idx = 0
        self.bg_images = self.load_backgrounds()
        self.current_bg = 0
        self.sys_info_display = ""
        self.show_popup = False
        self.popup_msg = ""

    def load_backgrounds(self):
        images = []
        if os.path.exists(IMAGE_PATH):
            for file in os.listdir(IMAGE_PATH):
                if file.lower().endswith((".png", ".jpg", ".jpeg")):
                    img = pygame.image.load(os.path.join(IMAGE_PATH, file)).convert()
                    img = pygame.transform.smoothscale(img, (self.info.current_w, self.info.current_h))
                    images.append(img)
        return images

    def handle_selection(self):
        sel = self.menu_items[self.categories[self.cat_idx]][self.item_idx]
        
        # --- AYARLAR MANTIĞI ---
        if sel == "Tema Değiştir":
            if self.bg_images:
                self.current_bg = (self.current_bg + 1) % len(self.bg_images)
            else:
                self.show_temp_msg("Hata: images klasörü boş!")
        
        elif sel == "Sistem Bilgisi":
            info = get_sys_info()
            self.show_temp_msg(info)
            
        elif sel == "Ağ Durumu":
            try:
                ip = subprocess.check_output(["hostname", "-I"]).decode().split()[0]
                self.show_temp_msg(f"Yerel IP: {ip}")
            except:
                self.show_temp_msg("Ağ Bağlantısı Yok!")

        elif sel == "Sistemi Güncelle":
            self.show_temp_msg("Güncellemeler denetleniyor... (GitHub)")
            # Updater zaten her açılışta güncelliyor, buraya manuel tetik eklenebilir.

        elif sel == "Terminali Aç":
            pygame.quit()
            subprocess.run(["xterm"]) # Terminali açar (xterm kurulu olmalı)
            sys.exit()

        elif sel == "Sistemi Kapat":
            subprocess.run(["sudo", "poweroff"])
        elif sel == "Yeniden Başlat":
            subprocess.run(["sudo", "reboot"])

    def show_temp_msg(self, msg):
        self.show_popup = True
        self.popup_msg = msg
        self.popup_time = time.time()

    def render(self):
        # Arka Plan
        if self.bg_images:
            self.screen.blit(self.bg_images[self.current_bg], (0, 0))
        else:
            self.screen.fill((10, 10, 30))

        # Seçili Kategoriyi Çiz
        curr_cat = self.categories[self.cat_idx]
        cat_txt = self.font_main.render(curr_cat, True, (255, 255, 255))
        self.screen.blit(cat_txt, (100, 80))

        # Alt Menüleri Çiz
        for i, item in enumerate(self.menu_items[curr_cat]):
            color = (0, 255, 100) if i == self.item_idx else (200, 200, 200)
            prefix = ">> " if i == self.item_idx else "   "
            txt = self.font_sub.render(prefix + item, True, color)
            self.screen.blit(txt, (120, 200 + (i * 60)))

        # Popup Mesajı (Bilgi kutusu)
        if self.show_popup:
            pygame.draw.rect(self.screen, (0, 0, 0), (self.info.current_w/2-200, self.info.current_h-150, 400, 80))
            pygame.draw.rect(self.screen, (0, 255, 100), (self.info.current_w/2-200, self.info.current_h-150, 400, 80), 2)
            p_txt = self.font_tiny.render(self.popup_msg, True, (0, 255, 100))
            self.screen.blit(p_txt, (self.info.current_w/2 - p_txt.get_width()/2, self.info.current_h-125))
            if time.time() - self.popup_time > 3: self.show_popup = False

        pygame.display.flip()

    def run(self):
        while True:
            self.render()
            for event in pygame.event.get():
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_RIGHT: self.cat_idx = (self.cat_idx + 1) % len(self.categories)
                    if event.key == pygame.K_LEFT: self.cat_idx = (self.cat_idx - 1) % len(self.categories)
                    if event.key == pygame.K_DOWN: self.item_idx = (self.item_idx + 1) % len(self.menu_items[self.categories[self.cat_idx]])
                    if event.key == pygame.K_UP: self.item_idx = (self.item_idx - 1) % len(self.menu_items[self.categories[self.cat_idx]])
                    if event.key == pygame.K_RETURN: self.handle_selection()
            self.clock.tick(60)

if __name__ == "__main__":
    LumanovosOS().run()
