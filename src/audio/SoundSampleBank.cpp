#include "SoundSampleBank.hpp"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <curl/curl.h>
#include <zip.h>
#include <openssl/sha.h>
#include <sqlite3.h>
#include <nlohmann/json.hpp>

namespace VR_DAW {

SoundSampleBank::SoundSampleBank() {
    initialize();
}

SoundSampleBank::~SoundSampleBank() {
    shutdown();
}

void SoundSampleBank::initialize() {
    // CURL initialisieren
    curl_global_init(CURL_GLOBAL_ALL);
    
    // Datenbank initialisieren
    sqlite3* db;
    sqlite3_open("samples.db", &db);
    
    // Tabellen erstellen
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS samples (
            name TEXT PRIMARY KEY,
            path TEXT,
            metadata TEXT,
            last_updated TIMESTAMP,
            is_free BOOLEAN,
            download_count INTEGER,
            rating FLOAT
        );
        
        CREATE TABLE IF NOT EXISTS categories (
            name TEXT,
            category TEXT,
            PRIMARY KEY (name, category)
        );
        
        CREATE TABLE IF NOT EXISTS updates (
            id INTEGER PRIMARY KEY,
            timestamp TIMESTAMP,
            status TEXT,
            details TEXT
        );
    )";
    
    char* errMsg = nullptr;
    sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (errMsg) {
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
    
    // Update-Zeitplan initialisieren
    nextUpdateTime = std::chrono::system_clock::now() + std::chrono::hours(24 * 60); // 2 Monate
}

void SoundSampleBank::shutdown() {
    // CURL aufräumen
    curl_global_cleanup();
    
    // Alle Samples speichern
    for (const auto& [name, data] : samples) {
        saveSample(name, data.metadata.source);
    }
}

void SoundSampleBank::addSample(const std::string& path, const SampleMetadata& metadata) {
    // Sample laden
    loadSample(path);
    
    // Metadaten validieren
    validateSampleMetadata(metadata);
    
    // Sample speichern
    SampleData data;
    data.metadata = metadata;
    data.isPlaying = false;
    data.volume = 1.0f;
    data.pan = 0.0f;
    data.pitch = 1.0f;
    data.speed = 1.0f;
    
    samples[metadata.name] = data;
    
    // Kategorie-Index aktualisieren
    for (const auto& category : metadata.tags) {
        updateCategoryIndex(metadata.name, static_cast<SampleCategory>(std::stoi(category)));
    }
    
    // Datenbank aktualisieren
    sqlite3* db;
    sqlite3_open("samples.db", &db);
    
    nlohmann::json metadataJson = {
        {"description", metadata.description},
        {"source", metadata.source},
        {"license", metadata.license},
        {"format", metadata.format},
        {"duration", metadata.duration},
        {"bpm", metadata.bpm},
        {"key", metadata.key},
        {"tags", metadata.tags},
        {"isFree", metadata.isFree},
        {"downloadUrl", metadata.downloadUrl},
        {"previewUrl", metadata.previewUrl},
        {"author", metadata.author},
        {"country", metadata.country},
        {"culture", metadata.culture},
        {"instrument", metadata.instrument},
        {"technique", metadata.technique},
        {"mood", metadata.mood},
        {"genre", metadata.genre},
        {"style", metadata.style},
        {"era", metadata.era},
        {"quality", metadata.quality},
        {"bitDepth", metadata.bitDepth},
        {"sampleRate", metadata.sampleRate},
        {"channels", metadata.channels},
        {"size", metadata.size},
        {"hash", metadata.hash},
        {"version", metadata.version}
    };
    
    const char* sql = "INSERT OR REPLACE INTO samples (name, path, metadata, last_updated, is_free) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    sqlite3_bind_text(stmt, 1, metadata.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, metadataJson.dump().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, std::chrono::system_clock::to_time_t(metadata.lastUpdated));
    sqlite3_bind_int(stmt, 5, metadata.isFree ? 1 : 0);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    // Aktivität protokollieren
    logActivity("Sample hinzugefügt: " + metadata.name);
}

void SoundSampleBank::removeSample(const std::string& name) {
    auto it = samples.find(name);
    if (it != samples.end()) {
        // Backup erstellen
        backupSample(name);
        
        // Aus Kategorie-Index entfernen
        for (const auto& category : it->second.metadata.tags) {
            removeFromCategoryIndex(name, static_cast<SampleCategory>(std::stoi(category)));
        }
        
        // Aus Speicher entfernen
        samples.erase(it);
        
        // Aus Datenbank entfernen
        sqlite3* db;
        sqlite3_open("samples.db", &db);
        
        const char* sql = "DELETE FROM samples WHERE name = ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        sql = "DELETE FROM categories WHERE name = ?;";
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        sqlite3_close(db);
        
        // Aktivität protokollieren
        logActivity("Sample entfernt: " + name);
    }
}

void SoundSampleBank::updateSample(const std::string& name, const SampleMetadata& metadata) {
    auto it = samples.find(name);
    if (it != samples.end()) {
        // Alte Kategorien entfernen
        for (const auto& category : it->second.metadata.tags) {
            removeFromCategoryIndex(name, static_cast<SampleCategory>(std::stoi(category)));
        }
        
        // Metadaten aktualisieren
        it->second.metadata = metadata;
        
        // Neue Kategorien hinzufügen
        for (const auto& category : metadata.tags) {
            updateCategoryIndex(name, static_cast<SampleCategory>(std::stoi(category)));
        }
        
        // Datenbank aktualisieren
        sqlite3* db;
        sqlite3_open("samples.db", &db);
        
        nlohmann::json metadataJson = {
            {"description", metadata.description},
            {"source", metadata.source},
            {"license", metadata.license},
            {"format", metadata.format},
            {"duration", metadata.duration},
            {"bpm", metadata.bpm},
            {"key", metadata.key},
            {"tags", metadata.tags},
            {"isFree", metadata.isFree},
            {"downloadUrl", metadata.downloadUrl},
            {"previewUrl", metadata.previewUrl},
            {"author", metadata.author},
            {"country", metadata.country},
            {"culture", metadata.culture},
            {"instrument", metadata.instrument},
            {"technique", metadata.technique},
            {"mood", metadata.mood},
            {"genre", metadata.genre},
            {"style", metadata.style},
            {"era", metadata.era},
            {"quality", metadata.quality},
            {"bitDepth", metadata.bitDepth},
            {"sampleRate", metadata.sampleRate},
            {"channels", metadata.channels},
            {"size", metadata.size},
            {"hash", metadata.hash},
            {"version", metadata.version}
        };
        
        const char* sql = "UPDATE samples SET metadata = ?, last_updated = ? WHERE name = ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        
        sqlite3_bind_text(stmt, 1, metadataJson.dump().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 2, std::chrono::system_clock::to_time_t(metadata.lastUpdated));
        sqlite3_bind_text(stmt, 3, name.c_str(), -1, SQLITE_STATIC);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        // Aktivität protokollieren
        logActivity("Sample aktualisiert: " + name);
    }
}

std::vector<SampleMetadata> SoundSampleBank::getSamplesByCategory(SampleCategory category) {
    std::vector<SampleMetadata> result;
    
    auto it = categoryIndex.find(category);
    if (it != categoryIndex.end()) {
        for (const auto& name : it->second) {
            auto sampleIt = samples.find(name);
            if (sampleIt != samples.end()) {
                result.push_back(sampleIt->second.metadata);
            }
        }
    }
    
    return result;
}

std::vector<SampleMetadata> SoundSampleBank::searchSamples(const std::string& query) {
    std::vector<SampleMetadata> result;
    
    // Datenbank durchsuchen
    sqlite3* db;
    sqlite3_open("samples.db", &db);
    
    const char* sql = "SELECT metadata FROM samples WHERE metadata LIKE ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    std::string searchPattern = "%" + query + "%";
    sqlite3_bind_text(stmt, 1, searchPattern.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* metadataJson = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        nlohmann::json json = nlohmann::json::parse(metadataJson);
        
        SampleMetadata metadata;
        metadata.description = json["description"];
        metadata.source = json["source"];
        metadata.license = json["license"];
        metadata.format = json["format"];
        metadata.duration = json["duration"];
        metadata.bpm = json["bpm"];
        metadata.key = json["key"];
        metadata.tags = json["tags"].get<std::vector<std::string>>();
        metadata.isFree = json["isFree"];
        metadata.downloadUrl = json["downloadUrl"];
        metadata.previewUrl = json["previewUrl"];
        metadata.author = json["author"];
        metadata.country = json["country"];
        metadata.culture = json["culture"];
        metadata.instrument = json["instrument"];
        metadata.technique = json["technique"];
        metadata.mood = json["mood"];
        metadata.genre = json["genre"];
        metadata.style = json["style"];
        metadata.era = json["era"];
        metadata.quality = json["quality"];
        metadata.bitDepth = json["bitDepth"];
        metadata.sampleRate = json["sampleRate"];
        metadata.channels = json["channels"];
        metadata.size = json["size"];
        metadata.hash = json["hash"];
        metadata.version = json["version"];
        
        result.push_back(metadata);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

void SoundSampleBank::playSample(const std::string& name) {
    auto it = samples.find(name);
    if (it != samples.end()) {
        it->second.isPlaying = true;
        
        // Audio-Engine starten
        juce::AudioDeviceManager deviceManager;
        deviceManager.initialise(2, 2, nullptr, true);
        
        // Sample abspielen
        juce::AudioSourcePlayer player;
        player.setSource(&it->second.buffer);
        deviceManager.addAudioCallback(&player);
        
        // Aktivität protokollieren
        logActivity("Sample abgespielt: " + name);
    }
}

void SoundSampleBank::stopSample(const std::string& name) {
    auto it = samples.find(name);
    if (it != samples.end()) {
        it->second.isPlaying = false;
        
        // Audio-Engine stoppen
        juce::AudioDeviceManager deviceManager;
        deviceManager.removeAudioCallback(nullptr);
        
        // Aktivität protokollieren
        logActivity("Sample gestoppt: " + name);
    }
}

void SoundSampleBank::checkForUpdates() {
    // Update-Zeit prüfen
    auto now = std::chrono::system_clock::now();
    if (now >= nextUpdateTime) {
        // Update durchführen
        updateSamples();
        
        // Nächste Update-Zeit setzen
        nextUpdateTime = now + std::chrono::hours(24 * 60); // 2 Monate
        
        // Update protokollieren
        sqlite3* db;
        sqlite3_open("samples.db", &db);
        
        const char* sql = "INSERT INTO updates (timestamp, status, details) VALUES (?, ?, ?);";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        
        sqlite3_bind_int64(stmt, 1, std::chrono::system_clock::to_time_t(now));
        sqlite3_bind_text(stmt, 2, "completed", -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, "Automatic update", -1, SQLITE_STATIC);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        // Benachrichtigung senden
        notifyUpdateAvailable();
    }
}

void SoundSampleBank::updateSamples() {
    // Kostenlose Sample-Quellen
    std::vector<std::string> sources = {
        "https://freesound.org/api/",
        "https://samplefocus.com/api/",
        "https://splice.com/api/",
        "https://looperman.com/api/",
        "https://sampleswap.org/api/",
        "https://soundbible.com/api/",
        "https://soundcloud.com/api/",
        "https://archive.org/api/",
        "https://ccmixter.org/api/",
        "https://incompetech.com/api/"
    };
    
    // CURL-Handle erstellen
    CURL* curl = curl_easy_init();
    
    for (const auto& source : sources) {
        // API-Anfrage senden
        curl_easy_setopt(curl, CURLOPT_URL, source.c_str());
        
        // Antwort verarbeiten
        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* contents, size_t size, size_t nmemb, std::string* userp) {
            userp->append((char*)contents, size * nmemb);
            return size * nmemb;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            // JSON-Antwort parsen
            nlohmann::json json = nlohmann::json::parse(response);
            
            // Samples verarbeiten
            for (const auto& sample : json["samples"]) {
                // Metadaten extrahieren
                SampleMetadata metadata;
                metadata.name = sample["name"];
                metadata.description = sample["description"];
                metadata.source = source;
                metadata.license = sample["license"];
                metadata.format = sample["format"];
                metadata.duration = sample["duration"];
                metadata.bpm = sample["bpm"];
                metadata.key = sample["key"];
                metadata.tags = sample["tags"].get<std::vector<std::string>>();
                metadata.isFree = sample["is_free"];
                metadata.downloadUrl = sample["download_url"];
                metadata.previewUrl = sample["preview_url"];
                metadata.author = sample["author"];
                metadata.country = sample["country"];
                metadata.culture = sample["culture"];
                metadata.instrument = sample["instrument"];
                metadata.technique = sample["technique"];
                metadata.mood = sample["mood"];
                metadata.genre = sample["genre"];
                metadata.style = sample["style"];
                metadata.era = sample["era"];
                metadata.quality = sample["quality"];
                metadata.bitDepth = sample["bit_depth"];
                metadata.sampleRate = sample["sample_rate"];
                metadata.channels = sample["channels"];
                metadata.size = sample["size"];
                metadata.hash = sample["hash"];
                metadata.version = sample["version"];
                metadata.lastUpdated = std::chrono::system_clock::now();
                
                // Sample herunterladen
                std::string path = "samples/" + metadata.name + "." + metadata.format;
                downloadSample(metadata.downloadUrl, path);
                
                // Sample hinzufügen
                addSample(path, metadata);
            }
        }
    }
    
    // CURL aufräumen
    curl_easy_cleanup(curl);
    
    // Speicher optimieren
    optimizeStorage();
}

void SoundSampleBank::downloadSample(const std::string& url, const std::string& path) {
    // CURL-Handle erstellen
    CURL* curl = curl_easy_init();
    
    // Datei öffnen
    FILE* fp = fopen(path.c_str(), "wb");
    
    // Download-Optionen setzen
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* ptr, size_t size, size_t nmemb, void* stream) {
        FILE* fp = (FILE*)stream;
        return fwrite(ptr, size, nmemb, fp);
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    
    // Download durchführen
    CURLcode res = curl_easy_perform(curl);
    
    // Aufräumen
    fclose(fp);
    curl_easy_cleanup(curl);
    
    // Download validieren
    validateDownload(url);
}

void SoundSampleBank::validateDownload(const std::string& url) {
    // SHA-256-Hash berechnen
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    
    // Datei lesen
    FILE* file = fopen(url.c_str(), "rb");
    if (file) {
        unsigned char buffer[1024];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), file)) != 0) {
            SHA256_Update(&sha256, buffer, bytes);
        }
        fclose(file);
    }
    
    SHA256_Final(hash, &sha256);
    
    // Hash als Hex-String
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    std::string hashStr = ss.str();
    
    // Hash in Metadaten speichern
    auto it = samples.find(url);
    if (it != samples.end()) {
        it->second.metadata.hash = hashStr;
    }
}

