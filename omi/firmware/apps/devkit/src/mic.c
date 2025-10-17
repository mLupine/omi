#include "device/core/mic.h"
#include "config.h"

int mic_start()
{
    return mic_start_with_gain(MIC_GAIN);
}
