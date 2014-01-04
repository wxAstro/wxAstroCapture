
#include "wxTimeFont.h"

/// expected font format is:
// w,h, n*(w*h) letter bytes
static const unsigned char T_Font7x10[] = { 7,10,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,255,255,255,000,000,
000,255,000,000,000,255,000,
000,255,000,000,000,255,000,
000,255,000,000,000,255,000,
000,255,000,000,000,255,000,
000,255,000,000,000,255,000,
000,000,255,255,255,000,000,
000,000,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,255,000,000,000,
000,255,255,255,000,000,000,
000,000,000,255,000,000,000,
000,000,000,255,000,000,000,
000,000,000,255,000,000,000,
000,000,000,255,000,000,000,
000,255,255,255,255,255,000,
000,000,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,255,255,255,000,000,
000,255,000,000,000,255,000,
000,000,000,000,000,255,000,
000,000,000,000,255,000,000,
000,000,000,255,000,000,000,
000,000,255,000,000,000,000,
000,255,255,255,255,255,000,
000,000,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,255,255,255,000,000,
000,255,000,000,000,255,000,
000,000,000,000,000,255,000,
000,000,000,255,255,000,000,
000,000,000,000,000,255,000,
000,255,000,000,000,255,000,
000,000,255,255,255,000,000,
000,000,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,255,000,000,
000,000,000,255,255,000,000,
000,000,255,000,255,000,000,
000,255,000,000,255,000,000,
000,255,255,255,255,255,000,
000,000,000,000,255,000,000,
000,000,000,000,255,000,000,
000,000,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,255,255,255,255,255,000,
000,255,000,000,000,000,000,
000,255,255,255,255,000,000,
000,000,000,000,000,255,000,
000,000,000,000,000,255,000,
000,255,000,000,000,255,000,
000,000,255,255,255,000,000,
000,000,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,255,255,000,000,
000,000,255,000,000,000,000,
000,255,000,000,000,000,000,
000,255,255,255,255,000,000,
000,255,000,000,000,255,000,
000,255,000,000,000,255,000,
000,000,255,255,255,000,000,
000,000,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,255,255,255,255,255,000,
000,000,000,000,000,255,000,
000,000,000,000,255,000,000,
000,000,000,000,255,000,000,
000,000,000,255,000,000,000,
000,000,000,255,000,000,000,
000,000,255,000,000,000,000,
000,000,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,255,255,255,000,000,
000,255,000,000,000,255,000,
000,255,000,000,000,255,000,
000,000,255,255,255,000,000,
000,255,000,000,000,255,000,
000,255,000,000,000,255,000,
000,000,255,255,255,000,000,
000,000,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,255,255,255,000,000,
000,255,000,000,000,255,000,
000,255,000,000,000,255,000,
000,000,255,255,255,255,000,
000,000,000,000,000,255,000,
000,000,000,000,255,000,000,
000,000,255,255,000,000,000,
000,000,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,255,000,000,000,
000,000,000,255,000,000,000,
000,000,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,255,000,000,000,
000,000,000,255,000,000,000,
000,000,000,000,000,000,000,
000,000,000,255,000,000,000,
000,000,000,255,000,000,000,
000,000,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,255,000,000,
000,000,000,000,255,000,000,
000,000,000,255,000,000,000,
000,000,000,255,000,000,000,
000,000,255,000,000,000,000,
000,000,255,000,000,000,000,
000,000,255,000,000,000,000,
000,255,000,000,000,000,000,
000,255,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,255,255,255,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,

000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000,
000,000,000,000,000,000,000};

