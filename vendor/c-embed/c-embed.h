/*
# c-embed
# embed virtual file systems into an c program
# - at build time
# - with zero dependencies
# - with zero code modifications
# - with zero clutter in your program
# author: nicholas mcdonald 2022
*/

#ifndef CEMBED
#define CEMBED

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>

// Needed for MS Visual Studio
#if defined(_WIN32)
#include <stdint.h>
#endif // _WIN32

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Needed for MS Visual Studio
#if defined(_WIN32)
typedef uint32_t u_int32_t;
#define __thread __declspec(thread)
#endif // _WIN32

u_int32_t hash(char* key);

typedef size_t epos_t;

struct EMAP_S {     // Map Indexing Struct
  u_int32_t hash;
  u_int32_t pos;
  u_int32_t size;
};
typedef struct EMAP_S EMAP;

struct EFILE_S {    // Virtual File Stream
  char* pos;
  char* end;
  size_t size;
  int err;
};
typedef struct EFILE_S EFILE;

// Error Handling

//__thread int eerrcode;
#define ethrow(err) { (eerrcode = (err)); return NULL; }
#define eerrno (eerrcode)

#define EERRCODE_SUCCESS (0)
#define EERRCODE_NOFILE (1)
#define EERRCODE_NOMAP (2)
#define EERRCODE_NULLSTREAM (3)
#define EERRCODE_OOBSTREAMPOS (4)

const char* eerrstr(int e);

#define eerror(c) printf("%s: (%u) %s\n", c, eerrcode, eerrstr(eerrcode))

// File Useage

#ifndef CEMBED_BUILD

extern char cembed_map_start; // Embedded Indexing Structure
extern char cembed_map_end;
extern char cembed_map_size;

extern char cembed_fs_start;  // Embedded Virtual File System
extern char cembed_fs_end;
extern char cembed_fs_size;

int estat(const char* file, struct stat* const s);
EFILE* eopen(const char* file, const char* mode);
int eclose(EFILE* e);
bool eeof(EFILE* e);
size_t eread(void* ptr, size_t size, size_t count, EFILE* stream);
int egetpos(EFILE* e, epos_t* pos);
char* egets(char* str, int num, EFILE* stream);
int egetc(EFILE* stream);
long int etell(EFILE* e);
void erewind(EFILE* e);
int eseek(EFILE* stream, long int offset, int origin);


// Preprocessor Translation

#ifdef CEMBED_TRANSLATE
#define FILE EFILE
#define fopen eopen
#define fclose eclose
#define feof eeof
#define fgets egets
#define fgetc egetc
#define perror eerror
#define fread eread
#define fseek eseek
#define ftell etell
#endif

#else // CEMBED_BUILD

// WARNING: This must be kept in sync with c-embed-impl.c
u_int32_t hash(char* key) {   // Hash Function: MurmurOAAT64
    u_int32_t h = 3323198485ul;
    for (; *key; ++key) {
        h ^= *key;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
