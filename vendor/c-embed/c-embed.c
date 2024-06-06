/*
# c-embed
# embed virtual file systems into an c program
# - at build time
# - with zero dependencies
# - with zero code modifications
# - with zero clutter in your program
# author: nicholas mcdonald 2022
*/

#define CEMBED_BUILD

#include "c-embed.h"
#include <dirent.h>
#include <string.h>

#define CEMBED_FILE "c-embed.o"       // Output File
#define CEMBED_TMPDIR "cembed_tmp"    // Temporary Directory
#if defined(_WIN32)
#define CEMBED_ARCH "pe-x86-64"
#else // _WIN32
#define CEMBED_ARCH "elf64-x86-64"    // Target Architecture
#endif // _WIN32

FILE* ms = NULL;    // Mapping Structure
FILE* fs = NULL;    // Virtual Filesystem
FILE* file = NULL;  // Embed Target File Pointer
u_int32_t pos = 0;  // Current Position

void cembed(char* filename){

  file = fopen(filename, "rb");  // Open the Embed Target File
  if(file == NULL){
    printf("Failed to open file %s.", filename);
    return;
  }

  fseek(file, 0, SEEK_END);     // Define Map
  EMAP map = {hash(filename), pos, (u_int32_t)ftell(file)};
  rewind (file);

  char* buf = malloc(sizeof(char)*(map.size));
  if(buf == NULL){
    printf("Memory error for file %s.", filename);
    return;
  }

  u_int32_t result = fread(buf, 1, map.size, file);
  if(result != map.size){
    printf("Read error for file %s.", filename);
    return;
  }

  fwrite(&map, sizeof map, 1, ms);  // Write Mapping Structure
  fwrite(buf, map.size, 1, fs);     // Write Virtual Filesystem

  free(buf);        // Free Buffer
  fclose(file);     // Close the File
  file = NULL;      // Reset the Pointer
  pos += map.size;  // Shift the Index Position

}

#if defined(_WIN32)
#define CEMBED_DIRENT_FILE (DT_REG)
#define CEMBED_DIRENT_DIR (DT_DIR)
#define CEMBED_MAXPATH (2048)
#else // _WIN32
#define CEMBED_DIRENT_FILE (8)
#define CEMBED_DIRENT_DIR (4)
#define CEMBED_MAXPATH (512)
#endif // _WIN32

void iterdir(char* d){

  char* fullpath = (char*)malloc(CEMBED_MAXPATH*sizeof(char));

  DIR *dir;
  struct dirent *ent;

  if ((dir = opendir(d)) != NULL) {

    while ((ent = readdir(dir)) != NULL) {

      if(strcmp(ent->d_name, ".") == 0) continue;
      if(strcmp(ent->d_name, "..") == 0) continue;

      if(ent->d_type == CEMBED_DIRENT_FILE){
        strcpy(fullpath, d);
#if defined(_WIN32)
        strcat(fullpath, "\\");
#else // _WIN32
        strcat(fullpath, "/");
#endif // _WIN32
        strcat(fullpath, ent->d_name);
        cembed(fullpath);
      }

      else if(ent->d_type == CEMBED_DIRENT_DIR){
        strcpy(fullpath, d);
#if defined(_WIN32)
        strcat(fullpath, "\\");
#else // _WIN32
        strcat(fullpath, "/");
#endif // _WIN32
        strcat(fullpath, ent->d_name);
        iterdir(fullpath);
      }

    }

    closedir(dir);

  }

  else {

    strcpy(fullpath, d);
    cembed(fullpath);

  }

  free(fullpath);

}

#include <windows.h>

