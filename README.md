# Minecraft Modloader Installer

A Qt 6 desktop application (C++17) that installs Minecraft modloaders and Vanilla server files.

Supported targets:

| Modloader | Client | Server |
|-----------|:------:|:------:|
| Vanilla   |   –    |   ✓    |
| Fabric    |   ✓    |   ✓    |
| Quilt     |   ✓    |   ✓    |
| Forge     |   ✓    |   ✓    |
| NeoForge  |   ✓    |   ✓    |
| OptiFine  |   ✓    |   –    |

## Features

- GUI workflow for fetching available versions and running installer steps
- Support for Client / Server / Both targets where supported
- Configurable Java executable path and install directory
- Optional Minecraft Launcher profile creation for client installs
- Server startup script generation with optional `nogui`
- Real-time install logs and progress updates

## Requirements

- CMake 3.16+
- C++17 compiler
- Qt 6.2+ (`Core`, `Gui`, `Widgets`, `Network`, `Concurrent`)
- Python 3 with the `requests` package
- Java 17+ (used to run official installer JARs)
- Internet access (version metadata and installer downloads)

Install Python dependency:

```bash
pip install requests
```

## Build

From the repository root:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run:

```bash
./build/MCModloaderInstaller                 # Linux/macOS
build\\Release\\MCModloaderInstaller.exe      # Windows (typical multi-config layout)
```

If Qt is not on your default search path, provide `CMAKE_PREFIX_PATH`:

```bash
cmake -B build \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/<toolchain> \
  -DCMAKE_BUILD_TYPE=Release
```

## Usage

1. Start the application.
2. Select a modloader from the sidebar.
3. Click **Fetch Versions**.
4. Choose version and install target options.
5. Click **Install** and monitor progress.

## How version data is fetched

The app runs `python/requestVer.py`, which writes metadata into a `version/` directory, including:

- `MojangData.json`
- `FabricGame.json`
- `FabricLoader.json`
- `QuiltGame.json`
- `QuiltLoader.json`
- `ForgeMaven.xml`
- `ForgePromotion.json`
- `NeoForgeMaven.xml`
- `OptiFineVersion.json`

`InstallerWorker` then uses the selected version data to download/run the appropriate installer workflow.

## Project structure

```text
.
├── CMakeLists.txt
├── python/
│   ├── requestVer.py
│   └── OptiToken.py
├── resources/
│   └── resources.qrc
└── src/
    ├── main.cpp
    ├── MainWindow.{h,cpp}
    ├── SidebarWidget.{h,cpp}
    ├── InstallerPage.{h,cpp}
    ├── VersionFetcher.{h,cpp}
    ├── InstallerWorker.{h,cpp}
    ├── ProgressDialog.{h,cpp}
    ├── NoguiDialog.{h,cpp}
    ├── VersionParser.{h,cpp}
    └── ModloaderInfo.h
```

## License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPL-3.0)**.
See [`LICENSE`](./LICENSE) for the full text.