void SoundSampleBank::optimizeStorage() {
    // Speicherplatz prüfen
    checkDiskSpace();
    
    // Alte Samples aufräumen
    cleanupOldSamples();
    
    // Samples komprimieren
    for (const auto& [name, data] : samples) {
        compressSample(data.metadata.source, "ogg");
    }
}

void SoundSampleBank::checkDiskSpace() {
    // Verfügbaren Speicherplatz prüfen
    struct statvfs stat;
    if (statvfs(".", &stat) == 0) {
        uint64_t freeSpace = stat.f_bsize * stat.f_bavail;
        uint64_t totalSpace = stat.f_bsize * stat.f_blocks;
        
        // Wenn weniger als 10% frei sind, alte Samples löschen
        if (freeSpace < totalSpace * 0.1) {
            cleanupOldSamples();
        }
    }
}

void SoundSampleBank::cleanupOldSamples() {
    // Samples nach letztem Zugriff sortieren
    std::vector<std::pair<std::string, std::chrono::system_clock::time_point>> accessTimes;
    for (const auto& [name, data] : samples) {
        accessTimes.push_back({name, data.metadata.lastUpdated});
    }
    
    std::sort(accessTimes.begin(), accessTimes.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        });
    
    // Älteste 20% der Samples löschen
    size_t numToDelete = accessTimes.size() * 0.2;
    for (size_t i = 0; i < numToDelete; i++) {
        removeSample(accessTimes[i].first);
    }
}

void SoundSampleBank::logActivity(const std::string& activity) {
    // Aktivität in Log-Datei schreiben
    std::ofstream logFile("sample_bank.log", std::ios::app);
    if (logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        logFile << std::ctime(&time) << ": " << activity << std::endl;
        logFile.close();
    }
}

} // namespace VR_DAW 