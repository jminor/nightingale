#define MINIAUDIO_IMPLEMENTATION

#include "audio.h"

#include <stdio.h>

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

ma_context context;
ma_device_config deviceConfig;
ma_device device;
ma_decoder decoder;
ma_data_source_node dataSourceNode;
ma_node_graph_config nodeGraphConfig;
ma_node_graph nodeGraph;

// a Pretty Fast Fast Fourier Transform library
#include "libs/pffft/pffft.c"

#define RECENT_DATA_SIZE    1024

static int recent_data_index = 0;
static float *recent_data = NULL;

int recent_data_size() {
    return RECENT_DATA_SIZE;
}

float *get_recent_data() {
    return recent_data;
}

#define FFT_SIZE    RECENT_DATA_SIZE

static float *fft_data = NULL;
static PFFFT_Setup *fft_setup;

int fft_size() {
    return FFT_SIZE;
}

float *calc_fft()
{
    pffft_transform_ordered(fft_setup, recent_data, fft_data, NULL, PFFFT_FORWARD);

    // make the data easier to read
    // via log(abs(x))
    float *p = fft_data;
    for (int i=0; i<FFT_SIZE; i++, p++) {
        *p = log(abs(*p));
    }

    return fft_data;
}

void init_recent_data_and_fft()
{
    recent_data = (float*)pffft_aligned_malloc(RECENT_DATA_SIZE * sizeof(float));
    fft_data = (float*)pffft_aligned_malloc(FFT_SIZE * sizeof(float));
    fft_setup = pffft_new_setup(FFT_SIZE, PFFFT_REAL);
}

static uint64_t __num_samples = 0;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

bool setup_audio()
{
    init_recent_data_and_fft();

    if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
        printf("Failed to initialize audio context.\n");
        return false;
    }

    ma_device_info* pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;
    if (ma_context_get_devices(&context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS) {
        printf("Failed to get audio device list.\n");
        return false;
    }

    // Loop over each device info and do something with it. Here we just print the name with their index.
    printf("Available Audio Devices:\n");
    for (ma_uint32 iDevice = 0; iDevice < playbackCount; iDevice += 1) {
        printf("  #%d: %s\n", iDevice, pPlaybackInfos[iDevice].name);
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate        = 48000;
    deviceConfig.dataCallback      = data_callback;

    nodeGraphConfig = ma_node_graph_config_init(deviceConfig.playback.channels);
    ma_result result = ma_node_graph_init(&nodeGraphConfig, NULL, &nodeGraph);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio node graph.\n");
        return false;
    }
    deviceConfig.pUserData         = &nodeGraph;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        return false;
    }

    char name[MA_MAX_DEVICE_NAME_LENGTH+1];
    if (ma_device_get_name(&device, ma_device_type_playback, name, sizeof(name), NULL) != MA_SUCCESS) {
        printf("Failed to get playback device name.\n");
        return false;
    }
    printf("Selected Audio Device: %s\n", name);

    return true;
}

void tear_down_audio()
{
    ma_device_stop(&device);
    ma_decoder_uninit(&decoder);
    ma_device_uninit(&device);
    ma_context_uninit(&context);
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_uint64 framesRead;
    ma_result result = ma_node_graph_read_pcm_frames(&nodeGraph, pOutput, frameCount, &framesRead);
    if (result != MA_SUCCESS) {
        return;
    }

    const int channels = 2;
    float* src = (float*)pOutput;
    ma_uint64 frames_to_copy = framesRead;
    while (frames_to_copy > channels-1) {
        float val = 0;
        for (int c=0; c<channels; c++) {
            val += *src++;
            frames_to_copy--;
        }
        val /= channels;
        recent_data[recent_data_index++] = val;
        if (recent_data_index >= RECENT_DATA_SIZE) recent_data_index = 0;
    }

//    float* source = (float*)pOutput;
//    int frame_size = sizeof(float);
//    ma_uint64 frames_to_copy = framesRead;
//    ma_uint64 frames_copied = 0;
//    ma_uint64 max_frames = RECENT_DATA_SIZE;
//
//    while (frames_to_copy > 0) {
//        ma_uint64 frames_free = max_frames - recent_data_index;
//        ma_uint64 chunk_size = MIN(frames_free, frames_to_copy);
//        memcpy(&recent_data[recent_data_index], &source[frames_copied], chunk_size * frame_size);
//        frames_to_copy -= chunk_size;
//        recent_data_index += chunk_size;
//        frames_copied += chunk_size;
//        if (recent_data_index >= max_frames) recent_data_index = 0;
//    }

    (void)pInput;
}

