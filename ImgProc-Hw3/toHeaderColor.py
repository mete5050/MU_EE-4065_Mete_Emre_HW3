from PIL import Image
import os

# === AYARLAR ===
input_path = r"C:\Users\meteAsus\Desktop\ImgProc\ALF.png"
output_dir = r"C:\Users\meteAsus\Desktop\ImgProc"
array_name = "g_color_img"
width, height = 64, 64  # STM32 RAM kapasitesine göre ayarlandı
# =================

os.makedirs(output_dir, exist_ok=True)
output_path = os.path.join(output_dir, "ALF_color.h")

# Görseli yükle ve RGB modunda yeniden boyutlandır
img = Image.open(input_path).convert("RGB").resize((width, height))
pixels = list(img.getdata()) # [(R1,G1,B1), (R2,G2,B2), ...]

with open(output_path, "w") as f:
    f.write("#ifndef ALF_COLOR_H\n#define ALF_COLOR_H\n\n#include <stdint.h>\n\n")
    # Mevcut IMG_W ve IMG_H ile çakışmaması için farklı isimler kullanabilirsiniz
    f.write(f"const uint8_t {array_name}[{width} * {height} * 3] = {{\n")

    for i, (r, g, b) in enumerate(pixels):
        f.write(f"{r}, {g}, {b}, ")
        if (i + 1) % 6 == 0: # Satır başına 6 piksel (okunabilirlik için)
            f.write("\n")

    f.write("};\n\n#endif\n")

print(f"[OK] Renkli header dosyası oluşturuldu: {output_path}")