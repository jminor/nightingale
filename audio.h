#include "miniaudio.h"

bool setup_audio();
bool load_audio_file(const char* path);
bool play_audio();
bool stop_audio();
uint32_t sample_rate();
int num_channels();
uint64_t num_samples();
const char *audio_format_str();
const char *audio_state_str();
uint64_t current_sample_position();
bool seek_audio(uint64_t targetFrame);
void tear_down_audio();
void set_volume(float vol);
int recent_data_size();
float *get_recent_data();
int fft_size();
float *calc_fft();
bool audio_looping();
void audio_set_looping(bool loop);
