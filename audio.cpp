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
ma_node_graph_config nodeGraphConfig;
ma_node_graph nodeGraph;
ma_data_source_node dataSourceNode;

int recent_data_index = 0;
float recent_data[1024*2];

static uint64_t __num_samples = 0;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

bool setup_audio()
{
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

    // Loop over each device info and do something with it. Here we just print the name with their index. You may want
    // to give the user the opportunity to choose which device they'd prefer.
    for (ma_uint32 iDevice = 0; iDevice < playbackCount; iDevice += 1) {
        printf("Audio Device #%d: %s\n", iDevice, pPlaybackInfos[iDevice].name);
    }

//    ma_device_config config = ma_device_config_init(ma_device_type_playback);
//    config.playback.pDeviceID = &pPlaybackInfos[chosenPlaybackDeviceIndex].id;
//    config.playback.format    = MY_FORMAT;
//    config.playback.channels  = MY_CHANNEL_COUNT;
//    config.sampleRate         = MY_SAMPLE_RATE;
//    config.dataCallback       = data_callback;
//    config.pUserData          = pMyCustomData;

    nodeGraphConfig = ma_node_graph_config_init(2);

    ma_result result = ma_node_graph_init(&nodeGraphConfig, NULL, &nodeGraph);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio node graph.\n");
        return false;
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate        = 48000;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &nodeGraph;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        return false;
    }

    return true;
}

void tear_down_audio()
{
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
//    ma_context_uninit(&context);
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
//    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
//    if (pDecoder == NULL) {
//        return;
//    }
//
//    ma_uint64 framesRead=0;
//    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, &framesRead);

    ma_uint64 framesRead;
    ma_result result = ma_node_graph_read_pcm_frames(&nodeGraph, pOutput, frameCount, &framesRead);
    if (result != MA_SUCCESS) {
        return;
    }

    float* source = (float*)pOutput;
    int frame_size = sizeof(float);
    ma_uint64 frames_to_copy = framesRead;
    ma_uint64 frames_copied = 0;
    ma_uint64 max_frames = sizeof(recent_data) / frame_size;

    while (frames_to_copy > 0) {
        ma_uint64 frames_free = max_frames - recent_data_index;
        ma_uint64 chunk_size = MIN(frames_free, frames_to_copy);
        memcpy(&recent_data[recent_data_index], &source[frames_copied], chunk_size * frame_size);
        frames_to_copy -= chunk_size;
        recent_data_index += chunk_size;
        frames_copied += chunk_size;
        if (recent_data_index >= max_frames) recent_data_index = 0;
    }

    (void)pInput;
}

bool load_audio_file(const char* path)
{
    ma_decoder_uninit(&decoder);

    ma_result result = ma_decoder_init_file(path, NULL, &decoder);
    if (result != MA_SUCCESS) {
        printf("Could not load file: %s\n", path);
        return false;
    }

    if (ma_decoder_get_length_in_pcm_frames(&decoder, &__num_samples) != MA_SUCCESS) {
        printf("Failed to determine audio file duration.\n");
        ma_decoder_uninit(&decoder);
        return false;
    }

    ma_data_source_node_config config = ma_data_source_node_config_init(&decoder);

    result = ma_data_source_node_init(&nodeGraph, &config, NULL, &dataSourceNode);
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
//    return decoder.outputSampleRate;
}

int num_channels()
{
    return decoder.outputChannels;
}

uint64_t num_samples()
{
    return __num_samples;
}

uint64_t current_sample_position()
{
    ma_uint64 cursor;

    ma_result result = ma_decoder_get_cursor_in_pcm_frames(&decoder, &cursor);
    if (result != MA_SUCCESS) {
        return 0;   // An error occurred.
    }

    return cursor;
}

bool seek_audio(uint64_t targetFrame)
{
    printf("seek %lld\n", targetFrame);
    ma_result result = ma_decoder_seek_to_pcm_frame(&decoder, targetFrame);
    if (result != MA_SUCCESS) {
        return false;   // An error occurred.
    }

    return true;
}

void set_volume(float vol)
{
//    ma_node_set_output_bus_volume(&dataSourceNode, 0, vol);
    ma_node_set_output_bus_volume(ma_node_graph_get_endpoint(&nodeGraph), 0, vol);
}
