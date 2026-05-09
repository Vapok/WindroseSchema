# WindroseSchema — Runtime property access patterns and pitfalls

This document captures lessons learned from implementing `WindroseRecipeModLoader` and debugging a series of crashes when reading `UObject` property data at runtime. Future agents and contributors should read this before writing any new property-reading code.

The game is **Windrose / R5** running on **UE5 (5.6.1)**. UE4SS RE-UE4SS wraps the game's reflection objects, but its C++ definitions do not always match the game's binary layout exactly. That gap is the source of most crashes documented here.

---

## 1. The broken `GetNextField` — do not walk property linked lists

### What it is

`PropertyHelper::GetNextField(FField* field)` (in `src/SDK/Helper/PropertyHelper.cpp`) advances a property iterator by reading `FField::Next` from a hardcoded byte offset:

```cpp
// PropertyHelper.cpp — current implementation
auto Next = *Helper::Casting::ptr_cast<FField**>(Field, 0x20);
```

### Why it crashes

In this game build, `FField::Next` is at offset **`0x18`**, not `0x20`. The UE5 `FField` layout is:

| Offset | Size | Field |
|--------|------|-------|
| `0x00` | 8 | `FFieldClass* ClassPrivate` |
| `0x08` | 8 | `FFieldVariant Owner` (tagged pointer, low bit = is-UObject) |
| `0x10` | 8 | (Owner padding / 2nd Owner word — build-specific) |
| `0x18` | 8 | **`FField* Next`** ← correct offset |
| `0x20` | 8 | `FName NamePrivate` ← what `GetNextField` reads instead |

Reading `NamePrivate` (a small FName index like `3114766`) as a pointer and then dereferencing it causes an access violation crash.

The bug in `GetNextField` was confirmed by dumping 64 bytes of a known `FField*`:

```
FField[Item] u64:
  [0]=0x7FF7...  ClassPrivate (FFieldClass*, points to executable)
  [1]=0x7FF7...  Owner pointer
  [2]=0x7FF6...  Owner (2nd word, tagged)
  [3]=0x1FC4...  ← REAL Next (heap address, points to "Count" FField)
  [4]=3114766    ← NamePrivate.ComparisonIndex (this is what GetNextField reads as Next!)
  [5]=69         ← NamePrivate.Number / FlagsPrivate
  ...
```

### The fix: never walk the property list; use `GetPropertyByName` instead

`PropertyHelper::GetPropertyByName` has overloads for both `UClass*` and `UScriptStruct*`:

```cpp
// PropertyHelper.h
FProperty* GetPropertyByName(RC::Unreal::UClass* Class, const RC::StringType& PropertyName);
FProperty* GetPropertyByName(RC::Unreal::UScriptStruct* Struct, const RC::StringType& PropertyName);
```

When you know the field names (which you do in any non-generic code), look them up directly:

```cpp
// CORRECT — no iteration, no GetNextField
auto* countProp = CastField<FIntProperty>(
    PropertyHelper::GetPropertyByName(itemStruct, TEXT("Count")));
auto* softProp  = CastField<FSoftObjectProperty>(
    PropertyHelper::GetPropertyByName(itemStruct, TEXT("Item")));
```

This replaces an entire `for (FField* f = struct->GetChildProperties(); f; f = GetNextField(f))` loop with two direct lookups. It is faster, simpler, and does not rely on the broken offset.

**Only walk the property linked list** (via `GetChildProperties` + `GetNextField`) when you genuinely cannot know the field name in advance. If you must, be aware the offset bug exists — any such loop may produce garbage after the first field.

---

## 2. Do not call `ExportTextItem_InContainer` on property values

### What it is

`FProperty::ExportTextItem_InContainer` (defined in RE-UE4SS `UnrealType.hpp`) looks like a clean way to convert a property value to a string without knowing its binary layout:

```cpp
RC::Unreal::FString exportedPath;
prop->ExportTextItem_InContainer(exportedPath, elemPtr, nullptr, nullptr, 0);
```

### Why it crashes

