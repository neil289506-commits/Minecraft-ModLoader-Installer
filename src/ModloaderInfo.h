#pragma once
#include <QString>
#include <QStringList>

enum class ModloaderType {
    Vanilla, Fabric, Quilt, Forge, NeoForge, OptiFine
};

enum class InstallTarget {
    Client, Server, Both
};

struct ModloaderVersion {
    QString mcVersion;
    QString loaderVersion;
    QString displayName;
    bool    isStable = true;
};

struct InstallConfig {
    ModloaderType    type;
    ModloaderVersion version;
    InstallTarget    target;
    QString          installDir;
    bool             createProfile = true;
    QString          javaPath;
    bool             useNogui = true;   // For server: generate start script with nogui
};

struct ModloaderCapability {
    bool    supportsClient;
    bool    supportsServer;
    QString iconResource;
    QString color;
    QString description;
};

inline ModloaderCapability capabilityOf(ModloaderType t) {
    switch (t) {
    case ModloaderType::Vanilla:  return {false,true, ":/vanilla.png", "#5B8A3C","Official Minecraft server jar"};
    case ModloaderType::Fabric:   return {true, true, ":/fabric.png",  "#DBB689","Lightweight, modern modding toolchain"};
    case ModloaderType::Quilt:    return {true, true, ":/quilt.png",   "#9A77CF","Community-driven Fabric fork"};
    case ModloaderType::Forge:    return {true, true, ":/forge.png",   "#E07433","The classic Minecraft modding platform"};
    case ModloaderType::NeoForge: return {true, true, ":/neoforge.png","#F4A623","Modern Forge continuation"};
    case ModloaderType::OptiFine: return {true, false,":/optifine.png","#4FC3F7","Performance & graphics enhancement"};
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
