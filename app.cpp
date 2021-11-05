// Nightingale Audio Player

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "imgui.h"

#include "audio.h"
#include "widgets.h"
#include "imguihelper.h"
#include "imgui_plot.h"
#include "imguifilesystem.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#ifdef HELLOIMGUI_USE_SDL_OPENGL3
#include <SDL.h>
#endif

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

bool LoadTexture(const char *path, ImTextureID *tex_id, ImVec2 *size);
bool MakeTexture(const unsigned char *pixels, ImTextureID *tex_id, ImVec2 size);
void DestroyTexture(ImTextureID *tex_id);

void DrawAudioPanel();
void DrawButtons(ImVec2 button_size);

void LoadFonts();

void LoadAudio(const char* path);
void Play();
void Pause();
void Stop();
void NextTrack();
void PrevTrack();
void Seek(float time);

bool OpenVideo(const char *filename);
void CloseVideo();

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

  // TODO: Use ImGuiFontStudio to bundle these fonts into the executable?
#ifdef EMSCRIPTEN
  Log("Skipping font loading on EMSCRIPTEN platform.");
  gTechFont = io.Fonts->AddFontDefault();
  gIconFont = gTechFont;
#else
  gTechFont = io.Fonts->AddFontFromFileTTF("fonts/ShareTechMono-Regular.ttf", 20.0f);
  static const ImWchar icon_fa_ranges[] = { 0xF000, 0xF18B, 0 };
  gIconFont = io.Fonts->AddFontFromFileTTF("fonts/fontawesome-webfont.ttf", 16.0f, NULL, icon_fa_ranges);
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

void QueueFolder(const char* folder)
{
  strncpy(appState.queue_folder, folder, sizeof(appState.queue_folder));
}

void NextTrack()
{
  ImGuiFs::PathStringVector files;
  ImGuiFs::DirectoryGetFiles(appState.queue_folder, files);
  // If there's no files in there, we're done.
  if (files.size() == 0) {
    return;
  }
  // Look through all but the last one...
  for(int i=0; i<files.size()-1; i++) {
    // If it matches the one we're on currently...
    if (!strcmp(files[i], appState.file_path)) {
      // Load the next one.
      LoadAudio(files[i+1]);
      return;
    }
  }
  // Didn't find it, so just pick the first one.
  // This works for wrap around, or if the file is not found.
  LoadAudio(files.front());
}

void PrevTrack()
{
  ImGuiFs::PathStringVector files;
  ImGuiFs::DirectoryGetFiles(appState.queue_folder, files);
  if (files.size() == 0) {
    return;
  }
  // Same strategy as NextTrack, but look at all but the first one.
  for(int i=1; i<files.size(); i++) {
    if (!strcmp(files[i], appState.file_path)) {
      LoadAudio(files[i-1]);
      return;
    }
  }
  LoadAudio(files.back());
}

void LoadAudio(const char* path)
{
  // Stop playing, otherwise the songs double-up
  Stop();

  appState.source = NULL;
  appState.audio_handle = 0;

  strncpy(appState.file_path, path, sizeof(appState.file_path));

  if (strlen(path) < 4) {
    Message("Invalid file path: %s", path);
    return;
  }

  SoLoud::result err=0;
  const char* ext = path+strlen(path) - 4;
  if (!strncmp(".mp3", ext, 4)) {
    err = appState.mp3.load(path);
    if (!err) {
      appState.source = &appState.mp3;
    }
  }else if (!strncmp(".wav", ext, 4)) {
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
    Message("FAIL: %s: %s", appState.audio.getErrorString(err), path);
  }else{
    // Message("LOAD: %s", path);
    Play();
  }
}

void MainInit(int argc, char**argv)
{
  Style_Mono();

  LoadFonts();

  SoLoud::result err = appState.audio.init();
  if (err) {
    Message("AUDIO FAIL: %s", appState.audio.getErrorString(err));
    return;
  }

  if (argc > 1) {
    LoadAudio(argv[1]);
    OpenVideo(argv[1]);
  }

  // QueueFolder("/Users/jminor/git/nightingale/audio");
  // NextTrack();
}

void MainCleanup()
{
  appState.audio.stopAll();
  appState.audio.deinit();
}

