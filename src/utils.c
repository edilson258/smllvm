#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *change_file_ext(char *fname_with_ext, char *ext) {
  size_t name_len = 0;
  while (name_len < strlen(fname_with_ext) && fname_with_ext[name_len] != '.') {
    name_len++;
  }
  size_t ext_len = strlen(ext);
  char *out = malloc(sizeof(char) * (name_len + ext_len + 1));
  strncpy(out, fname_with_ext, name_len);
  strcat(out + name_len, ext);
  out[name_len + ext_len] = '\0';
  return out;
}

void __unescape_str__(const char *src, char *dest);

char *unescape_str(const char *src) {
  // Allocate memory for the unescaped string
  char *dest = malloc(strlen(src) + 1); // +1 for null-terminator
  if (!dest) {
    perror("Unable to allocate memory");
    return NULL;
  }

  __unescape_str__(src, dest);
  return dest;
}

void __unescape_str__(const char *src, char *dest) {
  while (*src) {
    if (*src == '\\' && *(src + 1)) {
      switch (*(src + 1)) {
      case 'n':
        *dest = '\n';
        src += 2; // Skip the escape sequence
        break;
      case 't':
        *dest = '\t';
        src += 2;
        break;
      case 'r':
        *dest = '\r';
        src += 2;
        break;
      case '\\':
        *dest = '\\';
        src += 2;
        break;
      default:
        *dest = *src;
        src++;
        break;
      }
    } else {
      *dest = *src;
      src++;
    }
    dest++;
  }
  *dest = '\0'; // Null-terminate the destination string
}
