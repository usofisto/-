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
#include "particles.h"
#include "gui.h"
#include "render.h"
#include "resource_manager.h"
#include "menu.h"
#include "crafting.h"

// Имена предметов для отображения
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
        default: return Color{180, 180, 180, 255};
    }
}

void DrawItemIcon(Vector2 center, ItemType t, float size) {
    Color c = GetItemColor(t);
    switch(t) {
        case ITEM_WOOD: case ITEM_WOOD_PLANK:
            DrawRectangle(center.x - size*0.3f, center.y - size*0.5f, size*0.6f, size, c);
            DrawRectangle(center.x - size*0.25f, center.y - size*0.4f, size*0.15f, size*0.8f, Color{160,110,50,255});
            break;
        case ITEM_STONE:
            DrawCircleV(center, size*0.45f, c);
            DrawCircle(center.x - size*0.1f, center.y - size*0.1f, size*0.15f, Color{180,180,185,255});
            break;
        case ITEM_STICK:
            DrawRectangle(center.x - 1.5f, center.y - size*0.5f, 3, size, c);
            break;
        case ITEM_WOOD_PICKAXE: case ITEM_STONE_PICKAXE:
            DrawRectangle(center.x - 1.5f, center.y - size*0.1f, 3, size*0.6f, Color{139,90,43,255});
            DrawRectangle(center.x - size*0.3f, center.y - size*0.45f, size*0.6f, 6, c);
            break;
        case ITEM_WOOD_SWORD: case ITEM_STONE_SWORD:
            DrawRectangle(center.x - 1.5f, center.y + size*0.05f, 3, size*0.45f, Color{139,90,43,255});
            DrawTriangle({center.x, center.y - size*0.45f}, {center.x - 4, center.y - size*0.05f}, {center.x + 4, center.y - size*0.05f}, c);
            break;
        case ITEM_WOOD_AXE: case ITEM_STONE_AXE:
            DrawRectangle(center.x - 1.5f, center.y - size*0.1f, 3, size*0.6f, Color{139,90,43,255});
            DrawCircle(center.x + 5, center.y - size*0.35f, size*0.2f, c);
            break;
        case ITEM_HERB:
            DrawCircleV(center, size*0.4f, Color{34,197,94,80});
            DrawCircleV(center, size*0.25f, c);
            DrawRectangle(center.x - 1.5f, center.y - size*0.3f, 3, size*0.6f, Color{255,255,255,200});
            DrawRectangle(center.x - size*0.2f, center.y - 1.5f, size*0.4f, 3, Color{255,255,255,200});
            break;
        case ITEM_GOLDFLOWER:
            DrawCircleV(center, size*0.4f, Color{245,158,11,60});
            DrawCircleV(center, size*0.25f, c);
            DrawCircle(center.x, center.y, size*0.1f, Color{255,255,255,200});
            break;
        default:
            DrawRectangle(center.x - size*0.3f, center.y - size*0.3f, size*0.6f, size*0.6f, c);
            break;
    }
}

