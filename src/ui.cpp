#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "ui.h"

#include <windows.h>
#include <Servprov.h>
#define TX_USE_SPEAK
#include "../libs/TX/TXLib.h"

struct UI_Speaker
{
    double speed         = 16.0;
    bool   speak         = false;
    size_t curBufferSize = 0;
    size_t maxBufferSize = 0;
    char*  buffer        = NULL;
};

const size_t NOT_DELIM_SYMBOL_DELAY = 55;
const size_t DELIM_SYMBOL_DELAY     = 320;
const char*  PUNCTUATION_MARKS      = ",.!?";

size_t numOfOccurences(const char* str, char ch);

void UI_PrintDivider(size_t length, char divider)
{
    for (int i = 0; i < length; i++)
    {
        putchar(divider);
    }

    putchar('\n');
}

//-----------------------------------------------------------------------------
// 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14
//  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =
//                 H  E  L  L  O
//                 ^
//          (15 - 5) / 2  
//  
// length     = 15
// str length = 5
//-----------------------------------------------------------------------------
void UI_PrintCentered(size_t length, const char* str)
{
    assert(str != NULL);

    size_t strLength = strlen(str);

    if (strLength >= length)
    {
        printf("%s\n", str);
        return;
    }

    for (int i = (length - strLength) / 2; i > 0; i--)
    {
        putchar(' ');
    }

    printf("%s\n", str);
}

void UI_PrintOptions(const char* options, ...)
{
    assert(options != NULL);

    va_list messages = {};
    va_start(messages, &options);

    for (size_t i = 0; options[i] != '\0'; i++)
    {
        printf("  [%c] %s\n", options[i], va_arg(messages, char*));
    }

    va_end(messages);
}

//-----------------------------------------------------------------------------
//! Asks to choose an option from first to last (e.g. '0'-'3') or from alt 
//! characters from the user. If input is incorrect asks to enter an option 
//! again.
//!
//! @param [in] first  
//! @param [in] last   
//! @param [in] alt   
//!
//! @note alt can be NULL.
//!
//! @return option
//-----------------------------------------------------------------------------
char UI_GetOption(const char first, const char last, const char* alt)
{
    assert(first <= last);

    if (alt != NULL) { printf("  Choose an option (%c-%c or '%s'): ", first, last, alt); }
    else             { printf("  Choose an option (%c-%c): ",         first, last); }
    
    char option = 0;
    scanf(" %c", &option);
    while((option < first || option > last) && (alt == NULL || strchr(alt, option) == NULL))
    {
        if (alt != NULL) 
        { 
            printf("  Incorrect input. Please enter an option from %c to %c or '%s': ", first, last, alt); 
        }
        else             
        { 
            printf("  Incorrect input. Please enter an option from %c to %c: ", first, last); 
        }
        
        scanf(" %c", &option);
    }

    getchar(); // to skip '\n'

    return option;
}

//-----------------------------------------------------------------------------
//! Asks to choose an option from options from the user. If input is incorrect 
//! asks to enter an option again.
//!
//! @param [in] options  
//!
//! @return option
//-----------------------------------------------------------------------------
char UI_GetOption(const char* options)
{
    assert(options != NULL);

    size_t optionsCount = strlen(options);
    assert(optionsCount != 0);
    assert(optionsCount != 1);

    switch (optionsCount)
    {
        case 2:
            printf("  Choose either '%c' or '%c': ", options[0], options[1]);
            break;

        case 3:
            printf("  Choose '%c', '%c' or '%c': ", options[0], options[1], options[2]);
            break;

        default:
            printf("  Choose an option from ('%s'): ", options);
            break;
    }
    
    char option = 0;
    scanf(" %c", &option);
    while(strchr(options, option) == NULL)
    { 
        printf("  Incorrect input. Please enter an option from '%s': ", options); 
        
        scanf(" %c", &option);
    }

    getchar(); // to skip '\n'

    return option;
}

char* UI_AskStr(UI_Speaker* speaker, size_t size, const char* message, ...)
{
    assert(message != NULL);
    assert(size > 0);

    va_list args = {};
    va_start(args, &message);

    return UI_VAskStr(speaker, size, message, args);
}

char* UI_VAskStr(UI_Speaker* speaker, size_t size, const char* message, va_list args)
{
    assert(message != NULL);
    assert(size > 0);

    char* str = (char*) calloc(size, sizeof(char));
    if (str == NULL) { return NULL; }

    UI_VSAskStr(speaker, str, size, message, args);

    return str;
}

void UI_SAskStr(UI_Speaker* speaker, char* dst, size_t dstSize, const char* message, ...)
{
    assert(dst     != NULL);
    assert(message != NULL);

    va_list args = {};
    va_start(args, &message);

    UI_VSAskStr(speaker, dst, dstSize, message, args);

    va_end(args);
}

