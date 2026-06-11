<<<<<<< HEAD
import os
import requests
import re
import json
from datetime import datetime, UTC

# 建立 version 資料夾
VERSION_DIR = "version"
os.makedirs(VERSION_DIR, exist_ok=True)


def download(url, filename, name):
    filepath = os.path.join(VERSION_DIR, filename)

    r = requests.get(url, stream=True, timeout=30)
    r.raise_for_status()

    with open(filepath, "wb") as f:
        for chunk in r.iter_content(chunk_size=8192):
            if chunk:
                f.write(chunk)

    print(f"{name} 下載完成")


# Vanilla
download(
    "https://piston-meta.mojang.com/mc/game/version_manifest_v2.json",
    "MojangData.json",
    "Vanilla"
)

# Fabric
download(
    "https://meta.fabricmc.net/v2/versions/loader",
    "FabricMeta.json",
    "Fabric"
)

# Quilt
download(
    "https://meta.quiltmc.org/v3/versions/loader",
    "QuiltMeta.json",
    "Quilt"
)

# Forge Maven
download(
    "https://maven.minecraftforge.net/net/minecraftforge/forge/maven-metadata.xml",
    "ForgeMaven.xml",
    "Forge Maven"
)

# Forge Promotion
download(
    "https://files.minecraftforge.net/net/minecraftforge/forge/promotions_slim.json",
    "ForgePromotion.json",
    "Forge Promotion"
)

# NeoForge Maven
download(
    "https://maven.neoforged.net/releases/net/neoforged/neoforge/maven-metadata.xml",
    "NeoForgeMaven.xml",
    "NeoForge"
)

# OptiFine
URL = "https://optifine.net/downloads"

r = requests.get(
    URL,
    headers={
        "User-Agent": "Mozilla/5.0"
    },
    timeout=30
)

r.raise_for_status()

html = r.text

matches = re.findall(
    r'adloadx\?f=(OptiFine_(.*?)\.jar)',
    html
)

versions = []

for filename, version in matches:
    parts = version.split("_")

    if len(parts) < 2:
        continue

    versions.append({
        "minecraft": parts[0],
        "optifine": "_".join(parts[1:]),
        "file": filename,
        "mirror": f"https://optifine.net/adloadx?f={filename}"
    })

data = {
    "generated": datetime.now(UTC).isoformat(),
    "count": len(versions),
    "versions": versions
}

with open(
    os.path.join(VERSION_DIR, "OptiFineVersion.json"),
    "w",
    encoding="utf-8"
) as f:
    json.dump(
        data,
        f,
        indent=4,
        ensure_ascii=False
    )

print("OptiFine 下載完成")
print("\n全部版本資料下載完成")
=======
#!/usr/bin/env python3
import os, sys, json, re, requests
from datetime import datetime
try:
    from datetime import UTC
except ImportError:
    from datetime import timezone
    UTC = timezone.utc

VERSION_DIR = "version"
os.makedirs(VERSION_DIR, exist_ok=True)

def download(url, filename, name):
    try:
        print("[{}] Downloading...".format(name), flush=True)
        r = requests.get(url, stream=True, timeout=30, headers={"User-Agent": "Mozilla/5.0"})
        r.raise_for_status()
        with open(os.path.join(VERSION_DIR, filename), "wb") as f:
            for chunk in r.iter_content(chunk_size=8192):
                if chunk:
                    f.write(chunk)
        print("[{}] Done".format(name), flush=True)
        return True
    except Exception as e:
        print("[{}] Error: {}".format(name, e), flush=True)
        return False

try:
    # Vanilla
    download("https://piston-meta.mojang.com/mc/game/version_manifest_v2.json", "MojangData.json", "Vanilla")

    # Fabric: game versions (which MC versions Fabric supports) + loader versions
    download("https://meta.fabricmc.net/v2/versions/game", "FabricGame.json", "Fabric Game")
    download("https://meta.fabricmc.net/v2/versions/loader", "FabricLoader.json", "Fabric Loader")

    # Quilt: same structure
    download("https://meta.quiltmc.org/v3/versions/game", "QuiltGame.json", "Quilt Game")
    download("https://meta.quiltmc.org/v3/versions/loader", "QuiltLoader.json", "Quilt Loader")

    # Forge
    download("https://maven.minecraftforge.net/net/minecraftforge/forge/maven-metadata.xml", "ForgeMaven.xml", "Forge Maven")
    download("https://files.minecraftforge.net/net/minecraftforge/forge/promotions_slim.json", "ForgePromotion.json", "Forge Promos")

    # NeoForge
    download("https://maven.neoforged.net/releases/net/neoforged/neoforge/maven-metadata.xml", "NeoForgeMaven.xml", "NeoForge")

    # OptiFine
    try:
        print("[OptiFine] Downloading...", flush=True)
        r = requests.get("https://optifine.net/downloads", headers={"User-Agent": "Mozilla/5.0"}, timeout=30)
        r.raise_for_status()
        matches = re.findall(r"adloadx\?f=(OptiFine_(.*?)\.jar)", r.text)
        versions = []
        for filename, version in matches:
            parts = version.split("_")
            if len(parts) < 2:
                continue
            versions.append({
                "minecraft": parts[0],
                "optifine": "_".join(parts[1:]),
                "file": filename,
                "adloadx": "https://optifine.net/adloadx?f=" + filename
            })
        data = {"generated": datetime.now(UTC).isoformat(), "count": len(versions), "versions": versions}
        with open(os.path.join(VERSION_DIR, "OptiFineVersion.json"), "w", encoding="utf-8") as f:
            json.dump(data, f, indent=4, ensure_ascii=False)
        print("[OptiFine] Done ({} versions)".format(len(versions)), flush=True)
    except Exception as e:
        print("[OptiFine] Error: {}".format(e), flush=True)
        with open(os.path.join(VERSION_DIR, "OptiFineVersion.json"), "w") as f:
            json.dump({"versions": []}, f)

    print("\n[Complete] All version data downloaded", flush=True)
    sys.exit(0)
except Exception as e:
    print("[Fatal] {}".format(e), flush=True)
    sys.exit(1)
>>>>>>> f4c88bf (v1.0.0 initial open source release)