This method is an inline helper that ultimately calls `ExportTextItem`, which **is virtual**. The virtual dispatch goes through the game's vtable for `FSoftObjectProperty`. If the vtable index that UE4SS declares for `ExportTextItem` differs from the game's actual vtable (due to game-specific overrides, compiler differences, or UE version drift), the call lands in the wrong game function → crash.

This was observed crashing at the exact same log point as the manual byte-read approach, confirming it is not a safe substitute.

**Rule:** Avoid calling any method on a game `FProperty*` that resolves through a virtual dispatch unless you have verified the vtable index matches. Prefer non-virtual helpers (`ContainerPtrToValuePtr`, `GetElementSize`, `GetInner`, `GetStruct`, `GetPropertyByName`) which read well-known data-member offsets.

---

## 3. Reading `TSoftObjectProperty` values — verified memory layout

When you have an `FSoftObjectProperty*` and need the asset path string, use `ContainerPtrToValuePtr<void>` to get the raw value pointer, then read the `FName` fields directly via `memcpy`.

### Layout (CppSDK-verified for this game)

The property stores a `TSoftObjectPtr<T>` whose binary layout is **40 bytes (0x28)**:

```
Offset  Size  Field
0x00    8     FWeakObjectPtr  {int32 ObjectIndex, int32 ObjectSerialNumber}
0x08    8     FSoftObjectPath::AssetPath::PackageName  (FName)
0x10    8     FSoftObjectPath::AssetPath::AssetName    (FName)
0x18    16    FSoftObjectPath::SubPathString            (FUtf8String, rarely set)
```

> Source: `M:/Code/UE5Games/Windrose/5.6.1-0+UE5-R5/CppSDK/SDK/Basic.hpp`
> `TPersistentObjectPtr`: `FWeakObjectPtr WeakPtr` at `0x00`; `FSoftObjectPath ObjectID` at `0x08`.
> `FSoftObjectPath`: `FTopLevelAssetPath AssetPath` at `0x00`; `FUtf8String SubPathString` at `0x10`.
> `FTopLevelAssetPath`: `FName PackageName` at `0x00`; `FName AssetName` at `0x08`.

Note: there is **no `TagAtLastTest` field** in `TPersistentObjectPtr` in this game build. Earlier UE4/UE4SS versions included it, inflating `sizeof` to 48. The game's actual size is 40.

### Reading code

```cpp
const uint8* base = static_cast<const uint8*>(
    softProp->ContainerPtrToValuePtr<void>(elemPtr));
if (base)
{
    FName pkgName, astName;
    memcpy(&pkgName, base + 0x08, sizeof(FName));  // PackageName
    memcpy(&astName, base + 0x10, sizeof(FName));  // AssetName
    if (astName != NAME_None)
    {
        itemPath = RC::to_string(std::format(TEXT("{}.{}"),
            pkgName.ToString(), astName.ToString()));
    }
}
```

`FName::ToString()` is safe — it does a global name table lookup using `ComparisonIndex`, which is a simple integer read from GNames.
`FName::operator!=` / `NAME_None` comparison is safe — pure integer comparison, no vtable.

---

## 4. Safe vs. unsafe operations on game reflection objects

### Safe (non-virtual, reads known data-member offsets)

| Operation | Notes |
|-----------|-------|
| `FProperty::ContainerPtrToValuePtr<T>(container)` | Adds `Offset_Internal` to the container pointer. Non-virtual. |
| `FArrayProperty::GetInner()` | Returns `FProperty* Inner`. Non-virtual field access. |
| `FStructProperty::GetStruct()` | Returns `UScriptStruct*`. Non-virtual field access. |
| `FProperty::GetElementSize()` | Returns `ElementSize`. Non-virtual field access. |
| `UStruct::GetChildProperties()` | Returns `FField* ChildProperties`. Non-virtual, but result is only safe to use as the **first** field (do not iterate via GetNextField). |
| `FScriptArray::Num()` | Reads `ArrayNum` at fixed offset. Safe. |
| `FScriptArray::GetData()` | Returns `Data` pointer at offset 0. Safe. |
| `PropertyHelper::GetPropertyByName(UClass*, name)` | Walks the property linked list internally, but this function is tested and works. Safe for top-level object properties. |
| `PropertyHelper::GetPropertyByName(UScriptStruct*, name)` | Same, on a nested struct. Safe. |
| `FName::ToString()` | GNames table lookup by index. Safe when the index comes from a known-valid FName field. |
| `memcpy` from a valid `ContainerPtrToValuePtr` result | Safe for POD reads (FName, int32, etc.). |

