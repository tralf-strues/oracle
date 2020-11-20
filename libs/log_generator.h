#pragma once

#include <stdio.h>

static const char* LG_FILE_NAME = "log/log.html";
static const char* LG_STLE_NAME = "log.css";

//----------------------------------------------------------------------------- 
//! @defgroup LG_COLORS Supporting colors
//! @addtogroup LG_COLORS
//! @{

struct LG_Color
{
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
};

static const LG_Color LG_COLOR_BLACK  = { 0x00, 0x00, 0x00 };
static const LG_Color LG_COLOR_BLUE   = { 0x00, 0x00, 0xFF };
static const LG_Color LG_COLOR_GRAY   = { 0x80, 0x80, 0x80 };
static const LG_Color LG_COLOR_GREEN  = { 0x00, 0xFF, 0x00 };
static const LG_Color LG_COLOR_PINK   = { 0xFF, 0xC0, 0xCB };
static const LG_Color LG_COLOR_RED    = { 0xFF, 0x00, 0x00 };
static const LG_Color LG_COLOR_YELLOW = { 0xFF, 0xFF, 0x00 };

//! @}
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------- 
//! @defgroup LG_STYLE_CLASSES Style specifying classes
//! @attention There has to be log.css in log/ folder in order for the style 
//!            classes to work properly!
//! @addtogroup LG_STYLE_CLASSES
//! @{

struct LG_StyleClass
{
    const char* name = NULL;
};

//! Specifies error style (the text will be red and in bold) 
static const LG_StyleClass LG_STYLE_CLASS_ERROR   = { "LG_ERROR" };

//! Specifies error style (the text will be red and in bold) 
static const LG_StyleClass LG_STYLE_CLASS_GOOD   =  { "LG_GOOD" };

//! Specifies default style (the text will be black) 
static const LG_StyleClass LG_STYLE_CLASS_DEFAULT = { "LG_DEFAULT" };

//! @}
//-----------------------------------------------------------------------------

bool LG_Init              ();
bool LG_Close             ();
bool LG_IsInitialized     ();

void LG_AddImage          (const char* fileName, const char* htmlStyle);

void LG_LogMessage        (const char* format, LG_StyleClass styleClass, ...);
void LG_LogMessage        (const char* format, LG_Color color, ...);
void LG_WriteMessageStart (LG_StyleClass styleClass);
void LG_WriteMessageStart (LG_Color color);
void LG_WriteMessageEnd   ();

void LG_Write             (const char* format, LG_StyleClass styleClass, ...);
void LG_Write             (const char* format, LG_Color color, ...);
void LG_Write             (const char* format, ...);