void UI_VSAskStr(UI_Speaker* speaker, char* dst, size_t dstSize, const char* message, va_list args)
{
    assert(dst     != NULL);
    assert(message != NULL);

    if (speaker != NULL) { UI_VSay(speaker, message, args); }
    else                 { vprintf(message, args); }

    fgets(dst, dstSize, stdin);
    dst[strcspn(dst, "\r\n")] = '\0';
}

UI_Speaker* UI_NewSpeaker(size_t maxPhraseLength, bool speak)
{
    UI_Speaker* speaker = (UI_Speaker*) calloc(1, sizeof(UI_Speaker));
    if (speaker == NULL) { return NULL; }

    speaker->maxBufferSize = maxPhraseLength + 1;
    UI_SetMaxPhrLen(speaker, maxPhraseLength);

    speaker->speed = 1.0;
    speaker->speak = speak;

    return speaker;
}

void UI_DeleteSpeaker(UI_Speaker* speaker)
{
    assert(speaker != NULL);

    speaker->speed         = 0;
    speaker->speak         = false;
    speaker->curBufferSize = 0;
    speaker->maxBufferSize = 0;

    free(speaker->buffer);
    speaker->buffer = NULL;

    free(speaker);
}

double UI_GetSpeed(UI_Speaker* speaker)
{
    assert(speaker != NULL);
    return speaker->speed;
}

bool UI_GetSpeak(UI_Speaker* speaker)
{
    assert(speaker != NULL);
    return speaker->speak;
}

size_t UI_GetMaxPhrLen(UI_Speaker* speaker)
{
    assert(speaker != NULL);
    return speaker->maxBufferSize - 1;
}

void UI_SetSpeed(UI_Speaker* speaker, double speed)
{
    assert(speaker != NULL);
    speaker->speed = speed;
}

void UI_SetSpeak(UI_Speaker* speaker, bool speak)
{
    assert(speaker != NULL);
    speaker->speak = speak;
}

void UI_SetMaxPhrLen(UI_Speaker* speaker, size_t maxPhraseLength)
{
    assert(speaker != NULL);

    speaker->maxBufferSize = maxPhraseLength + 1;

    if (speaker->maxBufferSize > speaker->curBufferSize)
    {
        if (speaker->buffer != NULL)
        {
            speaker->buffer = (char*) realloc(speaker->buffer, speaker->maxBufferSize * sizeof(char));
        }
        else
        {
            speaker->buffer = (char*) calloc(speaker->maxBufferSize, sizeof(char));
        }
        assert(speaker->buffer != NULL);

        speaker->curBufferSize = speaker->maxBufferSize;
    }
}

size_t numOfOccurences(const char* src, const char* dst)
{
    assert(src != NULL);
    assert(dst != NULL);

    size_t count = 0;
    for (; *src != '\0'; src++) 
    { 
        if (strchr(dst, *src) != NULL) 
        {
            count++;
        } 
    }

    return count;
}

void UI_Say(UI_Speaker* speaker, const char* format, ...)
{
    assert(speaker != NULL);
    assert(format  != NULL);

    va_list args = {};
    va_start(args, format);

    UI_VSay(speaker, format, args);

    va_end(args);
}

void UI_VSay(UI_Speaker* speaker, const char* format, va_list args)
{
    assert(speaker != NULL);
    assert(format  != NULL);

    if (!UI_GetSpeak(speaker))
    {
        vprintf(format, args);
        va_end(args);
        return;
    }

    char* buffer = speaker->buffer;
    assert(buffer != NULL);

    char* strBufferEnd = buffer + UI_GetMaxPhrLen(speaker);

    vsnprintf(buffer, UI_GetMaxPhrLen(speaker) + 1, format, args);

    txSpeak("\a%s", buffer);

    const char* delims = " \t\n";

    char*  currStart  = buffer;
    char*  currEnd    = NULL;
    char   prevSymb   = 0;
    size_t puncMarks  = 0;
    size_t currLength = 0;
    bool   currDelims = true;

    while (true)
    {
        if (strchr(delims, *currStart) != NULL) { currDelims = true; }
        else                                    { currDelims = false; }

        currEnd = currStart + (currDelims ? strspn(currStart, delims) : strcspn(currStart, delims));
        prevSymb = *currEnd;
        *currEnd = '\0';

        printf("%s", currStart);

        if (!currDelims)
        {
            puncMarks  = numOfOccurences(currStart, PUNCTUATION_MARKS);
            currLength = currEnd - currStart;

            txSleep((NOT_DELIM_SYMBOL_DELAY * (currLength - puncMarks) + DELIM_SYMBOL_DELAY * puncMarks) / UI_GetSpeed(speaker));
        }

        *currEnd = prevSymb;
        if (currEnd >= strBufferEnd || *currEnd == '\0') { break; }
        currStart = currEnd;
    }
}