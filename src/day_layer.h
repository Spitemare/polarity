#pragma once
#include <pebble.h>

typedef Layer DayLayer;

DayLayer *day_layer_create(GRect frame);
void day_layer_destroy(DayLayer *this);