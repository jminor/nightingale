#define MINIAUDIO_IMPLEMENTATION

#include "audio.h"

#include <stdio.h>

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount);

    (void)pInput;
}

ma_decoder decoder;
ma_device_config deviceConfig;
ma_device device;

bool load_audio_file(const char* path)
{
	stop_audio();
	
	ma_result result;

    result = ma_decoder_init_file(path, NULL, &decoder);
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
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return false;
    }

    return true;
}

void stop_audio()
{
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
}

int sample_rate()
{
    return deviceConfig.sampleRate;
}
