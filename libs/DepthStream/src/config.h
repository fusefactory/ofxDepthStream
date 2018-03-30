#pragma once

#if !defined(__APPLE__) && !defined(WIN32)
  #define OPENNI_AVAILABLE
#endif

#define CHUNK 16384

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif
