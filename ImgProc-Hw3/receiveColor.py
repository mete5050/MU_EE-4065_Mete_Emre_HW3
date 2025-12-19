import serial, os, time
from datetime import datetime
from PIL import Image

# ==== AYARLAR ====
PORT = "COM8"  
BAUD = 115200
OUTPUT_DIR = "./capturesColor"
TIMEOUT = 5
# ==================

os.makedirs(OUTPUT_DIR, exist_ok=True)

def _parse_pnm_header(ser):
    """P5 (Gri) veya P6 (Renkli) başlığını okur."""
    line = ser.readline()
    if not line: return None, None, None, None
    
    magic = line.strip().decode('ascii', errors='ignore')
    if magic not in ["P5", "P6"]:
        return None, None, None, None

    # Boyut satırını bul
    while True:
        line = ser.readline()
        if not line: return None, None, None, None
        if line.startswith(b"#"): continue
        parts = line.strip().split()
        if len(parts) == 2:
            w, h = int(parts[0]), int(parts[1])
            break
    
    # Maxval satırı (255)
    ser.readline() 
    return magic, w, h, magic

def receive_image(ser, index=0):
    magic, w, h, _ = _parse_pnm_header(ser)
    if magic is None: return False

    # Renkli ise 3 katı veri oku
    bytes_per_pixel = 3 if magic == "P6" else 1
    total_bytes = w * h * bytes_per_pixel
    
    print(f"[i] Receiving {magic} image: {w}x{h} ({total_bytes} bytes)...")
    
    data = bytearray()
    while len(data) < total_bytes:
        chunk = ser.read(total_bytes - len(data))
        if not chunk: break
        data.extend(chunk)

    if len(data) < total_bytes:
        print("[!] Missing data!")
        return False

    # Mod belirleme: "L" (Luminance/Gri) veya "RGB"
    mode = "RGB" if magic == "P6" else "L"
    img = Image.frombytes(mode, (w, h), bytes(data))
    
    ts = datetime.now().strftime("%H%M%S")
    fname = f"capture_{index}_{ts}_{magic}.png"
    out_path = os.path.join(OUTPUT_DIR, fname)
    img.save(out_path)
    return True, out_path

def main():
    print(f"[*] Monitoring {PORT}...")
    with serial.Serial(PORT, BAUD, timeout=TIMEOUT) as ser:
        ser.reset_input_buffer()
        count = 0
        while True:
            try:
                ok, path = receive_image(ser, count)
                if ok:
                    print(f"[OK] Saved: {path}")
                    count += 1
            except KeyboardInterrupt:
                break
            except Exception as e:
                print(f"[!] Error: {e}")
                time.sleep(1)

if __name__ == "__main__":
    main()