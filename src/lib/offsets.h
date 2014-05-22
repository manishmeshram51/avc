#pragma once

const unsigned TABLE_OF_CONTENTS_OFFSET_OFFSET = 0x30;
const unsigned TABLE_OF_CONTENTS_LENGTH_OFFSET = 0x2E;
const unsigned ENDIANNESS_MARKER_OFFSET = 0x06;

const unsigned PAGE_HEIGHT_OFFSET = 0x3C; /* In the GLOBAL_INFO record. */
const unsigned PAGE_WIDTH_OFFSET = 0x08;  /* in the first PAGE record. */
const unsigned PAGE_SHAPE_SEQNUM_OFFSET = 0x02;  /* in the first PAGE record. */

const unsigned LINE_MIRRORED_OFFSET = 0x26;
const unsigned RECT_TOP_LEFT_OFFSET = 0x06;
const unsigned RECT_BOT_RIGHT_OFFSET = 0x0A;

const unsigned POLYGON_LINE_SEQNUM_OFFSET = 0x2E;
const unsigned POLYGON_CLOSED_MARKER_OFFSET = 0x38;

const unsigned SHAPE_TYPE_OFFSET = 0;
const unsigned LEFT_PAGE_RIGHT_BOUND_OFFSET = 0x3A;

const unsigned RECT_XFORM_ID_OFFSET = 0x1C; /* In the SHAPE record. */
const unsigned XFORM_ID_OFFSET = 0x16; /* In the XForm record. */
const unsigned XFORM_RECT_ROTATION_OFFSET = 0;
const unsigned XFORM_ROTATING_POINT_OFFSET = 0x12;

const unsigned XFORM_LENGTH_ONE_OFFSET = 0x0A;
const unsigned XFORM_LENGTH_TWO_OFFSET = 0x0E;
const unsigned XFORM_BREADTH_ONE_OFFSET = 0x0C;
const unsigned XFORM_BREADTH_TWO_OFFSET = 0x10;
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
