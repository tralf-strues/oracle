#pragma once

#include "binary_tree.h"
#include "ui.h"

struct Oracle;

Oracle*     summonOracle (const char* knowledgeBaseFileName, UI_Speaker* speaker);
void        banishOracle (Oracle* oracle);
UI_Speaker* getSpeaker   (Oracle* oracle);
                          
void game             (Oracle* oracle);
void definitionDialog (Oracle* oracle);
void comparisonDialog (Oracle* oracle);
void treeDiagram      (Oracle* oracle);
