
// app lifecycle can be called directly
void AppInit();
void AppUpdate();
void AppCleanup();

// read-only accessors can be called directly
float LengthInSeconds();
float* GetData();
unsigned int DataLen();
int GetChannels();

// all modifications of state must go through the scripting layer
int LuaStart();
int LuaRun(const char* format, ...);
void LuaEnd();

void Log(const char* format, ...);
void Message(const char* format, ...);
