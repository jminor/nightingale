
#ifdef __EMSCRIPTEN__
#define WITH_SDL2_STATIC
#endif

#ifdef __APPLE__
#define WITH_COREAUDIO
#endif

#ifdef __linux__
#define WITH_ALSA
#endif

#ifdef _WIN32
#define WITH_WINMM
#endif

#define YES_IMGUISOLOUD_SPEECH

#include "imguisoloud.cpp"
