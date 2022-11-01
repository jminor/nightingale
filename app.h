

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// #include "imguihelper.h"
// #include "imgui_plot.h"
// #include "imguifilesystem.h"
#include "imgui_internal.h"

// Struct that holds the application's state
struct AppState
{
  char file_path[PATH_MAX];
  char queue_folder[PATH_MAX];

  bool playing = false;
  float volume = 1.0f;
  float playhead = 0.0f;
  bool loop = false;
  uint32_t selection_start = 0;
  uint32_t selection_length = 1000;

  char message[1024];

  // SoLoud::Soloud audio;
  // SoLoud::handle audio_handle;

  // SoLoud::AudioSource *source;
  // SoLoud::Wav wav;
  // SoLoud::Modplug mod;

  bool show_main_window = true;
  bool mini_mode = false;
  bool show_style_editor = false;
  bool show_demo_window = false;
};

extern AppState appState;
extern ImFont *gTechFont;
extern ImFont *gIconFont;
