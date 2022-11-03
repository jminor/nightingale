#include "miniaudio.h"

//extern ma_decoder decoder;
//extern ma_device_config deviceConfig;
//extern ma_device device;

extern float recent_data[256];

bool setup_audio();
bool load_audio_file(const char* path);
bool play_audio();
bool stop_audio();
uint32_t sample_rate();
uint64_t num_samples();
uint64_t current_sample_position();
bool seek_audio(uint64_t targetFrame);
void tear_down_audio();