// Get the raw audio sample buffer (see also DataLen())
float* GetData()
{
  if (appState.source == &appState.wav) {
    return appState.wav.mData;
  }else if (appState.source == &appState.mod) {
    return (float*)appState.mod.mData;
  }else if (appState.source == &appState.mp3) {
    return appState.mp3.mSampleData;
  }
  return 0;
}

// Get the length, in samples, of the raw audio buffer (see also GetData())
unsigned int DataLen()
{
  if (appState.source == &appState.wav) {
    return appState.wav.mSampleCount;
  }else if (appState.source == &appState.mod) {
    return appState.mod.mDataLen / sizeof(float);
  }else if (appState.source == &appState.mp3) {
    return appState.mp3.mSampleCount;
  }
  return 0;
}

// How many channels are in the raw audio buffer?
int GetChannels()
{
  if (appState.source) {
    return appState.source->mChannels;
  }
  return 1;
}

// How long, in seconds, is the current audio source?
// Note that Modplug length estimate may be wrong due
// to looping, halting problem, etc.
float LengthInSeconds()
{
  if (appState.source == &appState.wav) {
    return appState.wav.getLength();
  }else if (appState.source == &appState.mod) {
    return appState.mod.getLength();
  }else if (appState.source == &appState.mp3) {
    return appState.mp3.getLength();
  }
  return 0;
}

void Play()
{
  if (!appState.source) {
    // do nothing
  }else if (appState.audio_handle) {
    appState.audio.setPause(appState.audio_handle, false);
    appState.playing = true;
  }else{
    appState.audio_handle = appState.audio.play(*appState.source, appState.volume);
    appState.audio.setLooping(appState.audio_handle, appState.loop);
    appState.playing = true;
  }
}

void Pause()
{
  if (appState.audio_handle) {
    appState.audio.setPause(appState.audio_handle, true);
  }
  appState.playing = false;
}

void Stop()
{
  Pause();
  Seek(0);
  // Don't clear the source or audio_handle, so we can hit Play() again.
  // appState.audio.stopAll();
  // Log("Stopped");
}

void Seek(float time)
{
  // Don't stop, just seek
  appState.playhead = time;
  appState.audio.seek(appState.audio_handle, appState.playhead);
  // Move the selection
  appState.selection_start = DataLen() * (appState.playhead / LengthInSeconds());
}

// Make a button using the fancy icon font
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

  dl->AddRectFilledMultiColor(
    ImVec2(
      pos.x,
      peak_y
    ),
    pos + size,
    peak_color,
    peak_color,
    base_color,
    base_color
  );

  dl->AddLine(
    ImVec2(
      pos.x,
      peak_y
    ),
    ImVec2(
      pos.x + size.x,
      peak_y
    ),
    highlight_color,
    2.0f
  );

  dl->AddRectFilledMultiColor(
    ImVec2(
      pos.x,
      volume_y
    ),
    pos + size,
    volume_color,
    volume_color,
    base_color,
    base_color
  );

  dl->AddLine(
    ImVec2(
      pos.x,
      volume_y
    ),
    ImVec2(
      pos.x + size.x,
      volume_y
    ),
    highlight_color,
    2.0f
    );
}

void AppUpdate()
{
  // Ask the audio system if we're still playing
  bool valid_handle = appState.audio_handle && appState.audio.isValidVoiceHandle(appState.audio_handle);
  bool should_be_playing = appState.playing;
  bool actually_playing = valid_handle && !appState.audio.getPause(appState.audio_handle);

  if (should_be_playing && !actually_playing) {
    // No longer playing
    appState.playing = false;
  }

  if (valid_handle) {
    appState.playhead = appState.audio.getStreamTime(appState.audio_handle);
    // Deal with looping (getStreamTime just keeps going beyond the song duration)
    appState.playhead = fmodf(appState.playhead, LengthInSeconds());
  }else{
    // The song stopped, so forget the handle
    appState.audio_handle = 0;
    appState.playhead = 0.0;
    // Did playback stop suddenly, like we hit the end of the song?
    if (should_be_playing && !actually_playing) {
      if (appState.loop) {
        // Log("Looping");
        Play();
      }else{
        // Log("Skipping to next track");
        NextTrack();
      }
    }
  }
  if (actually_playing) {
    appState.selection_start = DataLen() * appState.playhead / LengthInSeconds();
  }
}

