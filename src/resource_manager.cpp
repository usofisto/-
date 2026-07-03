#include "resource_manager.h"

Texture2D ResourceManager::LoadTex(const std::string &name, const std::string &path)
{
    if (textures.find(name) != textures.end())
    {
        return textures[name];
    }

    // Загружаем изображение
    Image img = LoadImage(path.c_str());
    if (img.data == nullptr)
    {
        return Texture2D{0};
    }

    // Конвертируем в RGBA для работы с альфа-каналом
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    // НЕ удаляем фон для текстур травы и деревьев — они уже с прозрачным фоном
    // Удаляем фон только для предметов (мечи, кирки, руда и т.д.)
    if (name.find("grass") == std::string::npos && 
        name.find("tree") == std::string::npos &&
        name.find("slime") == std::string::npos &&
        name.find("player") == std::string::npos &&
        name.find("lake") == std::string::npos &&
        name.find("campfire") == std::string::npos)
    {
        // Удаляем фон: белый, серый, светло-серый (только для предметов)
        Color *pixels = LoadImageColors(img);
        int total = img.width * img.height;

        for (int i = 0; i < total; i++)
        {
            unsigned char r = pixels[i].r;
            unsigned char g = pixels[i].g;
            unsigned char b = pixels[i].b;

            // Белый фон (r>240, g>240, b>240)
            if (r > 240 && g > 240 && b > 240)
            {
                pixels[i].a = 0;
            }
            // Светло-серый фон (r>200, g>200, b>200)
            else if (r > 200 && g > 200 && b > 200)
            {
                pixels[i].a = 0;
            }
            // Серый фон (r>170, g>170, b>170, разброс < 30)
            else if (r > 170 && g > 170 && b > 170 &&
                     abs(r - g) < 30 && abs(g - b) < 30 && abs(r - b) < 30)
            {
                pixels[i].a = 0;
            }
            // Чёрный фон (r<20, g<20, b<20)
            else if (r < 20 && g < 20 && b < 20)
            {
                pixels[i].a = 0;
            }
        }

        // Загружаем обработанные пиксели обратно
        Image processed = {
            .data = pixels,
            .width = img.width,
            .height = img.height,
            .mipmaps = 1,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
        };

        Texture2D tex = LoadTextureFromImage(processed);
        UnloadImageColors(pixels);
        UnloadImage(img);

        textures[name] = tex;
        return tex;
    }
    else
    {
        // Для травы, деревьев, слизней, игрока, озера, костра — загружаем как есть
        Texture2D tex = LoadTextureFromImage(img);
        UnloadImage(img);
        textures[name] = tex;
        return tex;
    }
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
