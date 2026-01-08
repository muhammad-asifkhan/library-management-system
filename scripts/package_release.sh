#!/bin/bash
RELEASE_DIR="release_$(date +'%Y%m%d')"
mkdir -p $RELEASE_DIR
cp -r build/library_console build/library_gui data README.md CMakeLists.txt $RELEASE_DIR/
zip -r $RELEASE_DIR.zip $RELEASE_DIR
rm -rf $RELEASE_DIR
echo "Release package created: $RELEASE_DIR.zip"

