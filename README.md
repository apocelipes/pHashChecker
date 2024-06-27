# pHashChecker

A GUI tool using libpHash to search similar images in your system.

Make sure you have installed Qt 6.5.0+ in your system.

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

To support AVIF images, you need to install `libheif` in your system:

```bash
# Ubuntu
sudo apt install libheif1 libheif-dev

# Arch Linux
sudo pacman -S libheif
```

To support WebP images, you need to install `imagemagick`:

```bash
# Ubuntu
sudo apt install imagemagick

# Arch Linux
sudo pacman -S imagemagick
```

## Screenshots

![mainLayout](screenshots/main_layout.png)

![progressing](screenshots/progressing.png)

![imageViewer](screenshots/image_viewer.gif)

![imageViewer](screenshots/hash_dialog.png)

## TODO

- [ ] unit testing
- [ ] more settings
