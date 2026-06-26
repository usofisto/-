import os
from PIL import Image

ASSETS = "C:/Users/DZV/Desktop/проверка способностей с нуля/assets"

def remove_background_and_crop(img, target_size=None):
    """Remove magenta/white/gray backgrounds, crop to content, optionally resize"""
    img = img.convert("RGBA")
    pixels = img.load()
    w, h = img.size
    
    for y in range(h):
        for x in range(w):
            r, g, b, a = pixels[x, y]
            
            is_bg = False
            
            # Magenta background
            if r > 180 and g < 100 and b > 180:
                is_bg = True
            # Stronger magenta
            elif r > 150 and g < 120 and b > 150 and (r + b) > 350:
                is_bg = True
            # White checkerboard
            elif r > 240 and g > 240 and b > 240:
                is_bg = True
            # Light gray checkerboard
            elif r > 200 and g > 200 and b > 200 and abs(r - g) < 30 and abs(g - b) < 30:
                is_bg = True
            # Pure magenta
            elif r == 255 and g == 0 and b == 255:
                is_bg = True
            # Near magenta
            elif r > 200 and g < 50 and b > 200:
                is_bg = True
                
            if is_bg:
                pixels[x, y] = (0, 0, 0, 0)
    
    # Crop to content
    bbox = img.getbbox()
    if bbox is None:
        return img
    
    img = img.crop(bbox)
    
    # Resize if target size specified
    if target_size:
        w, h = img.size
        ratio = min(target_size[0] / w, target_size[1] / h)
        new_w = max(1, int(w * ratio))
        new_h = max(1, int(h * ratio))
        img = img.resize((new_w, new_h), Image.NEAREST)
    
    return img

def process_texture(filepath, target_size=None):
    try:
        img = Image.open(filepath)
        processed = remove_background_and_crop(img, target_size)
        processed.save(filepath)
        return f"OK: {os.path.basename(filepath)} -> {processed.size}"
    except Exception as e:
        return f"ERR: {os.path.basename(filepath)}: {e}"

results = []

# Trees: target ~80px
for i in range(1, 11):
    path = f"{ASSETS}/trees/tree_{i}.png"
    if os.path.exists(path):
        results.append(process_texture(path, (80, 100)))

# Rocks: target ~50px
for i in range(1, 6):
    path = f"{ASSETS}/rocks/rock_{i}.png"
    if os.path.exists(path):
        results.append(process_texture(path, (50, 50)))

# Grass: target 100x100
for i in range(1, 11):
    path = f"{ASSETS}/grass/grass_{i}.png"
    if os.path.exists(path):
        results.append(process_texture(path, (100, 100)))

# Slimes: target ~48px
for name in ["slime_green", "slime_red", "slime_blue"]:
    path = f"{ASSETS}/slimes/{name}.png"
    if os.path.exists(path):
        results.append(process_texture(path, (48, 48)))

# Player: target ~48px
results.append(process_texture(f"{ASSETS}/player.png", (48, 48)))

# Campfire frames: target ~32px
for name in ["campfire_0", "campfire_1", "campfire_2", "campfire_3"]:
    results.append(process_texture(f"{ASSETS}/{name}.png", (32, 32)))

# Tools: target ~32px
for name in ["sword", "pickaxe", "shovel"]:
    results.append(process_texture(f"{ASSETS}/{name}.png", (32, 32)))

# Items: target ~24px
for name in ["herb", "goldflower", "wood_item"]:
    results.append(process_texture(f"{ASSETS}/{name}.png", (24, 24)))

# Lake
results.append(process_texture(f"{ASSETS}/lake.png", (300, 250)))

# Old fallback textures
for name in ["tree", "rock", "grass", "slime"]:
    path = f"{ASSETS}/{name}.png"
    if os.path.exists(path):
        sz = (64, 64) if name != "grass" else (100, 100)
        results.append(process_texture(path, sz))

for r in results:
    print(r)
