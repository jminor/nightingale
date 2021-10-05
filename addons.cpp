
#ifdef __EMSCRIPTEN__
#define WITH_SDL2_STATIC
#else
#define WITH_COREAUDIO
#endif

#define YES_IMGUISOLOUD_SPEECH

#include "imguisoloud.cpp"

#include "imgui_plot.cpp"
