#!/bin/bash
# create multiresolution windows icon
ICON_SRC=../../src/qt/res/icons/voicecoin.png
ICON_DST=../../src/qt/res/icons/voicecoin.ico
convert ${ICON_SRC} -resize 16x16 voicecoin-16.png
convert ${ICON_SRC} -resize 32x32 voicecoin-32.png
convert ${ICON_SRC} -resize 48x48 voicecoin-48.png
convert voicecoin-16.png voicecoin-32.png voicecoin-48.png ${ICON_DST}

