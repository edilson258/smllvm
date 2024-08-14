#ifndef SML_TYPE
#define SML_TYPE

#include <stddef.h>

typedef enum {
  TYPE_INT = 1,
  TYPE_STR = 2,
} Type;

#define TYPE(t)                                                                \
  ((t == TYPE_INT) ? "int" : (t == TYPE_STR) ? "str" : "UNKNOWN TYPE")

typedef struct {
  char *name;
  Type return_type;
  size_t param_count;
  Type *param_types;
} FnPrototype;

#endif
