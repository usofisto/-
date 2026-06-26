#include "resource_manager.h"

Texture2D ResourceManager::LoadTex(const std::string& name, const std::string& path) {
    if (textures.find(name) != textures.end()) {
        return textures[name];
    }
    
    // Загружаем изображение
    Image img = LoadImage(path.c_str());
    if (img.data == nullptr) {
        return Texture2D{0};
    }
    
    // Конвертируем в RGBA для работы с альфа-каналом
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    
    // Убираем фон при загрузке
    unsigned char* pixels = (unsigned char*)img.data;
    int width = img.width;
    int height = img.height;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 4;
            unsigned char r = pixels[index];
            unsigned char g = pixels[index + 1];
            unsigned char b = pixels[index + 2];
            
            // 1. Убираем маджента (фиолетовый) фон
            if (r > 180 && g < 100 && b > 180) {
                pixels[index + 3] = 0;
            }
            // 2. Убираем БЕЛЫЙ фон (деревья, камни)
            else if (r > 240 && g > 240 && b > 240) {
                pixels[index + 3] = 0;
            }
            // 3. Убираем ЧЁРНЫЕ пиксели шахматки (трава)
            else if (r < 20 && g < 20 && b < 20) {
                pixels[index + 3] = 0;
            }
            // 4. Убираем СВЕТЛО-СЕРЫЙ фон
            else if (r > 200 && g > 200 && b > 200 && abs(r - g) < 20 && abs(g - b) < 20) {
                pixels[index + 3] = 0;
            }
        }
    }
    
    // Загружаем текстуру из обработанного изображения
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    
    textures[name] = tex;
    return tex;
}

Texture2D ResourceManager::GetTex(const std::string& name) {
    if (textures.find(name) != textures.end()) {
        return textures[name];
    }
    return Texture2D{0};
}

void ResourceManager::UnloadAll() {
    for (auto& pair : textures) {
        UnloadTexture(pair.second);
    }
    textures.clear();
}
