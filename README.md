# pHashChecker

A GUI tool using libpHash to search similar images in your system.

Make sure you have installed Qt 6.2.0+ in your system.

To support AVIF images, you need to install `libheif` in your system:

```bash
# Ubuntu
sudo apt install libheif1

# Arch Linux
sudo pacman -S libheif
```

## Install

```bash
# building pHashChecker
git clone --recurse-submodules https://github.com/apocelipes/pHashChecker
cd pHashChecker
mkdir build && cd build
cmake ..
make

# running
./pHashChecker
```

## Screenshots

![mainLayout](screenshots/main_layout.png)

![progressing](screenshots/progressing.png)

![imageViewer](screenshots/image_viewer.gif)

![imageViewer](screenshots/hash_dialog.png)

## TODO

- [ ] unit testing
- [ ] more settings
- [ ] more animated effects
