#include "resource_manager.h"

Texture2D ResourceManager::LoadTex(const std::string &name, const std::string &path)
{
    if (textures.find(name) != textures.end())
    {
        return textures[name];
    }

    // Загружаем изображение как Image (в оперативную память)
    Image img = LoadImage(path.c_str());
    if (img.data == nullptr)
    {
        return Texture2D{0};
    }

    // Конвертируем в RGBA для работы с альфа-каналом
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    // Удаляем белый фон — заменяем на прозрачность
    ImageColorReplace(&img, WHITE, BLANK);
    
    // Удаляем светло-серый фон
    Color lightGray = { 220, 220, 220, 255 };
    ImageColorReplace(&img, lightGray, BLANK);
    
    // Удаляем серый фон
    Color gray = { 180, 180, 180, 255 };
    ImageColorReplace(&img, gray, BLANK);

    // Переносим обработанное изображение в видеопамять (в текстуру)
    Texture2D tex = LoadTextureFromImage(img);
    
    // Выгружаем Image из оперативной памяти
    UnloadImage(img);

    textures[name] = tex;
    return tex;
}

Texture2D ResourceManager::GetTex(const std::string &name)
{
    if (textures.find(name) != textures.end())
    {
        return textures[name];
    }
    return Texture2D{0};
}

void ResourceManager::UnloadAll()
{
    for (auto &pair : textures)
    {
        UnloadTexture(pair.second);
    }
    textures.clear();
}
