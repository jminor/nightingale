#include "miniaudio.h"

extern ma_decoder decoder;
extern ma_device_config deviceConfig;
extern ma_device device;

int play_audio_file(const char* path);
void stop_audio();
