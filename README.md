GOST R 34.11-2012 hash function with 512/256 bit digest / WebAssembly
=======================================================

This is work-in-progress rebuild of GOST R 34.11-2012 hash function ("Streebog") for WebAssembly using Emscripten.

A live example is available at https://crashdemons.github.io/streebog-wasm/

The standard for this hash function developed by the Center for
Information Protection and Special Communications of the Federal
Security Service of the Russian Federation with participation of the
Open joint-stock company "Information Technologies and Communication
Systems" (InfoTeCS JSC).

The standard published as [RFC 6986](https://tools.ietf.org/html/rfc6986).


Build requirements
------------------

* EMCC (Emscripten)


Compile and install
-------------------

 `./make-wasm.sh`
 
 Notes
 ---
 MMX and SSE support are currently disabled due to compile errors with EMCC.


Exported WASM Functions
---
The following functions are exported in the compiled Webassembly binary (.wasm) through emscripten.
`int version()` returns the current version of the Streebog WASM Support Library - used for indicating breaking changes.
`void* create_buffer(size_t size)` allocates and returns the address of a buffer of the indicated size in bytes. Internally this buffer is aligned to a multiple of 64. Buffers may be reused.
`void destroy_buffer(void* buffer)` frees a buffer previously created with `create_buffer`
`void* streebog_init(unsigned int digestSize)` creates, initializes, and returns a Streebog hash context to be used to hash an object. Digest size (in bits) must be 512 or 256.
`void streebog_update(void* context, void* buffer, size_t bufferLength)` Hash the additional buffer of data up to `bufferLength` bytes long.  Best performance is had when `bufferLength` is a multiple of 64.  The buffer provided is recommended to be created with `create_buffer` and freed with `destroy_buffer` when finished with it.
`void streebog_final(void* context, void* digestBuffer)` Finalizes the hashing process and sets the Streebog hash in the buffer provided. The buffer length must match the digest size in bytes (eg: Streebog 512 must have a digest buffer 512/8 or 64 bytes long). The buffer provided is recommended to be created with `create_buffer` and freed with `destroy_buffer` when finished with it.
`void streebog_cleanup(void* context)` Clears, destroys, and releases the hashing context (created by a call to `streebog_init`)

WASM Module cwrap equivalencies
---
Using emscripten cwrap, the above exported functions can be cwrapped as follows.
```
streebog={
      version: Module.cwrap('version', 'number', []),
      create_buffer: Module.cwrap('create_buffer', 'number', ['number']),
      destroy_buffer: Module.cwrap('destroy_buffer', '', ['number']),
      init: Module.cwrap('streebog_init', 'number', ['number']),
      update: Module.cwrap('streebog_update', '', ['number','number','number']),
      final: Module.cwrap('streebog_final', '', ['number','number']),
      cleanup: Module.cwrap('streebog_cleanup', '', ['number']),
}
```


Example using cwrapped API
---
For a simple example hashing an input, you might do something like the following after using cwrap:
```
//set up Streebog-512
var ctx = streebog.init(512);

//Create our input
var input = "test";
var inputArray = (new TextEncoder()).encode(input);
var inputBuffer = streebog.create_buffer(inputArray.length);
Module.HEAP8.set(inputArray, inputBuffer);

//Update the hash with the new input data
streebog.update(ctx, inputBuffer, inputArray.length);

//generate our hash digest output
var outputBuffer = streebog.create_buffer(512/8);
streebog.final(ctx,outputBuffer);

//retrieve the hash as a byte array or as a traditional hex string
var resultView = new Uint8Array(Module.HEAP8.buffer, outputBuffer, 512/8);
var result = new Uint8Array(resultView);
var resultHex = Array.prototype.map.call(result, x => ('00' + x.toString(16)).slice(-2)).join('');

//clean up as needed
streebog.cleanup(ctx);
streebog.destroy_buffer(inputBuffer);
streebog.destroy_buffer(outputBuffer);
```



Internal C API (not exposed in WASM)
---
The API to this implementation is quite straightforward and similar to
other hash function APIs.  Actually the CLI utility in this distribution
just use this API as underlying engine.  You may use this API to
implement GOST R 34.11-2012 in your application.

```c
GOST34112012Context
```

This is the hash context.  There should be one `GOST34112012Context`
for each object to be hashed.


```c
void GOST34112012Init(GOST34112012Context *CTX, const unsigned int digest_size);
```

Return initialized `GOST34112012Context` of specified hash size
(`digest_size`) on allocated memory block pointed by `CTX`.  Digest size
can be either 512 or 256.  Address of `CTX` must be 16-byte aligned.

```c
void GOST34112012Update(GOST34112012Context *CTX, const unsigned char *data, size_t len);
```

Hash some `data` of `len` bytes size.  The best performance results are
achieved when `len` is multiple of 64.
    
Note that this call does not modify original data in memory.  If
security is an issue, calling application should destroy that memory
block right after `GOST34112012Update()`, by e.g. `memset()` to zero.

```c
void GOST34112012Final(GOST34112012Context *CTX, unsigned char *digest);
```

Finalizes hashing process and set GOST R 34.11-2012 hash in memory block
pointed by `digest`.

```c
void GOST34112012Cleanup(GOST34112012Context *CTX);
```

The data in context including hash itself, buffer and internal
state zeroed-out.  Context totally destroyed and the object can't be
used anymore.  Calling application should `free()` memory used by this
context.

The following constants may be predefined somewhere in your application
code in order to adjust GOST R 34.11-2012 engine behavior:

* `__GOST3411_LITTLE_ENDIAN__`: define this constant on little-endian systems.

* `__GOST3411_BIG_ENDIAN__`: this constant will indicate big-endian system.

If neither of constants defined the engine defaults to little-endian
code.

* `__GOST3411_HAS_MMX__`: use MMX instructions to compute digest.

* `__GOST3411_HAS_SSE2__`: use SSE2 instruction set to speedup computation
of GOST R 34.11-2012 digest.

* `__GOST3411_HAS_SSE41__`: indicate to include SSE4.1 instructions set.

The best performance results achieved on SSE4.1 capable processors.  A slightly
less performance achieved on SSE2 capable processors.  The CLI utility in this
distribution tries its best to determine which of the instruction set to use.
It falls back to the portable code unless any of extensions detected.


Example of Internal C usage
----------------

```c
    #include <stdlib.h>
    #include <err.h>
    #include "gost3411-2012-core.h"

    ...

    GOST34112012Context *CTX;

    unsigned char digest[64];

    ...
        if (posix_memalign(&CTX, (size_t) 16, sizeof(GOST34112012Context)))
            err(EX_OSERR, NULL);

        GOST34112012Init(CTX, 512);
        ...
        GOST34112012Update(CTX, buffer, (size_t) bufsize);
        ...
        GOST34112012Update(CTX, buffer, (size_t) bufsize);
        ...
        /* call GOST34112012Update() for each block of data */
        ...
        GOST34112012Final(CTX, &digest[0]);
        ...
        /* You now have GOST R 34.11-2012 hash in 'digest' */
        ...
        GOST34112012Cleanup(CTX);
    ...

```

License
-------

This work is dual-licensed under BSD 2-Clause "Simplified" License and GNU
General Public License v2.0 (or later).  You can choose between one of them if
you use this software.

`SPDX-License-Identifier: BSD-2-Clause OR GPL-2.0+`


Author
------
Alexey Degtyarev <alexey@renatasystems.org>
