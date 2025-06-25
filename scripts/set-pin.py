#!/usr/bin/env python3

import os
import sys
import getpass
from argon2 import PasswordHasher

def print_usage():
    print("Usage: set-pin.py [username] <pin>")
    print("  username   Optional. Defaults to current user.")
    print("  pin        Required. A numeric PIN (e.g., 1234).")

def main():
    # Prevent the script from being run as root
    if os.geteuid() == 0:
        print("Error: This script must NOT be run as root.")
        sys.exit(1)

    # Validate usage and arguments
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        print_usage()
        sys.exit(1)

    if len(sys.argv) == 2:
        user = getpass.getuser()  # current user
        pin = sys.argv[1]
    else:
        user = sys.argv[1]
        pin = sys.argv[2]

    # Validate PIN
    if not pin.isdigit() or len(pin) < 4:
        print("Error: PIN must be numeric and at least 4 digits.")
        sys.exit(1)

    # Hash the PIN securely
    ph = PasswordHasher()
    pin_hash = ph.hash(pin)

    # Create target path
    hash_dir = "/etc/security/pin_hashes"
    if not os.path.exists(hash_dir):
        print(f"Warning: password required for creating the following directories '{hash_dir}'.")
        import subprocess
        subprocess.run(["sudo", "mkdir", "-p", hash_dir])
        subprocess.run(["sudo", "chown", "root:root", hash_dir])
        subprocess.run(["sudo", "chmod", "1733", hash_dir])

    hash_path = os.path.join(hash_dir, f"{user}.hash")

    # Write hash to file
    try:
        with open(hash_path, "w") as f:
            f.write(pin_hash)
    except PermissionError:
        print(f"Error: '{hash_dir}' has incorrect permission set.")
        sys.exit(1)

    # Set secure permissions
    os.chmod(hash_path, 0o600)

    print(f"PIN has been set.")

if __name__ == "__main__":
    main()
