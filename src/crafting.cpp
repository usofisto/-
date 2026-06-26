#include "crafting.h"
#include "game_types.h"
#include <algorithm>

CraftingSystem::CraftingSystem() {
    InitRecipes();
}

void CraftingSystem::InitRecipes() {
    CraftingRecipe r;

    // ==================== БАЗОВЫЕ РЕЦЕПТЫ ====================
    
    // Доски
    r = CraftingRecipe();
    r.name = "Доски";
    r.ingredients = {ITEM_WOOD};
    r.amounts = {1};
    r.result = ITEM_WOOD_PLANK;
    r.resultAmount = 4;
    recipes.push_back(r);

    // Палка
    r = CraftingRecipe();
    r.name = "Палка";
    r.ingredients = {ITEM_WOOD_PLANK};
    r.amounts = {2};
    r.result = ITEM_STICK;
    r.resultAmount = 4;
    recipes.push_back(r);

    // ==================== ИНСТРУМЕНТЫ ====================
    
    // Деревянная кирка
    r = CraftingRecipe();
    r.name = "Дерев. кирка";
    r.ingredients = {ITEM_WOOD_PLANK, ITEM_STICK};
    r.amounts = {3, 2};
    r.result = ITEM_WOOD_PICKAXE;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Деревянный меч
    r = CraftingRecipe();
    r.name = "Дерев. меч";
    r.ingredients = {ITEM_WOOD_PLANK, ITEM_STICK};
    r.amounts = {2, 1};
    r.result = ITEM_WOOD_SWORD;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Деревянный топор
    r = CraftingRecipe();
    r.name = "Дерев. топор";
    r.ingredients = {ITEM_WOOD_PLANK, ITEM_STICK};
    r.amounts = {3, 2};
    r.result = ITEM_WOOD_AXE;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Каменная кирка
    r = CraftingRecipe();
    r.name = "Камен. кирка";
    r.ingredients = {ITEM_STONE, ITEM_STICK};
    r.amounts = {3, 2};
    r.result = ITEM_STONE_PICKAXE;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Каменный меч
    r = CraftingRecipe();
    r.name = "Камен. меч";
    r.ingredients = {ITEM_STONE, ITEM_STICK};
    r.amounts = {2, 1};
    r.result = ITEM_STONE_SWORD;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Каменный топор
    r = CraftingRecipe();
    r.name = "Камен. топор";
    r.ingredients = {ITEM_STONE, ITEM_STICK};
    r.amounts = {2, 1};
    r.result = ITEM_STONE_AXE;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Железная кирка
    r = CraftingRecipe();
    r.name = "Желез. кирка";
    r.ingredients = {ITEM_IRON, ITEM_STICK};
    r.amounts = {3, 2};
    r.result = ITEM_IRON_PICKAXE;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Железный меч
    r = CraftingRecipe();
    r.name = "Желез. меч";
    r.ingredients = {ITEM_IRON, ITEM_STICK};
    r.amounts = {2, 1};
    r.result = ITEM_IRON_SWORD;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Железный топор
    r = CraftingRecipe();
    r.name = "Желез. топор";
    r.ingredients = {ITEM_IRON, ITEM_STICK};
    r.amounts = {3, 2};
    r.result = ITEM_IRON_AXE;
    r.resultAmount = 1;
    recipes.push_back(r);

    // ==================== ЕДА ====================
    
    // Хлеб
    r = CraftingRecipe();
    r.name = "Хлеб";
    r.ingredients = {ITEM_WHEAT};
    r.amounts = {3};
    r.result = ITEM_BREAD;
    r.resultAmount = 3;
    recipes.push_back(r);

    // Жареное мясо
    r = CraftingRecipe();
    r.name = "Жареное мясо";
    r.ingredients = {ITEM_MEAT, ITEM_WOOD};
    r.amounts = {1, 1};
    r.result = ITEM_COOKED_MEAT;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Суп
    r = CraftingRecipe();
    r.name = "Суп";
    r.ingredients = {ITEM_CARROT, ITEM_POTATO, ITEM_WOOD};
    r.amounts = {1, 1, 1};
    r.result = ITEM_SOUP;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Торт
    r = CraftingRecipe();
    r.name = "Торт";
    r.ingredients = {ITEM_WHEAT, ITEM_EGG, ITEM_SUGAR};
    r.amounts = {3, 2, 1};
    r.result = ITEM_CAKE;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Жареная рыба
    r = CraftingRecipe();
    r.name = "Жареная рыба";
    r.ingredients = {ITEM_FISH, ITEM_WOOD};
    r.amounts = {1, 1};
    r.result = ITEM_COOKED_FISH;
    r.resultAmount = 1;
    recipes.push_back(r);

    // ==================== ЗЕЛЬЯ ====================
    
    // Зелье здоровья
    r = CraftingRecipe();
    r.name = "Зелье HP";
    r.ingredients = {ITEM_HERB, ITEM_HERB, ITEM_GOLDFLOWER};
    r.amounts = {3, 0, 1};
    r.result = ITEM_POTION_HP;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Зелье скорости
    r = CraftingRecipe();
    r.name = "Зелье скорости";
    r.ingredients = {ITEM_HERB, ITEM_GOLDFLOWER, ITEM_MANA_CRYSTAL};
    r.amounts = {2, 1, 1};
    r.result = ITEM_POTION_SPEED;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Зелье силы
    r = CraftingRecipe();
    r.name = "Зелье силы";
    r.ingredients = {ITEM_HERB, ITEM_BONE, ITEM_MANA_CRYSTAL};
    r.amounts = {2, 2, 1};
    r.result = ITEM_POTION_STRENGTH;
    r.resultAmount = 1;
    recipes.push_back(r);

    // ==================== БРОНЯ ====================
    
    // Кожаный шлем
    r = CraftingRecipe();
    r.name = "Кожан. шлем";
    r.ingredients = {ITEM_LEATHER};
    r.amounts = {3};
    r.result = ITEM_LEATHER_HELMET;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Кожаный нагрудник
    r = CraftingRecipe();
    r.name = "Кожан. нагрудник";
    r.ingredients = {ITEM_LEATHER};
    r.amounts = {5};
    r.result = ITEM_LEATHER_CHEST;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Кожаные ботинки
    r = CraftingRecipe();
    r.name = "Кожан. ботинки";
    r.ingredients = {ITEM_LEATHER};
    r.amounts = {2};
    r.result = ITEM_LEATHER_BOOTS;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Железный шлем
    r = CraftingRecipe();
    r.name = "Желез. шлем";
    r.ingredients = {ITEM_IRON};
    r.amounts = {3};
    r.result = ITEM_IRON_HELMET;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Железный нагрудник
    r = CraftingRecipe();
    r.name = "Желез. нагрудник";
    r.ingredients = {ITEM_IRON};
    r.amounts = {5};
    r.result = ITEM_IRON_CHEST;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Железные ботинки
    r = CraftingRecipe();
    r.name = "Желез. ботинки";
    r.ingredients = {ITEM_IRON};
    r.amounts = {2};
    r.result = ITEM_IRON_BOOTS;
    r.resultAmount = 1;
    recipes.push_back(r);

    // ==================== ФЕРМЕРСТВО ====================
    
    // Семена пшеницы
    r = CraftingRecipe();
    r.name = "Семена пшеницы";
    r.ingredients = {ITEM_WHEAT};
    r.amounts = {1};
    r.result = ITEM_SEED_WHEAT;
    r.resultAmount = 3;
    recipes.push_back(r);

    // Семена моркови
    r = CraftingRecipe();
    r.name = "Семена моркови";
    r.ingredients = {ITEM_CARROT};
    r.amounts = {1};
    r.result = ITEM_SEED_CARROT;
    r.resultAmount = 3;
    recipes.push_back(r);

    // Семена картошки
    r = CraftingRecipe();
    r.name = "Семена картошки";
    r.ingredients = {ITEM_POTATO};
    r.amounts = {1};
    r.result = ITEM_SEED_POTATO;
    r.resultAmount = 3;
    recipes.push_back(r);

    // ==================== СТРОИТЕЛЬСТВО ====================
    
    // Деревянная стена
    r = CraftingRecipe();
    r.name = "Дерев. стена";
    r.ingredients = {ITEM_WOOD_PLANK};
    r.amounts = {4};
    r.result = ITEM_WALL_WOOD;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Каменная стена
    r = CraftingRecipe();
    r.name = "Камен. стена";
    r.ingredients = {ITEM_STONE};
    r.amounts = {4};
    r.result = ITEM_WALL_STONE;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Деревянный пол
    r = CraftingRecipe();
    r.name = "Дерев. пол";
    r.ingredients = {ITEM_WOOD_PLANK};
    r.amounts = {2};
    r.result = ITEM_FLOOR_WOOD;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Сундук
    r = CraftingRecipe();
    r.name = "Сундук";
    r.ingredients = {ITEM_WOOD_PLANK, ITEM_IRON};
    r.amounts = {5, 1};
    r.result = ITEM_CHEST;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Кровать
    r = CraftingRecipe();
    r.name = "Кровать";
    r.ingredients = {ITEM_WOOD_PLANK, ITEM_LEATHER};
    r.amounts = {3, 2};
    r.result = ITEM_BED;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Печь
    r = CraftingRecipe();
    r.name = "Печь";
    r.ingredients = {ITEM_STONE, ITEM_COAL};
    r.amounts = {8, 2};
    r.result = ITEM_FURNACE;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Факел
    r = CraftingRecipe();
    r.name = "Факел";
    r.ingredients = {ITEM_STICK, ITEM_COAL};
    r.amounts = {1, 1};
    r.result = ITEM_TORCH;
    r.resultAmount = 2;
    recipes.push_back(r);
}

bool CraftingSystem::CanCraft(const CraftingRecipe& recipe, const std::vector<ItemType>& inventory) {
    std::vector<int> counts(ITEM_COUNT, 0);
    for (auto t : inventory) {
        if (t != ITEM_NONE && t < (int)counts.size()) counts[t]++;
    }
    for (size_t i = 0; i < recipe.ingredients.size(); i++) {
        int need = recipe.amounts[i];
        if (need <= 0) continue;
        ItemType ing = recipe.ingredients[i];
        if (ing < (int)counts.size() && counts[ing] >= need) {
            counts[ing] -= need;
        } else {
            return false;
        }
    }
    return true;
}

Item CraftingSystem::Craft(const CraftingRecipe& recipe) {
    Item result;
    result.type = recipe.result;
    result.amount = recipe.resultAmount;
    return result;
}
