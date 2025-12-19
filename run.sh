#!/bin/bash
# Simple launcher for SDL3 program on Wayland

# Path to your executable
EXEC="./ChessGUI"

# Force Wayland driver
export SDL_VIDEODRIVER=wayland

# Optional: if using local SDL3 build
# export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Run the program
$EXEC

