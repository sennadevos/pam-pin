#!/bin/bash

# Build and install PAM module
mkdir -p build
gcc -fPIC -fno-stack-protector -c pam_src/pam_pin.c -o build/pam_pin.o -I/usr/include/security
gcc -shared -o build/pam_pin.so build/pam_pin.o -largon2
echo "WARNING: sudo rights required to install the pam module."
sudo install -m 755 build/pam_pin.so /lib/security/
echo "Succesfully installed pam module!"

# Install python scripts
echo "Installing python scripts..."
target="$HOME/.local/bin/set-pin.py"

if [ -f "$target" ]; then
  read -p "'$target' already exists. Overwrite? (y/n): " choice
  case "$choice" in
    y|Y )
      echo "Overwriting $target"
      cp scripts/set-pin.py "$target"
      ;;
    * )
      echo "Keeping existing $target"
      ;;
  esac
else
  echo "Installing $target"
  cp scripts/set-pin.py "$target"
fi
echo "Succesfully installed python scripts!"
