#!/bin/bash

# Farben für die Ausgabe
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}VR-DAW Build-Skript für macOS${NC}"
echo "====================================="

# Prüfe ob Homebrew installiert ist
if ! command -v brew &> /dev/null; then
    echo -e "${RED}Homebrew ist nicht installiert. Bitte installieren Sie Homebrew zuerst.${NC}"
    echo "Installation: /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
    exit 1
fi

# Installiere Abhängigkeiten
echo -e "${YELLOW}Installiere Abhängigkeiten...${NC}"
brew install cmake
brew install juce
brew install openvr
brew install cuda
brew install opencl
brew install websocketpp
brew install nlohmann-json

# Erstelle Build-Verzeichnis
echo -e "${YELLOW}Erstelle Build-Verzeichnis...${NC}"
mkdir -p build
cd build

# Konfiguriere CMake
echo -e "${YELLOW}Konfiguriere CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Release

# Baue das Projekt
echo -e "${YELLOW}Baue VR-DAW...${NC}"
cmake --build . --config Release

# Erstelle DMG
echo -e "${YELLOW}Erstelle DMG-Installer...${NC}"
cmake --install . --prefix "VR-DAW.app"

# Prüfe ob der Build erfolgreich war
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build erfolgreich!${NC}"
    echo "Die Anwendung finden Sie unter: build/VR-DAW.app"
else
    echo -e "${RED}Build fehlgeschlagen!${NC}"
    exit 1
fi

# Berechtigungen setzen
echo -e "${YELLOW}Setze Berechtigungen...${NC}"
chmod +x "VR-DAW.app/Contents/MacOS/VR-DAW"

echo -e "${GREEN}Fertig!${NC}"
echo "Sie können die Anwendung jetzt starten." 