int main() {
    const int windowWidth = 950;
    const int windowHeight = 650;
    InitWindow(windowWidth, windowHeight, "Legends of Green Meadow - 2D RPG");
    SetTargetFPS(60);
    bool isFullscreen = false;

    int codepoints[512];
    for (int i = 0; i < 128; i++) codepoints[i] = 32 + i;
    for (int i = 0; i < 256; i++) codepoints[128 + i] = 0x0400 + i;
    Font font = LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", 24, codepoints, 384);
    if (font.texture.id == 0) font = GetFontDefault();

    initBiomeRandom();
    InitParticles();

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
    bool showInventory = false;
    bool showCrafting = false;

    Color bgDark = Color{ 24, 26, 32, 255 };
    Color bgPanel = Color{ 34, 38, 48, 255 };
    Color textWhite = Color{ 243, 244, 246, 255 };
    Color textGray = Color{ 156, 163, 175, 255 };

    Vector2 playerPos = { 1000.0f, 1000.0f };
    Vector2 playerFacing = { 0.0f, 1.0f };
    float playerSpeed = 190.0f;
    float playerRadius = 12.0f;

    Camera2D camera = { 0 };
    camera.target = playerPos;
    camera.offset = Vector2{ windowWidth / 2.0f, windowHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

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

    int activeSlimeIndex = -1;
    int slimeHp = 0, slimeMaxHp = 0;
    std::string slimeType = "Зеленая Слизь";
    int combatTurnState = 0;
    float combatTimer = 0.0f;
    std::vector<std::string> combatLog;
    float screenShakeIntensity = 0.0f;
    float redFlashTimer = 0.0f;
    float slashEffectTimer = 0.0f;

    // Подсчёт предметов в инвентаре
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

    auto InitWorldEntities = [&]() {
        obstacles.clear(); ruins.clear(); boulders.clear();
        mapItems.clear(); slimes.clear(); floatingTexts.clear(); miniLog.clear();
        playerPos = Vector2{ 1000.0f, 1000.0f };
        playerFacing = Vector2{ 0.0f, 1.0f };
        camera.target = playerPos;
        survivalStats.health = player.maxHealth;
        survivalStats.maxHealth = player.maxHealth;

        AddLogMessage("Проснулся в лагере, вроде всё тихо.", miniLog);
        AddLogMessage("[WASD] бег  [E] добыча  [I] инв.  [C] крафт  [F11] fullscreen", miniLog);

        srand(6789);
        for (int x = 0; x < 20; ++x)
            for (int y = 0; y < 20; ++y) {
                int r = rand()%8, g = rand()%12, b = rand()%8;
                grassTiles[x][y] = Color{(unsigned char)(22+r),(unsigned char)(95+g),(unsigned char)(44+b),255};
            }

        ruins.push_back({Rectangle{750,1300,150,30}});
        ruins.push_back({Rectangle{1300,700,30,180}});

        for (int i = 0; i < 8; ++i) {
            Vector2 pos;
            while (true) {
                pos = {(float)(100+rand()%1800),(float)(100+rand()%1800)};
                float dc = sqrtf(powf(pos.x-1000,2)+powf(pos.y-1000,2));
                float dl = sqrtf(powf(pos.x-lakePos.x,2)+powf(pos.y-lakePos.y,2));
                if (dc>250 && dl>(lakeRadius+30)) break;
            }
            boulders.push_back({pos, 16.0f+(float)(rand()%6)});
        }

        int attempts = 0;
        while (obstacles.size() < 65 && attempts < 800) {
            attempts++;
            Vector2 c = {(float)(50+rand()%1900),(float)(50+rand()%1900)};
            float dc = sqrtf(powf(c.x-1000,2)+powf(c.y-1000,2));
            float dl = sqrtf(powf(c.x-lakePos.x,2)+powf(c.y-lakePos.y,2));
            if (dc<220 || dl<(lakeRadius+25)) continue;
            bool bad = false;
            for (auto& w : ruins) if (CheckCollisionCircleRec(c,25,w.rect)) {bad=true;break;}
            if (bad) continue;
            for (auto& o : obstacles) if (sqrtf(powf(c.x-o.position.x,2)+powf(c.y-o.position.y,2))<48) {bad=true;break;}
            if (!bad) obstacles.push_back({c,15.0f});
        }

        for (int i = 0; i < 15; ++i) {
            Vector2 pos;
            while (true) {
                pos = {(float)(80+rand()%1840),(float)(80+rand()%1840)};
                float d = sqrtf(powf(pos.x-1000,2)+powf(pos.y-1000,2));
                float dl = sqrtf(powf(pos.x-lakePos.x,2)+powf(pos.y-lakePos.y,2));
                if (d>200 && dl>(lakeRadius+20)) break;
            }
            mapItems.push_back({pos, ITEM_HERB, true, 0.0f});
            while (true) {
                pos = {(float)(80+rand()%1840),(float)(80+rand()%1840)};
                float d = sqrtf(powf(pos.x-1000,2)+powf(pos.y-1000,2));
                float dl = sqrtf(powf(pos.x-lakePos.x,2)+powf(pos.y-lakePos.y,2));
                if (d>200 && dl>(lakeRadius+20)) break;
            }
            mapItems.push_back({pos, ITEM_GOLDFLOWER, true, 0.0f});
        }

        for (int i = 0; i < 7; ++i) {
            Vector2 pos;
            while (true) {
                pos = {(float)(100+rand()%1800),(float)(100+rand()%1800)};
                float d = sqrtf(powf(pos.x-1000,2)+powf(pos.y-1000,2));
                float dl = sqrtf(powf(pos.x-lakePos.x,2)+powf(pos.y-lakePos.y,2));
                if (d>260 && dl>(lakeRadius+30)) break;
            }
            WanderingSlime s;
            s.position = pos; s.targetPosition = pos;
            s.wanderTimer = 0; s.speed = 40.0f+(float)(rand()%28);
            s.maxHp = 25+rand()%15; s.hp = s.maxHp;
            s.active = true; s.id = i;
            slimes.push_back(s);
        }
    };

    // === MAIN LOOP ===
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        framesCounter++;

        // F11 fullscreen toggle
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
        }

        // === ЛОГИКА ===
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
            if (IsKeyPressed(KEY_I) || IsKeyPressed(KEY_TAB)) { showInventory = !showInventory; showCrafting = false; }
            if (IsKeyPressed(KEY_C)) { showCrafting = !showCrafting; showInventory = false; }
            if (IsKeyPressed(KEY_ESCAPE)) { showInventory = false; showCrafting = false; }

            // Выживание
            dayNightCycle.Update(dt);
            survivalStats.hungerTimer -= dt;
            if (survivalStats.hungerTimer <= 0) {
                survivalStats.hunger = std::max(0, survivalStats.hunger - 1);
                survivalStats.hungerTimer = 30.0f;
            }
            if (survivalStats.hunger <= 0 && survivalStats.health > 0)
                survivalStats.health = std::max(0, survivalStats.health - 1);
            if (survivalStats.hunger > 10 && survivalStats.health < survivalStats.maxHealth)
                survivalStats.health = std::min(survivalStats.maxHealth, survivalStats.health + 1);
            player.health = survivalStats.health;

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

                    Vector2 nx = {playerPos.x+sx, playerPos.y};
                    bool cx = false;
                    if (nx.x<15||nx.x>1985) cx=true;
                    for (auto& o:obstacles) if (sqrtf(powf(nx.x-o.position.x,2)+powf(nx.y-o.position.y,2))<(playerRadius+o.radius)){cx=true;break;}
                    for (auto& b:boulders) if (sqrtf(powf(nx.x-b.position.x,2)+powf(nx.y-b.position.y,2))<(playerRadius+b.radius)){cx=true;break;}
                    for (auto& w:ruins) if (CheckCollisionCircleRec(nx,playerRadius,w.rect)){cx=true;break;}
                    if (sqrtf(powf(nx.x-lakePos.x,2)+powf(nx.y-lakePos.y,2))<(playerRadius+lakeRadius-6)) cx=true;
                    if (!cx) playerPos.x=nx.x;

                    Vector2 ny = {playerPos.x, playerPos.y+sy};
                    bool cy = false;
                    if (ny.y<15||ny.y>1985) cy=true;
                    for (auto& o:obstacles) if (sqrtf(powf(ny.x-o.position.x,2)+powf(ny.y-o.position.y,2))<(playerRadius+o.radius)){cy=true;break;}
                    for (auto& b:boulders) if (sqrtf(powf(ny.x-b.position.x,2)+powf(ny.y-b.position.y,2))<(playerRadius+b.radius)){cy=true;break;}
                    for (auto& w:ruins) if (CheckCollisionCircleRec(ny,playerRadius,w.rect)){cy=true;break;}
                    if (sqrtf(powf(ny.x-lakePos.x,2)+powf(ny.y-lakePos.y,2))<(playerRadius+lakeRadius-6)) cy=true;
                    if (!cy) playerPos.y=ny.y;

                    if (player.className=="Маг" && rand()%100<15) {
                        Vector2 tip = {playerPos.x+(playerFacing.x<0?-12:12), playerPos.y-12};
                        SpawnParticle(tip, {(float)(rand()%30-15),(float)(rand()%30-15)}, Color{253,224,71,255}, 2.0f, 0.4f+(rand()%4)/10.0f);
                    }
                }

                camera.target.x += (playerPos.x-camera.target.x)*0.08f;
                camera.target.y += (playerPos.y-camera.target.y)*0.08f;

                // Отдых в палатке
                float dtTent = sqrtf(powf(playerPos.x-tentPos.x,2)+powf(playerPos.y-tentPos.y,2));
                if (dtTent<45 && IsKeyPressed(KEY_E)) {
                    survivalStats.health = survivalStats.maxHealth;
                    survivalStats.hunger = survivalStats.maxHunger;
                    player.health = survivalStats.maxHealth;
                    AddLogMessage("Отдохнул в палатке. HP и голод полные.", miniLog);
                    floatingTexts.push_back({"+HP +Голод", Vector2{playerPos.x,playerPos.y-15}, Color{34,197,94,255}, 1,-45,1.2f,true});
                    for (int p=0;p<12;p++) { float a=p*6.28f/12; SpawnParticle(playerPos, {cosf(a)*45,sinf(a)*45}, Color{74,222,128,255}, 3, 0.6f); }
                }

                // Добыча дерева/камня [E]
                if (IsKeyPressed(KEY_E)) {
                    bool gathered = false;
                    for (auto& o:obstacles) {
                        float d = sqrtf(powf(playerPos.x-o.position.x,2)+powf(playerPos.y-o.position.y,2));
                        if (d<50) {
                            int amt = 1+rand()%2;
                            for (int w=0;w<amt;w++) player.inventory.push_back(ITEM_WOOD);
                            std::stringstream ss; ss<<"+"<<amt<<" Дерево";
                            floatingTexts.push_back({ss.str(), Vector2{playerPos.x,playerPos.y-15}, Color{139,90,43,255}, 1,-40,1.2f,true});
                            AddLogMessage("Срубил дерево, +"+std::to_string(amt)+" дерева.", miniLog);
                            gathered = true; break;
                        }
                    }
                    if (!gathered) for (auto& b:boulders) {
                        float d = sqrtf(powf(playerPos.x-b.position.x,2)+powf(playerPos.y-b.position.y,2));
                        if (d<35) {
                            int amt = 1+rand()%2;
                            for (int s=0;s<amt;s++) player.inventory.push_back(ITEM_STONE);
                            std::stringstream ss; ss<<"+"<<amt<<" Камень";
                            floatingTexts.push_back({ss.str(), Vector2{playerPos.x,playerPos.y-15}, Color{156,163,175,255}, 1,-40,1.2f,true});
                            AddLogMessage("Выбил камень, +"+std::to_string(amt)+" камня.", miniLog);
                            break;
                        }
                    }
                }

                // Сбор предметов на карте
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

                // Слизни
                for (size_t i=0;i<slimes.size();++i) {
                    auto& s=slimes[i];
                    if (!s.active) continue;
                    s.wanderTimer-=dt;
                    if (s.wanderTimer<=0) {
                        s.wanderTimer=2.5f+(rand()%35)/10.0f;
                        float a=(float)(rand()%360)*3.14159f/180;
                        float r=70+rand()%100;
                        s.targetPosition={s.position.x+cosf(a)*r, s.position.y+sinf(a)*r};
                        s.targetPosition.x=std::max(50.f,std::min(1950.f,s.targetPosition.x));
                        s.targetPosition.y=std::max(50.f,std::min(1950.f,s.targetPosition.y));
                        float dc=sqrtf(powf(s.targetPosition.x-1000,2)+powf(s.targetPosition.y-1000,2));
                        float dl=sqrtf(powf(s.targetPosition.x-lakePos.x,2)+powf(s.targetPosition.y-lakePos.y,2));
                        if (dc<250||dl<(lakeRadius+20)) s.targetPosition=s.position;
                    }
                    Vector2 toT={s.targetPosition.x-s.position.x,s.targetPosition.y-s.position.y};
                    float dt2=sqrtf(toT.x*toT.x+toT.y*toT.y);
                    if (dt2>2) { s.position.x+=(toT.x/dt2)*s.speed*dt; s.position.y+=(toT.y/dt2)*s.speed*dt; }
                    if (rand()%100<8) SpawnParticle(s.position, {0,0}, s.speed>60?Color{248,113,113,160}:Color{74,222,128,160}, 3+rand()%3, 0.6f+(rand()%5)/10.f);

                    float dp=sqrtf(powf(playerPos.x-s.position.x,2)+powf(playerPos.y-s.position.y,2));
                    if (dp<(playerRadius+14)) {
                        state=STATE_MEADOW_SLIME;
                        activeSlimeIndex=(int)i; slimeHp=s.hp; slimeMaxHp=s.maxHp;
                        slimeType=s.speed>60?"Быстрая Алая Слизь":"Зеленая Слизь";
                        combatLog.clear(); combatLog.push_back("!!! Нарвался на "+slimeType+"!");
                        combatTurnState=0; combatTimer=0; screenShakeIntensity=0; redFlashTimer=0; slashEffectTimer=0;
                    }
                }

                if (rand()%100<25) SpawnParticle(campfirePos, {(float)(rand()%30-15),(float)(-35-rand()%30)}, Color{245,158,11,255}, 2.5f+rand()%2, 0.8f+(rand()%6)/10.f);
            }

            for (auto& t:floatingTexts) { if (!t.active) continue; t.position.y+=t.ySpeed*dt; t.lifetime-=dt; t.alpha=t.lifetime/1.2f; if (t.lifetime<=0) t.active=false; }
            for (auto& item:miniLog) { if (item.second>0) item.second-=dt; }
        }
        else if (state==STATE_MEADOW_SLIME) {
            if (screenShakeIntensity>0) screenShakeIntensity-=dt*12;
            if (redFlashTimer>0) redFlashTimer-=dt;
            if (slashEffectTimer>0) slashEffectTimer-=dt;

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
                    floatingTexts.push_back({sd2.str(), Vector2{windowWidth/4.f+10,220}, Color{239,68,68,255}, 1,-40,1.2f,true});
                    if (player.health<=0) { player.isAlive=false; combatLog.push_back("Не выдержал..."); combatTurnState=3; }
                    else combatTurnState=0;
                    combatTimer=0;
                }
            } else if (combatTurnState==2) { combatTimer+=dt; if (combatTimer>=1.4f) { state=STATE_2D_WORLD; combatTimer=0; } }
            else if (combatTurnState==3) { combatTimer+=dt; if (combatTimer>=1.8f) { state=STATE_GAME_OVER; combatTimer=0; } }

            for (auto& t:floatingTexts) { if (!t.active) continue; t.position.y+=t.ySpeed*dt; t.lifetime-=dt; t.alpha=t.lifetime/1.2f; if (t.lifetime<=0) t.active=false; }
        }

        // === ОТРИСОВКА ===
        BeginDrawing();
        ClearBackground(bgDark);

        if (state==STATE_MENU) {
            MenuState ms=gameMenu.GetCurrentState();
            if (ms==MENU_MAIN) gameMenu.DrawMainMenu(font, windowWidth, windowHeight, framesCounter);
            else if (ms==MENU_WORLD_CREATE) gameMenu.DrawWorldCreateMenu(font, windowWidth, windowHeight);
            else if (ms==MENU_SETTINGS) gameMenu.DrawSettingsMenu(font, windowWidth, windowHeight);
            else gameMenu.DrawMainMenu(font, windowWidth, windowHeight, framesCounter);
        }
        else if (state==STATE_WELCOME) {
            // ===== MINECRAFT-СТИЛЬ ЭКРАН ВВОДА ИМЕНИ =====
            DrawRectangleGradientV(0, 0, windowWidth, windowHeight, Color{30,30,30,255}, Color{15,15,15,255});

            // Тайловый фон (как в Minecraft)
            for (int x=0; x<windowWidth; x+=32)
                for (int y=0; y<windowHeight; y+=32) {
                    unsigned char v = 25 + (rand()%8);
                    DrawRectangle(x, y, 32, 32, Color{v, (unsigned char)(v+10), v, 255});
                    DrawRectangleLines(x, y, 32, 32, Color{(unsigned char)(v-5),(unsigned char)(v+5),(unsigned char)(v-5),60});
                }

            // Заголовок по центру
            const char* title = "LEGENDS OF GREEN MEADOW";
            Vector2 ts = MeasureTextEx(font, title, 36, 1);
            DrawTextEx(font, title, {windowWidth/2.f - ts.x/2 + 2, 142}, 36, 1, Color{0,0,0,150});
            DrawTextEx(font, title, {windowWidth/2.f - ts.x/2, 140}, 36, 1, Color{80,200,120,255});

            // Подзаголовок
            const char* sub = "Введите имя вашего героя";
            Vector2 ss = MeasureTextEx(font, sub, 18, 1);
            DrawTextEx(font, sub, {windowWidth/2.f - ss.x/2, 200}, 18, 1, Color{180,180,180,255});

            // Поле ввода (Minecraft стиль)
            Rectangle inputRect = {windowWidth/2.f - 180, 250, 360, 50};
            DrawRectangleRec(inputRect, Color{50,50,50,255});
            DrawRectangleLinesEx(inputRect, 3, Color{30,30,30,255});
            // Внутренняя рамка
            Rectangle inner = {inputRect.x+4, inputRect.y+4, inputRect.width-8, inputRect.height-8};
            DrawRectangleRec(inner, Color{20,20,20,255});

            // Текст
            Vector2 ns = MeasureTextEx(font, playerNameInput.c_str(), 22, 1);
            DrawTextEx(font, playerNameInput.c_str(), {inputRect.x + inputRect.width/2 - ns.x/2, inputRect.y + 14}, 22, 1, Color{255,255,255,255});

            // Курсор
            if (((framesCounter/30)%2)==0 && playerNameInput.length()<14) {
                float cx = inputRect.x + inputRect.width/2 + ns.x/2 + 3;
                DrawRectangle((int)cx, (int)inputRect.y+12, 2, 26, Color{255,255,255,200});
            }

            // Кнопка "Играть"
            bool canPlay = !playerNameInput.empty();
            Rectangle btnPlay = {windowWidth/2.f - 100, 330, 200, 45};
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
            DrawTextEx(font, "Подсказки:", {50, windowHeight-90}, 14, 1, Color{120,120,120,255});
            DrawTextEx(font, "[WASD] бег   [E] добыча ресурсов   [I] инвентарь   [C] кRAFT   [F11] полноэкран", {50, windowHeight-70}, 13, 1, Color{100,100,100,255});
            DrawTextEx(font, "[TAB] инвентарь   [ESC] закрыть меню   [ENTER] подтвердить", {50, windowHeight-50}, 13, 1, Color{100,100,100,255});
        }
        else if (state==STATE_CLASS_SELECT) {
            DrawRectangle(0,0,windowWidth,95,bgPanel);
            DrawLine(0,95,windowWidth,95,Color{48,54,68,255});
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
        else {
            if (state==STATE_2D_WORLD) {
                BeginMode2D(camera);

                Texture2D grassTex = ResourceManager::Get().GetTex("grass");
                for (int tx=0;tx<20;++tx) for (int ty=0;ty<20;++ty) {
                    if (grassTex.id!=0) DrawTexture(grassTex, tx*grassTex.width, ty*grassTex.height, WHITE);
                    else { DrawRectangle(tx*100,ty*100,100,100,grassTiles[tx][ty]); DrawRectangleLines(tx*100,ty*100,100,100,Color{20,85,40,30}); }
                }

                srand(2345);
                for (int i=0;i<50;++i) {
                    int fx=rand()%2000, fy=rand()%2000;
                    float dc=sqrtf(powf(fx-1000.f,2)+powf(fy-1000.f,2));
                    float dl=sqrtf(powf(fx-lakePos.x,2)+powf(fy-lakePos.y,2));
                    if (dc>160&&dl>(lakeRadius+15)) {
                        Color fc=(rand()%2==0)?Color{244,114,182,160}:Color{253,224,71,160};
                        DrawCircle(fx,fy,2,fc); DrawCircle(fx,fy-2,1,Color{255,255,255,200});
                    }
                }

                DrawCircle(1000,1000,150,Color{63,63,70,255});
                DrawCircleLines(1000,1000,150,Color{82,82,91,255});
                DrawLineEx(campfirePos,tentPos,22,Color{82,82,91,255});
                DrawLineEx(campfirePos,tentPos,18,Color{113,113,122,255});
                DrawLineEx(Vector2{1000,1000},Vector2{1200,1010},24,Color{82,82,91,255});
                DrawLineEx(Vector2{1000,1000},Vector2{1200,1010},20,Color{113,113,122,255});

                DrawTent(tentPos);
                DrawCampfire(campfirePos, framesCounter);
                DrawWaterPond(lakePos, lakeRadius, framesCounter);

                float dtTent = sqrtf(powf(playerPos.x-tentPos.x,2)+powf(playerPos.y-tentPos.y,2));
                if (dtTent<45 && player.health<player.maxHealth) {
                    DrawRectangle(tentPos.x-70, tentPos.y-65, 140, 25, Color{30,30,30,200});
                    DrawTextEx(font, "[E] ОТДОХНУТЬ", Vector2{tentPos.x-55, tentPos.y-60}, 13, 1, Color{253,224,71,255});
                }

                // Подсказка у дерева
                for (auto& obs:obstacles) {
                    float d = sqrtf(powf(playerPos.x-obs.position.x,2)+powf(playerPos.y-obs.position.y,2));
                    if (d<60) {
                        DrawRectangle(obs.position.x-40, obs.position.y-55, 80, 18, Color{30,30,30,180});
                        DrawTextEx(font, "[E] Добыть", Vector2{obs.position.x-32, obs.position.y-52}, 12, 1, Color{200,180,120,255});
                    }
                }
                for (auto& b:boulders) {
                    float d = sqrtf(powf(playerPos.x-b.position.x,2)+powf(playerPos.y-b.position.y,2));
                    if (d<45) {
                        DrawRectangle(b.position.x-40, b.position.y-35, 80, 18, Color{30,30,30,180});
                        DrawTextEx(font, "[E] Камень", Vector2{b.position.x-32, b.position.y-32}, 12, 1, Color{180,180,180,255});
                    }
                }

                for (auto& w:ruins) DrawStoneWall(w);
                for (auto& item:mapItems) DrawMapItem(item);
                for (auto& s:slimes) DrawWanderingSlime(s, framesCounter, bgDark);
                for (auto& b:boulders) {
                    DrawEllipse(b.position.x,b.position.y+b.radius-2,b.radius,b.radius*0.4f,Color{0,0,0,100});
                    DrawCircleV(b.position,b.radius,Color{82,82,91,255});
                    DrawCircleV(b.position,b.radius-3,Color{113,113,122,255});
                    DrawCircle(b.position.x-b.radius*0.3f,b.position.y-b.radius*0.3f,b.radius*0.25f,Color{161,161,170,220});
                    DrawCircleLines(b.position.x,b.position.y,b.radius,Color{63,63,70,255});
                }
                for (auto& o:obstacles) { DrawEllipse(o.position.x,o.position.y+24,12,5,Color{0,0,0,100}); DrawTree(o.position,o.radius); }

                UpdateAndDrawParticles(dt);

                bool isMoving = (IsKeyDown(KEY_W)||IsKeyDown(KEY_UP)||IsKeyDown(KEY_S)||IsKeyDown(KEY_DOWN)||IsKeyDown(KEY_A)||IsKeyDown(KEY_LEFT)||IsKeyDown(KEY_D)||IsKeyDown(KEY_RIGHT)) && !showInventory && !showCrafting;
                DrawPlayerAvatar(player.className, playerPos, playerRadius, playerFacing, isMoving, framesCounter, bgDark);

                for (auto& t:floatingTexts) { if (!t.active) continue; DrawTextEx(font,t.text.c_str(),t.position,15,1,ColorAlpha(t.color,t.alpha)); }
                DrawRectangleLines(0,0,2000,2000,Color{220,38,38,120});
                EndMode2D();

                // Оверлей день/ночь
                if (dayNightCycle.currentPhase==DAY_EVENING) { float a=(dayNightCycle.timeOfDay-18)/3; DrawRectangle(0,0,windowWidth,windowHeight,Color{10,10,30,(unsigned char)(a*120)}); }
                else if (dayNightCycle.currentPhase==DAY_NIGHT) DrawRectangle(0,0,windowWidth,windowHeight,Color{10,10,30,120});
                else if (dayNightCycle.currentPhase==DAY_MORNING) { float a=1-(dayNightCycle.timeOfDay-6)/6; DrawRectangle(0,0,windowWidth,windowHeight,Color{10,10,30,(unsigned char)(a*80)}); }

                // === ИНВЕНТАРЬ ===
                if (showInventory) {
                    DrawRectangle(0,0,windowWidth,windowHeight,Color{0,0,0,200});
                    Rectangle invR={windowWidth/2.f-300, 50, 600, 540};
                    DrawRectangleRounded(invR, 0.02f, 4, Color{40,40,45,255});
                    DrawRectangleRoundedLines(invR, 0.02f, 4, 2, Color{70,70,75,255});

                    DrawTextEx(font, "ИНВЕНТАРЬ", {invR.x+20, invR.y+15}, 20, 1, Color{220,220,220,255});
                    DrawLine(invR.x+15,invR.y+45,invR.x+invR.width-15,invR.y+45,Color{70,70,75,255});

                    // Сетка слотов
                    int slotSize=65, slotPad=6, slotsX=8, slotsY=5;
                    float gx=invR.x+20, gy=invR.y+55;
                    int slotIdx=0;
                    // Собираем уникальные предметы с количеством
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

                    // Золото
                    DrawTextEx(font, ("Золото: "+std::to_string(player.gold)).c_str(), {infoR.x+10,btnY}, 14, 1, Color{245,158,11,255});

                    // Кнопка закрытия
                    if (DrawButton(font, {invR.x+invR.width/2-75, invR.y+invR.height-50, 150, 38}, "Закрыть [I]", Color{71,85,105,255}, Color{100,116,139,255}, Color{51,65,85,255}, Color{255,255,255,255}))
                        showInventory=false;
                }

                // === КРАФТ ===
                if (showCrafting) {
                    DrawRectangle(0,0,windowWidth,windowHeight,Color{0,0,0,200});
                    Rectangle cR={windowWidth/2.f-280, 40, 560, 560};
                    DrawRectangleRounded(cR, 0.02f, 4, Color{40,40,45,255});
                    DrawRectangleRoundedLines(cR, 0.02f, 4, 2, Color{70,70,75,255});

                    DrawTextEx(font, "КРАФТ", {cR.x+20,cR.y+15}, 20, 1, Color{220,220,220,255});
                    DrawLine(cR.x+15,cR.y+45,cR.x+cR.width-15,cR.y+45,Color{70,70,75,255});

                    // Ресурсы
                    int wC=CountItems(ITEM_WOOD), sC=CountItems(ITEM_STONE), pC=CountItems(ITEM_STICK), dC=CountItems(ITEM_WOOD_PLANK);
                    DrawTextEx(font, "Ресурсы:", {cR.x+20,cR.y+55}, 14, 1, Color{180,180,180,255});
                    std::stringstream res;
                    res<<"[Дерево:"<<wC<<"] [Камень:"<<sC<<"] [Палки:"<<pC<<"] [Доски:"<<dC<<"]";
                    DrawTextEx(font, res.str().c_str(), {cR.x+20,cR.y+75}, 13, 1, Color{160,160,160,255});

                    // Рецепты
                    float ry=cR.y+100;
                    auto& recipes=craftingSystem.GetRecipes();
                    for (auto& recipe:recipes) {
                        if (ry>cR.y+cR.height-60) break;
                        bool can=craftingSystem.CanCraft(recipe, player.inventory);
                        Color bg=can?Color{45,55,45,255}:Color{55,45,45,255};
                        Rectangle rR={cR.x+15, ry, cR.width-30, 55};
                        DrawRectangleRounded(rR, 0.05f, 4, bg);
                        DrawRectangleRoundedLines(rR, 0.05f, 4, 1, can?Color{80,140,80,255}:Color{120,80,80,255});

                        // Иконка результата
                        DrawItemIcon({rR.x+25, rR.y+20}, recipe.result, 20);
                        DrawTextEx(font, recipe.name.c_str(), {rR.x+50, rR.y+5}, 14, 1, Color{220,220,220,255});

                        // Ингредиенты
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
                                for (int r=0;r<recipe.resultAmount;r++) player.inventory.push_back(recipe.result);
                                AddLogMessage("Скрафтил: "+recipe.name, miniLog);
                                floatingTexts.push_back({"+"+recipe.name, {playerPos.x,playerPos.y-15}, Color{253,224,71,255}, 1,-40,1.2f,true});
                            }
                        }
                        ry+=62;
                    }
                    DrawTextEx(font, "[C] закрыть", {cR.x+cR.width-100, cR.y+cR.height-25}, 12, 1, Color{100,100,100,255});
                }

                // Лог
                if (!showInventory && !showCrafting) {
                    int logY=windowHeight-40;
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
            else if (state==STATE_MEADOW_SLIME) {
                Vector2 sh={0,0};
                if (screenShakeIntensity>0) { sh.x=(float)(rand()%20-10)*(screenShakeIntensity/10); sh.y=(float)(rand()%20-10)*(screenShakeIntensity/10); }

                DrawRectangle(30+sh.x,120+sh.y,windowWidth-60,490,bgPanel);
                DrawRectangleLines(30+sh.x,120+sh.y,windowWidth-60,490,Color{48,54,68,255});

                Rectangle pCard={80+sh.x,170+sh.y,240,280};
                DrawRectangleRounded(pCard,0.1f,4,Color{26,28,35,255});
                DrawRectangleRoundedLines(pCard,0.1f,4,1.5f,Color{99,102,241,255});
                DrawPlayerAvatar(player.className, Vector2{pCard.x+pCard.width/2,pCard.y+80}, playerRadius, playerFacing, false, 0, bgDark);

                std::string pName=player.name+" ("+player.className+")";
                DrawTextEx(font,pName.c_str(),{pCard.x+(pCard.width-MeasureTextEx(font,pName.c_str(),18,1).x)/2, pCard.y+150},18,1,textWhite);
                std::stringstream ssS; ssS<<"Урон: "<<player.damage<<"   Золото: "<<player.gold;
                DrawTextEx(font,ssS.str().c_str(),{pCard.x+(pCard.width-MeasureTextEx(font,ssS.str().c_str(),14,1).x)/2,pCard.y+185},14,1,textGray);
                DrawProgressBar(Rectangle{pCard.x+20,pCard.y+225,200,16},(float)player.health,(float)player.maxHealth,Color{239,68,68,255},Color{50,50,50,255});
                std::stringstream ssHP; ssHP<<player.health<<" / "<<player.maxHealth<<" HP";
                DrawTextEx(font,ssHP.str().c_str(),{pCard.x+(pCard.width-MeasureTextEx(font,ssHP.str().c_str(),12,1).x)/2,pCard.y+248},12,1,textWhite);

                DrawTextEx(font,"VS",{windowWidth/2.f-15+sh.x,280+sh.y},26,1,Color{239,68,68,180});

                Rectangle sCard={windowWidth-320+sh.x,170+sh.y,240,280};
                DrawRectangleRounded(sCard,0.1f,4,Color{26,28,35,255});
                DrawRectangleRoundedLines(sCard,0.1f,4,1.5f,Color{239,68,68,255});

                float cb=sinf(framesCounter*0.16f)*6;
                float srx=55+cb*0.7f, sry=45-cb*0.7f;
                Vector2 sc={sCard.x+sCard.width/2,sCard.y+80+cb*0.5f};
                Color sCol=slimeType.find("Алая")!=std::string::npos?Color{220,38,38,255}:Color{34,197,94,255};
                Color sGlow=slimeType.find("Алая")!=std::string::npos?Color{248,113,113,160}:Color{74,222,128,160};
                Color sOut=slimeType.find("Алая")!=std::string::npos?Color{127,29,29,255}:Color{20,83,45,255};

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
                if (redFlashTimer>0) DrawRectangle(30,120,windowWidth-60,490,Color{239,68,68,(unsigned char)(90*(redFlashTimer/0.15f))});

                Rectangle ccR={80+sh.x,470+sh.y,(float)(windowWidth-160),120};
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
                            if (slimeHp<=0) { int gr=15+rand()%16; player.gold+=gr; combatLog.push_back("Слизь разлетелась! +"+std::to_string(gr)+" золота."); if (activeSlimeIndex!=-1) slimes[activeSlimeIndex].active=false; combatTurnState=2; combatTimer=0; }
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

            // HUD
            if (state!=STATE_WELCOME&&state!=STATE_CLASS_SELECT&&state!=STATE_GAME_OVER) {
                DrawRectangle(0,0,windowWidth,95,bgPanel);
                DrawLine(0,95,windowWidth,95,Color{48,54,68,255});
                std::string pt=player.name+" ("+player.className+")";
                DrawTextEx(font,pt.c_str(),{30,18},20,1,textWhite);
                DrawTextEx(font,"HP:",{30,52},14,1,textGray);
                DrawProgressBar(Rectangle{65,51,200,16},(float)survivalStats.health,(float)survivalStats.maxHealth,Color{239,68,68,255},Color{50,50,50,255});
                std::stringstream ssH; ssH<<survivalStats.health<<"/"<<survivalStats.maxHealth;
                DrawTextEx(font,ssH.str().c_str(),{155,52},13,1,textWhite);

                DrawTextEx(font,"Голод:",{30,72},12,1,textGray);
                DrawProgressBar(Rectangle{75,71,100,10},(float)survivalStats.hunger,(float)survivalStats.maxHunger,Color{245,158,11,255},Color{50,50,50,255});

                DrawTextEx(font,("Урон: "+std::to_string(player.damage)).c_str(),{300,36},16,1,textWhite);
                DrawTextEx(font,("Золото: "+std::to_string(player.gold)).c_str(),{430,36},16,1,Color{245,158,11,255});

                if (state==STATE_2D_WORLD) {
                    int h=(int)dayNightCycle.timeOfDay, m=(int)((dayNightCycle.timeOfDay-h)*60);
                    std::stringstream ssT; ssT<<(h<10?"0":"")<<h<<":"<<(m<10?"0":"")<<m;
                    const char* ph[]={"Утро","День","Вечер","Ночь"};
                    DrawTextEx(font,ssT.str().c_str(),{600,36},16,1,textWhite);
                    DrawTextEx(font,ph[dayNightCycle.currentPhase],{680,36},14,1,dayNightCycle.currentPhase==DAY_NIGHT?Color{100,149,237,255}:textWhite);
                    DrawTextEx(font,"[WASD] бег  [E] добыча  [I] инв.  [C] крафт  [F11] fullscreen", {windowWidth-470,72}, 13, 1, textGray);
                }
            }
        }
        EndDrawing();
    }

    UnloadFont(font);
    CloseWindow();
    return 0;
}