AVFormatContext *fmt_ctx = NULL;
AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx;
int width, height;
enum AVPixelFormat pix_fmt;
AVStream *video_stream = NULL, *audio_stream = NULL;
const char *video_dst_filename = NULL;
const char *audio_dst_filename = NULL;
int video_stream_idx = -1, audio_stream_idx = -1;
int video_frame_count = 0;
int audio_frame_count = 0;

struct SwsContext *sws_ctx;
uint8_t *video_dst_data[4] = {NULL};
int      video_dst_linesize[4];
int video_dst_bufsize;

AVPacket *av_packet = NULL;
AVFrame *av_frame = NULL;


void CloseVideo()
{
  avcodec_free_context(&video_dec_ctx);
  avcodec_free_context(&audio_dec_ctx);
  avformat_close_input(&fmt_ctx);
  av_packet_free(&av_packet);
  av_frame_free(&av_frame);
  av_free(video_dst_data[0]);
  sws_freeContext(sws_ctx);
  sws_ctx = NULL;
}


static int open_codec_context(
  int *stream_idx,
  AVCodecContext **dec_ctx, 
  AVFormatContext *fmt_ctx, 
  enum AVMediaType type
  )
{
  int ret, stream_index;
  AVStream *st;
  const AVCodec *dec = NULL;

  ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
  if (ret < 0) {
    fprintf(stderr, "Could not find %s stream in input file\n",
      av_get_media_type_string(type));
    return ret;
  } else {
    stream_index = ret;
    st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
    dec = avcodec_find_decoder(st->codecpar->codec_id);
    if (!dec) {
      fprintf(stderr, "Failed to find %s codec\n",
        av_get_media_type_string(type));
      return AVERROR(EINVAL);
    }

        /* Allocate a codec context for the decoder */
    *dec_ctx = avcodec_alloc_context3(dec);
    if (!*dec_ctx) {
      fprintf(stderr, "Failed to allocate the %s codec context\n",
        av_get_media_type_string(type));
      return AVERROR(ENOMEM);
    }

        /* Copy codec parameters from input stream to output codec context */
    if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
      fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
        av_get_media_type_string(type));
      return ret;
    }

        /* Init the decoders */
    if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0) {
      fprintf(stderr, "Failed to open %s codec\n",
        av_get_media_type_string(type));
      return ret;
    }
    *stream_idx = stream_index;
  }

  return 0;
}


static int output_video_frame(AVFrame *frame, VideoFrame &video_frame)
{
    // printf("video_frame n:%d coded_n:%d\n",
    //        video_frame_count++, frame->coded_picture_number);

    // printf("video_dst_data %p %p %p %p\n",
    //        video_dst_data[0], video_dst_data[1], video_dst_data[2], video_dst_data[3]);

    // printf("video_dst_linesize %d %d %d %d\n",
    //        video_dst_linesize[0], video_dst_linesize[1], video_dst_linesize[2], video_dst_linesize[3]);

    // printf("frame %d x %d linesize %d %d %d %d pix_fmt:%d %s\n",
    //        frame->width, frame->height,
    //        frame->linesize[0], frame->linesize[1], frame->linesize[2], frame->linesize[3],
    //        frame->format, av_get_pix_fmt_name((AVPixelFormat)frame->format));

    if (sws_ctx) {
      sws_scale(
        sws_ctx,
        frame->data, frame->linesize,
        0, frame->height,
        video_dst_data, video_dst_linesize
        );
    }else{
      /* copy decoded frame to destination buffer:
       * this is required since rawvideo expects non aligned data */
      av_image_copy(video_dst_data, video_dst_linesize,
                    (const uint8_t **)(frame->data), frame->linesize,
                    pix_fmt, frame->width, frame->height);
    }

    video_frame.size = ImVec2(frame->width, frame->height);
    MakeTexture(video_dst_data[0], &video_frame.texture, video_frame.size);
    // MakeTexture(frame->data[0], &video_frame.texture, video_frame.size);

    return 0;
}

