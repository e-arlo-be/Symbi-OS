// JA: kludge
// coded that finds if and where libkallsyms is mapped in your address space
// designed to work during link loading so careful about what external
// routines it uses to avoid problems.  Based on testing I believe it
// is safe to use basic routines of string.h but not complex conversions

#include <stdint.h>
#include <string.h>
#include "findlibkallsyms.h"

#define MAPS_PATH "/proc/self/maps"
#define LINE_MAX 256

#define KALLSYMSLEN 14
char kallsyms[] = "libkallsyms.so";

static int
iskallsyms(const char *path, int len)
{
#if 0
  if (len < KALLSYMSLEN) return 0;
  for (int i=KALLSYMSLEN-1; i>=0; i--) {
    if (path[len] != kallsyms[i]) return 0;
    len--;
  }
  return 1;
#else
  (void) len;
  return (strstr(path, kallsyms) != NULL);
#endif
}


static int
hexstr2ull(char *str, int len, uintptr_t *val)
{
  unsigned long long v=0, p=1;
  unsigned char d;
  int i, rc;
  char c;
  
  for (i=0; i<len; i++) {
    c = str[i];
    if (c>='0' && c<='9') continue;
    if (c>='a' && c<='f') continue;
    if (c>='A' && c<='F') continue;
    break;
  }
  rc = i;
  i--;
  for (;i>=0; i--) {
    c = str[i];
    if (c>='0' && c<='9') d=c-'0';
    else if (c>='a' && c<='f') d=10 + (c - 'a');
    else if (c>='A' && c<='F') d=10 + (c - 'A');
    else return 0;
    v += d * p;
    p = p * 16;
  }
  *val = v;
  return rc;
}

int
find_libkallsyms(void **start, void **end)
{
  FILE *maps_file;
  char line[LINE_MAX];
  int rc = 0;

  *start = NULL; *end = NULL;

  // read /proc/self/maps
  maps_file = fopen(MAPS_PATH, "r");
  if (maps_file == NULL) {
    fprintf(stderr, "Error opening %s: %s\n", MAPS_PATH, strerror(errno));
    return rc;
  }
  
  while (fgets(line, LINE_MAX, maps_file) != NULL) {
    char *pathname_start = strrchr(line, '/'); // last '/' 

    if (pathname_start != NULL &&
	strstr(line, "[stack]") == NULL && strstr(line, "[heap]") == NULL) {
      // remove trailing new line
      size_t len = strlen(pathname_start);
      if (len > 0 && pathname_start[len - 1] == '\n') {
	pathname_start[len - 1] = '\0';
	len--;
      }
      
      if (iskallsyms(pathname_start, len-1)) {
	uintptr_t x;
	size_t llen = strlen(line);
	int n = hexstr2ull(line, llen, &x);
	if (n) {
	  n++; llen-=n;
	  *start = (void *)x;
	  n = hexstr2ull(&line[n], llen, &x);
	  if (n) {
	    *end = (void *)x;
	    rc = 1;
	  }
	} 
	break;
      }
    }
  }  

  if (fclose(maps_file) == EOF) {
    fprintf(stderr, "Error closing %s: %s\n", MAPS_PATH, strerror(errno));
  }
  return rc;
}
