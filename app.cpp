// Nightingale Audio Player

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define YES_IMGUISOLOUD_ALL
#include "imguisoloud.h"
#include "imgui_plot.h"
#include "imgui_internal.h"

#ifdef HELLOIMGUI_USE_SDL_OPENGL3
#include <SDL.h>
#endif

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
  SoLoud::Wav wav;

  bool initialized = false;
  bool show_demo_window = false;
};

AppState appState;

//   Files in the application assets/ folder are embedded automatically
//   (on iOS/Android/Emscripten)
ImFont *gTechFont = nullptr;
ImFont *gIconFont = nullptr;
void LoadFonts()
{
  ImGuiIO& io = ImGui::GetIO();

  gTechFont = io.Fonts->AddFontFromFileTTF("fonts/ShareTechMono-Regular.ttf", 20.0f);

  static const ImWchar icon_fa_ranges[] = { 0xF000, 0xF18B, 0 };
  gIconFont = io.Fonts->AddFontFromFileTTF("fonts/fontawesome-webfont.ttf", 20.0f, NULL, icon_fa_ranges);
}

void Style_Mono()
{
  ImGuiStyle& style = ImGui::GetStyle();
  style.Alpha = 1.0;
  style.WindowRounding = 3;
  style.GrabRounding = 1;
  style.GrabMinSize = 20;
  style.FrameRounding = 3;
  style.WindowBorderSize = 0;

  ImVec4* colors = style.Colors;
  colors[ImGuiCol_Text]                   = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TextDisabled]           = ImVec4(0.00f, 0.40f, 0.41f, 1.00f);
  colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
  colors[ImGuiCol_Border]                 = ImVec4(0.00f, 1.00f, 1.00f, 0.65f);
  colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg]                = ImVec4(0.44f, 0.80f, 0.80f, 0.18f);
  colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.44f, 0.80f, 0.80f, 0.27f);
  colors[ImGuiCol_FrameBgActive]          = ImVec4(0.44f, 0.81f, 0.86f, 0.66f);
  colors[ImGuiCol_TitleBg]                = ImVec4(0.14f, 0.18f, 0.21f, 0.73f);
  colors[ImGuiCol_TitleBgActive]          = ImVec4(0.00f, 1.00f, 1.00f, 0.27f);
  colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
  colors[ImGuiCol_MenuBarBg]              = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
  colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.22f, 0.29f, 0.30f, 0.71f);
  colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.00f, 1.00f, 1.00f, 0.44f);
  colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
  colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.80f, 0.99f, 0.99f, 1.00f);
  colors[ImGuiCol_CheckMark]              = ImVec4(0.00f, 1.00f, 1.00f, 0.68f);
  colors[ImGuiCol_SliderGrab]             = ImVec4(0.00f, 1.00f, 1.00f, 0.36f);
  colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.80f, 0.99f, 0.99f, 1.00f);
  colors[ImGuiCol_Button]                 = ImVec4(0.00f, 0.65f, 0.65f, 0.46f);
  colors[ImGuiCol_ButtonHovered]          = ImVec4(0.01f, 1.00f, 1.00f, 0.43f);
  colors[ImGuiCol_ButtonActive]           = ImVec4(0.80f, 0.99f, 0.99f, 1.00f);
  colors[ImGuiCol_Header]                 = ImVec4(0.00f, 1.00f, 1.00f, 0.33f);
  colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 1.00f, 1.00f, 0.42f);
  colors[ImGuiCol_HeaderActive]           = ImVec4(1.00f, 1.00f, 1.00f, 0.54f);
  colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
  colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
  colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
  colors[ImGuiCol_ResizeGrip]             = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
  colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
  colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.80f, 0.99f, 0.99f, 1.00f);
  colors[ImGuiCol_Tab]                    = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
  colors[ImGuiCol_TabHovered]             = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  colors[ImGuiCol_TabActive]              = ImVec4(0.31f, 0.50f, 0.74f, 1.00f);
  colors[ImGuiCol_TabUnfocused]           = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
  colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
  colors[ImGuiCol_DockingPreview]         = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
  colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_PlotLines]              = ImVec4(0.80f, 0.99f, 0.99f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_PlotHistogram]          = ImVec4(0.80f, 0.99f, 0.99f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 1.00f, 1.00f, 0.22f);
  colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
  colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.04f, 0.10f, 0.09f, 0.51f);
}

SoLoud::Speech speech;

void LoadSpeech(const char* text)
{
  speech.setText(text);
  appState.audio.play(speech);
  // printf("speech mFrames = %d\n", speech.mFrames);
  // printf("speech mElement size = %d\n", speech.mElement.getSize());
  // auto instance = (SoLoud::SpeechInstance*)speech.createInstance();
  // printf("speech instance.mSampleCount = %d\n", instance->mSampleCount);
  // printf("speech mElement size = %d\n", speech.mElement.getSize());
  // float* buffer = (float*)malloc(instance->mSampleCount*sizeof(float));
  // instance->getAudio(buffer, instance->mSampleCount);
  // appState.wav()
}

