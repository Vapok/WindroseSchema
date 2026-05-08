# WindroseSchema
WindroseSchema is a mod that allows modifying of data tables and blueprints in **Windrose** with json files without introducing conflicts with other mods that modify the same game files. As the name suggests, it uses json schema heavily so mod creators can have autocompletion and error checking when authoring table mods.

# Installation

In-depth installation guide can be found [here](https://okaetsu.github.io/WindroseSchema/docs/installation)

# Documentation

Documentation for modders can be found [here](https://okaetsu.github.io/WindroseSchema/docs/gettingstarted)

# Building from Source

1. You must complete the **Build requirements** over at [UE4SS Docs](https://docs.ue4ss.com/#build-requirements) and make sure your GitHub account is linked to Epic Games for Unreal Engine source access.

2. Fork this repository and clone it.

3. Execute this command: `git submodule update --init --recursive`

4. Execute the following:

Choose either MSVC or Ninja

MSVC (multi-configuration, slower, allows switching configs without reconfiguring)
```
cmake -B build -G "Visual Studio 17 2022"
```

or with Ninja (single-configuration, faster)
```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Game__Shipping__Win64
```

# Community mods

Public WindroseSchema mods can be linked here as they appear (this fork drops the Palworld-era Nexus showcase list).
