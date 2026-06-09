#include "VersionFetcher.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QXmlStreamReader>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>

#ifdef Q_OS_WIN
    #include <windows.h>
#endif

VersionFetcher::VersionFetcher(QObject *parent) : QObject(parent) {}

void VersionFetcher::fetchAll()
{
    if (m_fetching) return;

    // Locate or generate python script
    QString scriptPath = QCoreApplication::applicationDirPath() + "/requestVer.py";
    if (!QFile::exists(scriptPath)) {
        scriptPath = QDir::currentPath() + "/requestVer.py";
        if (!QFile::exists(scriptPath)) {
            // Auto-generate the script
            generatePythonScript(scriptPath);
        }
    }
    if (!QFile::exists(scriptPath)) {
        emit fetchError("Failed to create requestVer.py");
        return;
    }

    m_versionDir = QCoreApplication::applicationDirPath() + "/version";
    if (m_versionDir == QDir::currentPath() + "/version") {
        m_versionDir = QDir::currentPath() + "/version";
    }
    m_fetching = true;

    m_process = new QProcess(this);
    m_process->setWorkingDirectory(QCoreApplication::applicationDirPath());
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    // Hide console window on Windows (more aggressive)
#ifdef Q_OS_WIN
    m_process->setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args) {
        // CREATE_NO_WINDOW = 0x08000000
        // STARTF_USESHOWWINDOW = 0x01
        args->flags |= 0x08000000;  // CREATE_NO_WINDOW
        args->startupInfo->dwFlags |= 0x01;  // STARTF_USESHOWWINDOW
        args->startupInfo->wShowWindow = 0;  // SW_HIDE
    });
#endif

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &VersionFetcher::onProcessOutput);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &VersionFetcher::onProcessFinished);

    emit fetchProgress("Starting version fetch via Python...");

#ifdef Q_OS_WIN
    m_process->start("python", {scriptPath});
#else
    m_process->start("python3", {scriptPath});
#endif
}

void VersionFetcher::parseLocal(const QString &versionDir)
{
    m_versionDir = versionDir;
    parseVanilla(versionDir);
    parseFabric(versionDir);
    parseQuilt(versionDir);
    parseForge(versionDir);
    parseNeoForge(versionDir);
    parseOptiFine(versionDir);
}

void VersionFetcher::onProcessOutput()
{
    QString out = QString::fromUtf8(m_process->readAllStandardOutput()).trimmed();
    if (!out.isEmpty())
        emit fetchProgress(out);
}

void VersionFetcher::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    m_fetching = false;
    m_process->deleteLater();
    m_process = nullptr;

    if (exitCode != 0 || status != QProcess::NormalExit) {
        emit fetchError(QString("Python script exited with code %1").arg(exitCode));
        return;
    }

    emit fetchFinished();
    parseLocal(m_versionDir);
}

// ──────────────────────────── Parsers ─────────────────────────────────────

void VersionFetcher::parseVanilla(const QString &dir)
{
    QFile f(dir + "/MojangData.json");
    if (!f.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    QJsonArray arr = doc.object()["versions"].toArray();

    QList<ModloaderVersion> list;
    for (const QJsonValue &v : arr) {
        QJsonObject o = v.toObject();
        QString type = o["type"].toString();
        // Only release and snapshot
        if (type != "release" && type != "snapshot") continue;

        ModloaderVersion mv;
        mv.mcVersion    = o["id"].toString();
        mv.loaderVersion = "";
        mv.displayName  = mv.mcVersion + (type == "snapshot" ? " (snapshot)" : "");
        mv.isStable     = (type == "release");
        list.append(mv);
    }
    emit versionsReady(ModloaderType::Vanilla, list);
}

void VersionFetcher::parseFabric(const QString &dir)
{
    QFile f(dir + "/FabricMeta.json");
    if (!f.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    QJsonArray arr = doc.array();

    QList<ModloaderVersion> list;
    for (const QJsonValue &v : arr) {
        QJsonObject o = v.toObject();
        ModloaderVersion mv;
        mv.loaderVersion = o["version"].toString();
        mv.mcVersion     = "";   // Fabric loader works across MC versions
        mv.displayName   = mv.loaderVersion;
        mv.isStable      = o["stable"].toBool();
        list.append(mv);
    }
    emit versionsReady(ModloaderType::Fabric, list);
}

void VersionFetcher::parseQuilt(const QString &dir)
{
    QFile f(dir + "/QuiltMeta.json");
    if (!f.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    QJsonArray arr = doc.array();

    QList<ModloaderVersion> list;
    for (const QJsonValue &v : arr) {
        QJsonObject o = v.toObject();
        ModloaderVersion mv;
        mv.loaderVersion = o["version"].toString();
        mv.mcVersion     = "";
        mv.displayName   = mv.loaderVersion;
        mv.isStable      = !mv.loaderVersion.contains("beta") &&
                            !mv.loaderVersion.contains("alpha");
        list.append(mv);
    }
    emit versionsReady(ModloaderType::Quilt, list);
}

// Helper: parse <version> tags from Maven metadata XML
static QStringList parseMavenVersions(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};

    QXmlStreamReader xml(&f);
    QStringList versions;
    bool inVersioning = false;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("versioning")) inVersioning = true;
            if (inVersioning && xml.name() == QLatin1String("version"))
                versions << xml.readElementText();
        }
        if (xml.isEndElement() && xml.name() == QLatin1String("versioning"))
            inVersioning = false;
    }
    return versions;
}

