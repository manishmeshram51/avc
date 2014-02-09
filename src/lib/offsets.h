#pragma once

const unsigned TABLE_OF_CONTENTS_OFFSET_OFFSET = 0x30;
const unsigned TABLE_OF_CONTENTS_LENGTH_OFFSET = 0x2E;
const unsigned ENDIANNESS_MARKER_OFFSET = 0x06;

const unsigned PAGE_HEIGHT_OFFSET = 0x3C; /* In the GLOBAL_INFO record. */
const unsigned PAGE_WIDTH_OFFSET = 0x08;  /* in the first PAGE record. */
const unsigned PAGE_SHAPE_SEQNUM_OFFSET = 0x02;  /* in the first PAGE record. */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
