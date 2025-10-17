/*
 * Copyright (c) 2023 Omi Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "device/core/mic.h"
#include "device/core/settings.h"

int mic_start()
{
    uint8_t saved_gain = app_settings_get_mic_gain();
    return mic_start_with_gain(saved_gain);
}
