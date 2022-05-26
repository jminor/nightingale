#include "controller.h"
#include "state.h"
#include "audio.h"

#define LUA_IMPL
#include "minilua.h"

void LoadAudio(const char* path);
void Play();
void Pause();
void Stop();
void NextTrack();
void PrevTrack();
void Seek(float time);

void SetupBindings();
void LuaBind(const char* name, int (*fn)(lua_State*));

lua_State *L = NULL;
AppState appState;

void AppInit()
{
  LuaStart();
  LuaRun("print(%d)", 6 * 7);
  SetupBindings();

  SoLoud::result err = appState.audio.init();
  if (err)
  {
    Message("AUDIO FAIL: %s", appState.audio.getErrorString(err));
    return;
  }
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

void AppCleanup()
{
  LuaEnd();
  appState.audio.stopAll();
  appState.audio.deinit();
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

int LuaStart()
 {
  L = luaL_newstate();
  if(L == NULL) {
    Log("Lua initialization failed.");
    return -1;
  }
  luaL_openlibs(L);
  return 0;
}

void LuaBind(const char* name, int (*fn)(lua_State*))
{
  lua_register(L, name, fn);
}

int LuaRun(const char* format, ...)
{
  va_list args;
  va_start(args, format);

  static char program[1024];
  int len = vsnprintf(program, sizeof(program), format, args);

  va_end(args);

  if (len > sizeof(program)) {
    Log("ERROR: Program exceeded max size (%d)", sizeof(program));
    return -1;
  }

  printf("LUA RUN: %s\n", program);
  luaL_loadstring(L, program);
  lua_call(L, 0, 0);
  
  return 0;
}

void LuaEnd()
{
  lua_close(L);
}

static int _QueueFolder(lua_State* l)
{
  const char* s = lua_tostring(l, 1);
  QueueFolder(s);
  return 0;
}

static int _LoadAudio(lua_State* l)
{
  const char* s = lua_tostring(l, 1);
  LoadAudio(s);
  return 0;
}

static int _NextTrack(lua_State* l)
{
  NextTrack();
  return 0;
}

static int _PrevTrack(lua_State* l)
{
  PrevTrack();
  return 0;
}

static int _Seek(lua_State* l)
{
  float s = lua_tonumber(l, 1);
  Seek(s);
  return 0;
}

static int _Play(lua_State* l)
{
  Play();
  return 0;
}

static int _Pause(lua_State* l)
{
  Pause();
  return 0;
}

static int _Stop(lua_State* l)
{
  Stop();
  return 0;
}

void SetupBindings()
{
  LuaBind("QueueFolder", _QueueFolder);
  LuaBind("LoadAudio", _LoadAudio);

  LuaBind("NextTrack", _NextTrack);
  LuaBind("PrevTrack", _PrevTrack);

  LuaBind("Seek", _Seek);

  LuaBind("Play", _Play);
  LuaBind("Pause", _Pause);
  LuaBind("Stop", _Stop);
}