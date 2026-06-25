#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <cmath>
#include "raylib.h"

// Модули игры
#include "game_types.h"
#include "player.h"
#include "biom.h"
#include "particles.h"
#include "gui.h"
#include "render.h"
#include "resource_manager.h"
#include "menu.h"
#include "crafting.h"

int main() {
    const int windowWidth = 950;
    const int windowHeight = 650;
    InitWindow(windowWidth, windowHeight, "Legends of Green Meadow - 2D RPG");
    SetTargetFPS(60);

    // Загрузка кириллического шрифта
    int codepoints[512];
    for (int i = 0; i < 128; i++) codepoints[i] = 32 + i; // ASCII
    for (int i = 0; i < 256; i++) codepoints[128 + i] = 0x0400 + i; // Кириллица
    Font font = LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", 24, codepoints, 384);
    
    if (font.texture.id == 0) {
        font = GetFontDefault();
    }

    initBiomeRandom();
    InitParticles();
    
    // Загружаем текстуры из папки assets (PNG файлы)
    ResourceManager::Get().LoadTex("grass", "assets/grass.png");
    ResourceManager::Get().LoadTex("player", "assets/player.png");
    ResourceManager::Get().LoadTex("slime", "assets/slime.png");

    GameState state = STATE_MENU;
    Player player;
    std::string playerNameInput = "";
    int framesCounter = 0;
    
    GameMenu gameMenu;
    CraftingSystem craftingSystem;
    SurvivalStats survivalStats;
    DayNightCycle dayNightCycle;
    bool showCrafting = false;
    
    Color bgDark = Color{ 24, 26, 32, 255 };      // Темный фон сцен
    Color bgPanel = Color{ 34, 38, 48, 255 };     // Панели меню
    Color textWhite = Color{ 243, 244, 246, 255 }; // Белый текст
    Color textGray = Color{ 156, 163, 175, 255 };  // Серый текст
    Color btnNormal = Color{ 99, 102, 241, 255 };  // Индиго
    Color btnHover = Color{ 129, 140, 248, 255 };   // Светло-индиго
    Color btnClick = Color{ 79, 70, 229, 255 };    // Темно-индиго
    
    // Игровые переменные двумерного мира
    Vector2 playerPos = { 1000.0f, 1000.0f };
    Vector2 playerFacing = { 0.0f, 1.0f };
    float playerSpeed = 190.0f;
    float playerRadius = 12.0f;
    
    Camera2D camera = { 0 };
    camera.target = playerPos;
    camera.offset = Vector2{ windowWidth / 2.0f, windowHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    
    // Объекты окружения
    std::vector<Obstacle> obstacles;
    std::vector<StoneWall> ruins;
    std::vector<Obstacle> boulders;
    std::vector<MapItem> mapItems;
    std::vector<WanderingSlime> slimes;
    std::vector<FloatingText> floatingTexts;
    std::vector<std::pair<std::string, float>> miniLog;
    
    Color grassTiles[20][20];
    
    Vector2 campfirePos = { 960.0f, 1000.0f };
    Vector2 tentPos = { 1040.0f, 985.0f };
    Vector2 lakePos = { 400.0f, 400.0f };
    float lakeRadius = 150.0f;
    
    // Состояние боевой сцены
    int activeSlimeIndex = -1;
    int slimeHp = 0;
    int slimeMaxHp = 0;
    std::string slimeType = "Зеленая Слизь";
    
    int combatTurnState = 0; // 0: игрок, 1: соперник, 2: выход, 3: гибель
    float combatTimer = 0.0f;
    std::vector<std::string> combatLog;
    
    float screenShakeIntensity = 0.0f;
    float redFlashTimer = 0.0f;
    float slashEffectTimer = 0.0f;
    
    bool showInventory = false; // Открыт ли инвентарь

    auto InitWorldEntities = [&]() {
        obstacles.clear();
        ruins.clear();
        boulders.clear();
        mapItems.clear();
        slimes.clear();
        floatingTexts.clear();
        miniLog.clear();
        
        playerPos = Vector2{ 1000.0f, 1000.0f };
        playerFacing = Vector2{ 0.0f, 1.0f };
        camera.target = playerPos;
        
        AddLogMessage("Проснулся в лагере, вроде всё тихо.", miniLog);
        AddLogMessage("Бегаю стрелками/WASD. [I] - инвентарь.", miniLog);
        
        srand(6789);
        for (int x = 0; x < 20; ++x) {
            for (int y = 0; y < 20; ++y) {
                int rOffset = rand() % 8;
                int gOffset = rand() % 12;
                int bOffset = rand() % 8;
                grassTiles[x][y] = Color{ (unsigned char)(22 + rOffset), (unsigned char)(95 + gOffset), (unsigned char)(44 + bOffset), 255 };
            }
        }
        
        ruins.push_back({ Rectangle{ 750, 1300, 150, 30 } });
        ruins.push_back({ Rectangle{ 1300, 700, 30, 180 } });
        
        for (int i = 0; i < 8; ++i) {
            Vector2 pos;
            while (true) {
                pos = { (float)(100 + rand() % 1800), (float)(100 + rand() % 1800) };
                float dCamp = sqrtf(powf(pos.x - 1000.0f, 2) + powf(pos.y - 1000.0f, 2));
                float dLake = sqrtf(powf(pos.x - lakePos.x, 2) + powf(pos.y - lakePos.y, 2));
                if (dCamp > 250.0f && dLake > (lakeRadius + 30.0f)) break;
            }
            boulders.push_back({ pos, 16.0f + (float)(rand() % 6) });
        }
        
        int attempts = 0;
        while (obstacles.size() < 65 && attempts < 800) {
            attempts++;
            Vector2 candidate = { (float)(50 + rand() % 1900), (float)(50 + rand() % 1900) };
            
            float distToCamp = sqrtf(powf(candidate.x - 1000.0f, 2) + powf(candidate.y - 1000.0f, 2));
            float distToLake = sqrtf(powf(candidate.x - lakePos.x, 2) + powf(candidate.y - lakePos.y, 2));
            if (distToCamp < 220.0f || distToLake < (lakeRadius + 25.0f)) continue;
            
            bool inWall = false;
            for (const auto& wall : ruins) {
                if (CheckCollisionCircleRec(candidate, 25.0f, wall.rect)) {
                    inWall = true;
                    break;
                }
            }
            if (inWall) continue;
            
            bool overlap = false;
            for (const auto& obs : obstacles) {
                float d = sqrtf(powf(candidate.x - obs.position.x, 2) + powf(candidate.y - obs.position.y, 2));
                if (d < 48.0f) {
                    overlap = true;
                    break;
                }
            }
            if (!overlap) {
                obstacles.push_back({ candidate, 15.0f });
            }
        }
        
        for (int i = 0; i < 15; ++i) {
            Vector2 pos;
            while (true) {
                pos = { (float)(80 + rand() % 1840), (float)(80 + rand() % 1840) };
                float dist = sqrtf(powf(pos.x - 1000.0f, 2) + powf(pos.y - 1000.0f, 2));
                float distLake = sqrtf(powf(pos.x - lakePos.x, 2) + powf(pos.y - lakePos.y, 2));
                if (dist > 200.0f && distLake > (lakeRadius + 20.0f)) break;
            }
            mapItems.push_back({ pos, ITEM_HERB, true, 0.0f });
            
            while (true) {
                pos = { (float)(80 + rand() % 1840), (float)(80 + rand() % 1840) };
                float dist = sqrtf(powf(pos.x - 1000.0f, 2) + powf(pos.y - 1000.0f, 2));
                float distLake = sqrtf(powf(pos.x - lakePos.x, 2) + powf(pos.y - lakePos.y, 2));
                if (dist > 200.0f && distLake > (lakeRadius + 20.0f)) break;
            }
            mapItems.push_back({ pos, ITEM_GOLDFLOWER, true, 0.0f });
        }
        
        for (int i = 0; i < 7; ++i) {
            Vector2 pos;
            while (true) {
                pos = { (float)(100 + rand() % 1800), (float)(100 + rand() % 1800) };
                float dist = sqrtf(powf(pos.x - 1000.0f, 2) + powf(pos.y - 1000.0f, 2));
                float distLake = sqrtf(powf(pos.x - lakePos.x, 2) + powf(pos.y - lakePos.y, 2));
                if (dist > 260.0f && distLake > (lakeRadius + 30.0f)) break;
            }
            WanderingSlime s;
            s.position = pos;
            s.targetPosition = pos;
            s.wanderTimer = 0.0f;
            s.speed = 40.0f + (float)(rand() % 28);
            s.maxHp = 25 + rand() % 15;
            s.hp = s.maxHp;
            s.active = true;
            s.id = i;
            slimes.push_back(s);
        }
    };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        framesCounter++;
        
        // --- ЛОГИКА ---
        if (state == STATE_MENU) {
            gameMenu.HandleMainMenuInput(font, windowWidth, windowHeight);
            
            if (gameMenu.GetCurrentState() == MENU_SINGLEPLAYER) {
                gameMenu.SetCurrentState(MENU_WORLD_CREATE);
            }
            if (gameMenu.GetCurrentState() == MENU_WORLD_CREATE) {
                gameMenu.HandleWorldCreateInput(font, windowWidth, windowHeight);
            }
            if (gameMenu.GetCurrentState() == MENU_SETTINGS) {
                gameMenu.HandleSettingsInput(font, windowWidth, windowHeight);
            }
            if (gameMenu.GetCurrentState() == MENU_PLAYING) {
                state = STATE_WELCOME;
                gameMenu.SetCurrentState(MENU_MAIN);
            }
        }
        else if (state == STATE_WELCOME) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32 && key <= 125) || (key >= 0x0400 && key <= 0x04FF)) {
                    if (playerNameInput.length() < 14) {
                        AppendUnicodeToUTF8(playerNameInput, key);
                    }
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (!playerNameInput.empty()) {
                    do {
                        playerNameInput.pop_back();
                    } while (!playerNameInput.empty() && ((playerNameInput.back() & 0xC0) == 0x80));
                }
            }
        } 
        else if (state == STATE_2D_WORLD) {
            if (IsKeyPressed(KEY_I) || IsKeyPressed(KEY_TAB)) {
                showInventory = !showInventory;
                showCrafting = false;
            }
            if (IsKeyPressed(KEY_C)) {
                showCrafting = !showCrafting;
                showInventory = false;
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                showInventory = false;
                showCrafting = false;
            }
            
            // Обновление выживания
            dayNightCycle.Update(dt);
            survivalStats.hungerTimer -= dt;
            if (survivalStats.hungerTimer <= 0.0f) {
                survivalStats.hunger = std::max(0, survivalStats.hunger - 1);
                survivalStats.hungerTimer = 30.0f;
            }
            if (survivalStats.hunger <= 0 && survivalStats.health > 0) {
                survivalStats.health = std::max(0, survivalStats.health - 1);
            }
            if (survivalStats.hunger > 10 && survivalStats.health < survivalStats.maxHealth) {
                survivalStats.health = std::min(survivalStats.maxHealth, survivalStats.health + 1);
            }
            player.health = survivalStats.health;
            
            // Крафт: нажал Е рядом с деревом - получил дерево
            // (добавляем позже интеграцию)

            if (!showInventory && !showCrafting) {
                Vector2 moveDir = { 0, 0 };
                if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) moveDir.y -= 1;
                if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) moveDir.y += 1;
                if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) moveDir.x -= 1;
                if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) moveDir.x += 1;
                
                bool isMoving = (moveDir.x != 0 || moveDir.y != 0);
                
                if (isMoving) {
                    float len = sqrtf(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
                    moveDir.x /= len;
                    moveDir.y /= len;
                    
                    playerFacing = moveDir;
                    
                    float stepX = moveDir.x * playerSpeed * dt;
                    float stepY = moveDir.y * playerSpeed * dt;
                    
                    Vector2 nextPosX = { playerPos.x + stepX, playerPos.y };
                    bool colX = false;
                    
                    if (nextPosX.x < 15.0f || nextPosX.x > 1985.0f) colX = true;
                    
                    for (const auto& obs : obstacles) {
                        float dist = sqrtf(powf(nextPosX.x - obs.position.x, 2) + powf(nextPosX.y - obs.position.y, 2));
                        if (dist < (playerRadius + obs.radius)) { colX = true; break; }
                    }
                    for (const auto& boulder : boulders) {
                        float dist = sqrtf(powf(nextPosX.x - boulder.position.x, 2) + powf(nextPosX.y - boulder.position.y, 2));
                        if (dist < (playerRadius + boulder.radius)) { colX = true; break; }
                    }
                    for (const auto& wall : ruins) {
                        if (CheckCollisionCircleRec(nextPosX, playerRadius, wall.rect)) { colX = true; break; }
                    }
                    float distLakeX = sqrtf(powf(nextPosX.x - lakePos.x, 2) + powf(nextPosX.y - lakePos.y, 2));
                    if (distLakeX < (playerRadius + lakeRadius - 6.0f)) colX = true;
                    
                    if (!colX) playerPos.x = nextPosX.x;
                    
                    Vector2 nextPosY = { playerPos.x, playerPos.y + stepY };
                    bool colY = false;
                    
                    if (nextPosY.y < 15.0f || nextPosY.y > 1985.0f) colY = true;
                    
                    for (const auto& obs : obstacles) {
                        float dist = sqrtf(powf(nextPosY.x - obs.position.x, 2) + powf(nextPosY.y - obs.position.y, 2));
                        if (dist < (playerRadius + obs.radius)) { colY = true; break; }
                    }
                    for (const auto& boulder : boulders) {
                        float dist = sqrtf(powf(nextPosY.x - boulder.position.x, 2) + powf(nextPosY.y - boulder.position.y, 2));
                        if (dist < (playerRadius + boulder.radius)) { colY = true; break; }
                    }
                    for (const auto& wall : ruins) {
                        if (CheckCollisionCircleRec(nextPosY, playerRadius, wall.rect)) { colY = true; break; }
                    }
                    float distLakeY = sqrtf(powf(nextPosY.x - lakePos.x, 2) + powf(nextPosY.y - lakePos.y, 2));
                    if (distLakeY < (playerRadius + lakeRadius - 6.0f)) colY = true;
                    
                    if (!colY) playerPos.y = nextPosY.y;
                    
                    if (player.className == "Маг" && (rand() % 100 < 15)) {
                        Vector2 staffTip = { playerPos.x + (playerFacing.x < 0 ? -12 : 12), playerPos.y - 12 };
                        SpawnParticle(staffTip, Vector2{ (float)(rand() % 30 - 15), (float)(rand() % 30 - 15) }, Color{ 253, 224, 71, 255 }, 2.0f, 0.4f + (rand() % 4) / 10.0f);
                    }
                }
                
                camera.target.x += (playerPos.x - camera.target.x) * 0.08f;
                camera.target.y += (playerPos.y - camera.target.y) * 0.08f;
                
                float distToTent = sqrtf(powf(playerPos.x - tentPos.x, 2) + powf(playerPos.y - tentPos.y, 2));
                if (distToTent < 45.0f && player.health < player.maxHealth) {
                    if (IsKeyPressed(KEY_E)) {
                        player.health = player.maxHealth;
                        survivalStats.health = survivalStats.maxHealth;
                        AddLogMessage("Отлежался в палатке, чувствую себя как новый.", miniLog);
                        floatingTexts.push_back({ "+HP Восстановлено", Vector2{ playerPos.x, playerPos.y - 15.0f }, Color{ 34, 197, 94, 255 }, 1.0f, -45.0f, 1.2f, true });
                        for (int p = 0; p < 12; p++) {
                            float angle = p * (3.14159f * 2.0f / 12.0f);
                            SpawnParticle(playerPos, Vector2{ cosf(angle) * 45.0f, sinf(angle) * 45.0f }, Color{ 74, 222, 128, 255 }, 3.0f, 0.6f);
                        }
                    }
                }
                
                // Добыча дерева (нажал Е рядом с деревом)
                if (IsKeyPressed(KEY_E)) {
                    for (const auto& obs : obstacles) {
                        float dist = sqrtf(powf(playerPos.x - obs.position.x, 2) + powf(playerPos.y - obs.position.y, 2));
                        if (dist < 50.0f) {
                            int woodAmount = 1 + rand() % 2;
                            for (int w = 0; w < woodAmount; w++) player.inventory.push_back(ITEM_WOOD);
                            std::stringstream ssW; ssW << "+" << woodAmount << " Дерево";
                            floatingTexts.push_back({ ssW.str(), Vector2{ playerPos.x, playerPos.y - 15.0f }, Color{ 139, 90, 43, 255 }, 1.0f, -40.0f, 1.2f, true });
                            AddLogMessage("Срубил кусок дерева.", miniLog);
                            break;
                        }
                    }
                    for (const auto& boulder : boulders) {
                        float dist = sqrtf(powf(playerPos.x - boulder.position.x, 2) + powf(playerPos.y - boulder.position.y, 2));
                        if (dist < 35.0f) {
                            int stoneAmount = 1 + rand() % 2;
                            for (int s = 0; s < stoneAmount; s++) player.inventory.push_back(ITEM_STONE);
                            std::stringstream ssS; ssS << "+" << stoneAmount << " Камень";
                            floatingTexts.push_back({ ssS.str(), Vector2{ playerPos.x, playerPos.y - 15.0f }, Color{ 156, 163, 175, 255 }, 1.0f, -40.0f, 1.2f, true });
                            AddLogMessage("Выбил камень.", miniLog);
                            break;
                        }
                    }
                }
                
                for (auto& item : mapItems) {
                    if (!item.active) {
                        item.respawnTimer -= dt;
                        if (item.respawnTimer <= 0.0f) item.active = true;
                        continue;
                    }
                    
                    float distToItem = sqrtf(powf(playerPos.x - item.position.x, 2) + powf(playerPos.y - item.position.y, 2));
                    if (distToItem < (playerRadius + 8.0f)) {
                        item.active = false;
                        item.respawnTimer = 12.0f;
                        
                        if (item.type == ITEM_HERB) {
                            player.inventory.push_back(ITEM_HERB);
                            floatingTexts.push_back({ "+Трава", Vector2{ item.position.x, item.position.y - 10.0f }, Color{ 34, 197, 94, 255 }, 1.0f, -40.0f, 1.2f, true });
                            AddLogMessage("Сорвал лечебную траву. Положу в сумку.", miniLog);
                        } 
                        else if (item.type == ITEM_GOLDFLOWER) {
                            int goldLoot = 10 + rand() % 16;
                            player.gold += goldLoot;
                            std::stringstream ss; ss << "+" << goldLoot << " Золота";
                            floatingTexts.push_back({ ss.str(), Vector2{ item.position.x, item.position.y - 10.0f }, Color{ 253, 224, 71, 255 }, 1.0f, -40.0f, 1.2f, true });
                            std::stringstream ssLog; ssLog << "О, золотоцвет! Сорвал, +" << goldLoot << " монет.";
                            AddLogMessage(ssLog.str(), miniLog);
                        }
                    }
                }
                
                for (size_t i = 0; i < slimes.size(); ++i) {
                    auto& slime = slimes[i];
                    if (!slime.active) continue;
                    
                    slime.wanderTimer -= dt;
                    if (slime.wanderTimer <= 0.0f) {
                        slime.wanderTimer = 2.5f + (rand() % 35) / 10.0f;
                        float angle = (float)(rand() % 360) * (3.14159f / 180.0f);
                        float range = 70.0f + rand() % 100;
                        slime.targetPosition = { slime.position.x + cosf(angle) * range, slime.position.y + sinf(angle) * range };
                        slime.targetPosition.x = std::max(50.0f, std::min(1950.0f, slime.targetPosition.x));
                        slime.targetPosition.y = std::max(50.0f, std::min(1950.0f, slime.targetPosition.y));
                        
                        float dCamp = sqrtf(powf(slime.targetPosition.x - 1000.0f, 2) + powf(slime.targetPosition.y - 1000.0f, 2));
                        float dLake = sqrtf(powf(slime.targetPosition.x - lakePos.x, 2) + powf(slime.targetPosition.y - lakePos.y, 2));
                        if (dCamp < 250.0f || dLake < (lakeRadius + 20.0f)) slime.targetPosition = slime.position;
                    }
                    
                    Vector2 toTarget = { slime.targetPosition.x - slime.position.x, slime.targetPosition.y - slime.position.y };
                    float dTarget = sqrtf(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
                    if (dTarget > 2.0f) {
                        slime.position.x += (toTarget.x / dTarget) * slime.speed * dt;
                        slime.position.y += (toTarget.y / dTarget) * slime.speed * dt;
                    }
                    
                    if (rand() % 100 < 8) {
                        Color trailColor = slime.speed > 60.0f ? Color{ 248, 113, 113, 160 } : Color{ 74, 222, 128, 160 };
                        SpawnParticle(slime.position, Vector2{ 0.0f, 0.0f }, trailColor, 3.0f + rand() % 3, 0.6f + (rand() % 5) / 10.0f);
                    }
                    
                    float dPlayer = sqrtf(powf(playerPos.x - slime.position.x, 2) + powf(playerPos.y - slime.position.y, 2));
                    if (dPlayer < (playerRadius + 14.0f)) {
                        state = STATE_MEADOW_SLIME;
                        activeSlimeIndex = (int)i;
                        slimeHp = slime.hp;
                        slimeMaxHp = slime.maxHp;
                        slimeType = slime.speed > 60.0f ? "Быстрая Алая Слизь" : "Зеленая Слизь";
                        combatLog.clear();
                        combatLog.push_back("!!! Нарвался на " + slimeType + "! Придётся драться.");
                        combatTurnState = 0; combatTimer = 0.0f; screenShakeIntensity = 0.0f; redFlashTimer = 0.0f; slashEffectTimer = 0.0f;
                    }
                }
                
                if (rand() % 100 < 25) {
                    SpawnParticle(campfirePos, Vector2{ (float)(rand() % 30 - 15), (float)(-35 - rand() % 30) }, Color{ 245, 158, 11, 255 }, 2.5f + rand() % 2, 0.8f + (rand() % 6) / 10.0f);
                }
            }
            
            for (auto& txt : floatingTexts) {
                if (!txt.active) continue;
                txt.position.y += txt.ySpeed * dt;
                txt.lifetime -= dt;
                txt.alpha = txt.lifetime / 1.2f;
                if (txt.lifetime <= 0.0f) txt.active = false;
            }
            for (auto& item : miniLog) {
                if (item.second > 0.0f) item.second -= dt;
            }
        } 
        else if (state == STATE_MEADOW_SLIME) {
            if (screenShakeIntensity > 0.0f) screenShakeIntensity -= dt * 12.0f;
            if (redFlashTimer > 0.0f) redFlashTimer -= dt;
            if (slashEffectTimer > 0.0f) slashEffectTimer -= dt;
            
            if (combatTurnState == 1) {
                combatTimer += dt;
                if (combatTimer >= 0.8f) {
                    int slimeDamage = 6 + (rand() % 7);
                    if (player.className == "Воин") slimeDamage = std::max(3, slimeDamage - 3);
                    player.health = std::max(0, player.health - slimeDamage);
                    
                    std::stringstream ss; ss << "Слизь плюнула в меня, больно! Получил " << slimeDamage << " урона.";
                    combatLog.push_back(ss.str());
                    
                    redFlashTimer = 0.15f;
                    screenShakeIntensity = 10.0f;
                    
                    std::stringstream ssDmg; ssDmg << "-" << slimeDamage << " HP";
                    floatingTexts.push_back({ ssDmg.str(), Vector2{ windowWidth/4.0f + 10, 220 }, Color{ 239, 68, 68, 255 }, 1.0f, -40.0f, 1.2f, true });
                    
                    if (player.health <= 0) {
                        player.isAlive = false;
                        combatLog.push_back("Не выдержал... всё потемнело...");
                        combatTurnState = 3;
                    } else {
                        combatTurnState = 0;
                    }
                    combatTimer = 0.0f;
                }
            } else if (combatTurnState == 2) {
                combatTimer += dt;
                if (combatTimer >= 1.4f) { state = STATE_2D_WORLD; combatTimer = 0.0f; }
            } else if (combatTurnState == 3) {
                combatTimer += dt;
                if (combatTimer >= 1.8f) { state = STATE_GAME_OVER; combatTimer = 0.0f; }
            }
            
            for (auto& txt : floatingTexts) {
                if (!txt.active) continue;
                txt.position.y += txt.ySpeed * dt;
                txt.lifetime -= dt;
                txt.alpha = txt.lifetime / 1.2f;
                if (txt.lifetime <= 0.0f) txt.active = false;
            }
        }

        // --- ОТРИСОВКА ---
        BeginDrawing();
        ClearBackground(bgDark);

        if (state == STATE_MENU) {
            MenuState menuState = gameMenu.GetCurrentState();
            if (menuState == MENU_MAIN) {
                gameMenu.DrawMainMenu(font, windowWidth, windowHeight, framesCounter);
            } else if (menuState == MENU_SINGLEPLAYER) {
                gameMenu.DrawWorldCreateMenu(font, windowWidth, windowHeight);
            } else if (menuState == MENU_WORLD_CREATE) {
                gameMenu.DrawWorldCreateMenu(font, windowWidth, windowHeight);
            } else if (menuState == MENU_SETTINGS) {
                gameMenu.DrawSettingsMenu(font, windowWidth, windowHeight);
            } else {
                gameMenu.DrawMainMenu(font, windowWidth, windowHeight, framesCounter);
            }
        }
        else if (state == STATE_WELCOME) {
            // Улучшенный красивый стартовый экран
            DrawRectangleGradientV(0, 0, windowWidth, windowHeight, Color{ 15, 23, 42, 255 }, Color{ 30, 41, 59, 255 });
            
            // Задний фон - декоративные круги
            DrawCircle(windowWidth/2, windowHeight/2 - 100, 250, Color{ 56, 189, 248, 15 });
            DrawCircle(windowWidth/2, windowHeight/2 - 100, 180, Color{ 56, 189, 248, 25 });
            
            // Тень текста заголовка
            DrawTextEx(font, "ЛЕГЕНДЫ ЗЕЛЕНОЙ ПОЛЯНЫ", Vector2{ windowWidth/2.0f - MeasureTextEx(font, "ЛЕГЕНДЫ ЗЕЛЕНОЙ ПОЛЯНЫ", 42, 1).x/2.0f + 2, 122 }, 42, 1.0f, Color{ 0, 0, 0, 180 });
            // Заголовок
            DrawTextEx(font, "ЛЕГЕНДЫ ЗЕЛЕНОЙ ПОЛЯНЫ", Vector2{ windowWidth/2.0f - MeasureTextEx(font, "ЛЕГЕНДЫ ЗЕЛЕНОЙ ПОЛЯНЫ", 42, 1).x/2.0f, 120 }, 42, 1.0f, Color{ 56, 189, 248, 255 });
            
            const char* subtitle = "Двумерная RPG: Введите имя вашего героя";
            DrawTextEx(font, subtitle, Vector2{ windowWidth/2.0f - MeasureTextEx(font, subtitle, 18, 1).x/2.0f, 180 }, 18, 1.0f, textGray);
            
            // Поле ввода
            Rectangle inputRect = { windowWidth/2.0f - 160, 260, 320, 55 };
            DrawRectangleRounded(inputRect, 0.2f, 8, Color{ 15, 23, 42, 200 });
            DrawRectangleRoundedLines(inputRect, 0.2f, 8, 2.0f, Color{ 56, 189, 248, 180 });
            
            // Текст внутри поля
            Vector2 nameSize = MeasureTextEx(font, playerNameInput.c_str(), 24, 1.0f);
            DrawTextEx(font, playerNameInput.c_str(), Vector2{ inputRect.x + inputRect.width/2.0f - nameSize.x/2.0f, inputRect.y + 15 }, 24, 1.0f, textWhite);
            
            // Мигающий курсор
            if (((framesCounter / 30) % 2) == 0 && playerNameInput.length() < 14) {
                float cursorX = inputRect.x + inputRect.width/2.0f + nameSize.x/2.0f + 2;
                DrawRectangle((int)cursorX, (int)inputRect.y + 12, 2, 30, textWhite);
            }
            
            bool nameIsValid = !playerNameInput.empty();
            Color cBtn = nameIsValid ? Color{ 16, 185, 129, 255 } : Color{ 71, 85, 105, 255 };
            Color cHover = nameIsValid ? Color{ 52, 211, 153, 255 } : Color{ 71, 85, 105, 255 };
            Color cClick = nameIsValid ? Color{ 4, 120, 87, 255 } : Color{ 71, 85, 105, 255 };
            
            if (DrawButton(font, Rectangle{ windowWidth/2.0f - 100, 350, 200, 50 }, "НАЧАТЬ", cBtn, cHover, cClick, textWhite)) {
                if (nameIsValid) { player.name = playerNameInput; state = STATE_CLASS_SELECT; }
            }
            
        } else if (state == STATE_CLASS_SELECT) {
            DrawRectangle(0, 0, windowWidth, 95, bgPanel);
            DrawLine(0, 95, windowWidth, 95, Color{ 48, 54, 68, 255 });
            DrawTextEx(font, "ЛЕГЕНДЫ ЗЕЛЕНОЙ ПОЛЯНЫ", Vector2{ 30, 25 }, 28, 1.0f, textWhite);
            
            DrawTextEx(font, "Выберите класс вашего персонажа:", Vector2{ 310, 130 }, 22, 1.0f, textWhite);
            
            if (DrawClassCard(font, Rectangle{ 60, 190, 250, 350 }, "Воин", "HP: 120   DMG: 10", "Сила и выносливость - твое всё!\nНадежная защита и сильный клинок.", false, bgPanel, Color{ 48, 54, 68, 255 })) {
                player.className = "Воин"; player.maxHealth = 120; player.health = 120; player.damage = 10; player.gold = 100; player.isAlive = true;
                InitWorldEntities(); state = STATE_2D_WORLD;
            }
            if (DrawClassCard(font, Rectangle{ 350, 190, 250, 350 }, "Маг", "HP: 80    DMG: 20", "Тайная магия сокрушит врагов!\nОгромный урон, но хрупкое здоровье.", false, bgPanel, Color{ 48, 54, 68, 255 })) {
                player.className = "Маг"; player.maxHealth = 80; player.health = 80; player.damage = 20; player.gold = 100; player.isAlive = true;
                InitWorldEntities(); state = STATE_2D_WORLD;
            }
            if (DrawClassCard(font, Rectangle{ 640, 190, 250, 350 }, "Лучник", "HP: 100   DMG: 15", "Меткость и скорость - твои козыри!\nСбалансированный боец.", false, bgPanel, Color{ 48, 54, 68, 255 })) {
                player.className = "Лучник"; player.maxHealth = 100; player.health = 100; player.damage = 15; player.gold = 100; player.isAlive = true;
                InitWorldEntities(); state = STATE_2D_WORLD;
            }
        } 
        else {
            if (state == STATE_2D_WORLD) {
                BeginMode2D(camera);
                
                Texture2D grassTex = ResourceManager::Get().GetTex("grass");
                for (int tx = 0; tx < 20; ++tx) {
                    for (int ty = 0; ty < 20; ++ty) {
                        if (grassTex.id != 0) {
                            DrawTexture(grassTex, tx * grassTex.width, ty * grassTex.height, WHITE);
                        } else {
                            DrawRectangle(tx * 100, ty * 100, 100, 100, grassTiles[tx][ty]);
                            DrawRectangleLines(tx * 100, ty * 100, 100, 100, Color{ 20, 85, 40, 30 });
                        }
                    }
                }
                
                srand(2345);
                for (int i = 0; i < 50; ++i) {
                    int fx = rand() % 2000; int fy = rand() % 2000;
                    float dCamp = sqrtf(powf(fx - 1000.0f, 2) + powf(fy - 1000.0f, 2));
                    float dLake = sqrtf(powf(fx - lakePos.x, 2) + powf(fy - lakePos.y, 2));
                    if (dCamp > 160.0f && dLake > (lakeRadius + 15.0f)) {
                        Color fc = (rand() % 2 == 0) ? Color{ 244, 114, 182, 160 } : Color{ 253, 224, 71, 160 };
                        DrawCircle(fx, fy, 2.0f, fc); DrawCircle(fx, fy - 2, 1.0f, Color{ 255, 255, 255, 200 });
                    }
                }
                
                DrawCircle(1000, 1000, 150, Color{ 63, 63, 70, 255 });
                DrawCircleLines(1000, 1000, 150, Color{ 82, 82, 91, 255 });
                
                DrawLineEx(campfirePos, tentPos, 22.0f, Color{ 82, 82, 91, 255 });
                DrawLineEx(campfirePos, tentPos, 18.0f, Color{ 113, 113, 122, 255 });
                DrawLineEx(Vector2{ 1000.0f, 1000.0f }, Vector2{ 1200.0f, 1010.0f }, 24.0f, Color{ 82, 82, 91, 255 });
                DrawLineEx(Vector2{ 1000.0f, 1000.0f }, Vector2{ 1200.0f, 1010.0f }, 20.0f, Color{ 113, 113, 122, 255 });
                
                DrawTent(tentPos);
                DrawCampfire(campfirePos, framesCounter);
                DrawWaterPond(lakePos, lakeRadius, framesCounter);
                
                float distToTent = sqrtf(powf(playerPos.x - tentPos.x, 2) + powf(playerPos.y - tentPos.y, 2));
                if (distToTent < 45.0f && player.health < player.maxHealth) {
                    DrawRectangle(tentPos.x - 70, tentPos.y - 65, 140, 25, Color{ 30, 30, 30, 200 });
                    DrawTextEx(font, "[E] ОТДОХНУТЬ", Vector2{ tentPos.x - 55, tentPos.y - 60 }, 13, 1.0f, Color{ 253, 224, 71, 255 });
                }
                
                for (const auto& wall : ruins) DrawStoneWall(wall);
                for (const auto& item : mapItems) DrawMapItem(item);
                for (const auto& slime : slimes) DrawWanderingSlime(slime, framesCounter, bgDark);
                
                for (const auto& boulder : boulders) {
                    DrawEllipse(boulder.position.x, boulder.position.y + boulder.radius - 2.0f, boulder.radius, boulder.radius * 0.4f, Color{ 0, 0, 0, 100 });
                    DrawCircleV(boulder.position, boulder.radius, Color{ 82, 82, 91, 255 });
                    DrawCircleV(boulder.position, boulder.radius - 3.0f, Color{ 113, 113, 122, 255 });
                    DrawCircle(boulder.position.x - boulder.radius * 0.3f, boulder.position.y - boulder.radius * 0.3f, boulder.radius * 0.25f, Color{ 161, 161, 170, 220 });
                    DrawCircleLines(boulder.position.x, boulder.position.y, boulder.radius, Color{ 63, 63, 70, 255 });
                }
                
                for (const auto& obs : obstacles) {
                    DrawEllipse(obs.position.x, obs.position.y + 24, 12, 5, Color{ 0, 0, 0, 100 });
                    DrawTree(obs.position, obs.radius);
                }
                
                UpdateAndDrawParticles(dt);
                
                bool isMoving = (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN) ||
                                 IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && !showInventory;
                DrawPlayerAvatar(player.className, playerPos, playerRadius, playerFacing, isMoving, framesCounter, bgDark);
                
                for (const auto& txt : floatingTexts) {
                    if (!txt.active) continue;
                    DrawTextEx(font, txt.text.c_str(), txt.position, 15, 1.0f, ColorAlpha(txt.color, txt.alpha));
                }
                
                DrawRectangleLines(0, 0, 2000, 2000, Color{ 220, 38, 38, 120 });
                EndMode2D();
                
                // Оверлей день/ночь
                if (dayNightCycle.currentPhase == DAY_EVENING) {
                    float alpha = (dayNightCycle.timeOfDay - 18.0f) / 3.0f;
                    DrawRectangle(0, 0, windowWidth, windowHeight, Color{ 10, 10, 30, (unsigned char)(alpha * 120) });
                } else if (dayNightCycle.currentPhase == DAY_NIGHT) {
                    DrawRectangle(0, 0, windowWidth, windowHeight, Color{ 10, 10, 30, 120 });
                } else if (dayNightCycle.currentPhase == DAY_MORNING) {
                    float alpha = 1.0f - (dayNightCycle.timeOfDay - 6.0f) / 6.0f;
                    DrawRectangle(0, 0, windowWidth, windowHeight, Color{ 10, 10, 30, (unsigned char)(alpha * 80) });
                }
                
                // Панель крафта
                if (showCrafting) {
                    DrawRectangle(0, 0, windowWidth, windowHeight, Color{ 0, 0, 0, 180 });
                    Rectangle craftRect = { windowWidth/2.0f - 250, 80, 500, 480 };
                    DrawRectangleRounded(craftRect, 0.02f, 4, Color{ 35, 35, 40, 255 });
                    DrawRectangleRoundedLines(craftRect, 0.02f, 4, 2.0f, Color{ 60, 60, 65, 255 });
                    
                    DrawTextEx(font, "КРАФТ", Vector2{ craftRect.x + 20, craftRect.y + 15 }, 20, 1.0f, Color{ 200, 200, 200, 255 });
                    DrawLine(craftRect.x + 15, craftRect.y + 45, craftRect.x + craftRect.width - 15, craftRect.y + 45, Color{ 60, 60, 65, 255 });
                    
                    // Подсчёт ресурсов
                    int woodCount = 0, stoneCount = 0, stickCount = 0;
                    for (const auto& item : player.inventory) {
                        if (item == ITEM_WOOD) woodCount++;
                        if (item == ITEM_STONE) stoneCount++;
                        if (item == ITEM_STICK) stickCount++;
                    }
                    
                    std::stringstream ssInv; ssInv << "Ресурсы: " << woodCount << " дер. | " << stoneCount << " кам. | " << stickCount << " пал.";
                    DrawTextEx(font, ssInv.str().c_str(), Vector2{ craftRect.x + 20, craftRect.y + 55 }, 14, 1.0f, Color{ 180, 180, 180, 255 });
                    
                    // Рецепты
                    float recipeY = craftRect.y + 85;
                    auto& recipes = craftingSystem.GetRecipes();
                    int recipesShown = 0;
                    for (const auto& recipe : recipes) {
                        if (recipeY > craftRect.y + craftRect.height - 60) break;
                        
                        bool canCraft = craftingSystem.CanCraft(recipe, player.inventory);
                        Color bgColor = canCraft ? Color{ 45, 55, 45, 255 } : Color{ 55, 45, 45, 255 };
                        
                        Rectangle recipeRect = { craftRect.x + 15, recipeY, craftRect.width - 30, 35 };
                        DrawRectangleRounded(recipeRect, 0.05f, 4, bgColor);
                        
                        std::string recipeName = recipe.name + " x" + std::to_string(recipe.resultAmount);
                        DrawTextEx(font, recipeName.c_str(), Vector2{ recipeRect.x + 10, recipeRect.y + 8 }, 14, 1.0f, Color{ 220, 220, 220, 255 });
                        
                        if (canCraft && DrawButton(font, Rectangle{ recipeRect.x + recipeRect.width - 80, recipeRect.y + 3, 70, 28 }, "Крафт", Color{ 16, 185, 129, 255 }, Color{ 52, 211, 153, 255 }, Color{ 4, 120, 87, 255 }, Color{ 255, 255, 255, 255 })) {
                            // Убираем ингредиенты
                            for (size_t i = 0; i < recipe.ingredients.size(); i++) {
                                for (int a = 0; a < recipe.amounts[i]; a++) {
                                    auto it = std::find(player.inventory.begin(), player.inventory.end(), recipe.ingredients[i]);
                                    if (it != player.inventory.end()) player.inventory.erase(it);
                                }
                            }
                            // Даём результат
                            for (int r = 0; r < recipe.resultAmount; r++) {
                                player.inventory.push_back(recipe.result);
                            }
                            AddLogMessage("Скрафтил: " + recipe.name, miniLog);
                            floatingTexts.push_back({ "+" + recipe.name, Vector2{ playerPos.x, playerPos.y - 15.0f }, Color{ 253, 224, 71, 255 }, 1.0f, -40.0f, 1.2f, true });
                        }
                        
                        recipeY += 42;
                        recipesShown++;
                    }
                    
                    if (recipesShown == 0) {
                        DrawTextEx(font, "Нет доступных рецептов", Vector2{ craftRect.x + 20, recipeY }, 14, 1.0f, Color{ 120, 120, 120, 255 });
                    }
                    
                    DrawTextEx(font, "Нажмите [C] чтобы закрыть", Vector2{ craftRect.x + craftRect.width/2.0f - 100, craftRect.y + craftRect.height - 25 }, 12, 1.0f, Color{ 100, 100, 100, 255 });
                }
                
                if (!showInventory && !showCrafting) {
                    int logY = windowHeight - 40;
                    for (int i = (int)miniLog.size() - 1; i >= 0; --i) {
                        float life = miniLog[i].second;
                        if (life <= 0.0f) continue;
                        float alpha = std::min(1.0f, life);
                        Color col = ColorAlpha(textWhite, alpha);
                        if (miniLog[i].first.find("HP") != std::string::npos) col = ColorAlpha(Color{ 34, 197, 94, 255 }, alpha);
                        else if (miniLog[i].first.find("золото") != std::string::npos) col = ColorAlpha(Color{ 245, 158, 11, 255 }, alpha);
                        
                        Vector2 txtSize = MeasureTextEx(font, miniLog[i].first.c_str(), 14, 1.0f);
                        DrawRectangle(20, logY - 2, txtSize.x + 20, 20, ColorAlpha(bgDark, 180 * alpha));
                        DrawTextEx(font, miniLog[i].first.c_str(), Vector2{ 30, (float)logY }, 14, 1.0f, col);
                        logY -= 24;
                    }
                }
            } 
            else if (state == STATE_MEADOW_SLIME) {
                Vector2 shakeOffset = { 0, 0 };
                if (screenShakeIntensity > 0.0f) {
                    shakeOffset.x = (float)(rand() % 20 - 10) * (screenShakeIntensity / 10.0f);
                    shakeOffset.y = (float)(rand() % 20 - 10) * (screenShakeIntensity / 10.0f);
                }
                
                DrawRectangle(30 + shakeOffset.x, 120 + shakeOffset.y, windowWidth - 60, 490, bgPanel);
                DrawRectangleLines(30 + shakeOffset.x, 120 + shakeOffset.y, windowWidth - 60, 490, Color{ 48, 54, 68, 255 });
                
                Rectangle pCard = { 80 + shakeOffset.x, 170 + shakeOffset.y, 240, 280 };
                DrawRectangleRounded(pCard, 0.1f, 4, Color{ 26, 28, 35, 255 });
                DrawRectangleRoundedLines(pCard, 0.1f, 4, 1.5f, btnNormal);
                DrawPlayerAvatar(player.className, Vector2{ pCard.x + pCard.width/2.0f, pCard.y + 80 }, playerRadius, playerFacing, false, 0, bgDark);
                
                std::string pName = player.name + " (" + player.className + ")";
                DrawTextEx(font, pName.c_str(), Vector2{ pCard.x + (pCard.width - MeasureTextEx(font, pName.c_str(), 18, 1.0f).x)/2.0f, pCard.y + 150 }, 18, 1.0f, textWhite);
                
                std::stringstream ssStats; ssStats << "Урон: " << player.damage << "   Золото: " << player.gold;
                DrawTextEx(font, ssStats.str().c_str(), Vector2{ pCard.x + (pCard.width - MeasureTextEx(font, ssStats.str().c_str(), 14, 1).x)/2.0f, pCard.y + 185 }, 14, 1.0f, textGray);
                
                DrawProgressBar(Rectangle{ pCard.x + 20, pCard.y + 225, 200, 16 }, (float)player.health, (float)player.maxHealth, Color{ 239, 68, 68, 255 }, Color{ 50, 50, 50, 255 });
                std::stringstream ssPlayerHp; ssPlayerHp << player.health << " / " << player.maxHealth << " HP";
                DrawTextEx(font, ssPlayerHp.str().c_str(), Vector2{ pCard.x + (pCard.width - MeasureTextEx(font, ssPlayerHp.str().c_str(), 12, 1).x)/2.0f, pCard.y + 248 }, 12, 1.0f, textWhite);
                
                DrawTextEx(font, "VS", Vector2{ windowWidth/2.0f - 15 + shakeOffset.x, 280 + shakeOffset.y }, 26, 1.0f, Color{ 239, 68, 68, 180 });
                
                Rectangle sCard = { windowWidth - 320 + shakeOffset.x, 170 + shakeOffset.y, 240, 280 };
                DrawRectangleRounded(sCard, 0.1f, 4, Color{ 26, 28, 35, 255 });
                DrawRectangleRoundedLines(sCard, 0.1f, 4, 1.5f, Color{ 239, 68, 68, 255 });
                
                float combatSlimeBobbing = sinf(framesCounter * 0.16f) * 6.0f;
                float sRadX = 55.0f + combatSlimeBobbing * 0.7f; float sRadY = 45.0f - combatSlimeBobbing * 0.7f;
                Vector2 slimeCenter = { sCard.x + sCard.width/2.0f, sCard.y + 80 + combatSlimeBobbing * 0.5f };
                
                Color slimeColor = Color{ 34, 197, 94, 255 };
                Color slimeGlow = Color{ 74, 222, 128, 160 };
                Color slimeOutline = Color{ 20, 83, 45, 255 };
                if (slimeType.find("Алая") != std::string::npos) {
                    slimeColor = Color{ 220, 38, 38, 255 }; slimeGlow = Color{ 248, 113, 113, 160 }; slimeOutline = Color{ 127, 29, 29, 255 };
                }
                
                if (slimeHp > 0) {
                    DrawEllipse((int)slimeCenter.x, (int)slimeCenter.y, sRadX, sRadY, slimeOutline);
                    DrawEllipse((int)slimeCenter.x, (int)slimeCenter.y, sRadX - 3, sRadY - 3, slimeColor);
                    DrawEllipse((int)slimeCenter.x, (int)slimeCenter.y, sRadX - 5, sRadY - 5, ColorAlpha(slimeGlow, 0.45f));
                    DrawEllipse((int)slimeCenter.x - sRadX * 0.3f, (int)slimeCenter.y - sRadY * 0.3f, sRadX * 0.22f, sRadY * 0.18f, Color{ 255, 255, 255, 220 });
                    DrawCircle(slimeCenter.x - 14, slimeCenter.y - 6, 6, textWhite); DrawCircle(slimeCenter.x + 14, slimeCenter.y - 6, 6, textWhite);
                    DrawCircle(slimeCenter.x - 14, slimeCenter.y - 6, 2.5f, bgDark); DrawCircle(slimeCenter.x + 14, slimeCenter.y - 6, 2.5f, bgDark);
                } else {
                    DrawEllipse((int)slimeCenter.x, (int)slimeCenter.y + 30, sRadX + 15, 8, ColorAlpha(slimeColor, 120));
                }
                
                DrawTextEx(font, slimeType.c_str(), Vector2{ sCard.x + (sCard.width - MeasureTextEx(font, slimeType.c_str(), 18, 1).x)/2.0f, sCard.y + 150 }, 18, 1.0f, textWhite);
                
                if (slimeHp > 0) {
                    DrawProgressBar(Rectangle{ sCard.x + 20, sCard.y + 225, 200, 16 }, (float)slimeHp, (float)slimeMaxHp, Color{ 34, 197, 94, 255 }, Color{ 50, 50, 50, 255 });
                    std::stringstream ssSlimeHp; ssSlimeHp << slimeHp << " / " << slimeMaxHp << " HP";
                    DrawTextEx(font, ssSlimeHp.str().c_str(), Vector2{ sCard.x + (sCard.width - MeasureTextEx(font, ssSlimeHp.str().c_str(), 12, 1).x)/2.0f, sCard.y + 248 }, 12, 1.0f, textWhite);
                } else {
                    DrawTextEx(font, "ПОБЕЖДЕН", Vector2{ sCard.x + 75, sCard.y + 225 }, 16, 1.0f, Color{ 34, 197, 94, 255 });
                }
                
                if (slashEffectTimer > 0.0f) {
                    DrawLineEx(Vector2{ slimeCenter.x - 60, slimeCenter.y - 60 }, Vector2{ slimeCenter.x + 60, slimeCenter.y + 60 }, 6.0f, Color{ 255, 255, 255, 220 });
                    DrawLineEx(Vector2{ slimeCenter.x - 60, slimeCenter.y - 60 }, Vector2{ slimeCenter.x + 60, slimeCenter.y + 60 }, 2.0f, Color{ 239, 68, 68, 255 });
                }
                
                for (const auto& txt : floatingTexts) {
                    if (!txt.active) continue;
                    DrawTextEx(font, txt.text.c_str(), txt.position, 22, 1.0f, ColorAlpha(txt.color, txt.alpha));
                }
                
                if (redFlashTimer > 0.0f) DrawRectangle(30, 120, windowWidth - 60, 490, Color{ 239, 68, 68, (unsigned char)(90 * (redFlashTimer / 0.15f)) });
                
                Rectangle combatCtrlRect = { 80 + shakeOffset.x, 470 + shakeOffset.y, windowWidth - 160, 120 };
                DrawRectangleRounded(combatCtrlRect, 0.15f, 4, Color{ 26, 28, 35, 255 });
                DrawRectangleRoundedLines(combatCtrlRect, 0.15f, 4, 1.5f, Color{ 48, 54, 68, 255 });
                
                if (slimeHp > 0 && player.health > 0) {
                    if (combatTurnState == 0) {
                        if (DrawButton(font, Rectangle{ combatCtrlRect.x + 30, combatCtrlRect.y + 35, 220, 50 }, "Атаковать", Color{ 239, 68, 68, 255 }, Color{ 248, 113, 113, 255 }, Color{ 185, 28, 28, 255 }, textWhite)) {
                            int damageDealt = player.damage;
                            if (player.className == "Маг" && rand() % 100 < 30) { damageDealt *= 1.5f; combatLog.push_back("КРИТ! Магия сработала!"); }
                            slimeHp = std::max(0, slimeHp - damageDealt);
                            std::stringstream ss; ss << "Рубанул слизь! " << damageDealt << " урона."; combatLog.push_back(ss.str());
                            slashEffectTimer = 0.2f; screenShakeIntensity = 7.0f;
                            std::stringstream ssDmg; ssDmg << "-" << damageDealt;
                            floatingTexts.push_back({ ssDmg.str(), Vector2{ slimeCenter.x, slimeCenter.y - 30 }, Color{ 253, 224, 71, 255 }, 1.0f, -40.0f, 1.2f, true });
                            if (slimeHp <= 0) {
                                int goldReward = 15 + rand() % 16; player.gold += goldReward;
                                std::stringstream ssWin; ssWin << "Слизь разлетелась! В останках +" << goldReward << " монет."; combatLog.push_back(ssWin.str());
                                if (activeSlimeIndex != -1) slimes[activeSlimeIndex].active = false;
                                std::stringstream ssGoldFloat; ssGoldFloat << "+" << goldReward << " Золота";
                                floatingTexts.push_back({ ssGoldFloat.str(), Vector2{ slimeCenter.x, slimeCenter.y - 10 }, Color{ 253, 224, 71, 255 }, 1.0f, -45.0f, 1.4f, true });
                                combatTurnState = 2; combatTimer = 0.0f;
                            } else { combatTurnState = 1; combatTimer = 0.0f; }
                        }
                        if (DrawButton(font, Rectangle{ combatCtrlRect.x + 280, combatCtrlRect.y + 35, 220, 50 }, "Приручить", Color{ 16, 185, 129, 255 }, Color{ 52, 211, 153, 255 }, Color{ 4, 120, 87, 255 }, textWhite)) {
                            std::string res = handleSlimeBefriend(player); combatLog.push_back(res);
                            if (activeSlimeIndex != -1) slimes[activeSlimeIndex].active = false;
                            if (!player.isAlive) { combatTurnState = 3; combatTimer = 0.0f; }
                            else {
                                for (int p = 0; p < 15; p++) {
                                    float angle = p * (3.14159f * 2.0f / 15.0f);
                                    SpawnParticle(Vector2{ pCard.x + pCard.width/2.0f, pCard.y + 80 }, Vector2{ cosf(angle) * 60.0f, sinf(angle) * 60.0f }, Color{ 253, 224, 71, 255 }, 2.5f, 0.7f);
                                }
                                floatingTexts.push_back({ "УСПЕХ!", Vector2{ pCard.x + 20, pCard.y - 20 }, Color{ 253, 224, 71, 255 }, 1.0f, -40.0f, 1.5f, true });
                                combatTurnState = 2; combatTimer = 0.0f;
                            }
                        }
                        if (DrawButton(font, Rectangle{ combatCtrlRect.x + 530, combatCtrlRect.y + 35, 220, 50 }, "Сбежать", Color{ 107, 114, 128, 255 }, Color{ 156, 163, 175, 255 }, Color{ 75, 85, 99, 255 }, textWhite)) {
                            std::string res = handleSlimeRun(player); combatLog.push_back(res);
                            if (activeSlimeIndex != -1) {
                                float angle = (float)(rand() % 360) * (3.14159f / 180.0f);
                                slimes[activeSlimeIndex].position.x += cosf(angle) * 80.0f; slimes[activeSlimeIndex].position.y += sinf(angle) * 80.0f;
                            }
                            combatTurnState = 2; combatTimer = 0.0f;
                        }
                    } else {
                        DrawTextEx(font, combatLog.back().c_str(), Vector2{ combatCtrlRect.x + (combatCtrlRect.width - MeasureTextEx(font, combatLog.back().c_str(), 16, 1).x)/2.0f, combatCtrlRect.y + 48 }, 16, 1.0f, textWhite);
                    }
                } else {
                    DrawTextEx(font, combatLog.back().c_str(), Vector2{ combatCtrlRect.x + (combatCtrlRect.width - MeasureTextEx(font, combatLog.back().c_str(), 16, 1).x)/2.0f, combatCtrlRect.y + 48 }, 16, 1.0f, textWhite);
                }
            } 
            else if (state == STATE_GAME_OVER) {
                DrawTextEx(font, "ГЕРОЙ ПАЛ В БОЮ", Vector2{ 350, 180 }, 32, 1.0f, Color{ 239, 68, 68, 255 });
                DrawTextEx(font, "Ваше путешествие завершилось трагической гибелью...", Vector2{ 275, 240 }, 16, 1.0f, textGray);
                
                Rectangle statsRect = { 325, 290, 300, 135 };
                DrawRectangleRounded(statsRect, 0.1f, 4, bgPanel);
                DrawRectangleRoundedLines(statsRect, 0.1f, 4, 1.5f, Color{ 48, 54, 68, 255 });
                
                std::string fn = "Имя: " + player.name; DrawTextEx(font, fn.c_str(), Vector2{ statsRect.x + 20, statsRect.y + 20 }, 16, 1.0f, textWhite);
                std::string fc = "Класс: " + player.className; DrawTextEx(font, fc.c_str(), Vector2{ statsRect.x + 20, statsRect.y + 50 }, 16, 1.0f, textWhite);
                std::stringstream ssG; ssG << "Собрано золота: " << player.gold; DrawTextEx(font, ssG.str().c_str(), Vector2{ statsRect.x + 20, statsRect.y + 80 }, 16, 1.0f, Color{ 245, 158, 11, 255 });
                
                if (DrawButton(font, Rectangle{ 375, 465, 200, 45 }, "Начать сначала", btnNormal, btnHover, btnClick, textWhite)) {
                    playerNameInput = ""; state = STATE_MENU; showInventory = false; showCrafting = false; player.inventory.clear();
                    survivalStats = SurvivalStats();
                    dayNightCycle = DayNightCycle();
                    gameMenu.SetCurrentState(MENU_MAIN);
                }
            }
            
            // Верхняя панель HUD
            if (state != STATE_WELCOME && state != STATE_CLASS_SELECT && state != STATE_GAME_OVER) {
                DrawRectangle(0, 0, windowWidth, 95, bgPanel);
                DrawLine(0, 95, windowWidth, 95, Color{ 48, 54, 68, 255 });
                
                std::string playerTitle = player.name + " (" + player.className + ")";
                DrawTextEx(font, playerTitle.c_str(), Vector2{ 30, 18 }, 20, 1.0f, textWhite);
                DrawTextEx(font, "HP:", Vector2{ 30, 52 }, 14, 1.0f, textGray);
                DrawProgressBar(Rectangle{ 65, 51, 260, 16 }, (float)player.health, (float)player.maxHealth, Color{ 239, 68, 68, 255 }, Color{ 50, 50, 50, 255 });
                
                std::stringstream ssHP; ssHP << player.health << " / " << player.maxHealth;
                DrawTextEx(font, ssHP.str().c_str(), Vector2{ 155, 52 }, 13, 1.0f, textWhite);
                
                std::stringstream ssDmg; ssDmg << "Урон: " << player.damage;
                DrawTextEx(font, ssDmg.str().c_str(), Vector2{ 370, 36 }, 16, 1.0f, textWhite);
                
                std::stringstream ssGold; ssGold << "Золото: " << player.gold;
                DrawTextEx(font, ssGold.str().c_str(), Vector2{ 490, 36 }, 16, 1.0f, Color{ 245, 158, 11, 255 });
                
                if (state == STATE_2D_WORLD) {
                    DrawTextEx(font, "WASD - бег   [E] - добыча   [I] - инв.   [C] - крафт", Vector2{ windowWidth - 440, 36 }, 14, 1.0f, textGray);
                    
                    // Голод и выносливость
                    DrawTextEx(font, "Голод:", Vector2{ 30, 72 }, 12, 1.0f, textGray);
                    DrawProgressBar(Rectangle{ 75, 71, 120, 12 }, (float)survivalStats.hunger, (float)survivalStats.maxHunger, Color{ 245, 158, 11, 255 }, Color{ 50, 50, 50, 255 });
                    
                    DrawTextEx(font, "Время:", Vector2{ 210, 72 }, 12, 1.0f, textGray);
                    int hours = (int)dayNightCycle.timeOfDay;
                    int minutes = (int)((dayNightCycle.timeOfDay - hours) * 60);
                    std::stringstream ssTime; ssTime << (hours < 10 ? "0" : "") << hours << ":" << (minutes < 10 ? "0" : "") << minutes;
                    DrawTextEx(font, ssTime.str().c_str(), Vector2{ 260, 72 }, 12, 1.0f, textWhite);
                    
                    const char* phaseNames[] = {"Утро", "День", "Вечер", "Ночь"};
                    DrawTextEx(font, phaseNames[dayNightCycle.currentPhase], Vector2{ 320, 72 }, 12, 1.0f, 
                        dayNightCycle.currentPhase == DAY_NIGHT ? Color{ 100, 149, 237, 255 } : 
                        dayNightCycle.currentPhase == DAY_EVENING ? Color{ 253, 186, 73, 255 } : textWhite);
                }
            }

            // Отрисовка инвентаря
if (showInventory && state == STATE_2D_WORLD) {
                // Полноэкранный инвентарь в стиле Rust
                DrawRectangle(0, 0, windowWidth, windowHeight, ColorAlpha(Color{ 10, 10, 10, 240 }, 0.95f));
                
                // Основная панель инвентаря (как в Rust - по центру)
                Rectangle invRect = { windowWidth/2.0f - 300, windowHeight/2.0f - 250, 600, 500 };
                DrawRectangleRounded(invRect, 0.02f, 4, Color{ 35, 35, 40, 255 });
                DrawRectangleRoundedLines(invRect, 0.02f, 4, 2.0f, Color{ 60, 60, 65, 255 });
                
                // Заголовок
                DrawTextEx(font, "ИНВЕНТАРЬ", Vector2{ invRect.x + 20, invRect.y + 15 }, 20, 1.0f, Color{ 200, 200, 200, 255 });
                DrawLine(invRect.x + 15, invRect.y + 45, invRect.x + invRect.width - 15, invRect.y + 45, Color{ 60, 60, 65, 255 });
                
                // Сетка слотов (как в Rust - 5x4)
                int slotSize = 70;
                int slotPadding = 8;
                int slotsX = 5;
                int slotsY = 4;
                float gridStartX = invRect.x + 30;
                float gridStartY = invRect.y + 60;
                
                // Подсчитываем количество травы
                int herbCount = 0;
                int goldCount = 0;
                for (auto item : player.inventory) {
                    if (item == ITEM_HERB) herbCount++;
                    if (item == ITEM_GOLDFLOWER) goldCount++;
                }
                
                // Рисуем сетку слотов
                for (int y = 0; y < slotsY; y++) {
                    for (int x = 0; x < slotsX; x++) {
                        float slotX = gridStartX + x * (slotSize + slotPadding);
                        float slotY = gridStartY + y * (slotSize + slotPadding);
                        Rectangle slotRect = { slotX, slotY, (float)slotSize, (float)slotSize };
                        
                        // Фон слота
                        DrawRectangleRounded(slotRect, 0.05f, 4, Color{ 50, 50, 55, 255 });
                        DrawRectangleRoundedLines(slotRect, 0.05f, 4, 1.0f, Color{ 70, 70, 75, 255 });
                        
                        // Определяем предмет в слоте
                        int slotIndex = y * slotsX + x;
                        ItemType itemType = ITEM_NONE;
                        int itemCount = 0;
                        
                        if (slotIndex < herbCount) {
                            itemType = ITEM_HERB;
                            itemCount = herbCount;
                        } else if (slotIndex < herbCount + goldCount) {
                            itemType = ITEM_GOLDFLOWER;
                            itemCount = goldCount;
                        }
                        
                        // Отрисовка предмета в слоте
                        if (itemType != ITEM_NONE) {
                            Vector2 itemCenter = { slotX + slotSize/2.0f, slotY + slotSize/2.0f };
                            
                            if (itemType == ITEM_HERB) {
                                // Лечебная трава - крестик
                                DrawCircleV(itemCenter, 18, Color{ 34, 197, 94, 80 });
                                DrawCircleV(itemCenter, 12, Color{ 34, 197, 94, 200 });
                                DrawRectangle(itemCenter.x - 1.5f, itemCenter.y - 8, 3, 16, Color{ 255, 255, 255, 220 });
                                DrawRectangle(itemCenter.x - 8, itemCenter.y - 1.5f, 16, 3, Color{ 255, 255, 255, 220 });
                            } else if (itemType == ITEM_GOLDFLOWER) {
                                // Золотой цветок
                                DrawCircleV(itemCenter, 18, Color{ 245, 158, 11, 80 });
                                DrawCircleV(itemCenter, 12, Color{ 253, 224, 71, 200 });
                                DrawCircle(itemCenter.x, itemCenter.y, 4, Color{ 255, 255, 255, 180 });
                            }
                            
                            // Количество в правом нижнем углу
                            if (itemCount > 1) {
                                std::stringstream ssCount;
                                ssCount << "x" << itemCount;
                                DrawTextEx(font, ssCount.str().c_str(), Vector2{ slotX + slotSize - 25, slotY + slotSize - 18 }, 12, 1.0f, Color{ 255, 255, 255, 255 });
                            }
                        }
                    }
                }
                
                // Правая панель - информация о выбранном предмете
                Rectangle infoRect = { invRect.x + invRect.width - 180, invRect.y + 60, 150, 380 };
                DrawRectangleRounded(infoRect, 0.05f, 4, Color{ 45, 45, 50, 255 });
                DrawRectangleRoundedLines(infoRect, 0.05f, 4, 1.0f, Color{ 65, 65, 70, 255 });
                
                DrawTextEx(font, "ПРЕДМЕТЫ", Vector2{ infoRect.x + 10, infoRect.y + 10 }, 14, 1.0f, Color{ 180, 180, 180, 255 });
                
                // Кнопки действий
                float btnY = infoRect.y + 40;
                if (herbCount > 0) {
                    if (DrawButton(font, Rectangle{ infoRect.x + 10, btnY, 130, 35 }, "Использовать", Color{ 16, 185, 129, 255 }, Color{ 52, 211, 153, 255 }, Color{ 4, 120, 87, 255 }, Color{ 255, 255, 255, 255 })) {
                        if (player.health < player.maxHealth) {
                            int healAmount = 25;
                            player.health = std::min(player.maxHealth, player.health + healAmount);
                            auto it = std::find(player.inventory.begin(), player.inventory.end(), ITEM_HERB);
                            if (it != player.inventory.end()) player.inventory.erase(it);
                            AddLogMessage("Сжевал траву, восстановил " + std::to_string(healAmount) + " HP", miniLog);
                        } else {
                            AddLogMessage("Здоровье и так полное", miniLog);
                        }
                    }
                    btnY += 45;
                }
                
                if (goldCount > 0) {
                    DrawTextEx(font, ("Золото: " + std::to_string(goldCount)).c_str(), Vector2{ infoRect.x + 10, btnY }, 14, 1.0f, Color{ 245, 158, 11, 255 });
                    btnY += 30;
                }
                
                // Кнопка закрытия
                if (DrawButton(font, Rectangle{ invRect.x + invRect.width/2.0f - 75, invRect.y + invRect.height - 55, 150, 40 }, "Закрыть [I]", Color{ 71, 85, 105, 255 }, Color{ 100, 116, 139, 255 }, Color{ 51, 65, 85, 255 }, Color{ 255, 255, 255, 255 })) {
                    showInventory = false;
                }
                
                // Подсказка внизу
                DrawTextEx(font, "Нажмите [I] или [ESC] чтобы закрыть", Vector2{ invRect.x + invRect.width/2.0f - 130, invRect.y + invRect.height - 20 }, 12, 1.0f, Color{ 120, 120, 120, 255 });
            }
        }
        EndDrawing();
    }

    UnloadFont(font);
    CloseWindow();
    return 0;
}
