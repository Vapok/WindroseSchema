# Modify Recipe Output (WindroseSchema)

Patches a `R5BLRecipeData` asset at runtime to change how many items a recipe produces.

---

## Why `blueprints/`, not `raw/`

`R5BLRecipeData` is a `UPrimaryDataAsset` (inheriting from `R5JsonRuntimePDA`), not a `UDataTable`.
It lives under the `/R5BusinessRules/` content mount point and is patched the same way as any other
DataAsset: via the `blueprints/` loader with its full content path as the JSON key.

---

## Finding the recipe asset path

1. Open **FModel** with the game's IoStore paks and your `Mappings.usmap`.
2. Navigate to or search for the recipe. R5BusinessRules recipe assets follow a path pattern like:

   ```
   /R5BusinessRules/Recipes/<Category>/<AssetName>
   ```

3. Note the exact package path — that becomes the top-level key in the JSON.

---

## JSON shape

```jsonc
{
  "/R5BusinessRules/Recipes/Ammo/DA_RD_Ammo_FirearmProjectile_FineMetal_T02": {
    "RecipeResult": [
      {
        "Item": "/R5BusinessRules/InventoryItems/Ammo/DA_AID_Ammo_FirearmProjectile_FineMetal_T02.DA_AID_Ammo_FirearmProjectile_FineMetal_T02",
        "Count": 50
      }
    ]
  }
}
```

| Field | Type | Notes |
|-------|------|-------|
| Key (top-level) | full content path | `/R5BusinessRules/...` — no `_C` suffix, DataAssets have none |
| `RecipeResult` | `TArray<FR5BLRecipeItem>` | Plain array replaces the existing array entirely |
| `RecipeCost` | `TArray<FR5BLRecipeItem>` | Same shape — omit if you don't want to change it |
| `Item` | `TSoftObjectPtr` path string | Must be `Package.ObjectName` format (both segments) |
| `Count` | `int32` | New output quantity |

Only keys you include are modified. Omitting `RecipeCost` leaves ingredient requirements untouched.

---

## Timing

`R5BLRecipeData` assets are not in memory at `GameInstanceInit`. WindroseSchema queues the
patch and drains it once the first `AActor::PostInitializeComponents` fires. In the UE4SS log
you will see:

```
[WindroseSchema] '/R5BusinessRules/...' not in memory at GameInstanceInit, queuing for deferred patch.
[WindroseSchema] Applying deferred patch to DataAsset '...' (class 'R5BLRecipeData')...
[WindroseSchema] Applied deferred changes to DataAsset ...
```

---

## Install

Copy `WindroseSchemaPack/mods/ModifyRecipeOutput/` into:

```
…\ue4ss\Mods\WindroseSchema\mods\
```

Result:

```
…\Mods\WindroseSchema\mods\ModifyRecipeOutput\blueprints\modify_recipe_output.jsonc
```
