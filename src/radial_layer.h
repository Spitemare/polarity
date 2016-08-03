#pragma once
#include <pebble.h>

typedef Layer RadialLayer;

RadialLayer *radial_layer_create(GRect frame);
void radial_layer_destroy(RadialLayer *this);
void radial_layer_set_thickness(RadialLayer *this, uint16_t thickness);
void radial_layer_set_value(RadialLayer *this, uint32_t angle);
void radial_layer_set_color(RadialLayer *this, GColor color);
