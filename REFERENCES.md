# Windrose workspace references

This file documents the **Windrose** companion tree: what it contains, which parts are useful for **WindroseSchema** development and validation, and how they relate to mod authoring.

For how WindroseSchema loads mods and hooks the game, see **`ARCHITECTURE.md`**.

---

## Where this tree lives

The Windrose modding workspace is typically linked into this repository as a **junction** (or symlink) at:

**`Windrose/`** → `M:\Code\UE5Games\Windrose`

If your clone uses a different drive or parent folder, recreate the link locally; paths below are described relative to that **`Windrose/`** root.

### Naming: **R5** vs **Windrose**

The game sold as **Windrose** uses **`R5` as the Unreal / on-disk project id** almost everywhere that matters for engine references:

- **`/Script/R5.*`** — core game module types (e.g. **`UR5GameInstance`**, `/Script/R5.R5GameInstance`).
- **Paths under `R5/`** — cooked content layout, paks (`R5\Content\Paks\`), `ExtractedAssets\R5\`, build folders like `5.6.1-0+UE5-R5`, **R5BusinessRules**, etc.
- **Logs and tools** — will say **R5** even when people mean “the Windrose client.”

Separately, WindroseSchema (and the game) may still refer to a **`/Script/Windrose.*`** module for **gameplay types that were authored under that plugin/module name** (e.g. `WindroseUtility`, spawner structs, some enums). So: **“R5” = primary project / shipping shell; “Windrose” in `/Script/Windrose` = another module name, not the marketing title.** When validating dumps, search **both** `R5` and `Windrose` (and `UHTHeaderDump` **module folder names**) depending on what the code path uses.

---

## Top-level layout

| Name | Kind | Role |
|------|------|------|
| `.git` | Directory | Separate git repository for the Windrose workspace (not part of WindroseSchema’s history). |
| `5.6.1-0+UE5-R5/` | Directory | Build-labeled dumps: **CppSDK**, **Mappings**, **Dumpspace** JSON, **GObjects** text dumps, **IDAMappings**, etc. |
| `CXXHeaderDump/` | Directory | ~3k **Dumper-7**-style `.hpp` headers (many Blueprint-related types). |
| `Documentation/` | Directory | **Empty** in the surveyed tree—placeholder only. |
| `ExtractedAssets/` | Directory | Large **FModel**-style extraction: `Engine/`, `R5/`, `scriptobjects.bin` (~90k files). |
| `LUATypesDump/` | Directory | ~2.8k **UE4SS Lua** type stub files (`.lua`). |
| `Mod Store Packages/` | Directory | Packaging layouts (CurseForge, Nexus, Thunderstore). |
| `Mods/` | Directory | Local mod projects (`Reference Mods`, `VapokTweaks`), `build-dist.ps1`. |
| `UHTHeaderDump/` | Directory | **UHT**-style headers organized by engine/game **module** (~35k files under subfolders). |
| `MOD-DEVELOPMENT-SETUP-GUIDE.md` | File | End-to-end modding workflow (IoStore, `~mods`, tools, **usmap**, R5BusinessRules JSON). |
| `MOD-WORKSPACE-CONVENTIONS.md` | File | Conventions for this workspace. |
| `R5-5.6.1-0+UE5-d5e6fda.usmap` | File | **UE property map** for this build (UE4SS / FModel / CUE4Parse). |
| `FModelTMapOverrides.json` | File | **FModel** TMap serialization overrides. |
| `UAssetAPITMapOverrides.json` | File | **UAssetAPI** overrides. |
| `1777908548-ue4ss_static_mesh_data.csv` | File | UE4SS export: static mesh survey data. |
| `1777908560-ue4ss_actor_data.csv` | File | UE4SS export: actor survey data (~large). |
| `UE4SS_ObjectDump.txt` | File | UE4SS **object dump** (~171 MB)—names and paths at scale. |

---

## What is usable (by goal)

### 1. Validating WindroseSchema (classes, paths, enums, hooks)

Use these first when checking whether WindroseSchema still targets the right game types, module paths, and property names.

| Resource | Location | Use |
|----------|----------|-----|
| **UHT headers** | `UHTHeaderDump/<Module>/...` | Ground truth for `UCLASS` / `UPROPERTY` / includes—for example `UHTHeaderDump/R5/Public/R5GameInstance.h` for **`UR5GameInstance`** and **`/Script/R5.R5GameInstance`**. |
| **Dumpspace JSON** | `5.6.1-0+UE5-R5/Dumpspace/` | `ClassesInfo.json`, `StructsInfo.json`, `EnumsInfo.json`, `FunctionsInfo.json`, `OffsetsInfo.json`—programmatic search for types, enums (`EWindrose*`, etc.), functions, and global offsets (`GObjects`, `GWorld`, …). |
| **CppSDK** | `5.6.1-0+UE5-R5/CppSDK/` | Full Dumper-7 C++ SDK (`SDK.hpp`, `SDK/`, …)—layouts and names aligned with the **shipping** build; complementary to WindroseSchema’s own `include/SDK`. |
| **usmap** | `R5-5.6.1-0+UE5-d5e6fda.usmap` and `5.6.1-0+UE5-R5/Mappings/*.usmap` | Required for UE4SS / FModel / CUE4Parse reflection against **IoStore**-cooked content. |
| **UE4SS object dump** | `UE4SS_ObjectDump.txt` | Validate **object paths**, class names, and asset naming at scale (grep / selective reads). |
| **GObjects dumps** | `5.6.1-0+UE5-R5/GObjects-Dump.txt`, `GObjects-Dump-WithProperties.txt` | Deeper object/property listings; heavy—use targeted searches. |

**Note:** Byte **signatures** and **vtable indices** in WindroseSchema still come from the **executable** and reverse-engineering workflow; the dump tree helps you pick the *right symbol* to sign, not automatically the right bytes after a patch.

---

### 2. Authoring WindroseSchema JSON mods (`raw`, `blueprints`, paths)

| Resource | Location | Use |
|----------|----------|-----|
| **Extracted asset tree** | `ExtractedAssets/Engine/`, `ExtractedAssets/R5/`, `scriptobjects.bin` | Discover **cooked** asset paths (e.g. `/Game/...`) for **blueprint** JSON and general content reference. Very large—reference selectively, do not copy wholesale into WindroseSchema. |
| **Mod development guide** | `MOD-DEVELOPMENT-SETUP-GUIDE.md` | IoStore vs **`R5BusinessRules`** raw JSON, `~mods` / pak layout, retoc/repak, FModel, **usmap** version notes, install paths. |
| **Workspace conventions** | `MOD-WORKSPACE-CONVENTIONS.md` | How mods and dumps are organized in this workspace. |
| **FModel / UAssetAPI overrides** | `FModelTMapOverrides.json`, `UAssetAPITMapOverrides.json` | When editing **.uasset** with tools that need TMap fixes for this game. |

---

### 3. Lua / UE4SS scripting (not WindroseSchema C++ itself)

| Resource | Location | Use |
|----------|----------|-----|
| **Lua type dumps** | `LUATypesDump/*.lua` | Hints for **UE4SS Lua** mods and editor support; WindroseSchema does not load these files. |

---

### 4. Surveys and one-off data mining

| Resource | Location | Use |
|----------|----------|-----|
| **CXXHeaderDump** | `CXXHeaderDump/*.hpp` | Overlaps UHT / CppSDK; quick grep for generated class shapes. |
| **UE4SS CSV exports** | `1777908548-ue4ss_static_mesh_data.csv`, `1777908560-ue4ss_actor_data.csv` | Broad **mesh** / **actor** inventories—not primary for data-table schema validation. |

---

### 5. Local mods and distribution

| Resource | Location | Use |
|----------|----------|-----|
| **Reference mods** | `Mods/Reference Mods/` | Example packaged mods (`AlwaysShanties`, `WindroseBuildDegreesOfFreedom`, …). |
| **Active mod projects** | `Mods/VapokTweaks/`, etc. | Your own mod sources. |
| **Build script** | `Mods/build-dist.ps1` | Build/distribution automation for mods. |
| **Store package templates** | `Mod Store Packages/` | CurseForge / Nexus / Thunderstore packaging layouts. |

---

### 6. Reverse engineering (signatures, IDA)

| Resource | Location | Use |
|----------|----------|-----|
| **IDA mappings** | `5.6.1-0+UE5-R5/IDAMappings/` | IDA integration for this build—helps when refreshing **AOB patterns** or vtable layouts in disassembly. |

---

## Empty or non-sources

- **`Documentation/`** — No files in the surveyed tree; not a documentation source until populated.
- **`Mods/.gitkeep`** — Keeps the `Mods` directory in git only.

---

## Quick validation checklist (WindroseSchema ↔ Windrose tree)

1. **Game instance hook** — Compare `UHTHeaderDump/R5/Public/R5GameInstance.h` (and Dumpspace class entry) with WindroseSchema’s use of **`/Script/R5.R5GameInstance`** and the hooked vtable slot (must match **this** executable build).  
2. **Data tables / composites** — Search Dumpspace / UHT for `UDataTable`, composite table types, and `_Common` naming if the game still uses that pattern.  
3. **Enums** — `EnumsInfo.json` and UHT for **`EWindrose*`** names used by **`WindroseEnumLoader`** / schema tooling.  
4. **Blueprint JSON paths** — Cross-check `/Game/...` strings against **`ExtractedAssets`** (or live FModel with the same **usmap**).  
5. **Global offsets** — `OffsetsInfo.json` vs UE4SS / other tools for sanity (not necessarily identical to WindroseSchema’s `UnrealOffsets` path, which may be custom).

---

## External pointers (also in `README.md`)

- [WindroseSchema — Installation](https://okaetsu.github.io/WindroseSchema/docs/installation)  
- [WindroseSchema — Getting started](https://okaetsu.github.io/WindroseSchema/docs/gettingstarted)  
- [UE4SS documentation](https://docs.ue4ss.com/)

---

*This reference reflects the `Windrose/` tree as analyzed for WindroseSchema development. Folder counts and exact file names may drift if the Windrose workspace is refreshed; prefer the live tree when in doubt.*
