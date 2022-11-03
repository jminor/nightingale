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

int recent_data_index = 0;
float recent_data[256];

bool setup_audio()
{
    if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
        return false;
    }

    ma_device_info* pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;
    if (ma_context_get_devices(&context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS) {
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
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount);

    memcpy(recent_data, pOutput, MIN(sizeof(recent_data), frameCount*sizeof(float)));

    (void)pInput;
}

bool load_audio_file(const char* path)
{
    tear_down_audio();

    ma_result result = ma_decoder_init_file(path, NULL, &decoder);
    if (result != MA_SUCCESS) {
        printf("Could not load file: %s\n", path);
        return false;
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    // deviceConfig.playback.format   = decoder.outputFormat;
    // deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &decoder;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
        return false;
    }
    
    return true;
}

bool play_audio()
{
    if (ma_device_start(&device) != MA_SUCCESS) {
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

uint64_t num_samples()
{
    return ma_decoder_get_length_in_pcm_frames(&decoder);
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
