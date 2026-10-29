#include "base_arena.c"

#if LINUX
#include "linux/base/linux_base.c"
#elif WINDOWS
#include "win32/base/win32_base.c"
#endif
