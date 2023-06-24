#!/bin/bash

# Funktion zum Starten eines Terminals und Ausführen eines Befehls
start_terminal() {
  gnome-terminal -- bash -c "$1; exec bash"
}

# Befehle ausführen
cd build/
make clean
sleep 1
make -j 1

# Terminals starten und Befehle ausführen
cd bin/
start_terminal "./server" &
sleep 1
start_terminal "./client '::1' '7777'" &
start_terminal "./client '127.0.0.1' '7777'" &
#start_terminal "./client" &

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