static const unsigned char T_Font8x12[] = {8,12,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,255,255,255,255,000,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,000,255,255,255,255,000,000,
000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,255,000,000,000,
000,000,255,255,255,000,000,000,
000,000,000,000,255,000,000,000,
000,000,000,000,255,000,000,000,
000,000,000,000,255,000,000,000,
000,000,000,000,255,000,000,000,
000,000,000,000,255,000,000,000,
000,000,000,000,255,000,000,000,
000,000,255,255,255,255,255,000,
000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,255,255,255,255,000,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,000,000,000,000,000,255,000,
000,000,000,000,000,255,000,000,
000,000,000,255,255,000,000,000,
000,000,255,000,000,000,000,000,
000,255,000,000,000,000,000,000,
000,255,255,255,255,255,255,000,
000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,255,255,255,255,000,000,
000,255,000,000,000,000,255,000,
000,000,000,000,000,000,255,000,
000,000,000,000,000,000,255,000,
000,000,000,255,255,255,000,000,
000,000,000,000,000,000,255,000,
000,000,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,000,255,255,255,255,000,000,
000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,255,000,000,
000,000,000,000,255,255,000,000,
000,000,000,255,000,255,000,000,
000,000,255,000,000,255,000,000,
000,255,000,000,000,255,000,000,
000,255,255,255,255,255,255,000,
000,000,000,000,000,255,000,000,
000,000,000,000,000,255,000,000,
000,000,000,000,000,255,000,000,
000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,255,255,255,255,255,255,000,
000,255,000,000,000,000,000,000,
000,255,000,000,000,000,000,000,
000,255,255,255,255,255,000,000,
000,000,000,000,000,000,255,000,
000,000,000,000,000,000,255,000,
000,000,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,000,255,255,255,255,000,000,
000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,255,255,255,000,000,
000,000,255,000,000,000,000,000,
000,255,000,000,000,000,000,000,
000,255,255,255,255,255,000,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,000,255,255,255,255,000,000,
000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,255,255,255,255,255,255,000,
000,000,000,000,000,000,255,000,
000,000,000,000,000,255,000,000,
000,000,000,000,000,255,000,000,
000,000,000,000,255,000,000,000,
000,000,000,000,255,000,000,000,
000,000,000,255,000,000,000,000,
000,000,000,255,000,000,000,000,
000,000,255,000,000,000,000,000,
000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,255,255,255,255,000,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,000,255,255,255,255,000,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,000,255,255,255,255,000,000,
000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,255,255,255,255,000,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,255,000,000,000,000,255,000,
000,000,255,255,255,255,255,000,
000,000,000,000,000,000,255,000,
000,000,000,000,000,255,000,000,
000,000,255,255,255,000,000,000,
000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,255,000,000,000,000,
000,000,000,255,000,000,000,000,
000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,255,000,000,000,000,
000,000,000,255,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,255,000,000,000,000,
000,000,000,255,000,000,000,000,
000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,255,000,000,
000,000,000,000,000,255,000,000,
000,000,000,000,255,000,000,000,
000,000,000,000,255,000,000,000,
000,000,000,000,255,000,000,000,
000,000,000,255,000,000,000,000,
000,000,000,255,000,000,000,000,
000,000,000,255,000,000,000,000,
000,000,255,000,000,000,000,000,
000,000,255,000,000,000,000,000,
000,000,255,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,255,255,255,255,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000};

static const unsigned char T_Font10x16[] = {10,16,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,255,255,255,255,000,000,000,
000,000,255,255,255,255,255,255,000,000,
000,000,255,255,000,000,255,255,000,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,000,255,255,000,000,255,255,000,000,
000,000,255,255,255,255,255,255,000,000,
000,000,000,255,255,255,255,000,000,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,255,000,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,255,255,255,255,000,000,000,
000,000,255,255,000,255,255,000,000,000,
000,000,255,000,000,255,255,000,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,255,255,255,255,000,000,000,
000,000,255,255,255,255,255,255,000,000,
000,255,255,255,000,000,255,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,000,000,000,000,000,000,255,255,000,
000,000,000,000,000,000,000,255,255,000,
000,000,000,000,000,000,255,255,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,255,255,000,000,000,000,000,
000,000,255,255,000,000,000,000,000,000,
000,255,255,255,255,255,255,255,255,000,
000,255,255,255,255,255,255,255,255,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,255,255,255,255,000,000,000,
000,000,255,255,255,255,255,255,000,000,
000,255,255,255,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,000,000,000,000,000,000,255,255,000,
000,000,000,000,255,255,255,255,000,000,
000,000,000,000,255,255,255,255,000,000,
000,000,000,000,000,000,000,255,255,000,
000,000,000,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,255,000,000,255,255,255,000,
000,000,255,255,255,255,255,255,000,000,
000,000,000,255,255,255,255,000,000,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,255,255,000,000,
000,000,000,000,000,255,255,255,000,000,
000,000,000,000,000,255,255,255,000,000,
000,000,000,000,255,255,255,255,000,000,
000,000,000,255,255,000,255,255,000,000,
000,000,255,255,000,000,255,255,000,000,
000,000,255,255,000,000,255,255,000,000,
000,255,255,000,000,000,255,255,000,000,
000,255,255,255,255,255,255,255,255,000,
000,255,255,255,255,255,255,255,255,000,
000,000,000,000,000,000,255,255,000,000,
000,000,000,000,000,000,255,255,000,000,
000,000,000,000,000,000,255,255,000,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,255,255,255,255,255,255,255,000,
000,000,255,255,255,255,255,255,255,000,
000,000,255,255,000,000,000,000,000,000,
000,000,255,255,000,000,000,000,000,000,
000,255,255,255,255,255,255,000,000,000,
000,255,255,255,255,255,255,255,000,000,
000,255,255,000,000,000,255,255,255,000,
000,000,000,000,000,000,000,255,255,000,
000,000,000,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,255,000,000,255,255,255,000,
000,000,255,255,255,255,255,255,000,000,
000,000,000,255,255,255,255,000,000,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,255,255,255,255,000,000,000,
000,000,255,255,255,255,255,255,000,000,
000,000,255,255,000,000,000,255,255,000,
000,255,255,000,000,000,000,000,000,000,
000,255,255,000,000,000,000,000,000,000,
000,255,255,000,255,255,255,000,000,000,
000,255,255,255,255,255,255,255,000,000,
000,255,255,255,000,000,255,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,000,255,255,000,000,255,255,255,000,
000,000,255,255,255,255,255,255,000,000,
000,000,000,255,255,255,255,000,000,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,255,255,255,255,255,255,255,255,000,
000,255,255,255,255,255,255,255,255,000,
000,000,000,000,000,000,000,255,255,000,
000,000,000,000,000,000,255,255,000,000,
000,000,000,000,000,000,255,255,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,255,255,000,000,000,000,000,
000,000,000,255,255,000,000,000,000,000,
000,000,000,255,255,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,255,255,255,255,000,000,000,
000,000,255,255,255,255,255,255,000,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,000,255,255,255,255,255,255,000,000,
000,000,255,255,255,255,255,255,000,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,000,255,255,255,255,255,255,000,000,
000,000,000,255,255,255,255,000,000,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,255,255,255,255,000,000,000,
000,000,255,255,255,255,255,255,000,000,
000,255,255,255,000,000,255,255,000,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,000,000,000,000,255,255,000,
000,255,255,255,000,000,255,255,255,000,
000,000,255,255,255,255,255,255,255,000,
000,000,000,255,255,255,000,255,255,000,
000,000,000,000,000,000,000,255,255,000,
000,000,000,000,000,000,000,255,255,000,
000,255,255,000,000,000,255,255,000,000,
000,000,255,255,255,255,255,255,000,000,
000,000,000,255,255,255,255,000,000,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,255,255,000,000,
000,000,000,000,000,000,255,255,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,000,255,255,000,000,000,
000,000,000,000,255,255,255,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,000,255,255,000,000,000,000,
000,000,000,255,255,000,000,000,000,000,
000,000,000,255,255,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,255,255,255,255,255,255,000,000,
000,000,255,255,255,255,255,255,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,

000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000,
000,000,000,000,000,000,000,000,000,000};




