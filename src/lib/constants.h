/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __LIBPAGEMAKER_CONSTANTS_H__
#define __LIBPAGEMAKER_CONSTANTS_H__

#include <stdint.h>

/* PMD record types */
const uint16_t FONTS_PARENT = 0x0100;
const uint16_t PAGE = 0x0500;
const uint16_t PARA = 0x0b00;
const uint16_t TIFF = 0x0e00;
const uint16_t LINE_SET = 0x1100;
const uint16_t FONTS = 0x1300;
const uint16_t COLORS = 0x1500;
const uint16_t GLOBAL_INFO = 0x1800;
const uint16_t SHAPE = 0x1900;
const uint16_t TEXT_BLOCK = 0x1a00;
const uint16_t CHARS = 0x1c00;
const uint16_t XFORM = 0x2800;

/* Color record types */
const uint8_t CMYK = 0x08;
const uint8_t HLS = 0x10;
const uint8_t RGB = 0x18;

/* Shape record types */
const uint8_t TEXT_RECORD = 0x01;
const uint8_t LINE_RECORD = 0x03;
const uint8_t RECTANGLE_RECORD = 0x04;
const uint8_t ELLIPSE_RECORD = 0x05;
const uint8_t BITMAP_RECORD = 0x06;
const uint8_t METAFILE_RECORD = 0x0a;
const uint8_t POLYGON_RECORD = 0x0c;

/* Polygon flags */
const uint8_t REGULAR_POLYGON = 0x00;
const uint8_t POLYGON_OPEN = 0x01;
const uint8_t POLYGON_CLOSED = 0x03;

/* Various constants */
const uint16_t ENDIANNESS_MARKER = 0x99FF;
const uint16_t WARPED_ENDIANNESS_MARKER = 0xFF99;

const uint8_t SHAPE_TYPE_LINE = 1;
const uint8_t SHAPE_TYPE_POLY = 2;
const uint8_t SHAPE_TYPE_RECT = 3;
const uint8_t SHAPE_TYPE_ELLIPSE = 4;
const uint8_t SHAPE_TYPE_TEXTBOX = 5;
const uint8_t SHAPE_TYPE_BITMAP = 6;

/* Fill Constants */
const uint8_t FILL_NONE = 0;
const uint8_t FILL_PAPER = 1;
const uint8_t FILL_SOLID = 2;
const uint8_t FILL_VERTICAL_BARS = 3;
const uint8_t FILL_TIGHT_VERTICAL_BARS = 4;
const uint8_t FILL_HORIZONTAL_BARS = 5;
const uint8_t FILL_TIGHT_HORIZONTAL_BARS = 6;
const uint8_t FILL_TILTED_BARS = 7;
const uint8_t FILL_TIGHT_TILTED_BARS = 8;
const uint8_t FILL_GRID_BARS = 9;
const uint8_t FILL_TIGHT_GRID_BARS = 0x0A;

/* Stroke Constants */
const uint8_t STROKE_NORMAL = 0;
const uint8_t STROKE_LIGHT_LIGHT = 1;
const uint8_t STROKE_DARK_LIGHT = 2;
const uint8_t STROKE_LIGHT_DARK = 3;
const uint8_t STROKE_LIGHT_DARK_LIGHT = 4;
const uint8_t STROKE_DASHED = 5;
const uint8_t STROKE_SQUARE_DOTS = 6;
const uint8_t STROKE_CIRCULAR_DOTS = 7;

#endif /* __LIBPAGEMAKER_CONSTANTS_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
