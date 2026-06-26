"""Generate biome-specific trees and fix all backgrounds"""
from PIL import Image, ImageDraw
import random
import os

ASSETS = "C:/Users/DZV/Desktop/проверка способностей с нуля/assets"

BIOME_TREES = {
    "forest": {
        "trunk": [(80, 50, 25), (70, 45, 20), (90, 55, 30)],
        "leaves": [(30, 120, 45), (40, 140, 55), (25, 100, 35), (50, 150, 65)],
        "highlight": [(60, 180, 80), (80, 200, 100)],
    },
    "desert": {
        "trunk": [(120, 90, 50), (110, 80, 45), (130, 100, 55)],
        "leaves": [(160, 140, 60), (150, 130, 55), (170, 150, 65)],
        "highlight": [(190, 170, 80), (200, 180, 90)],
    },
    "snow": {
        "trunk": [(90, 70, 55), (80, 60, 45), (100, 80, 60)],
        "leaves": [(50, 80, 60), (60, 90, 70), (40, 70, 50), (55, 85, 65)],
        "highlight": [(180, 200, 190), (200, 220, 210)],
    },
    "swamp": {
        "trunk": [(60, 45, 25), (50, 35, 20), (70, 55, 30)],
        "leaves": [(40, 70, 30), (35, 65, 25), (45, 75, 35), (50, 80, 40)],
        "highlight": [(60, 100, 50), (70, 110, 60)],
    },
    "meadow": {
        "trunk": [(100, 65, 30), (90, 55, 25), (110, 75, 35)],
        "leaves": [(50, 150, 60), (60, 160, 70), (40, 140, 50), (70, 170, 80)],
        "highlight": [(100, 200, 120), (120, 220, 140)],
    }
}

def generate_tree(biome_name, index, size=120):
    """Generate a unique tree for a biome"""
    palette = BIOME_TREES[biome_name]
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    cx, cy = size // 2, size // 2
    
    # Trunk
    trunk_w = random.randint(8, 14)
    trunk_h = random.randint(30, 45)
    trunk_x = cx - trunk_w // 2
    trunk_y = cy + 10
    
    trunk_color = random.choice(palette["trunk"])
    trunk_dark = (max(0, trunk_color[0]-20), max(0, trunk_color[1]-20), max(0, trunk_color[2]-10))
    
    # Draw trunk with bark texture
    draw.rectangle([trunk_x, trunk_y, trunk_x + trunk_w, trunk_y + trunk_h], fill=trunk_color)
    # Bark lines
    for i in range(3):
        lx = trunk_x + random.randint(1, trunk_w-2)
        draw.line([lx, trunk_y+2, lx, trunk_y+trunk_h-2], fill=trunk_dark, width=1)
    
    # Roots
    draw.polygon([trunk_x, trunk_y+trunk_h, trunk_x-8, trunk_y+trunk_h+5, trunk_x+3, trunk_y+trunk_h], fill=trunk_dark)
    draw.polygon([trunk_x+trunk_w, trunk_y+trunk_h, trunk_x+trunk_w+8, trunk_y+trunk_h+5, trunk_x+trunk_w-3, trunk_y+trunk_h], fill=trunk_dark)
    
    # Canopy - multiple overlapping circles
    canopy_y = trunk_y - 20
    leaf_color = random.choice(palette["leaves"])
    leaf_dark = (max(0, leaf_color[0]-25), max(0, leaf_color[1]-25), max(0, leaf_color[2]-15))
    leaf_light = random.choice(palette["highlight"])
    
    # Main canopy shape
    canopy_r = random.randint(28, 38)
    # Draw several overlapping circles for natural look
    offsets = [(0, 0), (-12, -8), (12, -8), (-8, 5), (8, 5), (0, -15)]
    for ox, oy in offsets:
        r = canopy_r + random.randint(-8, 5)
        draw.ellipse([cx+ox-r, canopy_y+oy-r, cx+ox+r, canopy_y+oy+r], fill=leaf_color)
    
    # Darker shadow layer
    for ox, oy in offsets[:3]:
        r = canopy_r + random.randint(-10, 3)
        draw.ellipse([cx+ox-r+3, canopy_y+oy-r+3, cx+ox+r-3, canopy_y+oy+r-3], fill=leaf_dark)
    
    # Highlights
    for _ in range(5):
        hx = cx + random.randint(-20, 20)
        hy = canopy_y + random.randint(-15, 10)
        hr = random.randint(3, 6)
        draw.ellipse([hx-hr, hy-hr, hx+hr, hy+hr], fill=leaf_light)
    
    # Outline
    for ox, oy in offsets[:2]:
        r = canopy_r + random.randint(-8, 3)
        draw.ellipse([cx+ox-r, canopy_y+oy-r, cx+ox+r, canopy_y+oy+r], outline=leaf_dark, width=2)
    
    # For snow biome, add snow on branches
    if biome_name == "snow":
        for _ in range(8):
            sx = cx + random.randint(-25, 25)
            sy = canopy_y + random.randint(-20, 5)
            sw = random.randint(4, 8)
            sh = random.randint(2, 4)
            draw.rectangle([sx, sy, sx+sw, sy+sh], fill=(240, 245, 250, 200))
    
    # For desert, add cactus-like features
    if biome_name == "desert":
        # Sparse leaves
        pass
    
    return img

