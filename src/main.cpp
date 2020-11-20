//-----------------------------------------------------------------------------
//! THANKS TO
//! Sergey
//!  - for making the my easier by offering me to go and fuck myself
//!  - for team-brainstorming how to use tts the best way possible (hopefully)
//!  - for reminding me to process "not"
//!
//! Vasily
//!  - for helping to set up the ms speech api
//!
//! Arthur
//!  - for traverse functions idea
//-----------------------------------------------------------------------------

/* TODO 
#  1)  delete parent field in BTNode and isLeft function if won't be needed 
+  2)  get rid of the copy-paste in summonOracles
+  3)  dot instead of '>' use '-O'
+  4)  write errors to log file 
#  5)  add multiple languages option
#  6)  add pre-soviet-revolution language
-  7)  get rid of copy-paste in getOption
+- 8)  get rid of copy-paste in traversing
+  9)  move asking to enter an object to a separate function 
+  10) process empty lines
+  11) add option to choose database file
*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ui.h"
#include "oracle.h"
#include "../libs/log_generator.h"

const int    DIVIDER_SIZE = 50;
const char   DIVIDER_SYMB = '=';
const size_t MAX_STR_SIZE = 256;
const char*  DEFAULT_DB   = "res/database.txt";

bool running = true;

void dialogMain(char* databaseFileName, bool* speak);

int main()
{
    LG_Init();

    bool speak = true;

    char* dbFileName = (char*) calloc(MAX_STR_SIZE, sizeof(char));
    assert(dbFileName != NULL);
    strncpy(dbFileName, DEFAULT_DB, MAX_STR_SIZE);

    while (running)
    {
        dialogMain(dbFileName, &speak);
    }

    LG_Close();

    return 0;
}

void dialogMain(char* databaseFileName, bool* speak)
{
    assert(databaseFileName != NULL);
    assert(speak != NULL);

    Oracle* oracle = summonOracle(databaseFileName, UI_NewSpeaker(MAX_STR_SIZE, *speak));
    if (oracle == NULL)
    {
        running = false;
        return;
    }

    UI_PrintDivider(DIVIDER_SIZE, DIVIDER_SYMB);

    UI_PrintCentered(DIVIDER_SIZE, "Main menu");
    UI_PrintCentered(DIVIDER_SIZE, databaseFileName);
    UI_PrintOptions("012345x", 
                    "Game", 
                    "Definition", 
                    "Comparison",
                    "Tree diagram",
                    "Change database",
                    *speak ? "Disable voice" : "Enable voice",
                    "EXIT");

    UI_PrintDivider(DIVIDER_SIZE, DIVIDER_SYMB);

    const char option = UI_GetOption('0', '5', "x");

    switch (option)
    {
        case '0':
        {
            game(oracle);
            break;
        }

        case '1':
        {
            definitionDialog(oracle);
            break;
        }

        case '2':
        {
            comparisonDialog(oracle);
            break;
        }

        case '3':
        {
            treeDiagram(oracle);
            break;
        }

        case '4':
        {
            UI_SAskStr(getSpeaker(oracle), 
                       databaseFileName, 
                       MAX_STR_SIZE, 
                       "Enter the filename: ");
            break;
        }

        case '5':
        {
            *speak = !*speak;
            break;
        }

        case 'x':
        {
            running = false;
            break;
        }
    }

    banishOracle(oracle);

    printf("\n\n");
}