int main(int argc, char* argv[]){

  char fmt[CEMBED_MAXPATH];

  if(argc <= 1)
    return 0;

#if defined(_WIN32)
  sprintf(fmt, "if not exist %s mkdir %s", CEMBED_TMPDIR, CEMBED_TMPDIR);
  system(fmt);
#else // _WIN32
  sprintf(fmt, "if [ ! -d %s ]; then mkdir %s; fi;", CEMBED_TMPDIR, CEMBED_TMPDIR);
  system(fmt);
#endif // _WIN32

  // Build the Mapping Structure and Virtual File System

  ms = fopen("cembed.map", "wb");
  fs = fopen("cembed.fs", "wb");

  if(ms == NULL || fs == NULL){
    printf("Failed to initialize map and filesystem. Check permissions.");
    return 0;
  }

  for(int i = 1; i < argc; i++)
    iterdir(argv[i]);

  fclose(ms);
  fclose(fs);

  // Convert to Embeddable Symbols
#if defined(_WIN32)

  wchar_t cwd[MAX_PATH];
  wchar_t* p;
  p = _wgetcwd(cwd, MAX_PATH);
  if (!p) {
      return -1;
  }

  // Invoke objcopy via msys2 (ie C:\msys64\ucrt64.exe)
  sprintf(fmt, "c:\\msys64\\msys2_shell.cmd -defterm -no-start -ucrt64 -here -c \"objcopy -I binary -O %s "\
      "--redefine-sym _binary_cembed_map_start=cembed_map_start "\
      "--redefine-sym _binary_cembed_map_end=cembed_map_end "\
      "--redefine-sym _binary_cembed_map_size=cembed_map_size "\
      "cembed.map cembed.map.o \"", CEMBED_ARCH);
  system(fmt);

  sprintf(fmt, "c:\\msys64\\msys2_shell.cmd -defterm -no-start -ucrt64 -here -c \"mv cembed.map.o %s/cembed.map.o\"", CEMBED_TMPDIR);
  system(fmt);
  system("del cembed.map");

  sprintf(fmt, "c:\\msys64\\msys2_shell.cmd -defterm -no-start -ucrt64 -here -c \"objcopy -I binary -O %s "\
      "--redefine-sym _binary_cembed_fs_start=cembed_fs_start "\
      "--redefine-sym _binary_cembed_fs_end=cembed_fs_end "\
      "--redefine-sym _binary_cembed_fs_size=cembed_fs_size "\
      "cembed.fs cembed.fs.o \"", CEMBED_ARCH);
  system(fmt);

  sprintf(fmt, "c:\\msys64\\msys2_shell.cmd -defterm -no-start -ucrt64 -here -c \"mv cembed.fs.o %s/cembed.fs.o\"", CEMBED_TMPDIR);
  system(fmt);
  system("del cembed.fs");

  printf("%s/cembed.map.o", CEMBED_TMPDIR);
  printf("%s/cembed.fs.o", CEMBED_TMPDIR);

#else // _WINDOWS

  sprintf(fmt, "objcopy -I binary -O %s "\
          "--redefine-sym _binary_cembed_map_start=cembed_map_start "\
          "--redefine-sym _binary_cembed_map_end=cembed_map_end "\
          "--redefine-sym _binary_cembed_map_size=cembed_map_size "\
          "cembed.map cembed.map.o", CEMBED_ARCH);
  system(fmt);

  sprintf(fmt, "mv cembed.map.o %s/cembed.map.o", CEMBED_TMPDIR);
  system(fmt);
  system("rm cembed.map");

  sprintf(fmt, "objcopy -I binary -O %s "\
          "--redefine-sym _binary_cembed_fs_start=cembed_fs_start "\
          "--redefine-sym _binary_cembed_fs_end=cembed_fs_end "\
          "--redefine-sym _binary_cembed_fs_size=cembed_fs_size "\
          "cembed.fs cembed.fs.o", CEMBED_ARCH);
  system(fmt);

  sprintf(fmt, "mv cembed.fs.o %s/cembed.fs.o", CEMBED_TMPDIR);
  system(fmt);
  system("rm cembed.fs");

  sprintf(fmt, "ld -relocatable cembed_tmp/*.o -o %s", CEMBED_FILE);
  system(fmt);

  sprintf(fmt, "rm -rf %s", CEMBED_TMPDIR);
  system(fmt);

  printf("%s", CEMBED_FILE);

#endif // _WINDOWS

  return 0;

}
