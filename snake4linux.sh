#!/bin/sh
GAMEDIR="/userdata/roms/ports/snake4linux"
cd "$GAMEDIR"
export LD_LIBRARY_PATH="$GAMEDIR:$LD_LIBRARY_PATH"
export TERM=xterm-256color
/usr/bin/vaixterm --no-credit -w 640 -h 360 -e "$GAMEDIR/snake"