void VersionFetcher::parseForge(const QString &dir)
{
    // Promotion: recommended/latest per MC version
    QFile pf(dir + "/ForgePromotion.json");
    QMap<QString, QString> recommended;
    if (pf.open(QIODevice::ReadOnly)) {
        QJsonObject promos = QJsonDocument::fromJson(pf.readAll())
                                 .object()["promos"].toObject();
        for (auto it = promos.begin(); it != promos.end(); ++it) {
            if (it.key().endsWith("-recommended"))
                recommended[it.key().chopped(13)] = it.value().toString();
        }
    }

    QStringList raw = parseMavenVersions(dir + "/ForgeMaven.xml");
    QList<ModloaderVersion> list;

    for (const QString &ver : raw) {
        // Format: <mc>-<forge>
        int dash = ver.indexOf('-');
        if (dash < 0) continue;
        QString mc    = ver.left(dash);
        QString forge = ver.mid(dash + 1);

        ModloaderVersion mv;
        mv.mcVersion    = mc;
        mv.loaderVersion = forge;
        mv.displayName  = ver;
        mv.isStable     = recommended.value(mc) == forge;
        list.append(mv);
    }

    // Newest first
    std::reverse(list.begin(), list.end());
    emit versionsReady(ModloaderType::Forge, list);
}

void VersionFetcher::parseNeoForge(const QString &dir)
{
    QStringList raw = parseMavenVersions(dir + "/NeoForgeMaven.xml");
    QList<ModloaderVersion> list;

    for (const QString &ver : raw) {
        ModloaderVersion mv;
        // NeoForge versions look like 21.1.x where first two segments = MC 1.21.1
        QStringList parts = ver.split('.');
        if (parts.size() >= 2)
            mv.mcVersion = "1." + parts[0] + "." + parts[1];
        else
            mv.mcVersion = ver;
        mv.loaderVersion = ver;
        mv.displayName   = ver;
        mv.isStable      = !ver.contains("beta") && !ver.contains("alpha");
        list.append(mv);
    }

    std::reverse(list.begin(), list.end());
    emit versionsReady(ModloaderType::NeoForge, list);
}

void VersionFetcher::parseOptiFine(const QString &dir)
{
    QFile f(dir + "/OptiFineVersion.json");
    if (!f.open(QIODevice::ReadOnly)) return;

    QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();
    QJsonArray arr   = root["versions"].toArray();

    QList<ModloaderVersion> list;
    for (const QJsonValue &v : arr) {
        QJsonObject o = v.toObject();
        ModloaderVersion mv;
        mv.mcVersion    = o["minecraft"].toString();
        mv.loaderVersion = o["optifine"].toString();
        mv.displayName  = mv.mcVersion + " - " + mv.loaderVersion;
        mv.isStable     = !mv.loaderVersion.contains("pre");
        list.append(mv);
    }
    emit versionsReady(ModloaderType::OptiFine, list);
}

// ─────────────────────────── Generate Python Script ─────────────────────────

void VersionFetcher::generatePythonScript(const QString &scriptPath)
{
    // Use custom delimiter to avoid syntax issues
    const QString script = R"SCRIPT(#!/usr/bin/env python3
import os
import sys
import requests
import re
import json
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
        filepath = os.path.join(VERSION_DIR, filename)
        print("[{}] Downloading...".format(name), flush=True)
        r = requests.get(url, stream=True, timeout=30)
        r.raise_for_status()
        with open(filepath, "wb") as f:
            for chunk in r.iter_content(chunk_size=8192):
                if chunk:
                    f.write(chunk)
        print("[{}] Complete".format(name), flush=True)
        return True
    except Exception as e:
        print("[{}] Error: {}".format(name, str(e)), flush=True)
        return False

try:
    download("https://piston-meta.mojang.com/mc/game/version_manifest_v2.json", "MojangData.json", "Vanilla")
    download("https://meta.fabricmc.net/v2/versions/loader", "FabricMeta.json", "Fabric")
    download("https://meta.quiltmc.org/v3/versions/loader", "QuiltMeta.json", "Quilt")
    download("https://maven.minecraftforge.net/net/minecraftforge/forge/maven-metadata.xml", "ForgeMaven.xml", "Forge")
    download("https://files.minecraftforge.net/net/minecraftforge/forge/promotions_slim.json", "ForgePromotion.json", "ForgePromo")
    download("https://maven.neoforged.net/releases/net/neoforged/neoforge/maven-metadata.xml", "NeoForgeMaven.xml", "NeoForge")
    
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
            versions.append({"minecraft": parts[0], "optifine": "_".join(parts[1:]), "file": filename, "mirror": "https://optifine.net/adloadx?f=" + filename})
        data = {"generated": datetime.now(UTC).isoformat(), "count": len(versions), "versions": versions}
        with open(os.path.join(VERSION_DIR, "OptiFineVersion.json"), "w", encoding="utf-8") as f:
            json.dump(data, f, indent=4, ensure_ascii=False)
        print("[OptiFine] Complete", flush=True)
    except Exception as e:
        print("[OptiFine] Error: {}".format(str(e)), flush=True)
        with open(os.path.join(VERSION_DIR, "OptiFineVersion.json"), "w") as f:
            json.dump({"versions": []}, f)
    
    print("\nAll version data downloaded", flush=True)
    sys.exit(0)
except Exception as e:
    print("Fatal Error: {}".format(str(e)), flush=True)
    sys.exit(1)
)SCRIPT";

    QFile f(scriptPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit fetchError("Cannot create requestVer.py at: " + scriptPath);
        return;
    }

    f.write(script.toUtf8());
    f.close();

    emit fetchProgress("Auto-generated requestVer.py");
}
