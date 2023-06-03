#!/bin/bash

# Funktion zum Starten eines Terminals und Ausführen eines Befehls
start_terminal() {
gnome-terminal --window -- "$1" 
}

# Befehle ausführen
make clear
make -j 1

# Terminals starten und Befehle ausführen
start_terminal "./bin/server"
start_terminal "./bin/client"
start_terminal "./bin/client"
start_terminal "./bin/client"

# Funktion zum Schließen aller Terminals und Prozesse
close_terminals() {
  pkill -f gnome-terminal 
}

# Schließen aller Terminals und Prozesse bei Bedarf
trap close_terminals EXIT

# Endlosschleife, um das Skript am Laufen zu halten
while true; do
  sleep 1
done

