#include "miniaudio.h"

extern ma_decoder decoder;
extern ma_device_config deviceConfig;
extern ma_device device;

bool load_audio_file(const char* path);
bool play_audio();
void stop_audio();
int sample_rate();

