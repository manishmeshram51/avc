#pragma once
#include <stdint.h>

/* PMD record types */
const uint16_t PAGE = 0x0500;
const uint16_t GLOBAL_INFO = 0x1800;
const uint16_t SHAPE = 0x1900;
const uint16_t LINE_SET = 0x1100;
const uint16_t XFORM = 0x2800;

/* Shape record types */
const uint8_t POLYGON_RECORD = 0x0C;
const uint8_t LINE_RECORD = 0x03;
const uint8_t RECTANGLE_RECORD = 0x04;
const uint8_t ELLIPSE_RECORD = 0x05;

/* Polygon flags */
const uint8_t REGULAR_POLYGON = 0x00;
const uint8_t POLYGON_CLOSED = 0x03;
const uint8_t POLYGON_OPEN = 0x01;

/* Various constants */
const uint16_t ENDIANNESS_MARKER = 0x99FF;
const uint16_t WARPED_ENDIANNESS_MARKER = 0xFF99;

const uint8_t SHAPE_TYPE_LINE = 1;
const uint8_t SHAPE_TYPE_POLY = 2;
const uint8_t SHAPE_TYPE_RECT = 3;
const uint8_t SHAPE_TYPE_ELLIPSE = 4;

/* Fill Constants */
const uint8_t FILL_NONE = 0;
const uint8_t FILL_PAPER = 1;
const uint8_t FILL_SOLID = 2;
const uint8_t VERTICAL_BARS = 3;
const uint8_t TIGHT_VERTICAL_BARS = 4;
const uint8_t HORIZONTAL_BARS = 5;
const uint8_t TIGHT_HORIZONTAL_BARS = 6;
const uint8_t TILTED_BARS = 7;
const uint8_t TIGHT_TILTED_BARS = 8;
const uint8_t GRID_BARS = 9;
const uint8_t TIGHT_GRID_BARS = 0x0A;



const uint8_t NONE = 0;
const uint8_t REGISTRATION = 1;
const uint8_t PAPER = 2;
const uint8_t BLACK = 3;
const uint8_t RED = 4;
const uint8_t GREEN = 5;
const uint8_t BLUE = 6;
const uint8_t CYAN = 7;
const uint8_t MAGENTA = 8;
const uint8_t YELLOW = 9;

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
