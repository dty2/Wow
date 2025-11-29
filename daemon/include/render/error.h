#pragma once

#ifdef DEBUG
#define ASSERT(x)               \
  do {                          \
    if (!(x)) __builtin_trap(); \
  } while (0)

#else
#define ASSERT(x) ((void)0)
#endif

#define GLCall(x) \
  glClearError(); \
  x;              \
  ASSERT(glCheckError(#x))

void glCheckError();
void glClearError();
