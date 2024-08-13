#include <stdlib.h>
#include <string.h>

#include "stdlib.h"

#define BUILTIN_FNS_COUNT 1

BuiltinFn printf_fn() {
  BuiltinFn print_f;
  print_f.alias = "print";
  print_f.prototype.name = "printf";
  print_f.prototype.param_count = 1;
  print_f.prototype.param_types = malloc(sizeof(Type) * 1);
  print_f.prototype.param_types[0] = TYPE_STR;
  print_f.prototype.return_type = TYPE_INT;
  return print_f;
};

void init_std_lib(StdLib **lib) {
  *lib = malloc(sizeof(StdLib));
  (*lib)->builtin_fns_count = BUILTIN_FNS_COUNT;
  (*lib)->builtin_fns = malloc(sizeof(BuiltinFn) * BUILTIN_FNS_COUNT);
  (*lib)->builtin_fns[0] = printf_fn();
}

// Hashmap might be suitable
BuiltinFn *find_builtin_fn(const StdLib *stdlib, char *name) {
  for (size_t i = 0; i < stdlib->builtin_fns_count; ++i) {
    if (strcmp(name, stdlib->builtin_fns[i].alias) == 0) {
      return &stdlib->builtin_fns[i];
    }
  }
  return 0;
}