bool load_audio_file(const char* path)
{
    ma_decoder_uninit(&decoder);

    __num_samples = 0;

    ma_decoder_config decoderConfig = ma_decoder_config_init(deviceConfig.playback.format,
                                                      deviceConfig.playback.channels,
                                                      deviceConfig.sampleRate);
//    ma_decoder_config decoderConfig = ma_decoder_config_init_default();
    ma_result result = ma_decoder_init_file(path, &decoderConfig, &decoder);
    if (result != MA_SUCCESS) {
        printf("Could not load file: %s\n", path);
        return false;
    }

    ma_format format;
    ma_uint32 num_channels;
    ma_uint32 sample_rate;
    if (ma_decoder_get_data_format(&decoder, &format, &num_channels, &sample_rate, NULL, 0) != MA_SUCCESS) {
        printf("Failed to determine audio file format.\n");
        ma_decoder_uninit(&decoder);
        return false;
    }

    if (format != deviceConfig.playback.format) {
        printf("Audio file format mismatch %d != %d.\n", format, deviceConfig.playback.format);
    }
    if (num_channels != deviceConfig.playback.channels) {
        printf("Audio file channels mismatch %d != %d.\n", num_channels, deviceConfig.playback.channels);
    }
    if (sample_rate != deviceConfig.sampleRate) {
        printf("Audio file sample rate mismatch %d != %d.\n", sample_rate, deviceConfig.sampleRate);
    }

    if (ma_decoder_get_length_in_pcm_frames(&decoder, &__num_samples) != MA_SUCCESS) {
        printf("Failed to determine audio file duration.\n");
        ma_decoder_uninit(&decoder);
        return false;
    }

    ma_data_source_node_config nodeConfig = ma_data_source_node_config_init(&decoder);

    result = ma_data_source_node_init(&nodeGraph, &nodeConfig, NULL, &dataSourceNode);
    if (result != MA_SUCCESS) {
        printf("Failed to create data source node.\n");
        ma_decoder_uninit(&decoder);
        return false;
    }

    result = ma_node_attach_output_bus(&dataSourceNode, 0, ma_node_graph_get_endpoint(&nodeGraph), 0);
    if (result != MA_SUCCESS) {
        printf("Failed to attach node.\n");
        ma_decoder_uninit(&decoder);
        return false;
    }

    return true;
}

bool play_audio()
{
    ma_result result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        tear_down_audio();
        return false;
    }

    return true;
}

bool stop_audio()
{
    if (ma_device_stop(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        tear_down_audio();
        return false;
    }

    return true;
}

uint32_t sample_rate()
{
    return deviceConfig.sampleRate;
}

int num_channels()
{
    return decoder.outputChannels;
}

uint64_t num_samples()
{
    return __num_samples;
}

const char *audio_format_str()
{
    switch (deviceConfig.playback.format) {
        case ma_format_f32:
            return "f32";
        case ma_format_u8:
            return "u8";
        case ma_format_s16:
            return "s16";
        case ma_format_s24:
            return "s24";
        case ma_format_s32:
            return "s32";
        default:
            return "?";
    }
}

uint64_t current_sample_position()
{
    ma_uint64 cursor;

    ma_result result = ma_decoder_get_cursor_in_pcm_frames(&decoder, &cursor);
    if (result != MA_SUCCESS) {
        return 0;
    }

    return cursor;
}

bool seek_audio(uint64_t targetFrame)
{
    printf("seek %lld\n", targetFrame);
    ma_result result = ma_decoder_seek_to_pcm_frame(&decoder, targetFrame);
    if (result != MA_SUCCESS) {
        return false;
    }

    return true;
}

void set_volume(float vol)
{
//    ma_node_set_output_bus_volume(&dataSourceNode, 0, vol);
    ma_node_set_output_bus_volume(ma_node_graph_get_endpoint(&nodeGraph), 0, vol);
}
