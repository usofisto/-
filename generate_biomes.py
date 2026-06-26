"""Generate unique grass textures for each biome"""
from PIL import Image, ImageDraw
import random
import os

ASSETS = "C:/Users/DZV/Desktop/проверка способностей с нуля/assets"

# Biome definitions with color palettes
BIOMES = {
    "forest": {
        "name": "Лес",
        "base_colors": [(34, 120, 50), (40, 140, 55), (28, 100, 42), (45, 130, 60), (32, 110, 48)],
        "accent_colors": [(50, 160, 70), (60, 180, 80), (25, 90, 35)],
        "detail_colors": [(100, 200, 120), (80, 180, 100), (120, 220, 140)],
        "flower_colors": [(255, 200, 50), (255, 150, 100), (200, 100, 255)],
    },
    "desert": {
        "name": "Пустыня",
        "base_colors": [(210, 180, 120), (200, 170, 110), (220, 190, 130), (195, 165, 105), (215, 185, 125)],
        "accent_colors": [(180, 150, 90), (190, 160, 100), (170, 140, 80)],
        "detail_colors": [(230, 200, 140), (240, 210, 150)],
        "flower_colors": [(255, 180, 50), (200, 150, 50)],
    },
    "snow": {
        "name": "Снег",
        "base_colors": [(220, 230, 240), (210, 220, 235), (230, 235, 245), (215, 225, 238), (225, 232, 242)],
        "accent_colors": [(200, 210, 225), (190, 200, 220), (205, 215, 230)],
        "detail_colors": [(240, 245, 250), (245, 248, 252)],
        "flower_colors": [(180, 200, 230), (150, 180, 220)],
    },
    "swamp": {
        "name": "Болото",
        "base_colors": [(60, 80, 40), (55, 75, 35), (65, 85, 45), (50, 70, 30), (70, 90, 50)],
        "accent_colors": [(45, 65, 25), (40, 60, 20), (75, 95, 55)],
        "detail_colors": [(80, 110, 60), (90, 120, 70)],
        "flower_colors": [(120, 100, 60), (100, 80, 40)],
    },
    "meadow": {
        "name": "Луга",
        "base_colors": [(80, 160, 60), (90, 170, 70), (75, 155, 55), (85, 165, 65), (70, 150, 50)],
        "accent_colors": [(100, 180, 80), (110, 190, 90), (95, 175, 75)],
        "detail_colors": [(130, 210, 110), (140, 220, 120)],
        "flower_colors": [(255, 100, 150), (255, 200, 50), (200, 100, 255), (100, 200, 255)],
    }
}

def draw_pixel_grass(draw, x, y, color, size=2):
    """Draw a small grass blade"""
    draw.rectangle([x, y, x+size, y+size*2], fill=color)

def draw_flower(draw, x, y, color, size=3):
    """Draw a small flower"""
    # Stem
    stem_color = (max(0, color[0]-80), max(0, color[1]-60), max(0, color[2]-80))
    draw.rectangle([x, y, x+1, y+size], fill=stem_color)
    # Petals
    draw.rectangle([x-1, y-1, x+2, y+1], fill=color)
    # Center
    draw.rectangle([x, y, x+1, y+1], fill=(255, 255, 200))

def draw_rock_detail(draw, x, y, color, size=4):
    """Draw a small rock/pebble"""
    dark = (max(0, color[0]-30), max(0, color[1]-30), max(0, color[2]-30))
    draw.rectangle([x, y, x+size, y+size//2], fill=color)
    draw.rectangle([x, y, x+size-1, y+size//2-1], fill=dark)

def generate_grass_texture(biome_name, index, size=100):
    """Generate a unique grass texture for a biome"""
    biome = BIOMES[biome_name]
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Base color with variation
    base = random.choice(biome["base_colors"])
    # Fill with base color slightly varied
    for y in range(0, size, 4):
        for x in range(0, size, 4):
            r_var = random.randint(-10, 10)
            g_var = random.randint(-10, 10)
            b_var = random.randint(-10, 10)
            c = (
                max(0, min(255, base[0] + r_var)),
                max(0, min(255, base[1] + g_var)),
                max(0, min(255, base[2] + b_var)),
                255
            )
            draw.rectangle([x, y, x+3, y+3], fill=c)
    
    # Draw grass blades
    num_blades = random.randint(30, 60)
    for _ in range(num_blades):
        bx = random.randint(0, size-3)
        by = random.randint(0, size-3)
        accent = random.choice(biome["accent_colors"])
        # Add some variation
        accent = (
            max(0, min(255, accent[0] + random.randint(-15, 15))),
            max(0, min(255, accent[1] + random.randint(-15, 15))),
            max(0, min(255, accent[2] + random.randint(-15, 15))),
        )
        draw_pixel_grass(draw, bx, by, accent, random.randint(1, 3))
    
    # Draw highlights/detail blades
    num_details = random.randint(10, 25)
    for _ in range(num_details):
        bx = random.randint(0, size-2)
        by = random.randint(0, size-2)
        detail = random.choice(biome["detail_colors"])
        draw_pixel_grass(draw, bx, by, detail, 1)
    
    # Draw flowers (only for forest, meadow)
    if biome_name in ["forest", "meadow", "swamp"]:
        num_flowers = random.randint(2, 6)
        for _ in range(num_flowers):
            fx = random.randint(5, size-5)
            fy = random.randint(5, size-5)
            flower = random.choice(biome["flower_colors"])
            draw_flower(draw, fx, fy, flower)
    
    # Draw small rocks/pebbles (for desert, snow)
    if biome_name in ["desert", "snow"]:
        num_rocks = random.randint(3, 8)
        for _ in range(num_rocks):
            rx = random.randint(0, size-5)
            ry = random.randint(0, size-5)
            rock_c = random.choice(biome["accent_colors"])
            draw_rock_detail(draw, rx, ry, rock_c, random.randint(2, 4))
    
    # For snow biome, add snow patches
    if biome_name == "snow":
        for _ in range(15):
            sx = random.randint(0, size-4)
            sy = random.randint(0, size-4)
            draw.rectangle([sx, sy, sx+random.randint(2, 5), sy+random.randint(1, 3)], 
                         fill=(250, 252, 255, random.randint(150, 220)))
    
    # For swamp, add dark water spots
    if biome_name == "swamp":
        for _ in range(5):
            wx = random.randint(5, size-8)
            wy = random.randint(5, size-8)
            draw.ellipse([wx, wy, wx+random.randint(4, 8), wy+random.randint(3, 6)],
                        fill=(40, 60, 30, 180))
    
    return img

def generate_biome_grass():
    """Generate 5 grass textures for each biome"""
    for biome_name in BIOMES:
        biome_dir = f"{ASSETS}/grass/{biome_name}"
        os.makedirs(biome_dir, exist_ok=True)
        
        for i in range(1, 6):
            random.seed(hash(f"{biome_name}_{i}") % 2**31)
            img = generate_grass_texture(biome_name, i, 100)
            path = f"{biome_dir}/grass_{i}.png"
            img.save(path)
            print(f"Generated: {biome_name}/grass_{i}.png -> {img.size}")

if __name__ == "__main__":
    generate_biome_grass()
    print("Done! Generated biome grass textures.")
