#include "StringUtils.h"
#include <string.h>
#include <Arduino.h>

// Global shared buffer for temporary string operations
char globalStringBuffer[21];

void padStringToGlobalBuffer(const char* str, int length) {
    int strLen = strlen(str);
    
    // Copy the string, truncating if too long
    int copyLen = min(strLen, length);
    strncpy(globalStringBuffer, str, copyLen);
    
    // Pad with spaces if needed
    for (int i = copyLen; i < length; i++) {
        globalStringBuffer[i] = ' ';
    }
    
    // Null terminate
    globalStringBuffer[length] = '\0';
}

void centerStringToGlobalBuffer(const char* str, int length) {
    int strLen = strlen(str);
    
    // Truncate if too long
    int copyLen = min(strLen, length);
    
    // Calculate centering spaces
    int spaces = (length - copyLen) / 2;
    
    // Fill with leading spaces
    for (int i = 0; i < spaces; i++) {
        globalStringBuffer[i] = ' ';
    }
    
    // Copy the string
    strncpy(globalStringBuffer + spaces, str, copyLen);
    
    // Fill with trailing spaces
    for (int i = spaces + copyLen; i < length; i++) {
        globalStringBuffer[i] = ' ';
    }
    
    // Null terminate
    globalStringBuffer[length] = '\0';
}
