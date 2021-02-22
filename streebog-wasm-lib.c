#include "emscripten.h"

#include "gost3411-2012-core.h"

EMSCRIPTEN_KEEPALIVE
int version() {
  return 65565;
}