void LoadAudio(const char* path)
{
  strncpy(appState.file_path, path, sizeof(appState.file_path));
  
  auto err = appState.wav.load(path);
  if (err) {
    fprintf(stderr, "Failed to load WAV: %s: %s\n", appState.audio.getErrorString(err), path);
    LoadSpeech("Failed to load audio file.");
  }
}

void MainInit()
{
  Style_Mono();

  LoadFonts();

  auto err = appState.audio.init();
  if (err) {
    fprintf(stderr, "Failed to load initialize audio: %s\n", appState.audio.getErrorString(err));
  }
  LoadAudio(""); ///Users/jminor/Library/Mobile Documents/com~apple~CloudDocs/Sokpop Sources/sokpop-source 5/nosoksky.gmx/sound/audio/sou_radio_fragment5.wav");
  appState.audio.setVisualizationEnable(true);
}

void MainCleanup()
{
  appState.audio.deinit();
}

void Seek(float time) {
  appState.playhead = time;
  if (appState.audio_handle) {
    appState.audio.setPause(appState.audio_handle, true);
  }else{
    appState.audio_handle = appState.audio.play(appState.wav, appState.volume, 0.0f, true);
    appState.audio.setLooping(appState.audio_handle, appState.loop);
  }
  appState.audio.seek(appState.audio_handle, appState.playhead);
  appState.selection_start = appState.wav.mSampleCount * (appState.playhead / appState.wav.getLength());
}

bool IconButton(const char* label, const ImVec2 size=ImVec2(0,0))
{
  ImGui::PushFont(gIconFont);
  bool result = ImGui::Button(label, size);
  ImGui::PopFont();
  return result;
}

