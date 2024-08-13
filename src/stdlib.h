#ifndef SML_STD_LIB
#define SML_STD_LIB

#include "type.h"

typedef struct {
  FnPrototype prototype;
  char *alias;
} BuiltinFn;

typedef struct {
  BuiltinFn *builtin_fns;
  size_t builtin_fns_count;
} StdLib;

void init_std_lib(StdLib **lib);
BuiltinFn *find_builtin_fn(const StdLib *stdlib, char *name);

#endif
