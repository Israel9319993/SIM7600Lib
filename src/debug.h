// src/debug.h
#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG_SERIAL Serial

#ifdef DEBUG_ENABLED
  #define Debug_Print(x)    DEBUG_SERIAL.print(x)
  #define Debug_Println(x)  DEBUG_SERIAL.println(x)
  #define Debug_Printf(...) DEBUG_SERIAL.printf(__VA_ARGS__)
#else
  #define Debug_Print(x)
  #define Debug_Println(x)
  #define Debug_Printf(...)
#endif

#endif