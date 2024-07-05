#include "c-embed.h"
#include <dirent.h>

#if !defined(USE_CEMBED)
char cembed_map_start; // Embedded Indexing Structure
char cembed_map_end;
char cembed_map_size;
char cembed_fs_start;  // Embedded Virtual File System
char cembed_fs_end;
char cembed_fs_size;
#endif // USE_CEMBED

typedef struct EFILE_S EFILE;
__thread int eerrcode = 0;


u_int32_t hash(char* key) {   // Hash Function: MurmurOAAT64
    u_int32_t h = 3323198485ul;
    for (; *key; ++key) {
        h ^= *key;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

const char* eerrstr(int e) {
    switch (e) {
    case 0: return "Success.";
    case 1: return "No file found.";
    case 2: return "Mapping stucture error.";
    case 3: return "File stream pointer is NULL.";
    case 4: return "File stream pointer is out-of-bounds.";
    default: return "Unknown cembed error code.";
    };
}



int estat(const char* file, struct stat* const s) {

    EMAP* map = (EMAP*)(&cembed_map_start);
    const char* end = &cembed_map_end;

    if (map == NULL || end == NULL)
        ethrow(EERRCODE_NOMAP);

    const u_int32_t key = hash((char*)file);
    while (((char*)map != end) && (map->hash != key))
        map++;

    if (map->hash != key)
        ethrow(EERRCODE_NOFILE);

    s->st_size = map->size;
    s->st_mode = S_IFREG;
    s->st_mtime = time(0);
    return 0;
}

EFILE* eopen(const char* file, const char* mode) {

    EMAP* map = (EMAP*)(&cembed_map_start);
    const char* end = &cembed_map_end;

    if (map == NULL || end == NULL)
        ethrow(EERRCODE_NOMAP);

    const u_int32_t key = hash((char*)file);
    while (((char*)map != end) && (map->hash != key))
        map++;

    if (map->hash != key)
        ethrow(EERRCODE_NOFILE);

    EFILE* e = (EFILE*)malloc(sizeof * e);
    e->pos = (&cembed_fs_start + map->pos);
    e->end = (&cembed_fs_start + map->pos + map->size);
    e->size = map->size;

    return e;

}

int eclose(EFILE* e) {
    if (e)
        free(e);
    e = NULL;
    return 0;
}

bool eeof(EFILE* e) {
    if (e == NULL) {
        (eerrcode = (EERRCODE_NULLSTREAM));
        return true;
    }
    if (e->end < e->pos) {
        (eerrcode = (EERRCODE_OOBSTREAMPOS));
        return true;
    }
    if ((e->end - e->pos) - e->size < 0) {
        (eerrcode = (EERRCODE_OOBSTREAMPOS));
        return true;
    }
    return (e->end == e->pos);
}

size_t eread(void* ptr, size_t size, size_t count, EFILE* stream) {

    size_t rcount = size * count;
    if (stream->end - stream->pos < rcount) {
        size_t scount = stream->end - stream->pos;
        memcpy(ptr, (void*)stream->pos, scount);
        stream->pos = stream->end;
        return scount;
    }

    memcpy(ptr, (void*)stream->pos, rcount);
    stream->pos += rcount;
    return rcount;

}

int egetpos(EFILE* e, epos_t* pos) {

    if (e->end <= e->pos) {
        pos = NULL;
        return 1;
    }

    *pos = (epos_t)(e->end - e->pos);
    return 0;

}

char* egets(char* str, int num, EFILE* stream) {

    if (eeof(stream))
        return NULL;

    for (int i = 0; i < num && !eeof(stream) && *(stream->pos) != '\r'; i++)
        str[i] = *(stream->pos++);

    return str;

}

int egetc(EFILE* stream) {
    if (eeof(stream))
        return -1;
    return (int)(*(stream->pos++));
}

long int etell(EFILE* e) {
    return e->size - (e->end - e->pos);
}

void erewind(EFILE* e) {
    e->pos = (e->end - e->size);
}

int eseek(EFILE* stream, long int offset, int origin) {

    if (origin == SEEK_SET)
        stream->pos = stream->end - stream->size + offset;
    if (origin == SEEK_CUR)
        stream->pos += offset;
    if (origin == SEEK_END)
        stream->pos = stream->end + offset;

    if (stream->end < stream->pos || etell(stream) < 0) {
        (eerrcode = (EERRCODE_OOBSTREAMPOS));
        return true;
    }

    return 0;

}
