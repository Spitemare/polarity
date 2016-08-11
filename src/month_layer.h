#pragma once
#include <pebble.h>

typedef Layer MonthLayer;

MonthLayer *month_layer_create(GRect frame);
void month_layer_destroy(MonthLayer *this);
