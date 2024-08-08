typedef enum {
  TYPE_INT = 1,
  TYPE_STR,
} Type;

#define TYPE(t)                                                                \
  ((t == TYPE_INT) ? "int" : (t == TYPE_STR) ? "str" : "UNKNOWN TYPE")
