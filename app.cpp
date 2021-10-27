// Nightingale Audio Player

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "imgui.h"

#include "audio.h"
#include "embedded_font_ShareTechMono.inc"
#include "embedded_font_fontawesome.inc"
// #include "imguihelper.h"
// #include "imgui_plot.h"
// #include "imguifilesystem.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

void DrawAudioPanel();
void DrawButtons(ImVec2 button_size);
bool LoadAudio(const char* path);
void Play();
void Pause();
void Stop();
void NextTrack();
void PrevTrack();
void Seek(float time);

#include "app.h"

AppState appState;

ImFont *gTechFont = nullptr;
ImFont *gIconFont = nullptr;

// Log a message to the terminal
void Log(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "LOG: ");
    fprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

// Display a message in the GUI (and to the terminal)
void Message(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(appState.message, sizeof(appState.message), format, args);
    va_end(args);
    Log(appState.message);
}

// Files in the application fonts/ folder are supposed to be embedded
// automatically (on iOS/Android/Emscripten), but that's not wired up.
void LoadFonts()
{
    ImGuiIO& io = ImGui::GetIO();

    //  gTechFont = io.Fonts->AddFontDefault();
    //  gIconFont = gTechFont;
    gTechFont = io.Fonts->AddFontFromMemoryCompressedBase85TTF(
                    ShareTechMono_compressed_data_base85,
                    20.0f);
    static const ImWchar icon_fa_ranges[] = { 0xF000, 0xF18B, 0 };
    gIconFont = io.Fonts->AddFontFromMemoryCompressedBase85TTF(
                    fontawesome_compressed_data_base85,
                    16.0f, NULL, icon_fa_ranges);
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

void QueueFolder(const char* folder)
{
    strncpy(appState.queue_folder, folder, sizeof(appState.queue_folder));
}

void NextTrack()
{
    // ImGuiFs::PathStringVector files;
    // ImGuiFs::DirectoryGetFiles(appState.queue_folder, files);
    // if (files.size() == 0) {
    //   return;
    // }
    // for(int i=0; i<files.size()-1; i++) {
    //   if (!strcmp(files[i], appState.file_path)) {
    //     LoadAudio(files[i+1]);
    //     return;
    //   }
    // }
    // LoadAudio(files.front());
}

void PrevTrack()
{
    // ImGuiFs::PathStringVector files;
    // ImGuiFs::DirectoryGetFiles(appState.queue_folder, files);
    // if (files.size() == 0) {
    //   return;
    // }
    // for(int i=1; i<files.size(); i++) {
    //   if (!strcmp(files[i], appState.file_path)) {
    //     LoadAudio(files[i-1]);
    //     return;
    //   }
    // }
    // LoadAudio(files.back());
}

bool LoadAudio(const char* path)
{
    Stop();

    strncpy(appState.file_path, path, sizeof(appState.file_path));

    bool success = load_audio_file(appState.file_path);

    if (!success) {
        Message("Failed to load: %s", path);
        return false;
    }else{
        Message("Loaded: %s", path);
        return true;
    }
}

void MainInit(int argc, char** argv)
{
    Style_Mono();

    LoadFonts();

    setup_audio();

    if (argc > 1) {
        LoadAudio(argv[1]);
        Play();
    }
}

void MainCleanup()
{
    Stop();
    tear_down_audio();
}

// Get the raw audio sample buffer (see also DataLen())
float* GetData()
{
    // TODO: num channels?
    return recent_data;
}

// Get the length, in samples, of the raw audio buffer (see also GetData())
unsigned int DataLen()
{
    // TODO: num channels?
    return sizeof(recent_data) / sizeof(float);
}

// How many channels are in the raw audio buffer?
int GetChannels()
{
  return num_channels();
}

// How long, in seconds, is the current audio source?
// Note that Modplug length estimate may be wrong due
// to looping, halting problem, etc.
float LengthInSeconds()
{
    if (num_samples() == 0) return 0;
    return (double)num_samples() / (double)sample_rate();
}

void Play()
{
    play_audio();
    appState.playing = true;
}

void Pause()
{
    if (appState.playing) stop_audio();
    appState.playing = false;
}

void Stop()
{
    if (appState.playing) stop_audio();
    appState.playing = false;
}

void Seek(float time)
{
    int targetFrame = time * sample_rate();
    seek_audio(targetFrame);
    appState.playhead = time;
    // if (appState.audio_handle) {
    //   appState.audio.setPause(appState.audio_handle, true);
    // }else{
    //   appState.audio_handle = appState.audio.play(*appState.source, appState.volume, 0.0f, true);
    //   appState.audio.setLooping(appState.audio_handle, appState.loop);
    // }
    // appState.audio.seek(appState.audio_handle, appState.playhead);
    // appState.selection_start = DataLen() * (appState.playhead / LengthInSeconds());
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

// The volume meter looks low most of the time, so boost it up a bit
float boost(float v) {
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
    dl->AddRect(pos,
                pos + size,
                ImGui::GetColorU32(ImGuiCol_Border),
                style.FrameRounding);

  ImU32 base_color = ImGui::GetColorU32(ImGuiCol_FrameBg);
  ImU32 volume_color = ImLerpColors(
    base_color,
    ImGui::GetColorU32(ImGuiCol_SliderGrab),
    boost(volume)
  );
  ImU32 peak_color = ImLerpColors(
    base_color,
    ImGui::GetColorU32(ImGuiCol_SliderGrab),
    //IM_COL32(0xff, 0, 0, 0x88),
    boost(peak)
  );
  ImU32 highlight_color = ImGui::GetColorU32(ImGuiCol_Border);

  float volume_y = pos.y + size.y * (1.0 - volume);
  float peak_y = pos.y + size.y * (1.0 - peak);

    dl->AddRectFilledMultiColor(ImVec2(pos.x,
                                       pos.y + size.y * (1.0 - peak)),
                                pos + size,
                                peak_color,
                                peak_color,
                                base_color,
                                base_color);

    dl->AddRectFilledMultiColor(ImVec2(pos.x,
                                       pos.y + size.y * (1.0 - volume)),
                                pos + size,
                                volume_color,
                                volume_color,
                                base_color,
                                base_color);
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
    return ImGui::SplitterBehavior(bb,
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
    // bool was_playing = appState.playing;
    // appState.playing = appState.audio_handle && !appState.audio.getPause(appState.audio_handle);

    appState.playhead = (double)current_sample_position() / (double)sample_rate();

    // if (appState.audio_handle && appState.audio.isValidVoiceHandle(appState.audio_handle)) {
    //   appState.playhead = appState.audio.getStreamTime(appState.audio_handle);
    //   appState.playhead = fmodf(appState.playhead, LengthInSeconds());
    // }else{
    //   appState.audio_handle = 0;
    //   appState.playhead = 0.0;
    //   if (was_playing && !appState.playing) {
    //     if (appState.loop) {
    //       Play();
    //     }else{
    //       NextTrack();
    //     }
    //   }
    // }
    // if (appState.playing) {
    //   appState.selection_start = DataLen() * appState.playhead / LengthInSeconds();
    // }
}

static char buffer[256];
const char* timecode_from(float t) {

  // float fraction = t - floor(t);
  t = floor(t);
  int seconds = fmodf(t, 60.0);
  int minutes = fmodf(t/60.0, 60.0);
  int hours = floor(t/3600.0);

  snprintf(
    buffer, sizeof(buffer),
    "%d:%02d:%02d",
    hours, minutes, seconds); //, (int)(fraction*100.0));

  return buffer;
}

bool MainGui()
{
    AppUpdate();

    // ImGuiStyle& style = ImGui::GetStyle();

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
    char filename[PATH_MAX] = {""};
    // ImGuiFs::PathGetFileName(appState.file_path, filename);
    if (strlen(filename)) {
        snprintf(window_title, sizeof(window_title), "%s - %s%s", app_name, filename, window_id);
    }else{
        snprintf(window_title, sizeof(window_title), "%s%s", app_name, window_id);
    }

    ImGui::Begin(window_title,
                 &appState.show_main_window,
                 // ImGuiWindowFlags_NoResize |
                 // ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoTitleBar |
                 // ImGuiWindowFlags_NoBringToFrontOnFocus |
                 // ImGuiWindowFlags_NoDocking |
                 ImGuiWindowFlags_AlwaysAutoResize |
                 0);

    if (!appState.show_main_window) {
        return false;
    }

    ImVec2 button_size = ImVec2(ImGui::GetTextLineHeightWithSpacing(),
                                ImGui::GetTextLineHeightWithSpacing());

    if (IconButton("\uF00D", button_size)) {
        return false;
    }
    ImGui::SameLine();
    if (IconButton(appState.mini_mode ? "\uF077" : "\uF078", button_size)) {
        appState.mini_mode = !appState.mini_mode;
    }

    ImGui::SameLine();
    ImGui::TextUnformatted(strlen(filename) ? filename : app_name);

    if (LengthInSeconds() > 0) {
        ImGui::SameLine();
        ImGui::Text("/ %s", timecode_from(appState.playhead));

        ImGui::SameLine();
        ImGui::Text("/ %s", timecode_from(LengthInSeconds()));
    }

    ImGui::SameLine();
    DrawButtons(button_size);

    // ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - button_size.x + style.ItemSpacing.x);

    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    if (contentSize.y < 500) contentSize.y = 500;

    if (appState.mini_mode) {

    }else{
        // float splitter_size = 2.0f;
        // float w = contentSize.x - splitter_size - style.WindowPadding.x * 2;
        // float h = contentSize.y - style.WindowPadding.y * 2;
        // static float sz1 = 0;
        // static float sz2 = 0;
        // if (sz1 + sz2 != w) {
        //   float delta = (sz1 + sz2) - w;
        //   sz1 -= delta / 2;
        //   sz2 -= delta / 2;
        // }
        // Splitter(true, splitter_size, &sz1, &sz2, 8, 8, h, 8);
        // ImGui::BeginChild("1", ImVec2(sz1, h), true);

        // DrawAudioPanel();

        // ImGui::EndChild();
        // ImGui::SameLine();
        // ImGui::BeginChild("2", ImVec2(sz2, h), true);

        if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Audio"))
            {
                DrawAudioPanel();

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Theme"))
            {
                ImGui::ShowStyleEditor();

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        // ImGui::EndChild();
    }

    ImGui::End();

    if (appState.show_demo_window) {
        ImGui::ShowDemoWindow();
    }

    return true;
}


void DrawButtons(ImVec2 button_size)
{
    ImGuiStyle& style = ImGui::GetStyle();

    if (IconButton("\uF048##Prev", button_size)) {
        PrevTrack();
    }

    ImGui::SameLine();
    // toggle
    if (!appState.playing) {
        if (IconButton("\uF04B##Play", button_size)) {
            Play();
        }
    }else{
        if (IconButton("\uF04C##Pause", button_size)) {
            Pause();
        }
    }

    ImGui::SameLine();
    if (IconButton("\uF04D##Stop", button_size)) {
        Stop();
    }

    ImGui::SameLine();
    if (IconButton("\uF051##Next", button_size)) {
        NextTrack();
    }

  ImGui::SameLine();
  ImGui::PushStyleVar(
    ImGuiStyleVar_FrameBorderSize, 
    appState.loop ? 2 : 1
    );
  ImGui::PushStyleColor(
    ImGuiCol_Border, 
    style.Colors[appState.loop ? ImGuiCol_PlotLines : ImGuiCol_Border]
    );
  if (IconButton("\uF021##Loop", button_size)) {
    appState.loop = !appState.loop;
    appState.audio.setLooping(appState.audio_handle, appState.loop);
  }
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();

    ImGui::SameLine();

    const bool browseButtonPressed = IconButton("\uF07C##Load", button_size);                          // we need a trigger boolean variable
    // static ImGuiFs::Dialog dlg;
    // const char* chosenPath = dlg.chooseFileDialog(
    //   browseButtonPressed,
    //   dlg.getLastDirectory(),
    //   // ".wav;.669;.abc;.amf;.ams;.dbm;.dmf;.dsm;.far;.it;.j2b;.mdl;.med;.mid;.mod;.mt2;.mtm;.okt;.pat;.psm;.ptm;.s3m;.stm;.ult;.umx;.xm",
    //   ".wav;.mp3;.flac",
    //   "Load Audio File (wav, mp3, flac)"
    // );
    // if (strlen(chosenPath)>0) {
    //   LoadAudio(chosenPath);
    //   QueueFolder(dlg.getLastDirectory());
    // }
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

void ComputeAndDrawVolumeMeter(ImVec2 size)
{
  static float peak = 0;
  static float volume = 0;
  float max_sample = 0;
  float* data = appState.audio.getWave();
  for (int i=0; i<256; i++) {
    if (data[i] > max_sample) max_sample = data[i];
  }

  volume = volume * 0.9f + max_sample * 0.1f;
  peak = fmax(volume, peak - 0.001f);

  DrawVolumeMeter(
    "Audio Meter",
    size,
    volume,
    peak
  );

  // float dB = 20.0f * log10(volume);
  // ImGui::Text("%f dB", dB);

}

void DrawAudioPanel()
{
    // ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();

  ImGui::BeginGroup();

  float duration = LengthInSeconds();
  if (ImGui::SliderFloat("POS", &appState.playhead, 0.0f, duration, timecode_from(appState.playhead))) {
    Seek(appState.playhead);
  }

  float width = ImGui::CalcItemWidth();

  {
    ImGui::PlotConfig plot_config;
    plot_config.frame_size = ImVec2(width, 100);
    plot_config.values.ys = GetData();
    plot_config.values.count = DataLen();
    plot_config.scale.min = -1.0f;
    plot_config.scale.max = 1.0f;
    plot_config.selection.show = true;
    plot_config.selection.start = &appState.selection_start;
    plot_config.selection.length = &appState.selection_length;
    // plot_config.overlay_text = "Hello";
    if (ImGui::Plot("DAT", plot_config) == ImGui::PlotStatus::selection_updated) {
      Seek(appState.selection_start * LengthInSeconds() / DataLen());
      appState.selection_length = fmax(appState.selection_length, 256);
    }

    // ImVec2 size = ImGui::GetItemRectSize();
    ImVec2 corner = ImGui::GetItemRectMax();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      corner -= ImGui::GetWindowPos();
    }

    ImGui::SameLine(0,style.ItemInnerSpacing.x);
    ImGui::Text("DAT");
  }

  ImGui::SameLine();

  if (KnobFloat("VOL", &appState.volume, 0.01f, 0.0f, 1.0f, "%.2f")) {
    appState.audio.setVolume(appState.audio_handle, appState.volume);
  }

  // if (ImGui::SliderFloat("Volume", &appState.volume, 0.0f, 1.0f, "%.2f")) {
  //   appState.audio.setVolume(appState.audio_handle, appState.volume);
  // }

  float duration = LengthInSeconds();
  if (ImGui::SliderFloat("POS", &appState.playhead, 0.0f, duration)) {
    Seek(appState.playhead);
    appState.playing = false;
  }

    if (appState.playing) {
        // ImGui::SetMaxWaitBeforeNextFrame(1.0 / 30.0); // = 30fps
    }

    if (ImGui::SliderFloat("Volume", &appState.volume, 0.0f, 1.0f)) {
        // appState.audio.setVolume(appState.audio_handle, appState.volume);
    }

    float duration = LengthInSeconds();
    if (ImGui::SliderFloat("Playhead", &appState.playhead, 0.0f, duration+1)) {
        Seek(appState.playhead);
        // appState.playing = false;
    }

    // auto size = ImGui::GetItemRectSize();
    float width = ImGui::CalcItemWidth();

    if (false) { //appState.source == &appState.wav) {
        // ImGui::PlotConfig plot_config;
        // plot_config.frame_size = ImVec2(width, 100);
        // plot_config.values.ys = GetData() + appState.selection_start;
        // plot_config.values.count = appState.selection_length;
        // plot_config.scale.min = -1.0f;
        // plot_config.scale.max = 1.0f;
        // plot_config.selection.show = false;
        // // plot_config.selection.show = appState.playing;
        // // uint32_t start = 0, length = 256;
        // // plot_config.selection.start = &start;
        // // plot_config.selection.length = &length;
        // ImGui::Plot("Waveform", plot_config);
        // ImGui::SameLine();
        // ImGui::Text("Waveform\nDetail");
    }else{
        // this shows the mixed output waveform
        ImGui::PlotLines(
                         "Live Waveform",
                         GetData(), //appState.audio.getWave(),
                         DataLen() / GetChannels(),  // values_count
                         0,    // values_offset
                         nullptr, // overlay_text
                         -1.0f, // scale_min
                         1.0f, // scale_max
                         ImVec2(width,100), // graph_size
                         sizeof(float) * GetChannels()
                         );
    }
    // ImGui::PlotHistogram(
    //   "FFT",
    //   appState.audio.calcFFT(),
    //   256,  // values_count
    //   0,    // values_offset
    //   nullptr, // overlay_text
    //   FLT_MAX, // scale_min
    //   FLT_MAX, // scale_max
    //   ImVec2(width,100) // graph_size
    //   );

    // ImGui::PlotConfig plot_config;
    // plot_config.frame_size = ImVec2(width, 100);
    // plot_config.values.ys = GetData();
    // plot_config.values.count = DataLen();
    // plot_config.scale.min = -1.0f;
    // plot_config.scale.max = 1.0f;
    // plot_config.selection.show = true;
    // plot_config.selection.start = &appState.selection_start;
    // plot_config.selection.length = &appState.selection_length;
    // // plot_config.overlay_text = "Hello";
    // if (ImGui::Plot("Data", plot_config) == ImGui::PlotStatus::selection_updated) {
    //   Seek(appState.selection_start * LengthInSeconds() / DataLen());
    //   appState.playing = false;
    //   appState.selection_length = fmax(appState.selection_length, 256);
    // }

    ImVec2 size = ImGui::GetItemRectSize();
    ImVec2 corner = ImGui::GetItemRectMax();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        corner -= ImGui::GetWindowPos();
    }

    ImGui::SameLine();
    ImGui::Text("Waveform\nFull");

    // ImGui::PopItemWidth();

    static float peak = 0;
    static float volume = 0;
    float max_sample = 0;
    float* data = GetData(); //appState.audio.getWave();
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

    DrawVolumeMeter("Audio Meter",
                    size,
                    volume,
                    peak);

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
