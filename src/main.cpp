#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <cmath>
#include "raylib.h"

#include "game_types.h"
#include "player.h"
#include "biom.h"
#include "biome_system.h"
#include "game_systems.h"
#include "particles.h"
#include "gui.h"
#include "render.h"
#include "resource_manager.h"
#include "hud_renderer.h"
#include "menu.h"
#include "crafting.h"

// ==================== ПРЕДМЕТЫ ====================
const char* GetItemName(ItemType t) {
    switch(t) {
        case ITEM_WOOD: return "Дерево";
        case ITEM_STONE: return "Камень";
        case ITEM_COAL: return "Уголь";
        case ITEM_WOOD_PLANK: return "Доски";
        case ITEM_STICK: return "Палка";
        case ITEM_WOOD_PICKAXE: return "Дерев. кирка";
        case ITEM_WOOD_SWORD: return "Дерев. меч";
        case ITEM_WOOD_AXE: return "Дерев. топор";
        case ITEM_STONE_PICKAXE: return "Камен. кирка";
        case ITEM_STONE_SWORD: return "Камен. меч";
        case ITEM_STONE_AXE: return "Камен. топор";
        case ITEM_HERB: return "Леч. трава";
        case ITEM_GOLDFLOWER: return "Золотоцвет";
        case ITEM_APPLE: return "Яблоко";
        case ITEM_BREAD: return "Хлеб";
        case ITEM_TORCH: return "Факел";
        default: return "?";
    }
}

Color GetItemColor(ItemType t) {
    switch(t) {
        case ITEM_WOOD: case ITEM_WOOD_PLANK: return Color{139, 90, 43, 255};
        case ITEM_STONE: case ITEM_STONE_PICKAXE: case ITEM_STONE_SWORD: case ITEM_STONE_AXE:
            return Color{156, 163, 175, 255};
        case ITEM_STICK: return Color{180, 140, 80, 255};
        case ITEM_WOOD_PICKAXE: case ITEM_WOOD_SWORD: case ITEM_WOOD_AXE:
            return Color{160, 120, 60, 255};
        case ITEM_HERB: return Color{34, 197, 94, 255};
        case ITEM_GOLDFLOWER: return Color{253, 224, 71, 255};
        case ITEM_APPLE: return Color{220, 50, 50, 255};
        case ITEM_BREAD: return Color{210, 170, 90, 255};
        case ITEM_TORCH: return Color{245, 158, 11, 255};
        default: return Color{180, 180, 180, 255};
    }
}

void DrawItemIcon(Vector2 center, ItemType t, float size) {
    // Используем PNG текстуры для иконок
    std::string texKey;
    switch(t) {
        case ITEM_WOOD: texKey = "wood_item"; break;
        case ITEM_STONE: texKey = "rock_1"; break;
        case ITEM_STICK: texKey = "wood_item"; break;
        case ITEM_WOOD_PICKAXE: case ITEM_STONE_PICKAXE: texKey = "pickaxe"; break;
        case ITEM_WOOD_SWORD: case ITEM_STONE_SWORD: texKey = "sword"; break;
        case ITEM_WOOD_AXE: case ITEM_STONE_AXE: texKey = "shovel"; break;
        case ITEM_HERB: texKey = "herb"; break;
        case ITEM_GOLDFLOWER: texKey = "goldflower"; break;
        case ITEM_TORCH: texKey = "campfire_0"; break;
        case ITEM_BREAD: texKey = "herb"; break;
        case ITEM_WOOD_PLANK: texKey = "wood_item"; break;
        default: break;
    }
    
    Texture2D tex = {0};
    if (!texKey.empty()) tex = ResourceManager::Get().GetTex(texKey.c_str());
    
    if (tex.id != 0) {
        // Рисуем PNG текстуру с правильным масштабом
        float scale = size / tex.width;
        DrawTextureEx(tex, {center.x - size/2, center.y - size/2}, 0, scale, WHITE);
    } else {
        // Fallback процедурный
        Color c = GetItemColor(t);
        DrawRectangle(center.x - size*0.3f, center.y - size*0.3f, size*0.6f, size*0.6f, c);
    }
}

