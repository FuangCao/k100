#pragma once

#include "jwaoo_key.h"

void jwaoo_on_host_key_clicked(struct jwaoo_key_device *key, uint8_t count);
void jwaoo_on_host_key_long_clicked(struct jwaoo_key_device *key);

void jwaoo_on_client_key_state_changed(struct jwaoo_key_device *key, bool force);
void jwaoo_on_client_key_clicked(struct jwaoo_key_device *key, uint8_t count);
void jwaoo_on_client_key_long_clicked(struct jwaoo_key_device *key);
