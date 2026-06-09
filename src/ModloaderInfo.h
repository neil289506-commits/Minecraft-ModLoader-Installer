#pragma once
#include <QString>
#include <QStringList>

// Supported modloader types
enum class ModloaderType {
    Vanilla,
    Fabric,
    Quilt,
    Forge,
    NeoForge,
    OptiFine
};

// Installation target
enum class InstallTarget {
    Client,
    Server,
    Both
};

struct ModloaderVersion {
    QString mcVersion;       // Minecraft version (e.g. "1.21.1")
    QString loaderVersion;   // Loader version (e.g. "0.16.9")
    QString displayName;     // Human-readable name
    bool    isStable;        // Whether this is a stable/recommended build
};

struct InstallConfig {
    ModloaderType   type;
    ModloaderVersion version;
    InstallTarget   target;
    QString         installDir;   // Root install directory (e.g. .minecraft or server folder)
    bool            createProfile; // Create launcher profile (client only)
    QString         javaPath;
};

// Per-loader capability flags
struct ModloaderCapability {
    bool supportsClient;
    bool supportsServer;
    QString iconResource;  // :/icons/<name>.png
    QString color;         // Accent color hex
    QString description;
};

inline ModloaderCapability capabilityOf(ModloaderType t) {
    switch (t) {
    case ModloaderType::Vanilla:
        return { false, true,  ":/icons/vanilla.png",  "#5B8A3C", "Official Minecraft server jar" };
    case ModloaderType::Fabric:
        return { true,  true,  ":/icons/fabric.png",   "#DBB689", "Lightweight, modern modding toolchain" };
    case ModloaderType::Quilt:
        return { true,  true,  ":/icons/quilt.png",    "#9A77CF", "Community-driven Fabric fork" };
    case ModloaderType::Forge:
        return { true,  true,  ":/icons/forge.png",    "#E07433", "The classic Minecraft modding platform" };
    case ModloaderType::NeoForge:
        return { true,  true,  ":/icons/neoforge.png", "#F4A623", "Modern Forge continuation" };
    case ModloaderType::OptiFine:
        return { true,  false, ":/icons/optifine.png", "#4FC3F7", "Performance & graphics enhancement" };
    }
    return {};
}

inline QString modloaderName(ModloaderType t) {
    switch (t) {
    case ModloaderType::Vanilla:  return "Vanilla";
    case ModloaderType::Fabric:   return "Fabric";
    case ModloaderType::Quilt:    return "Quilt";
    case ModloaderType::Forge:    return "Forge";
    case ModloaderType::NeoForge: return "NeoForge";
    case ModloaderType::OptiFine: return "OptiFine";
    }
    return "Unknown";
}
