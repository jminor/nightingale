

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define YES_IMGUISOLOUD_ALL
#include "imguisoloud.h"
#include "imguinodegrapheditor.h"
#include "imguihelper.h"
#include "imgui_plot.h"
#include "imguifilesystem.h"
#include "imgui_internal.h"

// Struct that holds the application's state
struct AppState
{
  char file_path[4096];
  float volume = 1.0f;
  float playhead = 0.0f;
  bool loop = false;
  uint32_t selection_start = 0;
  uint32_t selection_length = 1000;

  SoLoud::Soloud audio;
  SoLoud::handle audio_handle;

  SoLoud::AudioSource *source;
  SoLoud::Speech speech;
  SoLoud::Wav wav;
  SoLoud::Modplug mod;

  char speech_text[1024];

  ImGui::NodeGraphEditor nge;
  
  bool show_main_window = true;
  bool show_style_editor = false;
  bool show_node_graph = true;
  bool show_demo_window = false;
};

extern AppState appState;
extern ImFont *gTechFont;
extern ImFont *gIconFont;
