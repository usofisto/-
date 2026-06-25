#pragma once
#include "raylib.h"
#include <map>
#include <string>

class ResourceManager {
public:
    static ResourceManager& Get() {
        static ResourceManager instance;
        return instance;
    }

    // Загрузка текстуры
    Texture2D LoadTex(const std::string& name, const std::string& path);
    // Получение текстуры по имени
    Texture2D GetTex(const std::string& name);
    // Очистка всех ресурсов
    void UnloadAll();

private:
    ResourceManager() {}
    ~ResourceManager() { UnloadAll(); }
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::map<std::string, Texture2D> textures;
};