### Unsafe / crash-prone

| Operation | Why |
|-----------|-----|
| `PropertyHelper::GetNextField(field)` | Reads `FField::Next` at offset `0x20`; actual offset is `0x18`. Returns garbage pointer → next `GetPropertyNameAsUTF8String` call crashes. |
| Any virtual call through `FProperty*` (`ExportTextItem`, `ExportText_InContainer`, etc.) | Vtable index may not match game binary. Observed crash with `ExportTextItem_InContainer` on `FSoftObjectProperty`. |
| `FProperty::GetValue_InContainer<TSoftObjectPtr<T>>(...)` | Our `UECustom::TSoftObjectPtr` size (48 bytes with `TagAtLastTest`) differs from game (40 bytes). Typed read copies wrong number of bytes. |

---

## 5. Debugging approach that works

When a crash happens immediately after a single log line with no further output, the following procedure pinpointed the cause in this codebase:

1. **Log before and after every individual operation** in the suspect function — not just per-loop but per-statement. Last visible log = last successful line.
2. **Dump raw `uint64_t` words** from suspicious pointer addresses to inspect actual memory layout without calling any methods.
3. **Identify valid heap pointers** in the dump (addresses in the `0x1FC...` / `0x269B...` range in this process) vs. garbage (low values like `0x2F8D10`).
4. **Verify FName indices** by logging raw `uint32` values — valid GNames indices are moderate positive integers (e.g. 5283062). If you see values like `0` or very large numbers, the offset is wrong.
5. Only call `FName::ToString()` after confirming the index looks valid from the raw dump.

---

## 6. Example: ReadItemArray — the final correct pattern

`WindroseRecipeModLoader::ReadItemArray` in `src/Loader/WindroseRecipeModLoader.cpp` is the reference implementation of all of the above:

```cpp
// Get the array property on the outer object
auto* arrayProp = CastField<FArrayProperty>(
    PropertyHelper::GetPropertyByName(objClass, propName));

// Get the element struct type
auto* innerProp = CastField<FStructProperty>(arrayProp->GetInner());
auto  itemStruct = innerProp->GetStruct();

// Look up fields by name — NO property linked-list iteration
auto* countProp = CastField<FIntProperty>(
    PropertyHelper::GetPropertyByName(itemStruct, TEXT("Count")));
auto* softProp  = CastField<FSoftObjectProperty>(
    PropertyHelper::GetPropertyByName(itemStruct, TEXT("Item")));

// Iterate array elements using raw pointer arithmetic (elemSize from GetElementSize)
uint8* dataBase = static_cast<uint8*>(arrayPtr->GetData());
for (int32 i = 0; i < arrayPtr->Num(); ++i)
{
    uint8* elemPtr = dataBase + i * elemSize;

    // Count: ContainerPtrToValuePtr directly
    auto* countPtr = countProp->ContainerPtrToValuePtr<int32>(elemPtr);

    // Soft object: memcpy at verified offsets from CppSDK
    const uint8* base = static_cast<const uint8*>(
        softProp->ContainerPtrToValuePtr<void>(elemPtr));
    FName pkgName, astName;
    memcpy(&pkgName, base + 0x08, sizeof(FName));
    memcpy(&astName, base + 0x10, sizeof(FName));
}
```

---

*Written after `WindroseRecipeModLoader` implementation — May 2026. Covers UE5 build `5.6.1-0+UE5-R5` with UE4SS RE-UE4SS.*
