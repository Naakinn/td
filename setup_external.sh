#!/bin/bash

TARGET=sqlite-amalgamation-3490100
TARGET_ZIP=$TARGET.zip

echo "External setup: downloading sqlite3 source code..."
wget https://www.sqlite.org/2025/$TARGET_ZIP && echo "Downloaded successfully" || { echo "Error while downloading sqlite source code"; exit 1; }

echo $TARGET_ZIP
echo $TARGET

echo "Unpacking source code..."
unzip ./$TARGET_ZIP && echo "Unpacked successfully" || { echo "Error while unpacking sqlite tar.gz file"; exit 1; }
rm ./$TARGET_ZIP

if ! [[ -d ./src || ./include ]]; then
	echo "No such directories to move source code to"
	echo "Check existence of './src' or './include' directory"
	exit 1
fi

echo "Moving files..."
mv ./$TARGET/sqlite3.c ./src
mv ./$TARGET/sqlite3.h ./include

rm -r ./$TARGET
