#include <pebble.h>

#ifdef PBL_HEALTH
#define RADIAL_THICKNESS PBL_IF_ROUND_ELSE(20, 16)
#else
#define RADIAL_THICKNESS 20
#endif