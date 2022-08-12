//
// Created by cth451 on 2020/05/15.
//

#ifndef MINEMAP_CONSTANTS_H
#define MINEMAP_CONSTANTS_H

#define INVALID_GAME_VER "Invalid Minecraft Version Specifier"

#define MISSING_GAME_VER "No game version or invalid version specified"
#define MISSING_IN_FILE  "No input file specified"
#define MISSING_OUT_FILE "No output file specified"
#define MISSING_MARKER "Missing marker type / operation"

#define INVALID_ARGUMENT "Invalid argument passed: {}"
#define INVALID_POSITION "Invalid position string"
#define INVALID_BANNER "Invalid banner specifier"
#define POSITION_OUT_OF_RANGE "Coordinates out of range"

#define MAP_NOT_COMPOUND "The map data tag is not a compound but a {}. Is this a map nbt file?"
#define COLORS_NOT_BYTES "Color data is not a byte array. Is this a map nbt file?"
#define BANNERS_MISSING "The map does not supports banners - update to a newer Minecraft version and try again"
#define BANNERS_MALFORMED "Banners in this map is malformed"
#define POSITION_MALFORMED "Position coordinates in this marker is malformed"
#define COLOR_MISMATCH "BUG: No color match for pixel at ({}, {}). Please submit a bug report."
#define COLOR_PARTIALLY_TRANSPARENT "Warning: {} partially transparent pixels are rendered as fully transparent!"
#define COLOR_OUT_OF_RANGE "Color code {} at offset {} is out of range!"

#ifdef USE_GM
#define MINEMAP_MAGICK_IMPLEMENTATION "GraphicsMagick"
#else
#define MINEMAP_MAGICK_IMPLEMENTATION "ImageMagick"
#endif
#define VERSION_MESSAGE "Application version " MINEMAP_APP_VER " with " MINEMAP_MAGICK_IMPLEMENTATION

#endif //CONSTANTS_H
