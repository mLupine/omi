#include "mic.h"

#include <haly/nrfy_gpio.h>
#include <nrfx_pdm.h>
#include <zephyr/audio/dmic.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "config.h"

LOG_MODULE_REGISTER(mic, CONFIG_LOG_DEFAULT_LEVEL);

#define MAX_SAMPLE_RATE 16000
#define SAMPLE_BIT_WIDTH 16
#define BYTES_PER_SAMPLE sizeof(int16_t)
#define CHANNELS 2

#define READ_TIMEOUT 1000

#define BLOCK_SIZE(sample_rate, number_of_channels) (BYTES_PER_SAMPLE * (sample_rate / 10) * number_of_channels)

#define MAX_BLOCK_SIZE BLOCK_SIZE(MAX_SAMPLE_RATE, 2)
#define BLOCK_COUNT 4

K_MEM_SLAB_DEFINE_STATIC(mem_slab, MAX_BLOCK_SIZE, BLOCK_COUNT, 4);

static const struct device *dmic_dev;
static volatile mix_handler callback_func = NULL;
static volatile bool mic_running = false;

#define MAX_FRAMES (MAX_SAMPLE_RATE / 10)
static int16_t mono_buffer[MAX_FRAMES];

static inline void
interleaved_stereo_to_mono(const int16_t *restrict interleaved, size_t frames, int16_t *restrict mono_out)
{
    for (size_t i = 0, j = 0; i < frames; ++i, j += 2) {
        int32_t left = (int32_t) interleaved[j + 0];
        int32_t right = (int32_t) interleaved[j + 1];
        int32_t sum = left + right;
        sum >>= 1;
        if (sum > 32767)
            sum = 32767;
        if (sum < -32768)
            sum = -32768;
        mono_out[i] = (int16_t) sum;
    }
}

static void process_audio_buffer(void *buffer, uint32_t size)
{
    __ASSERT_NO_MSG((size % (BYTES_PER_SAMPLE * CHANNELS)) == 0);
    size_t frames = size / (BYTES_PER_SAMPLE * CHANNELS);
    int16_t *inter = (int16_t *) buffer;

    if (frames > MAX_FRAMES) {
        LOG_ERR("Frame count %zu exceeds MAX_FRAMES %d", frames, MAX_FRAMES);
        k_mem_slab_free(&mem_slab, buffer);
        return;
    }

    interleaved_stereo_to_mono(inter, frames, mono_buffer);

    if (callback_func) {
        callback_func(mono_buffer);
    }

    k_mem_slab_free(&mem_slab, buffer);
}

static void mic_thread_function(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (mic_running) {
        void *buffer;
        uint32_t size;

        int ret = dmic_read(dmic_dev, 0, &buffer, &size, READ_TIMEOUT);
        if (ret < 0) {
            LOG_ERR("Read failed: %d", ret);
            continue;
        }

        LOG_DBG("Got buffer %p of %u bytes", buffer, size);
        process_audio_buffer(buffer, size);
    }
}

#define MIC_THREAD_STACK_SIZE 2048
#define MIC_THREAD_PRIORITY 5
K_THREAD_DEFINE(mic_thread_id,
                MIC_THREAD_STACK_SIZE,
                mic_thread_function,
                NULL,
                NULL,
                NULL,
                MIC_THREAD_PRIORITY,
                0,
                -1);

int mic_start()
{
    int ret;

    nrfy_gpio_cfg_output(PDM_PWR_PIN);
    nrfy_gpio_pin_set(PDM_PWR_PIN);

    dmic_dev = DEVICE_DT_GET(DT_ALIAS(dmic0));
    if (!device_is_ready(dmic_dev)) {
        LOG_ERR("%s is not ready", dmic_dev->name);
        return -ENODEV;
    }

    struct pcm_stream_cfg stream = {
        .pcm_width = SAMPLE_BIT_WIDTH,
        .mem_slab = &mem_slab,
        .pcm_rate = MAX_SAMPLE_RATE,
        .block_size = BLOCK_SIZE(MAX_SAMPLE_RATE, CHANNELS),
    };

    struct dmic_cfg cfg = {
        .io =
            {
                .min_pdm_clk_freq = 512000,
                .max_pdm_clk_freq = 3500000,
                .min_pdm_clk_dc = 48,
                .max_pdm_clk_dc = 52,
            },
        .streams = &stream,
        .channel =
            {
                .req_num_streams = 1,
                .req_num_chan = CHANNELS,
                .req_chan_map_lo =
                    dmic_build_channel_map(0, 0, PDM_CHAN_LEFT) | dmic_build_channel_map(1, 0, PDM_CHAN_RIGHT),
            },

    };

    LOG_INF("PCM output rate: %u, channels: %u", cfg.streams[0].pcm_rate, cfg.channel.req_num_chan);

    ret = dmic_configure(dmic_dev, &cfg);
    if (ret < 0) {
        LOG_ERR("Failed to configure the driver: %d", ret);
        return ret;
    }

    nrf_pdm_gain_set(NRF_PDM0, MIC_GAIN, MIC_GAIN);

    ret = dmic_trigger(dmic_dev, DMIC_TRIGGER_START);
    if (ret < 0) {
        LOG_ERR("START trigger failed: %d", ret);
        return ret;
    }

    mic_running = true;
    k_thread_start(mic_thread_id);

    LOG_INF("Microphone started");
    return 0;
}

void set_mic_callback(mix_handler callback)
{
    callback_func = callback;
}

void mic_off()
{
    nrfy_gpio_pin_clear(PDM_PWR_PIN);
}

void mic_on()
{
    nrfy_gpio_pin_set(PDM_PWR_PIN);
}
