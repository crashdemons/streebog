#include "emscripten.h"

#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "gost3411-2012-core.h"

/* For benchmarking */
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>



static void *
memalloc(const size_t size)
{
    void *p;

    /* Ensure p is on a 64-bit boundary. */ 
    if (posix_memalign(&p, (size_t) 64, size))
        err(EX_OSERR, NULL);

    return p;
}

EMSCRIPTEN_KEEPALIVE
int version() {
  return 7;
}


EMSCRIPTEN_KEEPALIVE
void* create_buffer(size_t size){
	return memalloc(size);
}

EMSCRIPTEN_KEEPALIVE
void destroy_buffer(void* buffer){
	free(buffer);
}

EMSCRIPTEN_KEEPALIVE
GOST34112012Context* streebog_init(const unsigned int digest_size){
	GOST34112012Context* CTX = memalloc(sizeof(GOST34112012Context));
	GOST34112012Init(CTX, digest_size);
	return CTX;
}

EMSCRIPTEN_KEEPALIVE
void streebog_update(GOST34112012Context* CTX, const unsigned char *data, size_t len){
	GOST34112012Update(CTX, data, len);
}

EMSCRIPTEN_KEEPALIVE
void streebog_final(GOST34112012Context *CTX, unsigned char *digest){
	GOST34112012Final(CTX, digest);
}

EMSCRIPTEN_KEEPALIVE
void streebog_cleanup(GOST34112012Context *CTX){
	GOST34112012Cleanup(CTX);
	free(CTX);
}











