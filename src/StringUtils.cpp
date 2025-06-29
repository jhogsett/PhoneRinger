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
