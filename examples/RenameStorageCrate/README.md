# Rename "Storage Crate" → "Happy Tree Friend" (WindroseSchema)

This example patches the display name on the small chest building item using the `blueprints/` loader.

---

## Why `blueprints/`, not `raw/`

`raw/` only patches **`UDataTable` rows**. "Storage Crate" is not a data table row — it is the `FText Name` UPROPERTY stored directly on the **`UR5BuildingItem` PrimaryDataAsset** at:

```
/Game/Gameplay/Building/BuildingUtilities/DA_BI_Utilities_Storage_StickBasket
```

The `blueprints/` loader handles `/Game/` asset paths and applies JSON properties directly to the loaded UObject. DataAssets have no Blueprint-generated `_C` class, so the loader uses the asset path as-is.

---

## Install

1. Build / deploy **WindroseSchema** as usual.
2. Copy `WindroseSchemaPack/mods/RenameStorageCrate/` into:

   ```
   …\ue4ss\Mods\WindroseSchema\mods\
   ```

   Result:

   ```
   …\Mods\WindroseSchema\mods\RenameStorageCrate\blueprints\rename_storage_crate.jsonc
   ```

3. Launch the game. In `UE4SS.log` you should see a line like:

   ```
   Applied changes to DA_BI_Utilities_Storage_StickBasket
   ```

---

## JSON shape

```jsonc
{
  "/Game/Gameplay/Building/BuildingUtilities/DA_BI_Utilities_Storage_StickBasket": {
    "Name": "Happy Tree Friend"
  }
}
```

- Top-level key: the `/Game/` asset path, **no `_C` suffix** (DataAssets have none).
- Inner key `"Name"`: matches `FText Name` on `UR5BuildingItem`.
- The value is a plain string; the loader maps it to `FText` automatically via `PropertyHelper`.

---

## How to find the right property for other building items

1. Open **FModel** with the game's IoStore paks and the `.usmap` mapping file.
2. Navigate to the `DA_BI_*` asset you want.
3. The exported JSON will show all UPROPERTY fields. Look for `Name` and `Description`.
4. The asset path in FModel maps directly to the top-level key in the JSON (replace the game content root with `/Game/`).
