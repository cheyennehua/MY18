#ifndef __ERROR_H
#define __ERROR_H

void Error_Handler(const char *s);

typedef uint32_t Time_T;    // in ms
typedef uint16_t Voltage_T; // in dV

// DE for double evaluate - dangerous stuff
#define MAX_DE(a, b) ((a > b) ? a : b)
#define MIN_DE(a, b) ((a < b) ? a : b)

#endif // ifndef __ERROR_H
