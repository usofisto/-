#pragma once
#include "raylib.h"
#include "game_types.h"
#include <vector>
#include <string>

class CraftingSystem {
public:
    CraftingSystem();
    void InitRecipes();
    bool CanCraft(const CraftingRecipe& recipe, const std::vector<ItemType>& inventory);
    Item Craft(const CraftingRecipe& recipe);
    const std::vector<CraftingRecipe>& GetRecipes() const { return recipes; }

private:
    std::vector<CraftingRecipe> recipes;
};
