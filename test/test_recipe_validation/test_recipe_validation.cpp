#include <unity.h>
#include <string.h>
#include <stdio.h>

#include "recipe.h"

void setUp(void) {}
void tearDown(void) {}

static Recipe validRecipe() {
    Recipe recipe{};
    snprintf(recipe.name, sizeof(recipe.name), "%s", "Valid recipe");
    recipe.maischtemp = RECIPE_MASH_TEMP_MIN;
    recipe.rasten = 2;
    recipe.rastTemp[1] = RECIPE_REST_TEMP_MIN;
    recipe.rastZeit[1] = RECIPE_REST_DURATION_MIN;
    recipe.rastTemp[2] = RECIPE_REST_TEMP_MAX;
    recipe.rastZeit[2] = RECIPE_REST_DURATION_MAX;
    recipe.endtemp = RECIPE_MASH_OUT_TEMP_MAX;
    recipe.kochzeit = RECIPE_BOIL_DURATION_MAX;
    recipe.hopfenanzahl = 2;
    recipe.hopfenZeit[1] = RECIPE_HOP_TIME_MIN;
    recipe.hopfenZeit[2] = recipe.kochzeit;
    return recipe;
}

static void assertError(const Recipe &recipe, RecipeValidationError error, uint8_t index = 0) {
    const RecipeValidationResult result = validateRecipe(recipe);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(error), static_cast<uint8_t>(result.error));
    TEST_ASSERT_EQUAL_UINT8(index, result.index);
}

void test_valid_recipe_and_limits(void) {
    Recipe recipe = validRecipe();
    for (size_t i = 0; i < RECIPE_NAME_CAPACITY - 1; ++i) {
        recipe.name[i] = 'a';
    }
    recipe.name[RECIPE_NAME_CAPACITY - 1] = '\0';
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RecipeValidationError::None),
                            static_cast<uint8_t>(validateRecipe(recipe).error));

    recipe.maischtemp = RECIPE_MASH_TEMP_MAX;
    recipe.endtemp = RECIPE_MASH_OUT_TEMP_MIN;
    recipe.kochzeit = RECIPE_BOIL_DURATION_MIN;
    recipe.hopfenZeit[2] = recipe.kochzeit;
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(RecipeValidationError::None),
                            static_cast<uint8_t>(validateRecipe(recipe).error));
}

void test_name_errors(void) {
    Recipe recipe = validRecipe();
    recipe.name[0] = '\0';
    assertError(recipe, RecipeValidationError::EmptyName);

    recipe = validRecipe();
    recipe.name[1] = '\n';
    assertError(recipe, RecipeValidationError::InvalidName);

    recipe = validRecipe();
    for (size_t i = 0; i < RECIPE_NAME_CAPACITY; ++i) {
        recipe.name[i] = 'a';
    }
    assertError(recipe, RecipeValidationError::InvalidName);
}

void test_scalar_bounds(void) {
    Recipe recipe = validRecipe();
    recipe.maischtemp = RECIPE_MASH_TEMP_MIN - 1;
    assertError(recipe, RecipeValidationError::MashTemperature);
    recipe.maischtemp = RECIPE_MASH_TEMP_MAX + 1;
    assertError(recipe, RecipeValidationError::MashTemperature);

    recipe = validRecipe();
    recipe.endtemp = RECIPE_MASH_OUT_TEMP_MIN - 1;
    assertError(recipe, RecipeValidationError::MashOutTemperature);
    recipe.endtemp = RECIPE_MASH_OUT_TEMP_MAX + 1;
    assertError(recipe, RecipeValidationError::MashOutTemperature);

    recipe = validRecipe();
    recipe.kochzeit = RECIPE_BOIL_DURATION_MIN - 1;
    assertError(recipe, RecipeValidationError::BoilDuration);
    recipe.kochzeit = RECIPE_BOIL_DURATION_MAX + 1;
    assertError(recipe, RecipeValidationError::BoilDuration);
}

void test_rest_bounds_and_indices(void) {
    Recipe recipe = validRecipe();
    recipe.rasten = 0;
    assertError(recipe, RecipeValidationError::RestCount);
    recipe.rasten = RECIPE_REST_COUNT_MAX + 1;
    assertError(recipe, RecipeValidationError::RestCount);

    recipe = validRecipe();
    recipe.rastTemp[2] = RECIPE_REST_TEMP_MIN - 1;
    assertError(recipe, RecipeValidationError::RestTemperature, 2);
    recipe.rastTemp[2] = RECIPE_REST_TEMP_MAX + 1;
    assertError(recipe, RecipeValidationError::RestTemperature, 2);

    recipe = validRecipe();
    recipe.rastZeit[2] = RECIPE_REST_DURATION_MIN - 1;
    assertError(recipe, RecipeValidationError::RestDuration, 2);
    recipe.rastZeit[2] = RECIPE_REST_DURATION_MAX + 1;
    assertError(recipe, RecipeValidationError::RestDuration, 2);
}

void test_hop_bounds_and_indices(void) {
    Recipe recipe = validRecipe();
    recipe.hopfenanzahl = 0;
    assertError(recipe, RecipeValidationError::HopCount);
    recipe.hopfenanzahl = RECIPE_HOP_COUNT_MAX + 1;
    assertError(recipe, RecipeValidationError::HopCount);

    recipe = validRecipe();
    recipe.hopfenZeit[2] = RECIPE_HOP_TIME_MIN - 1;
    assertError(recipe, RecipeValidationError::HopTime, 2);
    recipe.hopfenZeit[2] = recipe.kochzeit + 1;
    assertError(recipe, RecipeValidationError::HopTime, 2);
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_valid_recipe_and_limits);
    RUN_TEST(test_name_errors);
    RUN_TEST(test_scalar_bounds);
    RUN_TEST(test_rest_bounds_and_indices);
    RUN_TEST(test_hop_bounds_and_indices);
    return UNITY_END();
}