static int output_audio_frame(AVFrame *frame)
{
    // size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample(frame->format);
    // printf("audio_frame n:%d nb_samples:%d pts:%s\n",
    //        audio_frame_count++, frame->nb_samples,
    //        av_ts2timestr(frame->pts, &audio_dec_ctx->time_base));

    /* Write the raw audio data samples of the first plane. This works
     * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
     * most audio decoders output planar audio, which uses a separate
     * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
     * In other words, this code will write only the first audio channel
     * in these cases.
     * You should use libswresample or libavfilter to convert the frame
     * to packed data. */
    // fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);

    return 0;
}

bool OpenVideo(const char *filename)
{
  int ret = 0;

  /* open input file, and allocate format context */
  if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) < 0) {
    fprintf(stderr, "Could not open source file %s\n", filename);
    CloseVideo();
    return false;
  }

  /* retrieve stream information */
  if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
    fprintf(stderr, "Could not find stream information\n");
    CloseVideo();
    return false;
  }

  if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
    video_stream = fmt_ctx->streams[video_stream_idx];

    /* allocate image where the decoded image will be put */
    width = video_dec_ctx->width;
    height = video_dec_ctx->height;
    pix_fmt = AV_PIX_FMT_RGBA; //video_dec_ctx->pix_fmt; // AV_PIX_FMT_RGBA
    fprintf(stderr, "pixel format: %d = %s\n", pix_fmt, av_get_pix_fmt_name(pix_fmt));
    ret = av_image_alloc(
      video_dst_data, 
      video_dst_linesize,
      width, 
      height, 
      pix_fmt, 
      1
      );
    if (ret < 0) {
      fprintf(stderr, "Could not allocate raw video buffer\n");
      CloseVideo();
      return false;
    }
    video_dst_bufsize = ret;

    // we're going to need to convert this after decoding...
    if (pix_fmt != video_dec_ctx->pix_fmt) {
      sws_ctx = sws_getContext(
        width, height, video_dec_ctx->pix_fmt,
        width, height, pix_fmt,
        SWS_BILINEAR, NULL, NULL, NULL);
      if (!sws_ctx) {
        fprintf(stderr,
                  "Impossible to create scale context for the conversion "
                  "fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
                  av_get_pix_fmt_name(video_dec_ctx->pix_fmt), width, height,
                  av_get_pix_fmt_name(pix_fmt), width, height);
        CloseVideo();
        return false;
      }
    }
  }

  if (open_codec_context(&audio_stream_idx, &audio_dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
    audio_stream = fmt_ctx->streams[audio_stream_idx];
  }

  /* dump input information to stderr */
  av_dump_format(fmt_ctx, 0, filename, 0);

  if (!audio_stream && !video_stream) {
    fprintf(stderr, "Could not find audio or video stream in the input, aborting\n");
    CloseVideo();
    return false;
  }

  av_frame = av_frame_alloc();
  if (!av_frame) {
    fprintf(stderr, "Could not allocate frame\n");
    CloseVideo();
    return false;
  }

  av_packet = av_packet_alloc();
  if (!av_packet) {
    fprintf(stderr, "Could not allocate packet\n");
    CloseVideo();
    return false;
  }

  return true;
}

static int decode_packet(AVCodecContext *dec, const AVPacket *pkt, VideoFrame &video_frame)
{
  int ret = 0;

  // submit the packet to the decoder
  ret = avcodec_send_packet(dec, pkt);
  if (ret < 0) {
    fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
    return ret;
  }

  // get all the available frames from the decoder
  while (ret >= 0) {
    ret = avcodec_receive_frame(dec, av_frame);
    if (ret < 0) {
      // those two return values are special and mean there is no output
      // frame available, but there were no errors during decoding
      if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
        return 0;

      fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
      return ret;
    }

    // send the frame data
    if (dec->codec->type == AVMEDIA_TYPE_VIDEO) {
      ret = output_video_frame(av_frame, video_frame);
    }else{
      ret = output_audio_frame(av_frame);
    }

    av_frame_unref(av_frame);
    if (ret < 0) {
      return ret;
    }
  }

  return 0;
}

