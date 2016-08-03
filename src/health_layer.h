#pragma once
#include <pebble.h>

typedef Layer HealthLayer;

HealthLayer *health_layer_create(GRect frame);
void health_layer_destroy(HealthLayer *this);
