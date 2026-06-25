#include "resource_manager.h"

Texture2D ResourceManager::LoadTex(const std::string& name, const std::string& path) {
    if (textures.find(name) != textures.end()) {
        return textures[name];
    }
    
    // Пытаемся убрать фон маджента (FF00FF), который мы запросили у ИИ
    Image img = LoadImage(path.c_str());
    if (img.data != nullptr) {
        ImageColorReplace(&img, Color{255, 0, 255, 255}, BLANK);
        Texture2D tex = LoadTextureFromImage(img);
        UnloadImage(img);
        textures[name] = tex;
        return tex;
    }
    
    return Texture2D{0};
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
