// Nightingale Audio Player

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define YES_IMGUISOLOUD_MODPLUG
#include "imguisoloud.h"
#include "imguihelper.h"
#include "imgui_plot.h"
#include "imguifilesystem.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#ifdef HELLOIMGUI_USE_SDL_OPENGL3
#include <SDL.h>
#endif

void DrawAudioPanel();
void DrawButtons();

#include "app.h"

AppState appState;
ImFont *gTechFont = nullptr;
ImFont *gIconFont = nullptr;

void Log(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  fprintf(stderr, "LOG: ");
  fprintf(stderr, format, args);
  va_end(args);
}

void Message(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vsnprintf(appState.message, sizeof(appState.message), format, args);
  va_end(args);
  Log(appState.message);
}

//   Files in the application fonts/ folder are embedded automatically
//   (on iOS/Android/Emscripten)
void LoadFonts()
{
  ImGuiIO& io = ImGui::GetIO();

  // TODO: Use ImGuiFontStudio to bundle these fonts into the executable.
#ifdef EMSCRIPTEN
  Log("Skipping font loading on EMSCRIPTEN platform.");
  gTechFont = io.Fonts->AddFontDefault();
  gIconFont = gTechFont;
#else
  gTechFont = io.Fonts->AddFontFromFileTTF("fonts/ShareTechMono-Regular.ttf", 20.0f);
  static const ImWchar icon_fa_ranges[] = { 0xF000, 0xF18B, 0 };
  gIconFont = io.Fonts->AddFontFromFileTTF("fonts/fontawesome-webfont.ttf", 20.0f, NULL, icon_fa_ranges);
#endif
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
  style.ChildBorderSize = 0;
  style.FrameBorderSize = 1;

  // Based on this theme by enemymouse:
  // https://github.com/ocornut/imgui/issues/539#issuecomment-204412632
  // https://gist.github.com/enemymouse/c8aa24e247a1d7b9fc33d45091cbb8f0
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
  colors[ImGuiCol_TitleBg]                = ImVec4(0.14f, 0.18f, 0.21f, 0.78f);
  colors[ImGuiCol_TitleBgActive]          = ImVec4(0.00f, 0.54f, 0.55f, 0.78f);
  colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.78f);
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
  colors[ImGuiCol_Tab]                    = ImVec4(0.12f, 0.31f, 0.31f, 1.00f);
  colors[ImGuiCol_TabHovered]             = ImVec4(0.80f, 0.99f, 0.99f, 1.00f);
  colors[ImGuiCol_TabActive]              = ImVec4(0.00f, 0.62f, 0.62f, 1.00f);
  colors[ImGuiCol_TabUnfocused]           = ImVec4(0.08f, 0.15f, 0.15f, 1.00f);
  colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.43f, 0.43f, 1.00f);
  colors[ImGuiCol_DockingPreview]         = ImVec4(0.80f, 0.99f, 0.99f, 1.00f);
  colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_PlotLines]              = ImVec4(0.80f, 0.99f, 0.99f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_PlotHistogram]          = ImVec4(0.80f, 0.99f, 0.99f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 1.00f, 1.00f, 0.22f);
  colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
  colors[ImGuiCol_NavHighlight]           = ImVec4(0.94f, 0.98f, 0.26f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.04f, 0.10f, 0.09f, 0.51f);
}

void LoadAudio(const char* path)
{
  strncpy(appState.file_path, path, sizeof(appState.file_path));

  SoLoud::result err;
  if (!strncmp(".wav", path+strlen(path)-4, 4)) {
    err = appState.wav.load(path);
    if (!err) {
      appState.source = &appState.wav;
    }
  }else{
    err = appState.mod.load(path);
    if (!err) {
      appState.source = &appState.mod;
    }
  }
  if (err) {
    Message("Failed to load: %s: %s", appState.audio.getErrorString(err), path);
  }else{
    Message("Loaded: %s", path);
  }
}

void MainInit()
{
  Style_Mono();

  LoadFonts();

  SoLoud::result err = appState.audio.init();
  if (err) {
    Message("Failed to load initialize audio: %s", appState.audio.getErrorString(err));
  }
  // LoadAudio(""); ///Users/jminor/Library/Mobile Documents/com~apple~CloudDocs/Sokpop Sources/sokpop-source 5/nosoksky.gmx/sound/audio/sou_radio_fragment5.wav");
  appState.audio.setVisualizationEnable(true);
}