def aggressive_remove_background(img):
    """More aggressive background removal"""
    img = img.convert("RGBA")
    pixels = img.load()
    w, h = img.size
    
    # Collect edge pixel colors to detect background
    edge_colors = []
    for x in range(w):
        edge_colors.append(pixels[x, 0])
        edge_colors.append(pixels[x, h-1])
    for y in range(h):
        edge_colors.append(pixels[0, y])
        edge_colors.append(pixels[w-1, y])
    
    # Find most common edge color (likely background)
    from collections import Counter
    # Round colors to nearest 10 for grouping
    rounded = []
    for r, g, b, a in edge_colors:
        rounded.append((r//20*20, g//20*20, b//20*20))
    
    if rounded:
        most_common = Counter(rounded).most_common(1)[0][0]
        bg_r, bg_g, bg_b = most_common
        
        # Remove pixels similar to background color
        for y in range(h):
            for x in range(w):
                r, g, b, a = pixels[x, y]
                if (abs(r - bg_r) < 40 and abs(g - bg_g) < 40 and abs(b - bg_b) < 40):
                    pixels[x, y] = (0, 0, 0, 0)
    
    # Also remove magenta
    for y in range(h):
        for x in range(w):
            r, g, b, a = pixels[x, y]
            if r > 180 and g < 100 and b > 180:
                pixels[x, y] = (0, 0, 0, 0)
            elif r > 200 and g < 50 and b > 200:
                pixels[x, y] = (0, 0, 0, 0)
    
    # Crop to content
    bbox = img.getbbox()
    if bbox:
        img = img.crop(bbox)
    
    return img

def process_existing_textures():
    """Fix backgrounds on all existing textures"""
    dirs_to_process = [
        (f"{ASSETS}/trees", (120, 140)),
        (f"{ASSETS}/rocks", (55, 55)),
        (f"{ASSETS}/slimes", (48, 48)),
        (f"{ASSETS}", None),
    ]
    
    results = []
    for dir_path, target_size in dirs_to_process:
        if not os.path.exists(dir_path):
            continue
        for f in os.listdir(dir_path):
            if f.endswith(".png"):
                path = os.path.join(dir_path, f)
                try:
                    img = Image.open(path)
                    img = aggressive_remove_background(img)
                    if target_size:
                        w, h = img.size
                        ratio = min(target_size[0]/w, target_size[1]/h)
                        img = img.resize((max(1,int(w*ratio)), max(1,int(h*ratio))), Image.NEAREST)
                    img.save(path)
                    results.append(f"Fixed: {f} -> {img.size}")
                except Exception as e:
                    results.append(f"ERR: {f}: {e}")
    
    # Process individual files in assets root
    for f in ["player.png", "campfire_0.png", "campfire_1.png", "campfire_2.png", "campfire_3.png",
              "sword.png", "pickaxe.png", "shovel.png", "herb.png", "goldflower.png", "wood_item.png"]:
        path = f"{ASSETS}/{f}"
        if os.path.exists(path):
            try:
                img = Image.open(path)
                img = aggressive_remove_background(img)
                img.save(path)
                results.append(f"Fixed: {f} -> {img.size}")
            except Exception as e:
                results.append(f"ERR: {f}: {e}")
    
    return results

def generate_biome_trees():
    """Generate 3 tree variants for each biome"""
    for biome_name in BIOME_TREES:
        tree_dir = f"{ASSETS}/trees/{biome_name}"
        os.makedirs(tree_dir, exist_ok=True)
        
        for i in range(1, 4):
            random.seed(hash(f"{biome_name}_tree_{i}") % 2**31)
            img = generate_tree(biome_name, i, 120)
            path = f"{tree_dir}/tree_{i}.png"
            img.save(path)
            print(f"Generated: {biome_name}/tree_{i}.png -> {img.size}")

if __name__ == "__main__":
    print("=== Generating biome trees ===")
    generate_biome_trees()
    
    print("\n=== Fixing existing textures ===")
    results = process_existing_textures()
    for r in results:
        print(r)
    
    print("\nDone!")
