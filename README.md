# pam-pin

A lightweight Pluggable Authentication Module (PAM) for unix systems that allows users to authenticate using a secure PIN, hashed with Argon2. It includes:
- A PAM module written in C (`pam_pin.c`).
- A Python script to set user PINs (`set-pin.py`).
- An installation script (`install.sh`).

## Features
- [x] PIN-based authentication via PAM.
- [x] Secure hashing with Argon2.
- [x] User PIN management via a simple Python script.
- [x] PINs stored securely under `/etc/security/pin_hashes` with proper permissions.

## Installation
### Prerequisites
- GCC compiler (gcc)
- Python 3 with argon2-cffi installed:
    ```bash
    pip install argon2-cffi
    ``` 
- Argon2 library and headers:
    ```bash
    sudo apt install libargon2-0-dev
    ```

### Install Steps
1. Clone the repository.
2. Run the install script:
    ```bash
    ./install.sh
    ```
    This will:
    - Compile and install the PAM module to `/lib/security/`.
    - Copy the `set-pin.py` script to `~/.local/bin/set-pin.py` (you may want to add `~/.local/bin` to your PATH).

## Usage
### Setting a PIN
Run the set-pin.py script to set a PIN:
```bash
set-pin.py [USERNAME] <PIN>
```
 - Example (sets PIN for current user):
    ```bash
    set-pin.py 1234
    ```
    Or set for another user (requires appropriate permissions):
    ```bash
    set-pin.py ben 5678
    ```
**!! Do NOT run set-pin.py as root.** The script will refuse to run as root to prevent security issues.

### How it Works
- PIN hashes are stored in `/etc/security/pin_hashes/<username>.hash`.
- Files are owned  by root and have permissions `600`.
- The directory `/etc/security/pin_hashes` has permission `1733` (sticky bit set like `/tmp`).

## Configuring PAM
To enable PIN authentication for a service (e.g., `sudo`), edit its PAM configuration file.

Example for `sudo`:
```bash
sudo nano /etc/pam.d/sudo
```
Add the following line **at the top** (before other auth lines):
```
auth required pam_pin.so
```
Now, when using `sudo`, you will be prompted for a PIN.

> You can combine this with password authentication, or replace it entirely depending on the PAM configuration.

## Security Notes
- PINs are hashed using Argon2id, a modern memory-hard hashing algorithm.
- PIN hashes are stored securely with strict permissions.
- The module does not allow PIN entry via command-line arguments; it securely prompts through PAM's conversation interface.
- **Never store plaintext PINs.**

## Uninstallation
To uninstall:
1. Remove the PAM module:
```bash
sudo rm /lib/security/pam_pin.so
```
2. Remove the Python script:
```bash
rm ~/.local/bin/set-pin.py
```
3. Optionally, delete PIN hashes:
```bash
sudo rm -r /etc/security/pin_hashes
```

## Limitations
- This PAM module only supports authentication based on PINs stored locally.
- No PIN retry limits are implemented; rate limiting must be enforced externally (e.g., via PAM stack ordering or tools like `pam_tally`).

## Disclaimer
Use at your own risk. Misconfiguring PAM can result in being locked out of your system. Test configurations carefully and ensure you have alternative access (e.g., SSH with keys or recovery modes).

