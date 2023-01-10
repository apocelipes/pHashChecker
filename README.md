# pHashChecker

A GUI tool using libpHash to search similar images in your system.

Make sure you have installed Qt 6.2.0+ in your system.

## Install

```bash
# building pHashChecker
cd
git clone --recurse-submodules https://github.com/apocelipes/pHashChecker
mkdir build && cd build
cmake ..
make

# running
./pHashChecker
```

## Screenshots

![main_layout](screenshots/main_layout.png)

![progressing](screenshots/progressing.png)

![image_viewer](screenshots/image_viewer.gif)

![image_viewer](screenshots/hash_dialog.png)

## TODO

- [ ] unit testing
- [ ] more settings
- [ ] more animated effects
