#ifndef STRING_UTILS_H
#define STRING_UTILS_H

// Global shared buffer for temporary string operations
// Safe to use since Arduino is single-threaded
extern char globalStringBuffer[21];  // 20 chars + null terminator

// Helper function to safely pad a string to a buffer
void padStringToGlobalBuffer(const char* str, int length = 20);

// Helper function to center-justify a string in a buffer
void centerStringToGlobalBuffer(const char* str, int length = 20);

#endif
