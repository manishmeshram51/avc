/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libpagemaker project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __LIBPAGEMAKER_OFFSETS_H__
#define __LIBPAGEMAKER_OFFSETS_H__

const unsigned TABLE_OF_CONTENTS_OFFSET_OFFSET = 0x30;
const unsigned TABLE_OF_CONTENTS_LENGTH_OFFSET = 0x2E;
const unsigned ENDIANNESS_MARKER_OFFSET = 0x06;

const unsigned PAGE_HEIGHT_OFFSET = 0x3C; /* In the GLOBAL_INFO record. */
const unsigned PAGE_WIDTH_OFFSET = 0x08;  /* in the first PAGE record. */
const unsigned PAGE_SHAPE_SEQNUM_OFFSET = 0x02;  /* in the first PAGE record. */

const unsigned LINE_MIRRORED_OFFSET = 0x26;
const unsigned SHAPE_TOP_LEFT_OFFSET = 0x06;
const unsigned SHAPE_BOT_RIGHT_OFFSET = 0x0A;

const unsigned POLYGON_LINE_SEQNUM_OFFSET = 0x2E;
const unsigned POLYGON_CLOSED_MARKER_OFFSET = 0x38;

const unsigned SHAPE_TYPE_OFFSET = 0;
const unsigned LEFT_PAGE_RIGHT_BOUND_OFFSET = 0x3A;

const unsigned SHAPE_XFORM_ID_OFFSET = 0x1C; /* In the SHAPE record. */
const unsigned XFORM_ID_OFFSET = 0x16; /* In the XForm record. */
const unsigned XFORM_RECT_ROTATION_OFFSET = 0;
const unsigned XFORM_SKEW_OFFSET = 0x04;
const unsigned XFORM_ROTATING_POINT_OFFSET = 0x12;

const unsigned XFORM_TOP_LEFT_OFFSET = 0x0A;
const unsigned XFORM_BOT_RIGHT_OFFSET = 0x0E;

const unsigned SHAPE_FILL_COLOR_OFFSET = 0x04;
const unsigned SHAPE_FILL_TYPE_OFFSET = 0x26;
const unsigned SHAPE_FILL_OVERPRINT_OFFSET = 0x02;
const unsigned SHAPE_FILL_TINT_OFFSET = 0xe0;

const unsigned SHAPE_STROKE_COLOR_OFFSET = 0x28;
const unsigned SHAPE_STROKE_TYPE_OFFSET = 0x20;
const unsigned SHAPE_STROKE_OVERPRINT_OFFSET = 0x2a;
const unsigned SHAPE_STROKE_TINT_OFFSET = 0x2c;
const unsigned SHAPE_STROKE_WIDTH_OFFSET = 0x23;

const unsigned TEXT_BLOCK_OFFSET = 0x02;
const unsigned SHAPE_TEXT_BLOCK_ID_OFFSET = 0x20; /* In the SHAPE record. */
const unsigned TEXT_BLOCK_ID_OFFSET = 0x20; /* In the TEXT BLOCK record. */
const unsigned TEXT_BLOCK_TEXT_PROPS_ONE_OFFSET = 0x00;
const unsigned TEXT_BLOCK_TEXT_PROPS_TWO_OFFSET = 0x02;
const unsigned TEXT_BLOCK_TEXT_OFFSET = 0x04;
const unsigned TEXT_BLOCK_CHARS_OFFSET = 0x06;
const unsigned TEXT_BLOCK_PARA_OFFSET = 0x08;
const unsigned TEXT_BLOCK_TEXT_STYLE_OFFSET = 0x0A;

#endif /* __LIBPAGEMAKER_OFFSETS_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