void MainGui()
{
  ImGuiStyle& style = ImGui::GetStyle();

  if (!appState.initialized) {
    ImGuiIO& io = ImGui::GetIO();
    // ImGui::SetNextWindowPos(ImVec2(0,0));
    ImVec2 winSize = io.DisplaySize;
    ImGui::SetNextWindowSize(winSize);
  }
  ImGui::Begin(
      "Nightingale"
      // nullptr,
      // ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus
      );

  bool playing = appState.audio_handle && !appState.audio.getPause(appState.audio_handle);

  if (appState.audio_handle && appState.audio.isValidVoiceHandle(appState.audio_handle)) {
    appState.playhead = appState.audio.getStreamTime(appState.audio_handle);
    appState.playhead = fmodf(appState.playhead, appState.wav.getLength());
  }else{
    appState.audio_handle = 0;
    appState.playhead = 0.0;
  }
  if (playing) {
    appState.selection_start = appState.wav.mSampleCount * appState.playhead / appState.wav.getLength();
    // appState.selection_length = 256;
  }

  ImGui::SetNextWindowSize(ImVec2(400,200));
  if (ImGui::BeginPopupModal("Load file")) {
    ImGui::InputText("File path", appState.file_path, sizeof(appState.file_path));

    if(ImGui::Button("Load##now")) {
      LoadAudio(appState.file_path);
    }

    ImGui::SameLine();

    if(ImGui::Button("Close")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  ImGui::PushItemWidth(-100);

  if (ImGui::SliderFloat("Volume", &appState.volume, 0.0f, 1.0f)) {
    appState.audio.setVolume(appState.audio_handle, appState.volume);
  }

  float duration = appState.wav.getLength();
  if (ImGui::SliderFloat("Playhead", &appState.playhead, 0.0f, duration)) {
    Seek(appState.playhead);
    playing = false;
  }

  // auto size = ImGui::GetItemRectSize();
  float width = ImGui::CalcItemWidth();

  if (false && playing) {
    // this shows the mixed output waveform
    ImGui::PlotLines(
      "Live Waveform",
      appState.audio.getWave(),
      256,  // values_count
      0,    // values_offset
      nullptr, // overlay_text
      -1.0f, // scale_min
      1.0f, // scale_max
      ImVec2(width,100) // graph_size
      );
  }else{
    ImGui::PlotConfig plot_config;
    plot_config.frame_size = ImVec2(width, 100);
    plot_config.values.ys = appState.wav.mData + appState.selection_start;
    plot_config.values.count = appState.selection_length;
    plot_config.scale.min = -1.0f;
    plot_config.scale.max = 1.0f;
    plot_config.selection.show = false;
    // plot_config.selection.show = playing;
    // uint32_t start = 0, length = 256;
    // plot_config.selection.start = &start;
    // plot_config.selection.length = &length;
    ImGui::Plot("Waveform", plot_config);
    ImGui::SameLine();
    ImGui::Text("Waveform\nDetail");

    // ImGui::PlotLines(
    //   "Waveform 2",
    //   appState.wav.mData,
    //   appState.selection_length,
    //   // fmax(appState.selection_length, 256),  // values_count
    //   appState.selection_start,
    //   // fmin(appState.selection_start, appState.wav.mSampleCount-256),   // values_offset
    //   nullptr, // overlay_text
    //   -1.0f, // scale_min
    //   1.0f, // scale_max
    //   ImVec2(width,100) // graph_size
    //   );
  }
  ImGui::PlotHistogram(
    "FFT",
    appState.audio.calcFFT(),
    256,  // values_count
    0,    // values_offset
    nullptr, // overlay_text
    FLT_MAX, // scale_min
    FLT_MAX, // scale_max
    ImVec2(width,100) // graph_size
    );

  ImGui::PlotConfig plot_config;
  plot_config.frame_size = ImVec2(width, 200);
  plot_config.values.ys = appState.wav.mData;
  plot_config.values.count = appState.wav.mSampleCount;
  plot_config.scale.min = -1.0f;
  plot_config.scale.max = 1.0f;
  plot_config.selection.show = true;
  plot_config.selection.start = &appState.selection_start;
  plot_config.selection.length = &appState.selection_length;
  // plot_config.overlay_text = "Hello";
  if (ImGui::Plot("Data", plot_config) == ImGui::PlotStatus::selection_updated) {
    Seek(appState.selection_start * appState.wav.getLength() / appState.wav.mSampleCount);
    playing = false;
    appState.selection_length = fmax(appState.selection_length, 256);
  }

  ImGui::SameLine();
  ImGui::Text("Waveform\nFull");

  ImGui::PopItemWidth();

  int num_buttons = 6;
  ImVec2 button_size(
    (width - style.ItemSpacing.x*(num_buttons-1))/num_buttons,
    ImGui::GetTextLineHeight()*2
    );

  // toggle
  if (!playing) {
    if (IconButton("\uF04B##Play", button_size)) {
      if (appState.audio_handle) {
        appState.audio.setPause(appState.audio_handle, false);
      }else{
        appState.audio_handle = appState.audio.play(appState.wav, appState.volume);
        appState.audio.setLooping(appState.audio_handle, appState.loop);
      }
    }
  }else{
    if (IconButton("\uF04C##Pause", button_size)) {
      appState.audio.setPause(appState.audio_handle, true);
    }
  }

  ImGui::SameLine();

  if (IconButton("\uF04D##Stop", button_size)) {
    appState.audio.stopAll();
  }

  ImGui::SameLine();
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, appState.loop ? 2 : 0);
  // ImGui::PushStyleColor(ImGuiCol_Border, style.Colors[ImGuiCol_ButtonActive]);
  if (IconButton("\uF021##Loop", button_size)) {
    appState.loop = !appState.loop;
    appState.audio.setLooping(appState.audio_handle, appState.loop);
  }
  // ImGui::PopStyleColor();
  ImGui::PopStyleVar();

  // ##Cut
  // ##Copy
  // #Paste or 

  ImGui::SameLine();

  if (IconButton("\uF07C##Load", button_size)) {
    ImGui::OpenPopup("Load file");
  }

  ImGui::SameLine();

  if (IconButton("\uF0AE##Style", button_size)) {
    ImGui::OpenPopup("Style Editor");
  }

  ImGui::SameLine();

  if (IconButton("\uF013#Demo", button_size)) {
    appState.show_demo_window = !appState.show_demo_window;
  }

  if (appState.show_demo_window) {
    ImGui::ShowDemoWindow();
  }

  if (ImGui::BeginPopup("Style Editor")) {
    ImGui::ShowStyleEditor();
    ImGui::EndPopup();
  }

  ImGui::End();

  appState.initialized = true;
}

// int main(int, char **)
// {
//   HelloImGui::RunnerParams runnerParams;

//   runnerParams.appWindowParams.windowTitle = "Nightingale";
//   runnerParams.imGuiWindowParams.defaultImGuiWindowType =
//   HelloImGui::DefaultImGuiWindowType::ProvideFullScreenWindow;
//   runnerParams.imGuiWindowParams.showMenuBar = false;

//   runnerParams.callbacks.LoadAdditionalFonts = LoadFonts;
//   runnerParams.callbacks.SetupImGuiStyle = Style_Mono;
//   runnerParams.callbacks.ShowGui = []() {
//     MainGui();
//   };

//   MainInit();

//   HelloImGui::Run(runnerParams);
//   return 0;
// }