wxTimeFont::wxTimeFont(EFontSize fontSize)
: m_fontSize(fontSize)
{
   switch (m_fontSize) {
      case EFS_Small:   m_fontPtr = const_cast<wxUint8*>(T_Font7x10); break;
      case EFS_Mid:     m_fontPtr = const_cast<wxUint8*>(T_Font8x12); break;
      case EFS_Large:   m_fontPtr = const_cast<wxUint8*>(T_Font10x16); break;
      default: m_fontPtr = const_cast<wxUint8*>(T_Font8x12);
   }
   m_width  = *m_fontPtr++;
   m_height = *m_fontPtr++;
   m_size = m_width * m_height;
   // now the ptr is at the start of the data
}

wxTimeFont::~wxTimeFont()
{
}


const wxUint8* wxTimeFont::Letter(ELetterCode letter) const
{
   if (letter<wxTimeFont::ELC_SIZE) {
      return &m_fontPtr[letter * m_size];
   }
   else {
      return NULL;
   }
}
const wxUint8* wxTimeFont::Letter(wxChar letter) const
{
   // return the known letters
   switch (letter) {
      case wxT('0'): return Letter(wxTimeFont::ELC_0); break;
      case wxT('1'): return Letter(wxTimeFont::ELC_1); break;
      case wxT('2'): return Letter(wxTimeFont::ELC_2); break;
      case wxT('3'): return Letter(wxTimeFont::ELC_3); break;
      case wxT('4'): return Letter(wxTimeFont::ELC_4); break;
      case wxT('5'): return Letter(wxTimeFont::ELC_5); break;
      case wxT('6'): return Letter(wxTimeFont::ELC_6); break;
      case wxT('7'): return Letter(wxTimeFont::ELC_7); break;
      case wxT('8'): return Letter(wxTimeFont::ELC_8); break;
      case wxT('9'): return Letter(wxTimeFont::ELC_9); break;
      case wxT('.'): return Letter(wxTimeFont::ELC_Dot); break;
      case wxT(':'): return Letter(wxTimeFont::ELC_Colon); break;
      case wxT('/'): return Letter(wxTimeFont::ELC_Slash); break;
      case wxT('-'): return Letter(wxTimeFont::ELC_Dash); break;
      case wxT(' '): return Letter(wxTimeFont::ELC_Blank); break;
      default:
         return NULL;
   }
}
