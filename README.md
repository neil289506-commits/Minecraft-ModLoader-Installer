# Minecraft Modloader Installer

A Qt 6 C++ desktop application for installing Minecraft modloaders
(Vanilla server, Fabric, Quilt, Forge, NeoForge, OptiFine).

---

## Features

| Modloader | Client | Server |
|-----------|:------:|:------:|
| Vanilla   |   –    |   ✓    |
| Fabric    |   ✓    |   ✓    |
| Quilt     |   ✓    |   ✓    |
| Forge     |   ✓    |   ✓    |
| NeoForge  |   ✓    |   ✓    |
| OptiFine  |   ✓    |   –    |

- Version data fetched via **Python** (`requestVer.py`) — no hard-coded URLs in C++
- Installer runs official installer JARs via Java subprocess
- Real-time progress log with cancel support
- Configurable Java path, install directory, launcher profile creation

---

## Prerequisites

| Tool    | Minimum version |
|---------|-----------------|
| Qt      | 6.2             |
| CMake   | 3.16            |
| C++     | 17              |
| Python  | 3.9+  (`requests` library) |
| Java    | 17+  (for running installer JARs) |

Install Python `requests`:
```bash
pip install requests
```

---

## Build

```bash
# Clone / extract project
cd MCModloaderInstaller

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Run
./build/MCModloaderInstaller        # Linux/macOS
build\Release\MCModloaderInstaller.exe   # Windows
```

### Qt location hint (if Qt not on PATH)

```bash
cmake -B build \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64 \
  -DCMAKE_BUILD_TYPE=Release
```

---

## Project Layout

```
MCModloaderInstaller/
├── CMakeLists.txt
├── python/
│   └── requestVer.py          ← version fetcher (copied next to executable)
├── resources/
│   └── resources.qrc
└── src/
    ├── main.cpp
    ├── MainWindow.{h,cpp}     ← top-level window, wires everything together
    ├── SidebarWidget.{h,cpp}  ← left nav with modloader buttons
    ├── InstallerPage.{h,cpp}  ← right panel: version picker + install options
    ├── ProgressDialog.{h,cpp} ← modal log + progress bar during install
    ├── VersionFetcher.{h,cpp} ← runs requestVer.py, parses JSON/XML output
    ├── InstallerWorker.{h,cpp}← downloads & runs installer JARs
    ├── ModloaderInfo.h        ← enums + data structs (no .cpp needed)
    └── VersionParser.{h,cpp}  ← (reserved for future XML/JSON helpers)
```

---

## How It Works

### Version Fetching
1. User clicks **Fetch Versions**.
2. `VersionFetcher` spawns `python3 requestVer.py` as a `QProcess`.
3. Python downloads 7 data files into `./version/`.
4. After Python exits, C++ parses each file:
   - `MojangData.json`    → Vanilla version list
   - `FabricMeta.json`    → Fabric loader versions
   - `QuiltMeta.json`     → Quilt loader versions
   - `ForgeMaven.xml`     → Forge version list
   - `ForgePromotion.json`→ Forge recommended/latest flags
   - `NeoForgeMaven.xml`  → NeoForge version list
   - `OptiFineVersion.json`→ OptiFine scraped versions

### Installation
Each loader follows a common pattern inside `InstallerWorker`:

1. **Download** the official installer JAR via `QNetworkAccessManager`.
2. **Run** `java -jar <installer>.jar <args>` via `QProcess`.
3. **Emit** progress signals → `ProgressDialog` shows log lines + progress bar.

Vanilla server skips the JAR step: it resolves the direct download URL
from the Mojang version manifest and saves the server `.jar` directly.

---

## Adding Loader Icons

Place 64×64 PNG icons in `resources/icons/` and uncomment the lines
in `resources/resources.qrc`. The icon path in `ModloaderInfo.h` is
already set to `:/icons/<name>.png`.

---

## License

MIT — do whatever you want.