void MainCleanup()
{
  appState.audio.deinit();
}

float* GetData()
{
  if (appState.source == &appState.wav) {
    return appState.wav.mData;
  }else if (appState.source == &appState.mod) {
    return (float*)appState.mod.mData;
  }
  return 0;
}

unsigned int DataLen()
{
  if (appState.source == &appState.wav) {
    return appState.wav.mSampleCount;
  }else if (appState.source == &appState.mod) {
    return appState.mod.mDataLen / sizeof(float);
  }
  return 0;
}

float LengthInSeconds()
{
  if (appState.source == &appState.wav) {
    return appState.wav.getLength();
  }else if (appState.source == &appState.mod) {
    return appState.mod.getLength();
  }
  return 0;
}

void Seek(float time) {
  appState.playhead = time;
  if (appState.audio_handle) {
    appState.audio.setPause(appState.audio_handle, true);
  }else{
    appState.audio_handle = appState.audio.play(*appState.source, appState.volume, 0.0f, true);
    appState.audio.setLooping(appState.audio_handle, appState.loop);
  }
  appState.audio.seek(appState.audio_handle, appState.playhead);
  appState.selection_start = DataLen() * (appState.playhead / LengthInSeconds());
}

bool IconButton(const char* label, const ImVec2 size=ImVec2(0,0))
{
  ImGui::PushFont(gIconFont);
  bool result = ImGui::Button(label, size);
  ImGui::PopFont();
  return result;
}

ImU32 ImLerpColors(ImU32 col_a, ImU32 col_b, float t)
{
    int r = ImLerp((int)(col_a >> IM_COL32_R_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_R_SHIFT) & 0xFF, t);
    int g = ImLerp((int)(col_a >> IM_COL32_G_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_G_SHIFT) & 0xFF, t);
    int b = ImLerp((int)(col_a >> IM_COL32_B_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_B_SHIFT) & 0xFF, t);
    int a = ImLerp((int)(col_a >> IM_COL32_A_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_A_SHIFT) & 0xFF, t);
    return IM_COL32(r, g, b, a);
}

float qqq(float v) {
  return 1.0f - (1.0f - v) * (1.0f - v);
}

void DrawVolumeMeter(const char *label, ImVec2 size, float volume, float peak)
{
  // ImGuiIO& io = ImGui::GetIO();
  ImGuiStyle& style = ImGui::GetStyle();
  ImGuiWindow* window = ImGui::GetCurrentWindow();
  ImVec2 pos = window->DC.CursorPos; // in screen space

  ImGui::InvisibleButton(label, size);

  ImDrawList *dl = window->DrawList;
  dl->AddRect(
    pos,
    pos + size,
    ImGui::GetColorU32(ImGuiCol_Border),
    style.FrameRounding
  );

  ImU32 base_color = ImGui::GetColorU32(ImGuiCol_FrameBg);
  ImU32 volume_color = ImLerpColors(
    base_color,
    ImGui::GetColorU32(ImGuiCol_PlotHistogram),
    qqq(volume)
  );
  ImU32 peak_color = ImLerpColors(
    base_color,
    IM_COL32(0xff, 0, 0, 0x88),
    qqq(peak)
  );

  dl->AddRectFilledMultiColor(
    ImVec2(
      pos.x,
      pos.y + size.y * (1.0 - peak)
    ),
    pos + size,
    peak_color,
    peak_color,
    base_color,
    base_color
  );

  dl->AddRectFilledMultiColor(
    ImVec2(
      pos.x,
      pos.y + size.y * (1.0 - volume)
    ),
    pos + size,
    volume_color,
    volume_color,
    base_color,
    base_color
  );
}

bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f, float hover_extend=0.0f)
{
    using namespace ImGui;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
    return ImGui::SplitterBehavior(
      bb,
      id,
      split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, 
      size1, 
      size2, 
      min_size1, 
      min_size2, 
      hover_extend,
      0.0f    // hover_visibility_delay
      );
}

float limit(float t, float small, float big)
{
  return fmin(fmax(t, small), big);
}

void AppUpdate()
{
    appState.playing = appState.audio_handle && !appState.audio.getPause(appState.audio_handle);

  if (appState.audio_handle && appState.audio.isValidVoiceHandle(appState.audio_handle)) {
    appState.playhead = appState.audio.getStreamTime(appState.audio_handle);
    appState.playhead = fmodf(appState.playhead, LengthInSeconds());
  }else{
    appState.audio_handle = 0;
    appState.playhead = 0.0;
  }
  if (appState.playing) {
    appState.selection_start = DataLen() * appState.playhead / LengthInSeconds();
    // appState.selection_length = 256;
  }

}

