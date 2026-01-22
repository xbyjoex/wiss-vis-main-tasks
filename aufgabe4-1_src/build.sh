#!/bin/bash

# Build-Skript für Aufgabe 4.1
# Verwendung: ./build.sh

set -e  # Bei Fehler abbrechen

# Konfiguration - ANPASSEN FALLS NÖTIG
FANTOM_DIR="${FANTOM_DIR:-/Users/jonaspaul/Documents/Uni/Dev/WissenschaftlicheVisualisierung/FAnToM_MacOS26}"
SOURCE_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SOURCE_DIR/build"

echo "=========================================="
echo "Build für Aufgabe 4.1"
echo "=========================================="
echo "FAnToM Verzeichnis: $FANTOM_DIR"
echo "Source Verzeichnis: $SOURCE_DIR"
echo "Build Verzeichnis: $BUILD_DIR"
echo ""

# Prüfen ob FAnToM-Verzeichnis existiert
if [ ! -d "$FANTOM_DIR" ]; then
    echo "FEHLER: FAnToM-Verzeichnis nicht gefunden: $FANTOM_DIR"
    echo "Bitte setzen Sie FANTOM_DIR oder passen Sie das Skript an."
    exit 1
fi

# Prüfen ob share/FAnToM existiert
if [ ! -d "$FANTOM_DIR/share/FAnToM" ]; then
    echo "FEHLER: FAnToM-Installation scheint unvollständig zu sein."
    echo "Erwartet: $FANTOM_DIR/share/FAnToM"
    exit 1
fi

# Build-Verzeichnis erstellen
echo "Erstelle Build-Verzeichnis..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# CMake konfigurieren
echo ""
echo "Konfiguriere CMake..."
FANTOM_DIR="$FANTOM_DIR" cmake -D FANTOM_DIR="$FANTOM_DIR" ../

if [ $? -ne 0 ]; then
    echo "FEHLER: CMake-Konfiguration fehlgeschlagen!"
    exit 1
fi

# Kompilieren
echo ""
echo "Kompiliere Plugin..."
make -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ $? -ne 0 ]; then
    echo "FEHLER: Kompilierung fehlgeschlagen!"
    exit 1
fi

# Installieren
echo ""
echo "Installiere Plugin..."
make install

if [ $? -ne 0 ]; then
    echo "WARNUNG: Installation fehlgeschlagen, aber Build war erfolgreich."
    echo "Sie können FAnToM manuell mit -P $BUILD_DIR/lib/fantom-plugins starten."
else
    echo ""
    echo "=========================================="
    echo "Build erfolgreich abgeschlossen!"
    echo "=========================================="
    echo ""
    echo "Plugin wurde installiert in: $FANTOM_DIR"
    echo ""
    echo "FAnToM starten mit:"
    echo "  cd $BUILD_DIR && make run"
    echo ""
    echo "Oder manuell:"
    echo "  $FANTOM_DIR/bin/fantom -P $BUILD_DIR/lib/fantom-plugins"
    echo ""
fi
