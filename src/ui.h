#pragma once

#include <stdarg.h>
#include <stdio.h>

struct UI_Speaker;

void UI_PrintDivider  (size_t length, char divider);
void UI_PrintCentered (size_t length, const char* str);

void  UI_PrintOptions (const char* options, ...);
char  UI_GetOption    (const char first, const char last, const char* alt);
char  UI_GetOption    (const char* options);

char* UI_AskStr   (UI_Speaker* speaker, size_t size, const char* message, ...);
char* UI_VAskStr  (UI_Speaker* speaker, size_t size, const char* message, va_list args);
void  UI_SAskStr  (UI_Speaker* speaker, char* dst, size_t dstSize, const char* message, ...);
void  UI_VSAskStr (UI_Speaker* speaker, char* dst, size_t dstSize, const char* message, va_list args);

UI_Speaker* UI_NewSpeaker    (size_t maxPhraseLength, bool speak);
void        UI_DeleteSpeaker (UI_Speaker* speaker);

double UI_GetSpeed     (UI_Speaker* speaker);
bool   UI_GetSpeak     (UI_Speaker* speaker);
size_t UI_GetMaxPhrLen (UI_Speaker* speaker);

void   UI_SetSpeed     (UI_Speaker* speaker, double speed);
void   UI_SetSpeak     (UI_Speaker* speaker, bool speak);
void   UI_SetMaxPhrLen (UI_Speaker* speaker, size_t maxPhraseLength);

void   UI_Say          (UI_Speaker* speaker, const char* format, ...);
void   UI_VSay         (UI_Speaker* speaker, const char* format, va_list args);