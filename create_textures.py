from PIL import Image, ImageDraw
import math
import os

ASSETS = "C:/Users/DZV/Desktop/проверка способностей с нуля/assets"

def create_pickaxe():
    img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rectangle([28, 20, 36, 58], fill=(101, 67, 33, 255))
    d.rectangle([30, 22, 34, 56], fill=(120, 80, 40, 255))
    d.rectangle([16, 12, 48, 22], fill=(120, 120, 130, 255))
    d.rectangle([18, 14, 46, 20], fill=(156, 163, 175, 255))
    d.rectangle([14, 8, 20, 14], fill=(100, 100, 110, 255))
    d.rectangle([44, 8, 50, 14], fill=(100, 100, 110, 255))
    img.save(os.path.join(ASSETS, "pickaxe.png"))

def create_shovel():
    img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rectangle([29, 18, 35, 58], fill=(101, 67, 33, 255))
    d.rectangle([31, 20, 33, 56], fill=(120, 80, 40, 255))
    d.ellipse([20, 4, 44, 24], fill=(120, 120, 130, 255))
    d.ellipse([22, 6, 42, 20], fill=(156, 163, 175, 255))
    img.save(os.path.join(ASSETS, "shovel.png"))

def create_campfire_frames():
    for frame in range(4):
        img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
        d = ImageDraw.Draw(img)
        d.rectangle([18, 44, 46, 52], fill=(101, 67, 33, 255))
        d.rectangle([22, 40, 42, 48], fill=(120, 80, 40, 255))
        h = 22 + frame * 4
        for i, c in enumerate([(255,80,0,230),(255,140,0,210),(255,200,0,190)]):
            y = 44 - h + i*6
            w = 18 - i*3 + (frame%2)*3
            d.ellipse([32-w//2, y, 32+w//2, y+h//3], fill=c)
        if frame%2==0:
            d.point((26+frame*3, 8-frame), fill=(255,220,100,200))
            d.point((40-frame*2, 6), fill=(255,200,50,180))
        img.save(os.path.join(ASSETS, f"campfire_{frame}.png"))

def create_tree():
    img = Image.new('RGBA', (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.ellipse([20, 100, 108, 120], fill=(0,0,0,60))
    d.rectangle([54, 50, 74, 110], fill=(101, 67, 33, 255))
    d.rectangle([58, 52, 70, 108], fill=(120, 80, 40, 255))
    d.ellipse([20, 10, 108, 60], fill=(34, 120, 50, 255))
    d.ellipse([30, 0, 98, 50], fill=(40, 140, 55, 255))
    d.ellipse([40, -5, 88, 40], fill=(50, 160, 60, 255))
    d.ellipse([35, 15, 55, 35], fill=(45, 150, 58, 255))
    d.ellipse([70, 10, 90, 30], fill=(38, 130, 52, 255))
    img.save(os.path.join(ASSETS, "tree.png"))

def create_rock():
    img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.ellipse([10, 42, 54, 56], fill=(0,0,0,60))
    d.ellipse([8, 18, 56, 50], fill=(120, 120, 130, 255))
    d.ellipse([12, 20, 52, 46], fill=(140, 140, 150, 255))
    d.ellipse([18, 22, 32, 32], fill=(170, 170, 180, 200))
    d.line([20, 30, 40, 35], fill=(100, 100, 110, 255), width=2)
    d.line([35, 25, 42, 40], fill=(100, 100, 110, 255), width=1)
    img.save(os.path.join(ASSETS, "rock.png"))

def create_sword():
    img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rectangle([29, 40, 35, 58], fill=(101, 67, 33, 255))
    d.rectangle([22, 36, 42, 42], fill=(180, 180, 190, 255))
    d.polygon([(32, 4), (26, 36), (38, 36)], fill=(200, 200, 210, 255))
    d.polygon([(32, 8), (28, 34), (36, 34)], fill=(220, 220, 230, 255))
    img.save(os.path.join(ASSETS, "sword.png"))

def create_wood_item():
    img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.ellipse([10, 42, 54, 56], fill=(0,0,0,60))
    d.ellipse([8, 16, 56, 48], fill=(101, 67, 33, 255))
    d.ellipse([12, 18, 52, 44], fill=(120, 80, 40, 255))
    d.ellipse([22, 24, 32, 34], fill=(90, 55, 25, 255))
    d.ellipse([25, 27, 29, 31], fill=(70, 40, 18, 255))
    img.save(os.path.join(ASSETS, "wood_item.png"))

def create_herb():
    img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.line([32, 58, 32, 20], fill=(34, 140, 60, 255), width=3)
    d.line([24, 58, 28, 28], fill=(34, 130, 55, 255), width=2)
    d.line([40, 58, 36, 25], fill=(34, 135, 58, 255), width=2)
    d.ellipse([20, 14, 36, 30], fill=(34, 197, 94, 255))
    d.ellipse([28, 10, 44, 26], fill=(50, 210, 110, 255))
    d.ellipse([24, 18, 34, 28], fill=(60, 220, 120, 200))
    img.save(os.path.join(ASSETS, "herb.png"))

def create_goldflower():
    img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.line([32, 58, 32, 22], fill=(34, 140, 60, 255), width=3)
    d.ellipse([20, 8, 44, 32], fill=(253, 224, 71, 255))
    d.ellipse([24, 12, 40, 28], fill=(255, 240, 100, 255))
    d.ellipse([28, 16, 36, 24], fill=(255, 255, 200, 255))
    img.save(os.path.join(ASSETS, "goldflower.png"))

create_pickaxe()
create_shovel()
create_campfire_frames()
create_tree()
create_rock()
create_sword()
create_wood_item()
create_herb()
create_goldflower()
print("All textures created!")