void MainGui()
{
  AppUpdate();

  ImGuiStyle& style = ImGui::GetStyle();

  ImGuiIO& io = ImGui::GetIO();
  ImVec2 displaySize = io.DisplaySize;
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
  }else{
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(displaySize);
  }

  const char *app_name = "Nightingale";
  const char *window_id = "###MainWindow";
  char window_title[1024];
  const char *filename = strrchr(appState.file_path, '/');
  if (filename) {
    filename++;
    snprintf(window_title, sizeof(window_title), "%s - %s%s", app_name, filename, window_id);
  }else{
    snprintf(window_title, sizeof(window_title), "%s%s", app_name, window_id);
  }

  ImGui::Begin(
      window_title,
      &appState.show_main_window,
      // ImGuiWindowFlags_NoResize |
      // ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoTitleBar | 
      // ImGuiWindowFlags_NoBringToFrontOnFocus | 
      // ImGuiWindowFlags_NoDocking
      // ImGuiWindowFlags_AlwaysAutoResize
      0
      );

  if (!appState.show_main_window) {
    exit(0);
  }

  if (IconButton(appState.mini_mode ? "\uF077" : "\uF078")) {
    appState.mini_mode = !appState.mini_mode;
  }
  ImGui::SameLine();
  *(strchr(window_title, '#'))='\0';
  ImGui::Text("%s", window_title);

  // ImVec2 windowSize = ImGui::GetWindowSize();
  ImVec2 contentSize = ImGui::GetContentRegionAvail();

  if (appState.mini_mode) {
    DrawButtons();

  }else{
    float splitter_size = 2.0f;
    float w = contentSize.x - splitter_size - style.WindowPadding.x * 2;
    float h = contentSize.y - style.WindowPadding.y * 2;
    static float sz1 = 0;
    static float sz2 = 0;
    if (sz1 + sz2 != w) {
      float delta = (sz1 + sz2) - w;
      sz1 -= delta / 2;
      sz2 -= delta / 2;
    }
    Splitter(true, splitter_size, &sz1, &sz2, 8, 8, h, 8);
    ImGui::BeginChild("1", ImVec2(sz1, h), true);

    DrawAudioPanel();

    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("2", ImVec2(sz2, h), true);

    if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
    {
      if (ImGui::BeginTabItem("Nothing"))
      {
        ImGui::Text("This space intentionally left blank.");

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Theme"))
      {
        ImGui::ShowStyleEditor();

        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }

    ImGui::EndChild();
  }

  ImGui::End();

  if (appState.show_demo_window) {
    ImGui::ShowDemoWindow();
  }
}


void DrawButtons()
{
  ImGuiStyle& style = ImGui::GetStyle();
  // ImGuiIO& io = ImGui::GetIO();

  float width = ImGui::CalcItemWidth();

  int num_buttons = 5;
  ImVec2 button_size(
    (width - style.ItemSpacing.x*(num_buttons-1))/num_buttons,
    ImGui::GetTextLineHeight()*2
    );

  // toggle
  if (!appState.playing) {
    if (IconButton("\uF04B##Play", button_size)) {
      if (!appState.source) {
        // do nothing
      }else if (appState.audio_handle) {
        appState.audio.setPause(appState.audio_handle, false);
      }else{
        appState.audio_handle = appState.audio.play(*appState.source, appState.volume);
        appState.audio.setLooping(appState.audio_handle, appState.loop);
      }
    }
  }else{
    if (IconButton("\uF04C##Pause", button_size)) {
      if (appState.source && appState.audio_handle) {
        appState.audio.setPause(appState.audio_handle, true);
      }
    }
  }

  ImGui::SameLine();

  if (IconButton("\uF04D##Stop", button_size)) {
    appState.audio.stopAll();
  }

  ImGui::SameLine();
  ImGui::PushStyleVar(
    ImGuiStyleVar_FrameBorderSize, 
    appState.loop ? 2 : 1
    );
  ImGui::PushStyleColor(
    ImGuiCol_Border, 
    style.Colors[appState.loop ? ImGuiCol_NavHighlight : ImGuiCol_Border]
    );
  if (IconButton("\uF021##Loop", button_size)) {
    appState.loop = !appState.loop;
    appState.audio.setLooping(appState.audio_handle, appState.loop);
  }
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();

  ImGui::SameLine();

  const bool browseButtonPressed = IconButton("\uF07C##Load", button_size);                          // we need a trigger boolean variable
  static ImGuiFs::Dialog dlg;
  const char* chosenPath = dlg.chooseFileDialog(
    browseButtonPressed,
    dlg.getLastDirectory(),
    ".wav;.669;.abc;.amf;.ams;.dbm;.dmf;.dsm;.far;.it;.j2b;.mdl;.med;.mid;.mod;.mt2;.mtm;.okt;.pat;.psm;.ptm;.s3m;.stm;.ult;.umx;.xm",
    "Load Audio File"
  );
  if (strlen(chosenPath)>0) {
    LoadAudio(chosenPath);
  }
  ImGui::SameLine();

  // if (IconButton("\uF074##NodeGraph", button_size)) {
  //   appState.show_node_graph = !appState.show_node_graph;
  // }
  // ImGui::SameLine();

  // if (IconButton("\uF0AE##Style", button_size)) {
  //   appState.show_style_editor = !appState.show_style_editor;
  // }
  // ImGui::SameLine();

  if (IconButton("\uF013#Demo", button_size)) {
    appState.show_demo_window = !appState.show_demo_window;
  }
}

void DrawAudioPanel()
{
  // ImGuiStyle& style = ImGui::GetStyle();
  ImGuiIO& io = ImGui::GetIO();

  ImGui::PushItemWidth(-100);

  DrawButtons();

  if (ImGui::SliderFloat("Volume", &appState.volume, 0.0f, 1.0f)) {
    appState.audio.setVolume(appState.audio_handle, appState.volume);
  }

  float duration = LengthInSeconds();
  if (ImGui::SliderFloat("Playhead", &appState.playhead, 0.0f, duration)) {
    Seek(appState.playhead);
    appState.playing = false;
  }

  // auto size = ImGui::GetItemRectSize();
  float width = ImGui::CalcItemWidth();

  if (appState.source == &appState.mod) {
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
    plot_config.values.ys = GetData() + appState.selection_start;
    plot_config.values.count = appState.selection_length;
    plot_config.scale.min = -1.0f;
    plot_config.scale.max = 1.0f;
    plot_config.selection.show = false;
    // plot_config.selection.show = appState.playing;
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
  plot_config.values.ys = GetData();
  plot_config.values.count = DataLen();
  plot_config.scale.min = -1.0f;
  plot_config.scale.max = 1.0f;
  plot_config.selection.show = true;
  plot_config.selection.start = &appState.selection_start;
  plot_config.selection.length = &appState.selection_length;
  // plot_config.overlay_text = "Hello";
  if (ImGui::Plot("Data", plot_config) == ImGui::PlotStatus::selection_updated) {
    Seek(appState.selection_start * LengthInSeconds() / DataLen());
    appState.playing = false;
    appState.selection_length = fmax(appState.selection_length, 256);
  }

  ImVec2 size = ImGui::GetItemRectSize();
  ImVec2 corner = ImGui::GetItemRectMax();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    corner -= ImGui::GetWindowPos();
  }

  ImGui::SameLine();
  ImGui::Text("Waveform\nFull");

  ImGui::PopItemWidth();

  static float peak = 0;
  static float volume = 0;
  float max_sample = 0;
  float* data = appState.audio.getWave();
  for (int i=0; i<256; i++) {
    if (data[i] > max_sample) max_sample = data[i];
  }

  volume = volume * 0.9f + max_sample * 0.1f;
  peak = fmax(volume, peak - 0.001f);

  // ImGui::SetCursorPos(
  //   ImVec2(
  //     corner.x + style.ItemSpacing.x,
  //     corner.y - size.y
  //   )
  // );

  size.y *= 0.7f;
  size.x = 85;

  DrawVolumeMeter(
    "Audio Meter",
    size,
    volume,
    peak
  );

  // float dB = 20.0f * log10(volume);
  // ImGui::Text("%f dB", dB);

  ImGui::Spacing();

  // if (strlen(appState.file_path)>0) {
  //   ImGui::Text("%s", appState.file_path);
  // }else{
  //   ImGui::Text("No file loaded.");
  // }
  ImGui::Text("%s", appState.message);
}
