# ChronoWall

Time-based wallpaper manager that changes your desktop background automatically.

## Building and Installing

### Manual Method
```bash
# Install dependencies
sudo apt install build-essential cmake qt6-base-dev   # Debian/Ubuntu
sudo dnf install cmake qt6-qtbase-devel              # Fedora/RHEL
sudo pacman -S cmake qt6-base                        # Arch Linux

# Clone and build
git clone https://github.com/m6rc0sp/chronowall.git
cd chronowall
mkdir build && cd build
cmake ..
make -j$(nproc)

# Install
sudo make install
```

## Initial Setup

1. Install service (first time):
```bash
chronowall --install
```

2. Check service status:
```bash
systemctl --user status chronowall
```

3. View real-time logs:
```bash
journalctl --user -f -u chronowall
```

## Usage

ChronoWall can be run in three ways:

1. Graphical Interface:
```bash
chronowall
```

2. Daemon Mode:
```bash
chronowall --daemon
```

3. Through the applications menu (ChronoWall)

The application will continue running in the system tray.

## Project Structure

```
chronowall/
├── src/
│   ├── core/           # Core logic
│   ├── ui/            # User interface
│   ├── utils/         # Utilities
│   └── main.cpp
├── resources/         # System resources
└── CMakeLists.txt
```

## Packaging and Installation

### Simplified Method (Recommended)
```bash
# Give execution permission to script (first time only)
chmod +x packaging/rpm/build.sh

# Build RPM
packaging/rpm/build.sh

# The RPM will be generated in pasta_to_build/rpmbuild/RPMS/x86_64/
```

### Manual Method

#### Debian/Ubuntu (deb)
```bash
cd packaging/debian
dpkg-buildpackage -b -us -uc
```

#### Fedora/RHEL (rpm)
```bash
# Create rpmbuild structure if needed
mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# Copy spec file
cp packaging/rpm/chronowall.spec ~/rpmbuild/SPECS/

# Create source file
tar czf ~/rpmbuild/SOURCES/chronowall-1.0.0.tar.gz .

# Build RPM
rpmbuild -ba ~/rpmbuild/SPECS/chronowall.spec

# Install
sudo dnf install ~/rpmbuild/RPMS/x86_64/chronowall-1.0.0-1*.rpm
```

#### Arch Linux (PKGBUILD) (Not tested yet)
```bash
# Enter Arch packaging directory
cd packaging/arch

# Build and install
makepkg -si

# Or just build
makepkg -s

# Install manually if built separately
sudo pacman -U chronowall-1.0.0-1-x86_64.pkg.tar.zst
```

## Uninstallation

1. Stop and disable service:
```bash
systemctl --user stop chronowall
systemctl --user disable chronowall
```

2. Remove files:
```bash
sudo make uninstall    # If installed via make
```

3. Clean configurations (optional):
```bash
rm -rf ~/.config/chronowall
rm -rf ~/.cache/chronowall
```

## Contributing

Feel free to open issues or send pull requests!
