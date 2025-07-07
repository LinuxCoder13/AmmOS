#!/bin/bash

INSTALL_DIR="$HOME/.local/ANG"

mkdir -p "$INSTALL_DIR"
cp -r ./* "$INSTALL_DIR"

echo "Installed to $INSTALL_DIR"
echo "Add $INSTALL_DIR to your PATH or include paths as needed."
