#include "crafting.h"
#include "game_types.h"
#include <algorithm>

CraftingSystem::CraftingSystem() {
    InitRecipes();
}

void CraftingSystem::InitRecipes() {
    CraftingRecipe r;

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
    r.ingredients = {ITEM_WOOD_PLANK, ITEM_WOOD_PLANK};
    r.amounts = {1, 1};
    r.result = ITEM_STICK;
    r.resultAmount = 4;
    recipes.push_back(r);

    // Деревянная кирка
    r = CraftingRecipe();
    r.name = "Деревянная кирка";
    r.ingredients = {ITEM_WOOD_PLANK, ITEM_WOOD_PLANK, ITEM_WOOD_PLANK, ITEM_STICK};
    r.amounts = {3, 0, 0, 2};
    r.result = ITEM_WOOD_PICKAXE;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Деревянный меч
    r = CraftingRecipe();
    r.name = "Деревянный меч";
    r.ingredients = {ITEM_WOOD_PLANK, ITEM_WOOD_PLANK, ITEM_STICK};
    r.amounts = {2, 0, 1};
    r.result = ITEM_WOOD_SWORD;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Деревянный топор
    r = CraftingRecipe();
    r.name = "Деревянный топор";
    r.ingredients = {ITEM_WOOD_PLANK, ITEM_WOOD_PLANK, ITEM_STICK};
    r.amounts = {2, 0, 1};
    r.result = ITEM_WOOD_AXE;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Каменная кирка
    r = CraftingRecipe();
    r.name = "Каменная кирка";
    r.ingredients = {ITEM_STONE, ITEM_STONE, ITEM_STONE, ITEM_STICK};
    r.amounts = {3, 0, 0, 2};
    r.result = ITEM_STONE_PICKAXE;
    r.resultAmount = 1;
    recipes.push_back(r);

    // Каменный меч
    r = CraftingRecipe();
    r.name = "Каменный меч";
    r.ingredients = {ITEM_STONE, ITEM_STONE, ITEM_STICK};
    r.amounts = {2, 0, 1};
    r.result = ITEM_STONE_SWORD;
    r.resultAmount = 1;
    recipes.push_back(r);
}

bool CraftingSystem::CanCraft(const CraftingRecipe& recipe, const std::vector<ItemType>& inventory) {
    std::vector<int> counts(ITEM_MANA_CRYSTAL + 1, 0);
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
