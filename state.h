#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "audio.h"
#include "imguihelper.h"
#include "imgui_plot.h"
#include "imguifilesystem.h"
#include "imgui_internal.h"

struct VideoFrame
{
  ImTextureID texture = 0;
  ImVec2 size = ImVec2(0,0);
};

// Struct that holds the application's state
struct AppState
{
  char file_path[ImGuiFs::MAX_PATH_BYTES];
  char queue_folder[ImGuiFs::MAX_PATH_BYTES];

  bool playing = false;
  float volume = 1.0f;
  float playhead = 0.0f;
  bool loop = false;
  uint32_t selection_start = 0;
  uint32_t selection_length = 1000;

  char message[1024];

  SoLoud::Soloud audio;
  SoLoud::handle audio_handle;

  SoLoud::AudioSource *source;
  SoLoud::Wav wav;
  SoLoud::Modplug mod;
  SoLoud::Mp3 mp3;

  int num_frames = 1024;
  VideoFrame frames[1024];
  int loop_start = 1;
  int loop_end = 1;

  bool show_main_window = true;
  bool mini_mode = false;
  bool show_style_editor = false;
  bool show_demo_window = false;
};

extern AppState appState;