// ==================== MAIN ====================
int main() {
    const int windowWidth = 950;
    const int windowHeight = 650;
    InitWindow(windowWidth, windowHeight, "Legends of Green Meadow - 2D RPG");
    SetTargetFPS(60);
    bool isFullscreen = false;

    // Шрифт с кириллицей
    int codepoints[512];
    for (int i = 0; i < 128; i++) codepoints[i] = 32 + i;
    for (int i = 0; i < 256; i++) codepoints[128 + i] = 0x0400 + i;
    Font font = LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", 24, codepoints, 384);
    if (font.texture.id == 0) font = GetFontDefault();

    initBiomeRandom();
    InitParticles();

    // ==================== ЗАГРУЗКА ТЕКСТУР ====================
    ResourceManager::Get().LoadTex("grass", "assets/grass.png");
    ResourceManager::Get().LoadTex("player", "assets/player.png");
    ResourceManager::Get().LoadTex("slime", "assets/slime.png");
    ResourceManager::Get().LoadTex("pickaxe", "assets/pickaxe.png");
    ResourceManager::Get().LoadTex("shovel", "assets/shovel.png");
    ResourceManager::Get().LoadTex("sword", "assets/sword.png");
    ResourceManager::Get().LoadTex("tree", "assets/tree.png");
    ResourceManager::Get().LoadTex("rock", "assets/rock.png");
    ResourceManager::Get().LoadTex("campfire_0", "assets/campfire_0.png");
    ResourceManager::Get().LoadTex("campfire_1", "assets/campfire_1.png");
    ResourceManager::Get().LoadTex("campfire_2", "assets/campfire_2.png");
    ResourceManager::Get().LoadTex("campfire_3", "assets/campfire_3.png");
    ResourceManager::Get().LoadTex("wood_item", "assets/wood_item.png");
    ResourceManager::Get().LoadTex("herb", "assets/herb.png");
    ResourceManager::Get().LoadTex("goldflower", "assets/goldflower.png");

    // Новые текстуры деревьев (10 видов)
    for (int i = 1; i <= 10; i++) {
        std::string key = "tree_" + std::to_string(i);
        std::string path = "assets/trees/" + key + ".png";
        ResourceManager::Get().LoadTex(key.c_str(), path.c_str());
    }

    // Новые текстуры травы (10 чанков)
    for (int i = 1; i <= 10; i++) {
        std::string key = "grass_" + std::to_string(i);
        std::string path = "assets/grass/" + key + ".png";
        ResourceManager::Get().LoadTex(key.c_str(), path.c_str());
    }

    // Новые текстуры камней (5 видов)
    for (int i = 1; i <= 5; i++) {
        std::string key = "rock_" + std::to_string(i);
        std::string path = "assets/rocks/" + key + ".png";
        ResourceManager::Get().LoadTex(key.c_str(), path.c_str());
    }

    // Новые текстуры слизней
    ResourceManager::Get().LoadTex("slime_green", "assets/slimes/slime_green.png");
    ResourceManager::Get().LoadTex("slime_red", "assets/slimes/slime_red.png");
    ResourceManager::Get().LoadTex("slime_blue", "assets/slimes/slime_blue.png");

    // Озеро
    ResourceManager::Get().LoadTex("lake", "assets/lake.png");

    // ==================== ИГРОВЫЕ ПЕРЕМЕННЫЕ ====================
    GameState state = STATE_MENU;
    Player player;
    std::string playerNameInput = "";
    int framesCounter = 0;

    GameMenu gameMenu;
    CraftingSystem craftingSystem;
    FarmingSystem farmingSystem;
    WeatherController weatherController;
    QuestController questController;
    TradingSystem tradingSystem;
    FishingController fishingController;
    SurvivalStats survivalStats;
    DayNightCycle dayNightCycle;
    bool showInventory = false;
    bool showCrafting = false;

    // Цвета
    Color bgDark = Color{ 24, 26, 32, 255 };
    Color bgPanel = Color{ 34, 38, 48, 255 };
    Color textWhite = Color{ 243, 244, 246, 255 };
    Color textGray = Color{ 156, 163, 175, 255 };

    // Позиция игрока
    Vector2 playerPos = { 1500.0f, 1500.0f };
    Vector2 playerFacing = { 0.0f, 1.0f };
    float playerSpeed = 190.0f;
    float playerRadius = 12.0f;

    // Камера
    Camera2D camera = { 0 };
    camera.target = playerPos;
    camera.offset = Vector2{ windowWidth / 2.0f, windowHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Мир: деревья, камни, руины, предметы, слизни
    std::vector<TreeInfo> trees;
    std::vector<RockInfo> rocks;
    std::vector<StoneWall> ruins;
    std::vector<MapItem> mapItems;
    std::vector<WanderingSlime> slimes;
    std::vector<FloatingText> floatingTexts;
    std::vector<std::pair<std::string, float>> miniLog;

    // Травяные плитки
    Color grassTiles[30][30];

    // Объекты лагеря
    Vector2 campfirePos = { 1460.0f, 1000.0f };
    Vector2 tentPos = { 1540.0f, 985.0f };
    Vector2 lakePos = { 600.0f, 400.0f };
    float lakeRadius = 150.0f;

    // Бой
    int activeSlimeIndex = -1;
    int slimeHp = 0, slimeMaxHp = 0;
    std::string slimeType = "Зеленая Слизь";
    int combatTurnState = 0;
    float combatTimer = 0.0f;
    std::vector<std::string> combatLog;
    float screenShakeIntensity = 0.0f;
    float redFlashTimer = 0.0f;
    float slashEffectTimer = 0.0f;

    // Взмах оружием
    float weaponSwingTimer = 0.0f;
    float weaponSwingAngle = 0.0f;

    // ==================== ВСПОМОГАТЕЛЬНЫЕ ЛЯМБДЫ ====================
    auto CountItems = [&](ItemType t) -> int {
        int count = 0;
        for (auto& item : player.inventory) if (item == t) count++;
        return count;
    };
    auto RemoveOneItem = [&](ItemType t) {
        for (auto it = player.inventory.begin(); it != player.inventory.end(); ++it) {
            if (*it == t) { player.inventory.erase(it); return; }
        }
    };

    // ==================== ГЕНЕРАЦИЯ МИРА ====================
    auto InitWorldEntities = [&]() {
        trees.clear(); rocks.clear(); ruins.clear();
        mapItems.clear(); slimes.clear(); floatingTexts.clear(); miniLog.clear();
        playerPos = Vector2{ 1500.0f, 1500.0f };
        playerFacing = Vector2{ 0.0f, 1.0f };
        camera.target = playerPos;
        survivalStats.health = player.maxHealth;
        survivalStats.maxHealth = player.maxHealth;

        AddLogMessage("Проснулся в лагере, вроде всё тихо.", miniLog);
        AddLogMessage("[WASD] бег  [E] добыча  [I] инв.  [C] крафт  [F11] fullscreen", miniLog);

        srand(6789);
        for (int x = 0; x < 30; ++x)
            for (int y = 0; y < 30; ++y) {
                int r = rand()%8, g = rand()%12, b = rand()%8;
                grassTiles[x][y] = Color{(unsigned char)(22+r),(unsigned char)(95+g),(unsigned char)(44+b),255};
            }

        // Руины
        ruins.push_back({Rectangle{750,1300,150,30}});
        ruins.push_back({Rectangle{1300,700,30,180}});

        // Камни (с HP)
        for (int i = 0; i < 8; ++i) {
            Vector2 pos;
            while (true) {
                pos = {(float)(100+rand()%1800),(float)(100+rand()%1800)};
                float dc = sqrtf(powf(pos.x-1500,2)+powf(pos.y-1500,2));
                float dl = sqrtf(powf(pos.x-lakePos.x,2)+powf(pos.y-lakePos.y,2));
                if (dc>250 && dl>(lakeRadius+30)) break;
            }
            RockInfo ri;
            ri.position = pos;
            ri.radius = 16.0f + (float)(rand()%6);
            ri.hp = 4; ri.maxHp = 4;
            ri.active = true;
            ri.shakeTimer = 0.0f;
            ri.respawnTimer = 0.0f;
            ri.rockType = 1 + rand()%5; // 5 видов камней
            rocks.push_back(ri);
        }

        // Деревья (с HP)
        int attempts = 0;
        while (trees.size() < 40 && attempts < 800) {
            attempts++;
            Vector2 c = {(float)(50+rand()%1900),(float)(50+rand()%1900)};
            float dc = sqrtf(powf(c.x-1500,2)+powf(c.y-1500,2));
            float dl = sqrtf(powf(c.x-lakePos.x,2)+powf(c.y-lakePos.y,2));
            if (dc<220 || dl<(lakeRadius+25)) continue;
            bool bad = false;
            for (auto& w : ruins) if (CheckCollisionCircleRec(c,25,w.rect)) {bad=true;break;}
            if (bad) continue;
            for (auto& t : trees) if (sqrtf(powf(c.x-t.position.x,2)+powf(c.y-t.position.y,2))<55) {bad=true;break;}
            if (!bad) {
                TreeInfo ti;
                ti.position = c;
                ti.radius = 15.0f;
                ti.hp = 5; ti.maxHp = 5;
                ti.active = true;
                ti.shakeTimer = 0.0f;
                ti.respawnTimer = 0.0f;
                ti.treeType = 1 + rand()%10; // 10 видов деревьев
                ti.biomeType = (int)GetBiomeAtPosition(c.x, c.y); // Биом по позиции
                trees.push_back(ti);
            }
        }

        // Предметы на карте (трава, золотоцвет)
        for (int i = 0; i < 15; ++i) {
            Vector2 pos;
            while (true) {
                pos = {(float)(80+rand()%1840),(float)(80+rand()%1840)};
                float d = sqrtf(powf(pos.x-1500,2)+powf(pos.y-1500,2));
                float dl = sqrtf(powf(pos.x-lakePos.x,2)+powf(pos.y-lakePos.y,2));
                if (d>200 && dl>(lakeRadius+20)) break;
            }
            mapItems.push_back({pos, ITEM_HERB, true, 0.0f});
            while (true) {
                pos = {(float)(80+rand()%1840),(float)(80+rand()%1840)};
                float d = sqrtf(powf(pos.x-1500,2)+powf(pos.y-1500,2));
                float dl = sqrtf(powf(pos.x-lakePos.x,2)+powf(pos.y-lakePos.y,2));
                if (d>200 && dl>(lakeRadius+20)) break;
            }
            mapItems.push_back({pos, ITEM_GOLDFLOWER, true, 0.0f});
        }

        // Слизни
        for (int i = 0; i < 7; ++i) {
            Vector2 pos;
            while (true) {
                pos = {(float)(100+rand()%1800),(float)(100+rand()%1800)};
                float d = sqrtf(powf(pos.x-1500,2)+powf(pos.y-1500,2));
                float dl = sqrtf(powf(pos.x-lakePos.x,2)+powf(pos.y-lakePos.y,2));
                if (d>260 && dl>(lakeRadius+30)) break;
            }
            WanderingSlime s;
            s.position = pos; s.targetPosition = pos;
            s.wanderTimer = 0; s.speed = 40.0f+(float)(rand()%28);
            s.maxHp = 25+rand()%15; s.hp = s.maxHp;
            s.active = true; s.id = i;
            s.slimeType = 1 + rand()%3; // 3 вида слизней
            slimes.push_back(s);
        }
        
        // Инициализация новых систем
        farmingSystem.plots.clear();
        // Добавляем фермерские грядки рядом с лагерем
        for (int i = 0; i < 6; i++) {
            farmingSystem.AddPlot(Vector2{1500.0f + (i % 3) * 40.0f, 1560.0f + (i / 3) * 40.0f});
        }
        
        // Инициализация торговца
        tradingSystem.InitMerchant(Vector2{1400.0f, 1500.0f});
        
        // Инициализация рыбацких точек
        fishingController.spots.clear();
        fishingController.AddSpot(Vector2{lakePos.x + 100, lakePos.y});
        fishingController.AddSpot(Vector2{lakePos.x - 100, lakePos.y});
        fishingController.AddSpot(Vector2{lakePos.x, lakePos.y + 100});
        
        // Генерация квестов
        questController.GenerateDailyQuests(dayNightCycle.dayCount);
        
        // Волки (появляются с 3 дня)
        if (dayNightCycle.dayCount >= 3) {
            for (int i = 0; i < 3; i++) {
                Vector2 pos;
                while (true) {
                    pos = {(float)(200+rand()%2600),(float)(200+rand()%2600)};
                    float d = sqrtf(powf(pos.x-1500,2)+powf(pos.y-1500,2));
                    if (d > 400) break;
                }
                Wolf w;
                w.position = pos; w.targetPosition = pos;
                w.wanderTimer = 0; w.speed = 60.0f;
                w.maxHp = 40 + dayNightCycle.dayCount * 5;
                w.hp = w.maxHp; w.damage = 8;
                w.active = true; w.isChasing = false;
                w.attackCooldown = 0;
            }
        }
        
        // Скелеты (ночные враги)
        if (dayNightCycle.dayCount >= 2) {
            for (int i = 0; i < 2; i++) {
                Vector2 pos;
                while (true) {
                    pos = {(float)(200+rand()%2600),(float)(200+rand()%2600)};
                    float d = sqrtf(powf(pos.x-1500,2)+powf(pos.y-1500,2));
                    if (d > 500) break;
                }
                Skeleton sk;
                sk.position = pos; sk.targetPosition = pos;
                sk.wanderTimer = 0; sk.speed = 35.0f;
                sk.maxHp = 50 + dayNightCycle.dayCount * 8;
                sk.hp = sk.maxHp; sk.damage = 12;
                sk.active = true; sk.attackCooldown = 0;
            }
        }
    };

    // ==================== ГЛАВНЫЙ ЦИКЛ ====================
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        framesCounter++;

        // ==================== F11 FULLSCREEN ====================
        if (IsKeyPressed(KEY_F11)) {
            isFullscreen = !isFullscreen;
            if (isFullscreen) {
                int monitor = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
                ToggleFullscreen();
            } else {
                ToggleFullscreen();
                SetWindowSize(windowWidth, windowHeight);
            }
            // Обновляем offset камеры под новый размер окна
            camera.offset = Vector2{ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
        }

        // ==================== ЛОГИКА ====================
        if (state == STATE_MENU) {
            gameMenu.HandleMainMenuInput(font, windowWidth, windowHeight);
            MenuState ms = gameMenu.GetCurrentState();
            if (ms == MENU_SINGLEPLAYER) gameMenu.SetCurrentState(MENU_WORLD_CREATE);
            if (ms == MENU_WORLD_CREATE) gameMenu.HandleWorldCreateInput(font, windowWidth, windowHeight);
            if (ms == MENU_SETTINGS) gameMenu.HandleSettingsInput(font, windowWidth, windowHeight);
            if (ms == MENU_PLAYING) {
                state = STATE_WELCOME;
                gameMenu.SetCurrentState(MENU_MAIN);
            }
        }
        else if (state == STATE_WELCOME) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32 && key <= 125) || (key >= 0x0400 && key <= 0x04FF)) {
                    if (playerNameInput.length() < 14) AppendUnicodeToUTF8(playerNameInput, key);
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !playerNameInput.empty()) {
                do { playerNameInput.pop_back(); }
                while (!playerNameInput.empty() && ((playerNameInput.back() & 0xC0) == 0x80));
            }
            if (IsKeyPressed(KEY_ENTER) && !playerNameInput.empty()) {
                player.name = playerNameInput;
                state = STATE_CLASS_SELECT;
            }
        }
        else if (state == STATE_2D_WORLD) {
            // Показ/скрытие инвентаря и крафта
            if (IsKeyPressed(KEY_I) || IsKeyPressed(KEY_TAB)) { showInventory = !showInventory; showCrafting = false; }
            if (IsKeyPressed(KEY_C)) { showCrafting = !showCrafting; showInventory = false; }
            if (IsKeyPressed(KEY_ESCAPE)) { showInventory = false; showCrafting = false; }

            // Таймер взмаха оружием
            if (weaponSwingTimer > 0) {
                weaponSwingTimer -= dt;
                weaponSwingAngle = -60.0f + (1.0f - weaponSwingTimer / 0.3f) * 180.0f;
            }

            // Обновление дня/ночи
            dayNightCycle.Update(dt);
            
            // Обновление новых систем
            farmingSystem.Update(dt);
            float weatherSpeedMod = 1.0f;
            int currentBiome = (int)GetBiomeAtPosition(playerPos.x, playerPos.y);
            weatherController.Update(dt, currentBiome, weatherSpeedMod);
            fishingController.Update(dt);
            player.UpdateEffects(dt);
            
            // Проверка квестов
            if (dayNightCycle.dayCount != questController.quests.dailyQuests.size()) {
                questController.GenerateDailyQuests(dayNightCycle.dayCount);
            }

            // Голод
            survivalStats.hungerTimer -= dt;
            if (survivalStats.hungerTimer <= 0) {
                survivalStats.hunger = std::max(0, survivalStats.hunger - 1);
                survivalStats.hungerTimer = 30.0f;
            }
            // Голод: 0 HP => теряем здоровье
            if (survivalStats.hunger <= 0 && survivalStats.health > 0)
                survivalStats.health = std::max(0, survivalStats.health - 1);
            // Регенерация если голод > 10
            if (survivalStats.hunger > 10 && survivalStats.health < survivalStats.maxHealth)
                survivalStats.health = std::min(survivalStats.maxHealth, survivalStats.health + 1);
            player.health = survivalStats.health;

            // ==================== ДВИЖЕНИЕ ====================
            if (!showInventory && !showCrafting) {
                Vector2 moveDir = {0,0};
                if (IsKeyDown(KEY_W)||IsKeyDown(KEY_UP)) moveDir.y-=1;
                if (IsKeyDown(KEY_S)||IsKeyDown(KEY_DOWN)) moveDir.y+=1;
                if (IsKeyDown(KEY_A)||IsKeyDown(KEY_LEFT)) moveDir.x-=1;
                if (IsKeyDown(KEY_D)||IsKeyDown(KEY_RIGHT)) moveDir.x+=1;

                bool isMoving = (moveDir.x!=0||moveDir.y!=0);
                if (isMoving) {
                    float len = sqrtf(moveDir.x*moveDir.x+moveDir.y*moveDir.y);
                    moveDir.x/=len; moveDir.y/=len;
                    playerFacing = moveDir;
                    float sx = moveDir.x*playerSpeed*dt;
                    float sy = moveDir.y*playerSpeed*dt;

                    // Столкновения по X
                    Vector2 nx = {playerPos.x+sx, playerPos.y};
                    bool cx = false;
                    if (nx.x<15||nx.x>2985) cx=true;
                    for (auto& t:trees) if (t.active && sqrtf(powf(nx.x-t.position.x,2)+powf(nx.y-t.position.y,2))<(playerRadius+t.radius*0.4f)){cx=true;break;}
                    for (auto& r:rocks) if (r.active && sqrtf(powf(nx.x-r.position.x,2)+powf(nx.y-r.position.y,2))<(playerRadius+r.radius*0.5f)){cx=true;break;}
                    for (auto& w:ruins) if (CheckCollisionCircleRec(nx,playerRadius,w.rect)){cx=true;break;}
                    if (sqrtf(powf(nx.x-lakePos.x,2)+powf(nx.y-lakePos.y,2))<(playerRadius+lakeRadius-6)) cx=true;
                    if (!cx) playerPos.x=nx.x;

                    // Столкновения по Y
                    Vector2 ny = {playerPos.x, playerPos.y+sy};
                    bool cy = false;
                    if (ny.y<15||ny.y>2985) cy=true;
                    for (auto& t:trees) if (t.active && sqrtf(powf(ny.x-t.position.x,2)+powf(ny.y-t.position.y,2))<(playerRadius+t.radius*0.4f)){cy=true;break;}
                    for (auto& r:rocks) if (r.active && sqrtf(powf(ny.x-r.position.x,2)+powf(ny.y-r.position.y,2))<(playerRadius+r.radius*0.5f)){cy=true;break;}
                    for (auto& w:ruins) if (CheckCollisionCircleRec(ny,playerRadius,w.rect)){cy=true;break;}
                    if (sqrtf(powf(ny.x-lakePos.x,2)+powf(ny.y-lakePos.y,2))<(playerRadius+lakeRadius-6)) cy=true;
                    if (!cy) playerPos.y=ny.y;

                    // Частицы мага
                    if (player.className=="Маг" && rand()%100<15) {
                        Vector2 tip = {playerPos.x+(playerFacing.x<0?-12:12), playerPos.y-12};
                        SpawnParticle(tip, {(float)(rand()%30-15),(float)(rand()%30-15)}, Color{253,224,71,255}, 2.0f, 0.4f+(rand()%4)/10.0f);
                    }
                    // Частицы пыли при ходьбе
                    if (rand()%100<20) {
                        SpawnParticle({playerPos.x+(float)(rand()%10-5), playerPos.y+10}, {(float)(rand()%16-8),(float)(-rand()%8)}, Color{180,170,150,180}, 2.0f+(rand()%2), 0.3f+(rand()%3)/10.0f);
                    }
                }

                // Камера следит за игроком
                camera.target.x += (playerPos.x-camera.target.x)*0.08f;
                camera.target.y += (playerPos.y-camera.target.y)*0.08f;

                // ==================== ОТДЫХ В ПАЛАТКЕ ====================
                float dtTent = sqrtf(powf(playerPos.x-tentPos.x,2)+powf(playerPos.y-tentPos.y,2));
                if (dtTent<45 && IsKeyPressed(KEY_E)) {
                    survivalStats.health = survivalStats.maxHealth;
                    survivalStats.hunger = survivalStats.maxHunger;
                    player.health = survivalStats.maxHealth;
                    AddLogMessage("Отдохнул в палатке. HP и голод полные.", miniLog);
                    floatingTexts.push_back({"+HP +Голод", Vector2{playerPos.x,playerPos.y-15}, Color{34,197,94,255}, 1,-45,1.2f,true});
                    for (int p=0;p<12;p++) { float a=p*6.28f/12; SpawnParticle(playerPos, {cosf(a)*45,sinf(a)*45}, Color{74,222,128,255}, 3, 0.6f); }
                }

                // ==================== ДОБЫЧА РЕСУРСОВ [E] ====================
                bool gathered = false;
                if (IsKeyPressed(KEY_E)) {

                    // Добыча дерева
                    for (auto& t : trees) {
                        if (!t.active) continue;
                        float d = sqrtf(powf(playerPos.x-t.position.x,2)+powf(playerPos.y-t.position.y,2));
                        if (d < 50) {
                            t.hp--;
                            t.shakeTimer = 0.2f;
                            weaponSwingTimer = 0.3f;

                            // Частицы
                            for (int p=0;p<6;p++)
                                SpawnParticle(t.position, {(float)(rand()%20-10),(float)(-15-rand()%15)}, Color{139,90,43,255}, 2, 0.5f);

                            if (t.hp <= 0) {
                                t.active = false;
                                t.respawnTimer = 60.0f;
                                int amt = 2 + rand()%3;
                                for (int w=0;w<amt;w++) player.inventory.push_back(ITEM_WOOD);
                                std::stringstream ss; ss<<"+"<<amt<<" Дерево";
                                floatingTexts.push_back({ss.str(), Vector2{t.position.x,t.position.y-20}, Color{139,90,43,255}, 1,-40,1.2f,true});
                                AddLogMessage("Срубил дерево, +"+std::to_string(amt)+" дерева.", miniLog);
                                player.levelSystem.AddXP(10);
                                questController.UpdateQuest(QUEST_GATHER_WOOD);
                            } else {
                                std::stringstream ss; ss<<"-1 HP ("<<t.hp<<"/"<<t.maxHp<<")";
                                floatingTexts.push_back({ss.str(), Vector2{t.position.x,t.position.y-20}, Color{200,200,200,255}, 1,-40,1.0f,true});
                            }
                            gathered = true;
                            break;
                        }
                    }

                    // Добыча камня
                    if (!gathered) {
                        for (auto& r : rocks) {
                            if (!r.active) continue;
                            float d = sqrtf(powf(playerPos.x-r.position.x,2)+powf(playerPos.y-r.position.y,2));
                            if (d < 40) {
                                r.hp--;
                                r.shakeTimer = 0.2f;
                                weaponSwingTimer = 0.3f;

                                for (int p=0;p<4;p++)
                                    SpawnParticle(r.position, {(float)(rand()%16-8),(float)(-12-rand()%12)}, Color{156,163,175,255}, 2, 0.4f);

                                if (r.hp <= 0) {
                                    r.active = false;
                                    r.respawnTimer = 45.0f;
                                    int amt = 2 + rand()%2;
                                    for (int s=0;s<amt;s++) player.inventory.push_back(ITEM_STONE);
                                    std::stringstream ss; ss<<"+"<<amt<<" Камень";
                                    floatingTexts.push_back({ss.str(), Vector2{r.position.x,r.position.y-20}, Color{156,163,175,255}, 1,-40,1.2f,true});
                                    AddLogMessage("Выбил камень, +"+std::to_string(amt)+" камня.", miniLog);
                                    player.levelSystem.AddXP(10);
                                    questController.UpdateQuest(QUEST_GATHER_STONE);
                                } else {
                                    std::stringstream ss; ss<<"-1 HP ("<<r.hp<<"/"<<r.maxHp<<")";
                                    floatingTexts.push_back({ss.str(), Vector2{r.position.x,r.position.y-20}, Color{200,200,200,255}, 1,-40,1.0f,true});
                                }
                                gathered = true;
                                break;
                            }
                        }
                    }
                }
                
                // ==================== ФЕРМЕРСТВО [E] ====================
                if (!gathered && IsKeyPressed(KEY_E)) {
                    // Собрать урожай
                    if (farmingSystem.Harvest(playerPos, player)) {
                        AddLogMessage("Собрал урожай!", miniLog);
                        floatingTexts.push_back({"+Урожай", playerPos, Color{34,197,94,255}, 1,-40,1.2f,true});
                        gathered = true;
                    }
                    // Посадить семена (если есть)
                    else if (player.CountItems(ITEM_SEED_WHEAT) > 0) {
                        if (farmingSystem.PlantSeed(playerPos, CROP_WHEAT, player)) {
                            AddLogMessage("Посадил пшеницу.", miniLog);
                            gathered = true;
                        }
                    }
                    else if (player.CountItems(ITEM_SEED_CARROT) > 0) {
                        if (farmingSystem.PlantSeed(playerPos, CROP_CARROT, player)) {
                            AddLogMessage("Посадил морковь.", miniLog);
                            gathered = true;
                        }
                    }
                    else if (player.CountItems(ITEM_SEED_POTATO) > 0) {
                        if (farmingSystem.PlantSeed(playerPos, CROP_POTATO, player)) {
                            AddLogMessage("Посадил картошку.", miniLog);
                            gathered = true;
                        }
                    }
                }
                
                // ==================== РЫБАЛКА [F] ====================
                if (IsKeyPressed(KEY_F)) {
                    if (fishingController.isFishing) {
                        if (fishingController.TryCatch()) {
                            player.AddItem(ITEM_FISH);
                            player.levelSystem.AddXP(15);
                            AddLogMessage("Поймал рыбу! +15 XP", miniLog);
                            floatingTexts.push_back({"+Рыба", playerPos, Color{100,150,255,255}, 1,-40,1.2f,true});
                            questController.UpdateQuest(QUEST_FISH);
                        }
                    } else {
                        if (fishingController.StartFishing(playerPos)) {
                            AddLogMessage("Забросил удочку...", miniLog);
                        }
                    }
                }
                
                // ==================== ТОРГОВЛЯ [T] ====================
                if (IsKeyPressed(KEY_T)) {
                    if (tradingSystem.IsNearMerchant(playerPos)) {
                        // Простая торговля - купить зелье HP за золото
                        if (player.gold >= 40) {
                            player.gold -= 40;
                            player.AddItem(ITEM_POTION_HP);
                            AddLogMessage("Купил зелье HP за 40 золота.", miniLog);
                            floatingTexts.push_back({"+Зелье HP", playerPos, Color{239,68,68,255}, 1,-40,1.2f,true});
                        } else {
                            AddLogMessage("Недостаточно золота! Нужно 40.", miniLog);
                        }
                    }
                }
                
                // ==================== ПОЛИВКА [R] ====================
                if (IsKeyPressed(KEY_R)) {
                    if (farmingSystem.WaterPlot(playerPos)) {
                        AddLogMessage("Полил грядку.", miniLog);
                    }
                }

                // ==================== СБОР ПРЕДМЕТОВ НА КАРТЕ ====================
                for (auto& item:mapItems) {
                    if (!item.active) { item.respawnTimer-=dt; if (item.respawnTimer<=0) item.active=true; continue; }
                    float d = sqrtf(powf(playerPos.x-item.position.x,2)+powf(playerPos.y-item.position.y,2));
                    if (d<(playerRadius+8)) {
                        item.active=false; item.respawnTimer=12;
                        if (item.type==ITEM_HERB) {
                            player.inventory.push_back(ITEM_HERB);
                            floatingTexts.push_back({"+Трава", Vector2{item.position.x,item.position.y-10}, Color{34,197,94,255}, 1,-40,1.2f,true});
                            AddLogMessage("Сорвал лечебную траву.", miniLog);
                        } else if (item.type==ITEM_GOLDFLOWER) {
                            int gold = 10+rand()%16;
                            player.gold += gold;
                            std::stringstream ss; ss<<"+"<<gold<<" Золота";
                            floatingTexts.push_back({ss.str(), Vector2{item.position.x,item.position.y-10}, Color{253,224,71,255}, 1,-40,1.2f,true});
                            std::stringstream sl; sl<<"Сорвал золотоцвет, +"<<gold<<" монет.";
                            AddLogMessage(sl.str(), miniLog);
                        }
                    }
                }

                // ==================== СЛИЗНИ ====================
                for (size_t i=0;i<slimes.size();++i) {
                    auto& s=slimes[i];
                    if (!s.active) continue;
                    s.wanderTimer-=dt;
                    if (s.wanderTimer<=0) {
                        s.wanderTimer=2.5f+(rand()%35)/10.0f;
                        float a=(float)(rand()%360)*3.14159f/180;
                        float r=70+rand()%100;
                        s.targetPosition={s.position.x+cosf(a)*r, s.position.y+sinf(a)*r};
                        s.targetPosition.x=std::max(50.f,std::min(2950.f,s.targetPosition.x));
                        s.targetPosition.y=std::max(50.f,std::min(2950.f,s.targetPosition.y));
                        float dc=sqrtf(powf(s.targetPosition.x-1000,2)+powf(s.targetPosition.y-1000,2));
                        float dl=sqrtf(powf(s.targetPosition.x-lakePos.x,2)+powf(s.targetPosition.y-lakePos.y,2));
                        if (dc<250||dl<(lakeRadius+20)) s.targetPosition=s.position;
                    }
                    Vector2 toT={s.targetPosition.x-s.position.x,s.targetPosition.y-s.position.y};
                    float dt2=sqrtf(toT.x*toT.x+toT.y*toT.y);
                    if (dt2>2) { s.position.x+=(toT.x/dt2)*s.speed*dt; s.position.y+=(toT.y/dt2)*s.speed*dt; }
                    if (rand()%100<8) {
                        Color pCol;
                        switch (s.slimeType) {
                            case 2: pCol = Color{248,113,113,160}; break;
                            case 3: pCol = Color{96,165,250,160}; break;
                            default: pCol = Color{74,222,128,160}; break;
                        }
                        SpawnParticle(s.position, {0,0}, pCol, 3+rand()%3, 0.6f+(rand()%5)/10.f);
                    }

                    // Встреча со слизнем
                    float dp=sqrtf(powf(playerPos.x-s.position.x,2)+powf(playerPos.y-s.position.y,2));
                    if (dp<(playerRadius+14)) {
                        state=STATE_MEADOW_SLIME;
                        activeSlimeIndex=(int)i; slimeHp=s.hp; slimeMaxHp=s.maxHp;
                        // Определяем тип слизня
                        switch (s.slimeType) {
                            case 2: slimeType="Алая Слизь"; break;
                            case 3: slimeType="Ледяная Слизь"; break;
                            default: slimeType="Зеленая Слизь"; break;
                        }
                        combatLog.clear(); combatLog.push_back("!!! Нарвался на "+slimeType+"!");
                        combatTurnState=0; combatTimer=0; screenShakeIntensity=0; redFlashTimer=0; slashEffectTimer=0;
                    }
                }

                // Частицы костра
                if (rand()%100<25) SpawnParticle(campfirePos, {(float)(rand()%30-15),(float)(-35-rand()%30)}, Color{245,158,11,255}, 2.5f+rand()%2, 0.8f+(rand()%6)/10.f);
            }

            // Респавн деревьев и камней
            for (auto& t : trees) {
                if (!t.active) {
                    t.respawnTimer -= dt;
                    if (t.respawnTimer <= 0) { t.active = true; t.hp = t.maxHp; }
                }
                if (t.shakeTimer > 0) t.shakeTimer -= dt;
            }
            for (auto& r : rocks) {
                if (!r.active) {
                    r.respawnTimer -= dt;
                    if (r.respawnTimer <= 0) { r.active = true; r.hp = r.maxHp; }
                }
                if (r.shakeTimer > 0) r.shakeTimer -= dt;
            }

            // Обновление текстов
            for (auto& t:floatingTexts) { if (!t.active) continue; t.position.y+=t.ySpeed*dt; t.lifetime-=dt; t.alpha=t.lifetime/1.2f; if (t.lifetime<=0) t.active=false; }
            for (auto& item:miniLog) { if (item.second>0) item.second-=dt; }
        }
        // ==================== БОЙ ====================
        else if (state==STATE_MEADOW_SLIME) {
            if (screenShakeIntensity>0) screenShakeIntensity-=dt*12;
            if (redFlashTimer>0) redFlashTimer-=dt;
            if (slashEffectTimer>0) slashEffectTimer-=dt;

            // Ход слизня
            if (combatTurnState==1) {
                combatTimer+=dt;
                if (combatTimer>=0.8f) {
                    int sd=6+(rand()%7);
                    if (player.className=="Воин") sd=std::max(3,sd-3);
                    survivalStats.health=std::max(0,survivalStats.health-sd);
                    player.health=survivalStats.health;
                    std::stringstream ss; ss<<"Слизь атаковала! -"<<sd<<" HP";
                    combatLog.push_back(ss.str());
                    redFlashTimer=0.15f; screenShakeIntensity=10;
                    std::stringstream sd2; sd2<<"-"<<sd<<" HP";
                    floatingTexts.push_back({sd2.str(), Vector2{(float)(GetScreenWidth()/4+10),220}, Color{239,68,68,255}, 1,-40,1.2f,true});
                    if (player.health<=0) { player.isAlive=false; combatLog.push_back("Не выдержал..."); combatTurnState=3; }
                    else combatTurnState=0;
                    combatTimer=0;
                }
            } else if (combatTurnState==2) { combatTimer+=dt; if (combatTimer>=1.4f) { state=STATE_2D_WORLD; combatTimer=0; } }
            else if (combatTurnState==3) { combatTimer+=dt; if (combatTimer>=1.8f) { state=STATE_GAME_OVER; combatTimer=0; } }

            for (auto& t:floatingTexts) { if (!t.active) continue; t.position.y+=t.ySpeed*dt; t.lifetime-=dt; t.alpha=t.lifetime/1.2f; if (t.lifetime<=0) t.active=false; }
        }

        // ==================== ОТРИСОВКА ====================
        BeginDrawing();
        ClearBackground(bgDark);
        
        // Включаем правильный режим смешивания для прозрачности
        BeginBlendMode(BLEND_ALPHA);

        if (state==STATE_MENU) {
            MenuState ms=gameMenu.GetCurrentState();
            if (ms==MENU_MAIN) gameMenu.DrawMainMenu(font, windowWidth, windowHeight, framesCounter);
            else if (ms==MENU_WORLD_CREATE) gameMenu.DrawWorldCreateMenu(font, windowWidth, windowHeight);
            else if (ms==MENU_SETTINGS) gameMenu.DrawSettingsMenu(font, windowWidth, windowHeight);
            else gameMenu.DrawMainMenu(font, windowWidth, windowHeight, framesCounter);
        }
        else if (state==STATE_WELCOME) {
            // ===== ЭКРАН ВВОДА ИМЕНИ (Minecraft стиль) =====
            DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), Color{30,30,30,255}, Color{15,15,15,255});

            // Тайловый фон
            for (int x=0; x<GetScreenWidth(); x+=32)
                for (int y=0; y<GetScreenHeight(); y+=32) {
                    unsigned char v = 25 + (rand()%8);
                    DrawRectangle(x, y, 32, 32, Color{v, (unsigned char)(v+10), v, 255});
                    DrawRectangleLines(x, y, 32, 32, Color{(unsigned char)(v-5),(unsigned char)(v+5),(unsigned char)(v-5),60});
                }

            float sw = GetScreenWidth(), sh = GetScreenHeight();

            // Заголовок
            const char* title = "LEGENDS OF GREEN MEADOW";
            Vector2 ts = MeasureTextEx(font, title, 36, 1);
            DrawTextEx(font, title, {sw/2.f - ts.x/2 + 2, sh*0.2f + 2}, 36, 1, Color{0,0,0,150});
            DrawTextEx(font, title, {sw/2.f - ts.x/2, sh*0.2f}, 36, 1, Color{80,200,120,255});

            const char* sub = "Введите имя вашего героя";
            Vector2 ss2 = MeasureTextEx(font, sub, 18, 1);
            DrawTextEx(font, sub, {sw/2.f - ss2.x/2, sh*0.3f}, 18, 1, Color{180,180,180,255});

            // Поле ввода
            Rectangle inputRect = {sw/2.f - 180, sh*0.38f, 360, 50};
            DrawRectangleRec(inputRect, Color{50,50,50,255});
            DrawRectangleLinesEx(inputRect, 3, Color{30,30,30,255});
            Rectangle inner = {inputRect.x+4, inputRect.y+4, inputRect.width-8, inputRect.height-8};
            DrawRectangleRec(inner, Color{20,20,20,255});

            Vector2 ns = MeasureTextEx(font, playerNameInput.c_str(), 22, 1);
            DrawTextEx(font, playerNameInput.c_str(), {inputRect.x + inputRect.width/2 - ns.x/2, inputRect.y + 14}, 22, 1, Color{255,255,255,255});

            if (((framesCounter/30)%2)==0 && playerNameInput.length()<14) {
                float cx2 = inputRect.x + inputRect.width/2 + ns.x/2 + 3;
                DrawRectangle((int)cx2, (int)inputRect.y+12, 2, 26, Color{255,255,255,200});
            }

            // Кнопка "Играть"
            bool canPlay = !playerNameInput.empty();
            Rectangle btnPlay = {sw/2.f - 100, sh*0.52f, 200, 45};
            Color btnCol = canPlay ? Color{80,160,80,255} : Color{60,60,60,255};
            Color btnHi = canPlay ? Color{100,200,100,255} : Color{70,70,70,255};
            bool hovPlay = CheckCollisionPointRec(GetMousePosition(), btnPlay);
            DrawRectangleRec(btnPlay, hovPlay && canPlay ? btnHi : btnCol);
            DrawRectangleLinesEx(btnPlay, 2, Color{40,40,40,255});
            Vector2 bts = MeasureTextEx(font, "ИГРАТЬ", 20, 1);
            DrawTextEx(font, "ИГРАТЬ", {btnPlay.x+btnPlay.width/2-bts.x/2, btnPlay.y+12}, 20, 1, Color{255,255,255,255});

            if (hovPlay && canPlay && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                player.name = playerNameInput;
                state = STATE_CLASS_SELECT;
            }
            if (IsKeyPressed(KEY_ENTER) && canPlay) {
                player.name = playerNameInput;
                state = STATE_CLASS_SELECT;
            }

            // Подсказки
            DrawTextEx(font, "Подсказки:", {50, sh-90}, 14, 1, Color{120,120,120,255});
            DrawTextEx(font, "[WASD] бег   [E] добыча ресурсов   [I] инвентарь   [C] крафт   [F11] полноэкран", {50, sh-70}, 13, 1, Color{100,100,100,255});
            DrawTextEx(font, "[TAB] инвентарь   [ESC] закрыть меню   [ENTER] подтвердить", {50, sh-50}, 13, 1, Color{100,100,100,255});
        }
        else if (state==STATE_CLASS_SELECT) {
            DrawRectangle(0,0,GetScreenWidth(),95,bgPanel);
            DrawLine(0,95,GetScreenWidth(),95,Color{48,54,68,255});
            DrawTextEx(font, "ЛЕГЕНДЫ ЗЕЛЕНОЙ ПОЛЯНЫ", Vector2{30,25}, 28, 1, textWhite);
            DrawTextEx(font, "Выберите класс вашего персонажа:", Vector2{310,130}, 22, 1, textWhite);

            if (DrawClassCard(font, Rectangle{60,190,250,350}, "Воин", "HP: 120   DMG: 10", "Сила и выносливость!\nНадежная защита.", false, bgPanel, Color{48,54,68,255})) {
                player.className="Воин"; player.maxHealth=120; player.health=120; player.damage=10; player.gold=100; player.isAlive=true;
                InitWorldEntities(); state=STATE_2D_WORLD;
            }
            if (DrawClassCard(font, Rectangle{350,190,250,350}, "Маг", "HP: 80    DMG: 20", "Тайная магия!\nОгромный урон.", false, bgPanel, Color{48,54,68,255})) {
                player.className="Маг"; player.maxHealth=80; player.health=80; player.damage=20; player.gold=100; player.isAlive=true;
                InitWorldEntities(); state=STATE_2D_WORLD;
            }
            if (DrawClassCard(font, Rectangle{640,190,250,350}, "Лучник", "HP: 100   DMG: 15", "Меткость и скорость!\nСбалансированный.", false, bgPanel, Color{48,54,68,255})) {
                player.className="Лучник"; player.maxHealth=100; player.health=100; player.damage=15; player.gold=100; player.isAlive=true;
                InitWorldEntities(); state=STATE_2D_WORLD;
            }
        }
        // ==================== ОТРИСОВКА 2D МИРА ====================
        else {
            if (state==STATE_2D_WORLD) {
                BeginMode2D(camera);

                // Рисуем сплошной фон для всего мира (единый цвет)
                DrawRectangle(0, 0, 3000, 3000, Color{34, 85, 44, 255}); // Тёмно-зелёный
                
                // Рисуем биомы с плавными переходами
                for (int tx=0;tx<30;++tx) for (int ty=0;ty<30;++ty) {
                    float worldX = tx * 100.0f + 50.0f;
                    float worldY = ty * 100.0f + 50.0f;
                    BiomeType tileBiome = GetBiomeAtPosition(worldX, worldY);
                    const BiomeInfo& info = GetBiomeInfo(tileBiome);
                    // Рисуем с перекрытием чтобы убрать зазоры
                    DrawRectangle(tx*100 - 1, ty*100 - 1, 102, 102, info.groundColor);
                }
                
                // Добавляем детали травы (маленькие кружочки разных оттенков зелёного)
                srand(1234);
                for (int tx=0;tx<30;++tx) for (int ty=0;ty<30;++ty) {
                    BiomeType tileBiome = GetBiomeAtPosition(tx*100+50, ty*100+50);
                    const BiomeInfo& info = GetBiomeInfo(tileBiome);
                    
                    // Рисуем 5-8 деталей травы на каждый тайл
                    int details = 5 + rand()%4;
                    for (int d=0; d<details; d++) {
                        float dx = tx*100 + rand()%80 + 10;
                        float dy = ty*100 + rand()%80 + 10;
                        float size = 2 + rand()%4;
                        Color grassColor = {
                            (unsigned char)(info.groundColor.r + rand()%30 - 15),
                            (unsigned char)(info.groundColor.g + rand()%30 - 15),
                            (unsigned char)(info.groundColor.b + rand()%20 - 10),
                            255
                        };
                        DrawCircle(dx, dy, size, grassColor);
                    }
                }

                // Цветочки
                srand(2345);
                for (int i=0;i<50;++i) {
                    int fx=rand()%3000, fy=rand()%3000;
                    float dc=sqrtf(powf(fx-1500.f,2)+powf(fy-1500.f,2));
                    float dl=sqrtf(powf(fx-lakePos.x,2)+powf(fy-lakePos.y,2));
                    if (dc>160&&dl>(lakeRadius+15)) {
                        Color fc=(rand()%2==0)?Color{244,114,182,160}:Color{253,224,71,160};
                        DrawCircle(fx,fy,2,fc); DrawCircle(fx,fy-2,1,Color{255,255,255,200});
                    }
                }

                // Дороги
                DrawCircle(1500,1500,150,Color{63,63,70,255});
                DrawCircleLines(1500,1500,150,Color{82,82,91,255});
                DrawLineEx(campfirePos,tentPos,22,Color{82,82,91,255});
                DrawLineEx(campfirePos,tentPos,18,Color{113,113,122,255});
                DrawLineEx(Vector2{1500,1500},Vector2{1700,1510},24,Color{82,82,91,255});
                DrawLineEx(Vector2{1500,1500},Vector2{1700,1510},20,Color{113,113,122,255});

                // Палатка
                DrawTent(tentPos);

                // ==================== КОСТЁР (ПРОЦЕДУРНЫЙ) ====================
                // Свечение костра ночью
                if (dayNightCycle.currentPhase==DAY_NIGHT || dayNightCycle.currentPhase==DAY_EVENING) {
                    float glowPulse=sinf(framesCounter*0.1f)*0.15f+0.85f;
                    float glowR=80*glowPulse;
                    DrawCircle(campfirePos.x, campfirePos.y-10, glowR, Color{255,180,50,25});
                    DrawCircle(campfirePos.x, campfirePos.y-10, glowR*0.6f, Color{255,200,80,35});
                }
                // Рисуем костёр процедурно
                DrawCampfire(campfirePos, framesCounter);

                // ==================== ОЗЕРО (ПРОЦЕДУРНОЕ) ====================
                // Основная вода
                DrawCircleV(lakePos, lakeRadius, Color{30, 100, 170, 200});
                DrawCircleV(lakePos, lakeRadius - 5, Color{40, 120, 190, 180});
                // Волны
                for (int i = 0; i < 3; i++) {
                    float waveR = lakeRadius - 20 - i * 20;
                    float waveOff = sinf(framesCounter * 0.05f + i * 2.0f) * 5;
                    DrawCircleLines(lakePos.x + waveOff, lakePos.y + waveOff * 0.5f, waveR, Color{100, 180, 240, 80});
                }
                // Блики на воде
                float blink = sinf(framesCounter * 0.08f) * 0.5f + 0.5f;
                DrawCircle(lakePos.x - 25, lakePos.y - 25, 10, Color{200, 230, 255, (unsigned char)(60 * blink)});
                DrawCircle(lakePos.x + 18, lakePos.y + 12, 6, Color{200, 230, 255, (unsigned char)(40 * blink)});
                DrawCircle(lakePos.x + 5, lakePos.y - 35, 4, Color{220, 240, 255, (unsigned char)(50 * blink)});

                // Подсказка палатки
                float dtTent2 = sqrtf(powf(playerPos.x-tentPos.x,2)+powf(playerPos.y-tentPos.y,2));
                if (dtTent2<45 && player.health<player.maxHealth) {
                    DrawRectangle(tentPos.x-70, tentPos.y-65, 140, 25, Color{30,30,30,200});
                    DrawTextEx(font, "[E] ОТДОХНУТЬ", Vector2{tentPos.x-55, tentPos.y-60}, 13, 1, Color{253,224,71,255});
                }

                // ==================== ДЕРЕВЬЯ (ПРОЦЕДУРНЫЕ) ====================
                for (auto& t : trees) {
                    if (!t.active) continue;

                    float offX = 0;
                    if (t.shakeTimer > 0) offX = sinf(t.shakeTimer * 40) * 3;

                    // Тень под деревом
                    DrawEllipse(t.position.x+offX, t.position.y+24, 18, 7, Color{0,0,0,100});
                    // Рисуем дерево процедурно
                    DrawTree({t.position.x+offX, t.position.y}, t.radius);

                    // Подсказка добычи
                    float d = sqrtf(powf(playerPos.x-t.position.x,2)+powf(playerPos.y-t.position.y,2));
                    if (d < 60) {
                        DrawRectangle(t.position.x-40, t.position.y-60, 80, 18, Color{30,30,30,180});
                        DrawTextEx(font, "[E] Дерево", Vector2{t.position.x-30, t.position.y-57}, 12, 1, Color{200,180,120,255});
                        // Полоска HP дерева
                        DrawRectangle(t.position.x-25, t.position.y-70, 50, 6, Color{50,50,50,200});
                        DrawRectangle(t.position.x-25, t.position.y-70, (int)(50.0f * t.hp / t.maxHp), 6, Color{34,197,94,255});
                    }
                }

                // ==================== КАМНИ (ПРОЦЕДУРНЫЕ) ====================
                for (auto& r : rocks) {
                    if (!r.active) continue;

                    float offX = 0;
                    if (r.shakeTimer > 0) offX = sinf(r.shakeTimer * 40) * 2;

                    // Тень под камнем
                    DrawEllipse(r.position.x+offX, r.position.y+r.radius-2, r.radius, r.radius*0.4f, Color{0,0,0,100});
                    // Рисуем камень процедурно
                    DrawCircleV({r.position.x+offX, r.position.y}, r.radius, Color{82,82,91,255});
                    DrawCircleV({r.position.x+offX, r.position.y}, r.radius-3, Color{113,113,122,255});

                    // Подсказка
                    float d = sqrtf(powf(playerPos.x-r.position.x,2)+powf(playerPos.y-r.position.y,2));
                    if (d < 45) {
                        DrawRectangle(r.position.x-40, r.position.y-35, 80, 18, Color{30,30,30,180});
                        DrawTextEx(font, "[E] Камень", Vector2{r.position.x-30, r.position.y-32}, 12, 1, Color{180,180,180,255});
                        DrawRectangle(r.position.x-25, r.position.y-45, 50, 6, Color{50,50,50,200});
                        DrawRectangle(r.position.x-25, r.position.y-45, (int)(50.0f * r.hp / r.maxHp), 6, Color{156,163,175,255});
                    }
                }

                // Руины и предметы
                for (auto& w:ruins) DrawStoneWall(w);
                for (auto& item:mapItems) DrawMapItem(item);
                for (auto& s:slimes) DrawWanderingSlime(s, framesCounter, bgDark);

                UpdateAndDrawParticles(dt);

                // ==================== ИГРОК (ПРОЦЕДУРНЫЙ) ====================
                bool isMoving = (IsKeyDown(KEY_W)||IsKeyDown(KEY_UP)||IsKeyDown(KEY_S)||IsKeyDown(KEY_DOWN)||IsKeyDown(KEY_A)||IsKeyDown(KEY_LEFT)||IsKeyDown(KEY_D)||IsKeyDown(KEY_RIGHT)) && !showInventory && !showCrafting;
                // Рисуем игрока процедурно (без PNG)
                DrawPlayerAvatar(player.className, playerPos, playerRadius, playerFacing, isMoving, framesCounter, bgDark);

                // Взмах оружием при добыче
                if (weaponSwingTimer > 0) {
                    float swingRad = weaponSwingAngle * 3.14159f / 180.0f;
                    float armX = playerPos.x + cosf(swingRad) * 20;
                    float armY = playerPos.y - 10 + sinf(swingRad) * 20;
                    // Рисуем текстуру кирки/меча
                    Texture2D wpnTex = ResourceManager::Get().GetTex("sword");
                    if (wpnTex.id != 0) {
                        DrawTextureEx(wpnTex, {armX - 16, armY - 16}, weaponSwingAngle, 0.5f, WHITE);
                    } else {
                        DrawLineEx({playerPos.x, playerPos.y - 10}, {armX, armY}, 3, Color{200,200,210,255});
                    }
                }

                // Плавающие тексты
                for (auto& t:floatingTexts) { if (!t.active) continue; DrawTextEx(font,t.text.c_str(),t.position,15,1,ColorAlpha(t.color,t.alpha)); }
                DrawRectangleLines(0,0,2000,2000,Color{220,38,38,120});
                EndMode2D();

                // ==================== ОВЕРЛЕЙ ДЕНЬ/НОЧЬ (на весь экран) ====================
                if (dayNightCycle.currentPhase==DAY_EVENING) {
                    float a=(dayNightCycle.timeOfDay-18)/3;
                    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),Color{10,10,30,(unsigned char)(a*120)});
                }
                else if (dayNightCycle.currentPhase==DAY_NIGHT) {
                    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),Color{10,10,30,120});
                    // Звёзды ночью
                    srand(7777);
                    for (int i=0; i<60; i++) {
                        int sx=rand()%GetScreenWidth(), sy=rand()%(GetScreenHeight()/2);
                        float twinkle=sinf(framesCounter*0.05f+i*1.7f)*0.3f+0.7f;
                        unsigned char a=(unsigned char)(180*twinkle);
                        DrawCircle(sx,sy,1+(rand()%2),Color{255,255,230,a});
                    }
                }
                else if (dayNightCycle.currentPhase==DAY_MORNING) {
                    float a=1-(dayNightCycle.timeOfDay-6)/6;
                    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),Color{10,10,30,(unsigned char)(a*80)});
                }

                // ==================== ИНВЕНТАРЬ ====================
                if (showInventory) {
                    float sw = GetScreenWidth(), sh = GetScreenHeight();
                    DrawRectangle(0,0,sw,sh,Color{0,0,0,200});
                    Rectangle invR={sw/2.f-300, 50, 600, 540};
                    DrawRectangleRounded(invR, 0.02f, 4, Color{40,40,45,255});
                    DrawRectangleRoundedLines(invR, 0.02f, 4, 2, Color{70,70,75,255});

                    DrawTextEx(font, "ИНВЕНТАРЬ", {invR.x+20, invR.y+15}, 20, 1, Color{220,220,220,255});
                    DrawLine(invR.x+15,invR.y+45,invR.x+invR.width-15,invR.y+45,Color{70,70,75,255});

                    int slotSize=65, slotPad=6, slotsX=8, slotsY=5;
                    float gx=invR.x+20, gy=invR.y+55;
                    int slotIdx=0;
                    std::vector<std::pair<ItemType,int>> grouped;
                    std::vector<int> seen(ITEM_MANA_CRYSTAL+1,0);
                    for (auto& it:player.inventory) {
                        if (it!=ITEM_NONE && !seen[it]) {
                            int cnt=CountItems(it);
                            grouped.push_back({it,cnt});
                            seen[it]=1;
                        }
                    }

                    for (int y=0;y<slotsY;y++) for (int x=0;x<slotsX;x++) {
                        float sx=gx+x*(slotSize+slotPad), sy=gy+y*(slotSize+slotPad);
                        Rectangle sR={sx,sy,(float)slotSize,(float)slotSize};
                        DrawRectangleRounded(sR, 0.05f, 4, Color{55,55,60,255});
                        DrawRectangleRoundedLines(sR, 0.05f, 4, 1, Color{80,80,85,255});

                        if (slotIdx<(int)grouped.size()) {
                            auto& [type,cnt]=grouped[slotIdx];
                            DrawItemIcon({sx+slotSize/2.f, sy+slotSize/2.f}, type, 30);
                            DrawTextEx(font, GetItemName(type), {sx+4, sy+slotSize-16}, 11, 1, Color{200,200,200,255});
                            if (cnt>1) {
                                std::stringstream sc; sc<<"x"<<cnt;
                                DrawTextEx(font, sc.str().c_str(), {sx+slotSize-25, sy+4}, 12, 1, Color{255,255,255,255});
                            }
                            slotIdx++;
                        }
                    }

                    // Правая панель
                    Rectangle infoR={invR.x+invR.width-170, invR.y+55, 145, 300};
                    DrawRectangleRounded(infoR, 0.05f, 4, Color{50,50,55,255});
                    DrawRectangleRoundedLines(infoR, 0.05f, 4, 1, Color{70,70,75,255});
                    DrawTextEx(font, "ДЕЙСТВИЯ", {infoR.x+10,infoR.y+10}, 14, 1, Color{180,180,180,255});

                    float btnY=infoR.y+35;
                    int herbCnt=CountItems(ITEM_HERB);
                    if (herbCnt>0) {
                        if (DrawButton(font, {infoR.x+8,btnY,130,32}, "Лечиться", Color{16,185,129,255}, Color{52,211,153,255}, Color{4,120,87,255}, Color{255,255,255,255})) {
                            if (survivalStats.health<survivalStats.maxHealth) {
                                survivalStats.health=std::min(survivalStats.maxHealth, survivalStats.health+25);
                                player.health=survivalStats.health;
                                RemoveOneItem(ITEM_HERB);
                                AddLogMessage("Съел траву, +25 HP", miniLog);
                            } else AddLogMessage("HP и так полное", miniLog);
                        }
                        btnY+=40;
                    }
                    int breadCnt=CountItems(ITEM_BREAD);
                    if (breadCnt>0) {
                        if (DrawButton(font, {infoR.x+8,btnY,130,32}, "Есть хлеб", Color{180,140,40,255}, Color{210,170,60,255}, Color{140,100,20,255}, Color{255,255,255,255})) {
                            survivalStats.hunger=std::min(survivalStats.maxHunger, survivalStats.hunger+8);
                            RemoveOneItem(ITEM_BREAD);
                            AddLogMessage("Съел хлеб, +8 голод", miniLog);
                        }
                        btnY+=40;
                    }

                    DrawTextEx(font, ("Золото: "+std::to_string(player.gold)).c_str(), {infoR.x+10,btnY}, 14, 1, Color{245,158,11,255});

                    if (DrawButton(font, {invR.x+invR.width/2-75, invR.y+invR.height-50, 150, 38}, "Закрыть [I]", Color{71,85,105,255}, Color{100,116,139,255}, Color{51,65,85,255}, Color{255,255,255,255}))
                        showInventory=false;
                }

                // ==================== КРАФТ ====================
                if (showCrafting) {
                    float sw = GetScreenWidth(), sh = GetScreenHeight();
                    DrawRectangle(0,0,sw,sh,Color{0,0,0,200});
                    Rectangle cR={sw/2.f-280, 40, 560, 560};
                    DrawRectangleRounded(cR, 0.02f, 4, Color{40,40,45,255});
                    DrawRectangleRoundedLines(cR, 0.02f, 4, 2, Color{70,70,75,255});

                    DrawTextEx(font, "КРАФТ", {cR.x+20,cR.y+15}, 20, 1, Color{220,220,220,255});
                    DrawLine(cR.x+15,cR.y+45,cR.x+cR.width-15,cR.y+45,Color{70,70,75,255});

                    int wC=CountItems(ITEM_WOOD), sC=CountItems(ITEM_STONE), pC=CountItems(ITEM_STICK), dC=CountItems(ITEM_WOOD_PLANK);
                    DrawTextEx(font, "Ресурсы:", {cR.x+20,cR.y+55}, 14, 1, Color{180,180,180,255});
                    std::stringstream res;
                    res<<"[Дерево:"<<wC<<"] [Камень:"<<sC<<"] [Палки:"<<pC<<"] [Доски:"<<dC<<"]";
                    DrawTextEx(font, res.str().c_str(), {cR.x+20,cR.y+75}, 13, 1, Color{160,160,160,255});

                    float ry=cR.y+100;
                    auto& recipes=craftingSystem.GetRecipes();
                    for (auto& recipe:recipes) {
                        if (ry>cR.y+cR.height-60) break;
                        bool can=craftingSystem.CanCraft(recipe, player.inventory);
                        Color bg=can?Color{45,55,45,255}:Color{55,45,45,255};
                        Rectangle rR={cR.x+15, ry, cR.width-30, 55};
                        DrawRectangleRounded(rR, 0.05f, 4, bg);
                        DrawRectangleRoundedLines(rR, 0.05f, 4, 1, can?Color{80,140,80,255}:Color{120,80,80,255});

                        DrawItemIcon({rR.x+25, rR.y+20}, recipe.result, 20);
                        DrawTextEx(font, recipe.name.c_str(), {rR.x+50, rR.y+5}, 14, 1, Color{220,220,220,255});

                        std::string ingr;
                        for (size_t i=0;i<recipe.ingredients.size();i++) {
                            if (recipe.amounts[i]<=0) continue;
                            if (!ingr.empty()) ingr+=", ";
                            ingr+=std::to_string(recipe.amounts[i])+" "+GetItemName(recipe.ingredients[i]);
                        }
                        DrawTextEx(font, ingr.c_str(), {rR.x+50, rR.y+25}, 11, 1, Color{140,140,140,255});

                        if (can) {
                            if (DrawButton(font, {rR.x+rR.width-90,rR.y+12,80,30}, "Крафт", Color{16,185,129,255}, Color{52,211,153,255}, Color{4,120,87,255}, Color{255,255,255,255})) {
                                for (size_t i=0;i<recipe.ingredients.size();i++)
                                    for (int a=0;a<recipe.amounts[i];a++) RemoveOneItem(recipe.ingredients[i]);
                                for (int r2=0;r2<recipe.resultAmount;r2++) player.inventory.push_back(recipe.result);
                                AddLogMessage("Скрафтил: "+recipe.name, miniLog);
                                floatingTexts.push_back({"+"+recipe.name, {playerPos.x,playerPos.y-15}, Color{253,224,71,255}, 1,-40,1.2f,true});
                            }
                        }
                        ry+=62;
                    }
                    DrawTextEx(font, "[C] закрыть", {cR.x+cR.width-100, cR.y+cR.height-25}, 12, 1, Color{100,100,100,255});
                }

                // ==================== ЛОГ ====================
                if (!showInventory && !showCrafting) {
                    int logY=GetScreenHeight()-40;
                    for (int i=(int)miniLog.size()-1;i>=0;--i) {
                        float life=miniLog[i].second;
                        if (life<=0) continue;
                        float alpha=std::min(1.f,life);
                        Color col=ColorAlpha(textWhite,alpha);
                        if (miniLog[i].first.find("HP")!=std::string::npos) col=ColorAlpha(Color{34,197,94,255},alpha);
                        else if (miniLog[i].first.find("золот")!=std::string::npos||miniLog[i].first.find("Золот")!=std::string::npos) col=ColorAlpha(Color{245,158,11,255},alpha);
                        Vector2 ts2=MeasureTextEx(font,miniLog[i].first.c_str(),14,1);
                        DrawRectangle(20,logY-2,ts2.x+20,20,ColorAlpha(bgDark,180*alpha));
                        DrawTextEx(font,miniLog[i].first.c_str(),{30,(float)logY},14,1,col);
                        logY-=24;
                    }
                }
            }
            // ==================== БОЙ ====================
            else if (state==STATE_MEADOW_SLIME) {
                Vector2 sh={0,0};
                if (screenShakeIntensity>0) { sh.x=(float)(rand()%20-10)*(screenShakeIntensity/10); sh.y=(float)(rand()%20-10)*(screenShakeIntensity/10); }

                float sw = GetScreenWidth();
                DrawRectangle(30+sh.x,120+sh.y,sw-60,490,bgPanel);
                DrawRectangleLines(30+sh.x,120+sh.y,sw-60,490,Color{48,54,68,255});

                Rectangle pCard={80+sh.x,170+sh.y,240,280};
                DrawRectangleRounded(pCard,0.1f,4,Color{26,28,35,255});
                DrawRectangleRoundedLines(pCard,0.1f,4,1.5f,Color{99,102,241,255});
                DrawPlayerAvatar(player.className, Vector2{pCard.x+pCard.width/2,pCard.y+80}, playerRadius, playerFacing, false, 0, bgDark);

                std::string pName=player.name+" ("+player.className+")";
                DrawTextEx(font,pName.c_str(),{pCard.x+(pCard.width-MeasureTextEx(font,pName.c_str(),18,1).x)/2, pCard.y+150},18,1,textWhite);
                std::stringstream ssS; ssS<<"Урон: "<<player.damage<<"   Золото: "<<player.gold;
                DrawTextEx(font,ssS.str().c_str(),{pCard.x+(pCard.width-MeasureTextEx(font,ssS.str().c_str(),14,1).x)/2,pCard.y+185},14,1,textGray);

                // HP игрока полоской
                DrawProgressBar(Rectangle{pCard.x+20,pCard.y+225,200,16},(float)player.health,(float)player.maxHealth,Color{239,68,68,255},Color{50,50,50,255});
                std::stringstream ssHP; ssHP<<player.health<<" / "<<player.maxHealth<<" HP";
                DrawTextEx(font,ssHP.str().c_str(),{pCard.x+(pCard.width-MeasureTextEx(font,ssHP.str().c_str(),12,1).x)/2,pCard.y+248},12,1,textWhite);

                DrawTextEx(font,"VS",{sw/2.f-15+sh.x,280+sh.y},26,1,Color{239,68,68,180});

                Rectangle sCard={sw-320+sh.x,170+sh.y,240,280};
                DrawRectangleRounded(sCard,0.1f,4,Color{26,28,35,255});
                DrawRectangleRoundedLines(sCard,0.1f,4,1.5f,Color{239,68,68,255});

                float cb=sinf(framesCounter*0.16f)*6;
                float srx=55+cb*0.7f, sry=45-cb*0.7f;
                Vector2 sc={sCard.x+sCard.width/2,sCard.y+80+cb*0.5f};
                Color sCol=slimeType.find("Алая")!=std::string::npos?Color{220,38,38,255}:slimeType.find("Ледяная")!=std::string::npos?Color{59,130,246,255}:Color{34,197,94,255};
                Color sGlow=slimeType.find("Алая")!=std::string::npos?Color{248,113,113,160}:slimeType.find("Ледяная")!=std::string::npos?Color{96,165,250,160}:Color{74,222,128,160};
                Color sOut=slimeType.find("Алая")!=std::string::npos?Color{127,29,29,255}:slimeType.find("Ледяная")!=std::string::npos?Color{29,78,137,255}:Color{20,83,45,255};

                if (slimeHp>0) {
                    DrawEllipse((int)sc.x,(int)sc.y,srx,sry,sOut);
                    DrawEllipse((int)sc.x,(int)sc.y,srx-3,sry-3,sCol);
                    DrawEllipse((int)sc.x,(int)sc.y,srx-5,sry-5,ColorAlpha(sGlow,0.45f));
                    DrawEllipse((int)sc.x-srx*0.3f,(int)sc.y-sry*0.3f,srx*0.22f,sry*0.18f,Color{255,255,255,220});
                    DrawCircle(sc.x-14,sc.y-6,6,textWhite); DrawCircle(sc.x+14,sc.y-6,6,textWhite);
                    DrawCircle(sc.x-14,sc.y-6,2.5f,bgDark); DrawCircle(sc.x+14,sc.y-6,2.5f,bgDark);
                } else DrawEllipse((int)sc.x,(int)sc.y+30,srx+15,8,ColorAlpha(sCol,120));

                DrawTextEx(font,slimeType.c_str(),{sCard.x+(sCard.width-MeasureTextEx(font,slimeType.c_str(),18,1).x)/2,sCard.y+150},18,1,textWhite);
                if (slimeHp>0) {
                    DrawProgressBar(Rectangle{sCard.x+20,sCard.y+225,200,16},(float)slimeHp,(float)slimeMaxHp,Color{34,197,94,255},Color{50,50,50,255});
                    std::stringstream sSH; sSH<<slimeHp<<" / "<<slimeMaxHp<<" HP";
                    DrawTextEx(font,sSH.str().c_str(),{sCard.x+(sCard.width-MeasureTextEx(font,sSH.str().c_str(),12,1).x)/2,sCard.y+248},12,1,textWhite);
                } else DrawTextEx(font,"ПОБЕЖДЕН",{sCard.x+75,sCard.y+225},16,1,Color{34,197,94,255});

                if (slashEffectTimer>0) {
                    DrawLineEx({sc.x-60,sc.y-60},{sc.x+60,sc.y+60},6,Color{255,255,255,220});
                    DrawLineEx({sc.x-60,sc.y-60},{sc.x+60,sc.y+60},2,Color{239,68,68,255});
                }
                for (auto& t:floatingTexts) { if (!t.active) continue; DrawTextEx(font,t.text.c_str(),t.position,22,1,ColorAlpha(t.color,t.alpha)); }
                if (redFlashTimer>0) DrawRectangle(30,120,sw-60,490,Color{239,68,68,(unsigned char)(90*(redFlashTimer/0.15f))});

                Rectangle ccR={80+sh.x,470+sh.y,(float)(sw-160),120};
                DrawRectangleRounded(ccR,0.15f,4,Color{26,28,35,255});
                DrawRectangleRoundedLines(ccR,0.15f,4,1.5f,Color{48,54,68,255});

                if (slimeHp>0&&player.health>0) {
                    if (combatTurnState==0) {
                        if (DrawButton(font,{ccR.x+30,ccR.y+35,220,50},"Атаковать",Color{239,68,68,255},Color{248,113,113,255},Color{185,28,28,255},textWhite)) {
                            int dmg=player.damage;
                            if (player.className=="Маг"&&rand()%100<30){dmg=(int)(dmg*1.5f);combatLog.push_back("КРИТ!");}
                            slimeHp=std::max(0,slimeHp-dmg);
                            std::stringstream s; s<<"Рубанул! "<<dmg<<" урона."; combatLog.push_back(s.str());
                            slashEffectTimer=0.2f; screenShakeIntensity=7;
                            std::stringstream sd; sd<<"-"<<dmg;
                            floatingTexts.push_back({sd.str(),{sc.x,sc.y-30},Color{253,224,71,255},1,-40,1.2f,true});
                            if (slimeHp<=0) { int gr=15+rand()%16; player.gold+=gr; player.levelSystem.AddXP(25); questController.UpdateQuest(QUEST_KILL_SLIMES); combatLog.push_back("Слизь разлетелась! +"+std::to_string(gr)+" золота. +25 XP"); if (activeSlimeIndex!=-1) slimes[activeSlimeIndex].active=false; combatTurnState=2; combatTimer=0; }
                            else { combatTurnState=1; combatTimer=0; }
                        }
                        if (DrawButton(font,{ccR.x+280,ccR.y+35,220,50},"Приручить",Color{16,185,129,255},Color{52,211,153,255},Color{4,120,87,255},textWhite)) {
                            std::string res=handleSlimeBefriend(player); combatLog.push_back(res);
                            if (activeSlimeIndex!=-1) slimes[activeSlimeIndex].active=false;
                            if (!player.isAlive){combatTurnState=3;combatTimer=0;}
                            else { combatTurnState=2; combatTimer=0; }
                        }
                        if (DrawButton(font,{ccR.x+530,ccR.y+35,220,50},"Сбежать",Color{107,114,128,255},Color{156,163,175,255},Color{75,85,99,255},textWhite)) {
                            std::string res=handleSlimeRun(player); combatLog.push_back(res);
                            if (activeSlimeIndex!=-1) { float a=(float)(rand()%360)*3.14159f/180; slimes[activeSlimeIndex].position.x+=cosf(a)*80; slimes[activeSlimeIndex].position.y+=sinf(a)*80; }
                            combatTurnState=2; combatTimer=0;
                        }
                    } else DrawTextEx(font,combatLog.back().c_str(),{ccR.x+(ccR.width-MeasureTextEx(font,combatLog.back().c_str(),16,1).x)/2,ccR.y+48},16,1,textWhite);
                } else DrawTextEx(font,combatLog.back().c_str(),{ccR.x+(ccR.width-MeasureTextEx(font,combatLog.back().c_str(),16,1).x)/2,ccR.y+48},16,1,textWhite);
            }
            // ==================== GAME OVER ====================
            else if (state==STATE_GAME_OVER) {
                DrawTextEx(font,"ГЕРОЙ ПАЛ В БОЮ",{350,180},32,1,Color{239,68,68,255});
                DrawTextEx(font,"Путешествие завершилось...",{275,240},16,1,textGray);
                Rectangle sR={325,290,300,135};
                DrawRectangleRounded(sR,0.1f,4,bgPanel);
                DrawRectangleRoundedLines(sR,0.1f,4,1.5f,Color{48,54,68,255});
                DrawTextEx(font,("Имя: "+player.name).c_str(),{sR.x+20,sR.y+20},16,1,textWhite);
                DrawTextEx(font,("Класс: "+player.className).c_str(),{sR.x+20,sR.y+50},16,1,textWhite);
                DrawTextEx(font,("Золото: "+std::to_string(player.gold)).c_str(),{sR.x+20,sR.y+80},16,1,Color{245,158,11,255});
                if (DrawButton(font,{375,465,200,45},"Начать сначала",Color{99,102,241,255},Color{129,140,248,255},Color{79,70,229,255},textWhite)) {
                    playerNameInput=""; state=STATE_MENU; showInventory=false; showCrafting=false;
                    player.inventory.clear(); survivalStats=SurvivalStats(); dayNightCycle=DayNightCycle();
                    gameMenu.SetCurrentState(MENU_MAIN);
                }
            }

            // ==================== HUD ====================
            if (state!=STATE_WELCOME&&state!=STATE_CLASS_SELECT&&state!=STATE_GAME_OVER) {
                DrawGameHUD(font, 
                    survivalStats.health, survivalStats.maxHealth,
                    survivalStats.hunger, survivalStats.maxHunger,
                    player.levelSystem.xp, player.levelSystem.xpToNext,
                    player.levelSystem.level, player.gold, player.damage,
                    dayNightCycle.timeOfDay, dayNightCycle.dayCount,
                    playerPos, lakePos, lakeRadius,
                    campfirePos, tentPos,
                    nullptr, nullptr, nullptr);
            }
        }
        
        // Восстанавливаем режим смешивания
        EndBlendMode();
        
        EndDrawing();
    }

    UnloadFont(font);
    CloseWindow();
    return 0;
}
