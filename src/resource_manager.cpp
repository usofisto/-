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
    
    // Убираем фиолетовый фон (маджента FF00FF)
    // Проходим по всем пикселям и заменяем похожие цвета на прозрачные
    unsigned char* pixels = (unsigned char*)img.data;
    int width = img.width;
    int height = img.height;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 4;
            unsigned char r = pixels[index];
            unsigned char g = pixels[index + 1];
            unsigned char b = pixels[index + 2];
            
            // Проверяем, похож ли цвет на маджента (фиолетовый)
            // Допускаем оттенки фиолетового
            if (r > 200 && g < 80 && b > 200) {
                pixels[index + 3] = 0; // Делаем прозрачным
            }
            // Также убираем очень яркий фиолетовый
            else if (r > 180 && g < 100 && b > 180 && (r + b) > 400) {
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