bool LoadNextVideoFrame(VideoFrame &video_frame)
{
  int ret = 0;

  if (fmt_ctx == NULL) {
    if (!OpenVideo(appState.file_path)) {
      return false;
    }
  }

  while (av_read_frame(fmt_ctx, av_packet) >= 0) {
        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
    if (av_packet->stream_index == video_stream_idx) {
      ret = decode_packet(video_dec_ctx, av_packet, video_frame);
      if (ret == 0) {
        // we got one frame, return now
        av_packet_unref(av_packet);
        return true;
      }
    }
    else if (av_packet->stream_index == audio_stream_idx) {
      // ret = decode_packet(audio_dec_ctx, av_packet);
    }
    av_packet_unref(av_packet);
    if (ret < 0)
      break;
  }

    // flush the decoders
  // if (video_dec_ctx)
  //   decode_packet(video_dec_ctx, NULL);
  // if (audio_dec_ctx)
  //   decode_packet(audio_dec_ctx, NULL);

  return false;
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

void DrawImage(ImTextureID tex_id, ImVec2 size)
{
  ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
  ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
  ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
  ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
  ImGui::Image(tex_id, size, uv_min, uv_max, tint_col, border_col);
}

void MainGui()
{
  AppUpdate();

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
  char filename[ImGuiFs::MAX_FILENAME_BYTES] = {""};
  ImGuiFs::PathGetFileName(appState.file_path, filename);
  if (strlen(filename)) {
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
      // ImGuiWindowFlags_NoDocking |
      ImGuiWindowFlags_AlwaysAutoResize |
      0
      );

  if (!appState.show_main_window) {
    MainCleanup();
    exit(0);
  }

  ImVec2 button_size = ImVec2(
    ImGui::GetTextLineHeightWithSpacing(),
    ImGui::GetTextLineHeightWithSpacing()
    );

  if (IconButton("\uF00D", button_size)) {
    MainCleanup();
    exit(0);
  }
  ImGui::SameLine();
  if (IconButton(appState.mini_mode ? "\uF077" : "\uF078", button_size)) {
    appState.mini_mode = !appState.mini_mode;
  }

  ImGui::SameLine();
  ImGui::Text("%s", strlen(filename) ? filename : app_name);

  if (strlen(filename)) {

    ImGui::SameLine();
    ImGui::Text("/");

    ImGui::SameLine();
    ImGui::Text("%s", timecode_from(appState.playhead));
    ImGui::SameLine();
    ImGui::Text("/");
    ImGui::SameLine();
    ImGui::Text("%s", timecode_from(LengthInSeconds()));
  }

  ImGui::SameLine();
  DrawButtons(button_size);

  static int frame=1;
  static int loaded_frames=0;
  if (frame >= appState.num_frames) {
    frame = appState.loop_start;
  }
  if (appState.loop_end > appState.loop_start && frame > appState.loop_end) {
    frame = appState.loop_start;
  }
  VideoFrame &video_frame = appState.frames[frame];
  if (video_frame.texture == 0) {
    // fprintf(stderr, "Loading frame %d\n", loaded_frames);
    if (LoadNextVideoFrame(video_frame)) {
      loaded_frames++;
    }else{
      appState.loop_end = frame-1;
    }
  }

  static float fps = 30.0;

  static double last_frame_time = ImGui::GetTime();
  double now = ImGui::GetTime();
  // double desired_fps = 60.0;
  // double desired_frame_duration = 1.0 / desired_fps;
  double duration_of_last_frame = now - last_frame_time;
  // double last_frame_error = duration_of_last_frame - desired_frame_duration;
  // double waiting_time = fmin(desired_frame_duration, desired_frame_duration - last_frame_error);
  // ImGui::SetMaxWaitBeforeNextFrame(fmax(0, waiting_time));
  // Conclusion: SetMaxWaitBeforeNextFrame is not suitable for playback rate control.

  float waiting_time = 1.0/fps;
  frame++;
  last_frame_time = now;

  if (video_frame.texture) {
    DrawImage(video_frame.texture, video_frame.size / 2);
  }
  ImGui::Text("Frame %d of %d @ %.1f wait %0.3f dur %0.3f", frame, loaded_frames, io.Framerate, waiting_time, duration_of_last_frame);

  ImGui::SliderInt("Frame", &frame, 1, appState.num_frames);
  ImGui::DragIntRange2("Loop", &appState.loop_start, &appState.loop_end, 1.0f, 1, appState.num_frames);

  ImGui::SliderFloat("FPS", &fps, 1.0, 120.0);
  ImGui::SetMaxWaitBeforeNextFrame(waiting_time);

  // ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - button_size.x + style.ItemSpacing.x);

  ImVec2 contentSize = ImGui::GetContentRegionAvail();
  if (contentSize.y < 500) contentSize.y = 500;

  if (appState.mini_mode) {
    appState.audio.setVisualizationEnable(false);

    if (appState.playing) {
      // for the time counter
      ImGui::SetMaxWaitBeforeNextFrame(1.0);
    }

  }else{
    appState.audio.setVisualizationEnable(true);

    if (appState.playing) {
      // for the waveforms, etc.
      ImGui::SetMaxWaitBeforeNextFrame(1.0 / 30.0);
    }

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
      if (ImGui::BeginTabItem("AUD"))
      {
        DrawAudioPanel();

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("THM"))
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
  static ImGuiFs::Dialog dlg;
  // ImGui::SetNextWindowPos(ImVec2(300,300));
  const char* chosenPath = dlg.chooseFileDialog(
    browseButtonPressed,
    dlg.getLastDirectory(),
    NULL,
    // ".mp3;.wav;.669;.abc;.amf;.ams;.dbm;.dmf;.dsm;.far;.it;.j2b;.mdl;.med;.mid;.mod;.mt2;.mtm;.okt;.pat;.psm;.ptm;.s3m;.stm;.ult;.umx;.xm",
    "Load Audio File"
  );
  if (strlen(chosenPath)>0) {
    LoadAudio(chosenPath);
    CloseVideo();
    OpenVideo(chosenPath);
    QueueFolder(dlg.getLastDirectory());
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
  ImGuiStyle& style = ImGui::GetStyle();
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

  if (appState.source == &appState.wav || appState.source == &appState.mp3) {
    ImGui::PlotLines(
      "WAV",
      GetData() + appState.selection_start,
      appState.selection_length / GetChannels(),  // values_count
      0,    // values_offset
      nullptr, // overlay_text
      -1.0f, // scale_min
      1.0f, // scale_max
      ImVec2(width,100), // graph_size
      sizeof(float)*GetChannels() // stride
      );
    uint32_t low = 256;
    uint32_t high = appState.source->mBaseSamplerate;
    ImGui::SameLine();
    KnobScalar("##WAV", ImGuiDataType_U32, &appState.selection_length, 100.0f, &low, &high, "%d", 0);
  }else{
    // this shows the mixed output waveform
    ImGui::PlotLines(
      "WAV",
      appState.audio.getWave(),
      256,  // values_count
      0,    // values_offset
      nullptr, // overlay_text
      -1.0f, // scale_min
      1.0f, // scale_max
      ImVec2(width,100) // graph_size
      );
  }
  static int fft_zoom = 256;
  ImGui::PlotHistogram(
    "FFT",
    appState.audio.calcFFT(),
    fft_zoom,  // values_count
    0,    // values_offset
    nullptr, // overlay_text
    FLT_MAX, // scale_min
    FLT_MAX, // scale_max
    ImVec2(width,100) // graph_size
    );
  ImGui::SameLine();
  KnobInt("##FFT", &fft_zoom, 1.0f, 16, 256);
  // ImGui::SliderInt("Z##FFT", &fft_zoom, 16, 256);

  ImGui::EndGroup();

  ImVec2 group_size = ImGui::GetItemRectSize();

  ImGui::SameLine();
  ComputeAndDrawVolumeMeter(ImVec2(50, group_size.y));

  // ImGui::SameLine();
  // if (ImGui::VSliderFloat("##VOL", ImVec2(25, group_size.y), &appState.volume, 0.0f, 1.0f, "")) {
  //   appState.audio.setVolume(appState.audio_handle, appState.volume);
  // }

  ImGui::Spacing();

  // if (strlen(appState.file_path)>0) {
  //   ImGui::Text("%s", appState.file_path);
  // }else{
  //   ImGui::Text("No file loaded.");
  // }
  ImGui::Text("%s", appState.message);
}
