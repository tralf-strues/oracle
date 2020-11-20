#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "oracle.h"
#include "binary_tree.h"
#include "../libs/file_manager.h"
#include "../libs/log_generator.h"

typedef BTNode* elem_t;
#include "../libs/stack.h"

#define CHECK_NULL(value, action) if (value == NULL) { action; }

struct Oracle
{
    BinaryTree* tree     = NULL;
    const char* fileName = NULL;
    Text*       database = NULL;
    UI_Speaker* speaker  = NULL;
};

static const size_t MAX_STRING_LENGTH = 128;

bool   loadDatabase     (Oracle* oracle);
void   saveDatabase     (Oracle* oracle);
void   saveNode         (BTNode* node, FILE* file);
                            
void   subtreeConstruct (BTNode* node, Text* text);
bool   isTreeCorrect    (BinaryTree* tree);
bool   isNodeCorrect    (BTNode* node, va_list args);
                          
void   printCurrentLine (Text* text);
                          
void   finishGame       (Oracle* oracle, BTNode* node);
void   defeat           (Oracle* oracle, BTNode* node);
                          
void   definition       (Oracle* oracle, BTNode* object, BTNode* start);
void   comparison       (Oracle* oracle, BTNode* object1, BTNode* object2);
Stack* getPathFromRoot  (BinaryTree* tree, BTNode* node);

void   subtreeDiagram   (FILE* file, BTNode* node);

Oracle* summonOracle(const char* knowledgeBaseFileName, UI_Speaker* speaker)
{
    assert(knowledgeBaseFileName != NULL);
    assert(speaker != NULL);

    Oracle* oracle = (Oracle*) calloc(1, sizeof(Oracle));
    CHECK_NULL(oracle, return NULL);

    oracle->tree = newTree();
    CHECK_NULL(oracle->tree, return NULL);

    oracle->fileName = knowledgeBaseFileName;
    oracle->database = NULL;
    oracle->speaker  = speaker;

    if (loadDatabase(oracle) == false)
    {
        deleteTree(oracle->tree);
        free(oracle);

        LG_Write("ERROR: couldn't read the database\n", LG_STYLE_CLASS_ERROR);

        return NULL;
    }

    return oracle;
}

void banishOracle(Oracle* oracle)
{
    assert(oracle != NULL);
    assert(oracle->tree != NULL);

    deleteTree(oracle->tree);
    oracle->tree = NULL;

    UI_DeleteSpeaker(oracle->speaker);

    if (oracle->database != NULL) { deleteText(oracle->database); }

    free(oracle);
}

UI_Speaker* getSpeaker(Oracle* oracle)
{
    assert(oracle != NULL);
    return oracle->speaker;
}

bool loadDatabase(Oracle* oracle)
{
    assert(oracle != NULL);
    assert(oracle->fileName != NULL);

    oracle->database = readTextFromFile(oracle->fileName);
    CHECK_NULL(oracle->database, LG_Write("ERROR: Couldn't read file '%s'\n", LG_STYLE_CLASS_ERROR, oracle->fileName); return false);

    setRoot(oracle->tree, newNode());

    subtreeConstruct(getRoot(oracle->tree), oracle->database);

    if (!isTreeCorrect(oracle->tree))
    {
        LG_Write("ERROR: Incorrect database content - there are questions with only one answer\n", LG_STYLE_CLASS_ERROR);

        return false;
    }

    return true;
}

void saveDatabase(Oracle* oracle)
{
    assert(oracle != NULL);

    FILE* file = fopen(oracle->fileName, "w");
    CHECK_NULL(file, LG_Write("ERROR: Couldn't open file '%s'\n", LG_STYLE_CLASS_ERROR, oracle->fileName); return);

    saveNode(getRoot(oracle->tree), file);

    fclose(file);
}

#define SAVE_SUBTREE(getSide) if (getSide(node) != NULL)         \
                              {                                  \
                                  fprintf(file, "{\n");          \
                                  saveNode(getSide(node), file); \
                                  fprintf(file, "}\n");          \
                              }

void saveNode(BTNode* node, FILE* file)
{
    assert(node != NULL);
    assert(file != NULL);

    fprintf(file, "\"%s\"\n", getValue(node));

    SAVE_SUBTREE(getRight);
    SAVE_SUBTREE(getLeft);
}

#define TREE_CONSTRUCT_ERROR(message) LG_Write(message, LG_STYLE_CLASS_ERROR); \
                                      printCurrentLine(text);                  \
                                      return;

void subtreeConstruct(BTNode* node, Text* text)
{
    assert(node != NULL);
    assert(text != NULL);

    char* currLine = nextTextLine(text);
    CHECK_NULL(currLine, return);

    const char* openingBracket = strchr(currLine, '{');
    const char* closingBracket = strchr(currLine, '}');

    if (openingBracket != NULL && closingBracket != NULL)
    {
        TREE_CONSTRUCT_ERROR("ERROR: database syntax error, '{' and '}' on the same line: ");
    }
    else if (openingBracket != NULL)
    {
        if (getRight(node) == NULL)
        {
            setRight(node, newNode());
            setParent(getRight(node), node);
            subtreeConstruct(getRight(node), text);
        }
        else if (getLeft(node) == NULL)
        {
            setLeft(node, newNode());
            setParent(getLeft(node), node);
            subtreeConstruct(getLeft(node), text);
        }
        else
        {   
            TREE_CONSTRUCT_ERROR("ERROR: database syntax error, opening block after definition of both options: ");  
        }
    }
    else if (closingBracket != NULL)
    {
        subtreeConstruct(getParent(node), text);      
    }
    else
    {
        char* openingQuote = strchr(currLine, '\"');

        if (openingQuote == NULL)
        {
            subtreeConstruct(node, text);
            return;      
        }

        char* closingQuote = strchr(openingQuote + 1, '\"');

        if (closingQuote == NULL)
        {
            TREE_CONSTRUCT_ERROR("ERROR: database syntax error, no closing \" is found: ");
        }

        if (closingQuote - openingQuote <= 1)
        {
            TREE_CONSTRUCT_ERROR("ERROR: database syntax error, no string token is found: ");
        }

        closingQuote[0] = '\0';

        setValue(node, openingQuote + 1);

        subtreeConstruct(node, text);     
    }
}

bool isTreeCorrect(BinaryTree* tree)
{
    assert(tree != NULL);
    
    if (getRoot(tree) == NULL) { return false; }

    bool isCorrect = true;
    preOrderTraverse(getRoot(tree), &isNodeCorrect, &isCorrect);

    return isCorrect;
}

bool isNodeCorrect(BTNode* node, va_list args)
{
    assert(node != NULL);

    bool* isCorrect = va_arg(args, bool*);
    va_end(args);

    if ((getLeft(node) == NULL && getRight(node) != NULL) || (getLeft(node) != NULL && getRight(node) == NULL))
    {
        *isCorrect = false;

        return !BT_TRAVERSE_RUN;
    }

    return BT_TRAVERSE_RUN;
}

void printCurrentLine(Text* text)
{
    assert(text != NULL);

    size_t currLineNumber = getCurrentLineNumber(text);
    LG_Write("line %u\n%5u | %s\n", LG_STYLE_CLASS_ERROR, 
             currLineNumber + 1, 
             currLineNumber + 1, 
             getLine(text, currLineNumber));
}

void game(Oracle* oracle)
{
    assert(oracle != NULL);
    assert(getRoot(oracle->tree) != NULL);

    BTNode* currNode = getRoot(oracle->tree);
    char    answer   = 0;
    while (true)
    {
        if (getLeft(currNode) == NULL)
        {
            finishGame(oracle, currNode);

            break;
        }

        UI_Say(oracle->speaker, "  -Is it %s?\n", getValue(currNode));
        answer = UI_GetOption("yn");
        printf("\n");

        if (answer == 'y')
        {
            currNode = getRight(currNode);
        }
        else
        {
            currNode = getLeft(currNode);
        }
    }
}

void finishGame(Oracle* oracle, BTNode* node)
{
    assert(oracle         != NULL);
    assert(node           != NULL);
    assert(getLeft(node)  == NULL);
    assert(getRight(node) == NULL);

    UI_Say(oracle->speaker, "  -I know! You are thinking about... %s! Am I right?\n", getValue(node));
    char answer = UI_GetOption("yn");

    if (answer == 'y')
    {
        UI_Say(oracle->speaker, "  -I have won, as always!)\n");
    }
    else
    {
        defeat(oracle, node);
    } 
}

void defeat(Oracle* oracle, BTNode* node)
{
    assert(oracle != NULL);
    assert(node != NULL);

    char* newObject = UI_AskStr(getSpeaker(oracle), MAX_STRING_LENGTH, "\n  -You got me :( What/whom are you thinking about? ");

    BTNode* existingObject = findNode(oracle->tree, newObject);
    if (existingObject != NULL)
    {
        UI_Say(oracle->speaker, "\n  -Oh... I actually knew this one.\n");
        definition(oracle, existingObject, NULL);

        return;
    }

    char* newQuestion = UI_AskStr(getSpeaker(oracle), MAX_STRING_LENGTH, "\n  -How %s differs from %s? ", newObject, getValue(node));

    char* questionStart  = newQuestion;
    char* notStart       = strstr(newQuestion, "not");
    int   notLength      = strlen("not");
    int   questionLength = strlen(newQuestion);
    if (notStart != NULL)
    {
        int symbolsCountBeforeNot = strspn(newQuestion, " \t");
        int symbolsCountAfterNot  = strspn(notStart + notLength, " \t");

        // check if 'not' is the first word in the question 
        if (symbolsCountBeforeNot == notStart - newQuestion) 
        { 
            questionStart = notStart + notLength + symbolsCountAfterNot;

            if (questionStart >= newQuestion + questionLength) { questionStart = newQuestion; }
        }
    }

    bool isNot = questionStart != newQuestion;

    BTNode* newNode1 = newNode(getValue(node));
    BTNode* newNode2 = newNode(newObject);

    setParent(newNode1, node);
    setParent(newNode2, node);

    setLeft(node, !isNot ? newNode1 : newNode2);
    setRight(node, !isNot ? newNode2 : newNode1);

    setValue(node, questionStart);  

    UI_Say(oracle->speaker, "\n  -From now on you won't be able to outplay me!\n");  

    saveDatabase(oracle);

    free(newObject);
    free(newQuestion);
}

void definitionDialog(Oracle* oracle)
{
    assert(oracle != NULL);

    char* object = UI_AskStr(getSpeaker(oracle), MAX_STRING_LENGTH, "\n  -What object do you want the definition of? ");

    BTNode* node = findNode(oracle->tree, object);

    if (node == NULL)
    {
        UI_Say(oracle->speaker, "\n  -I don't know what/who '%s' is.\n", object);
        return;
    }

    if (getLeft(node) == NULL)
    {
        printf("  -");
        definition(oracle, node, NULL);
        printf("\n");
    }
    else
    {
        UI_Say(oracle->speaker, "\n  -It's not an object, are you trying to trick me?..\n");
    }

    free(object);
}

void definition(Oracle* oracle, BTNode* object, BTNode* start)
{
    assert(oracle != NULL);
    assert(object != NULL);

    Stack* stack = getPathFromRoot(oracle->tree, object);

    if (start != NULL)
    {
        while (stackTop(stack) != start)
        {
            stackPop(stack);
        }
    }

    UI_Say(oracle->speaker, "%s is ", getValue(object));

    BTNode* currNode = NULL;
    while (stackSize(stack) > 1)
    {
        currNode = stackPop(stack);

        if (isLeft(stackTop(stack)))
        {
            UI_Say(oracle->speaker, "not ");
        }

        UI_Say(oracle->speaker, "%s", getValue(currNode));

        if (stackSize(stack) > 1)
        {
            printf(", ");
        }
    }

    deleteStack(stack);
}

void comparisonDialog(Oracle* oracle)
{
    assert(oracle != NULL);

    char* str1 = UI_AskStr(getSpeaker(oracle), MAX_STRING_LENGTH, "\n  -What objects do you want the definition of?\n"
                                                                  "   Object1: ");

    char* str2 = UI_AskStr(getSpeaker(oracle), MAX_STRING_LENGTH, "   Object2: ");

    BTNode* object1 = findNode(oracle->tree, str1);
    BTNode* object2 = findNode(oracle->tree, str2);

    if (object1 == NULL || object2 == NULL)
    {
        UI_Say(oracle->speaker, "\n  -I don't know these objects.\n");
        return;
    }

    if (getLeft(object1) != NULL || getLeft(object2) != NULL)
    {
        UI_Say(oracle->speaker, "\n  -Those aren't objects, are you trying to trick me?..\n");
    }
    else
    {
        comparison(oracle, object1, object2);
    }

    free(str1);
    free(str2);
}

void comparison(Oracle* oracle, BTNode* object1, BTNode* object2)
{
    assert(oracle != NULL);
    assert(object1 != NULL);
    assert(object2 != NULL);

    Stack* stack1 = getPathFromRoot(oracle->tree, object1);
    Stack* stack2 = getPathFromRoot(oracle->tree, object2);
    stackPop(stack1);
    stackPop(stack2);

    BTNode* currNode = getRoot(oracle->tree);
    while (stackSize(stack1) > 1 && stackSize(stack2) > 1 && stackTop(stack1) == stackTop(stack2))
    {
        if (currNode == getRoot(oracle->tree))
        {
            UI_Say(oracle->speaker, "   They both are ");
        }

        if (isLeft(stackTop(stack1))) { UI_Say(oracle->speaker, "not "); }

        UI_Say(oracle->speaker, "%s", getValue(currNode));

        currNode = stackTop(stack1);
        stackPop(stack1);
        stackPop(stack2);

        if (stackTop(stack1) == stackTop(stack2))
        {
            printf(", ");
        }
    }

    if (currNode != getRoot(oracle->tree))
    {
        UI_Say(oracle->speaker, "\n   But ");
    }

    if (currNode != getRoot(oracle->tree))
    {
        definition(oracle, object1, currNode);
        UI_Say(oracle->speaker, " and\n   ");
        definition(oracle, object2, currNode);
    }
    else
    {
        definition(oracle, object1, NULL);
        UI_Say(oracle->speaker, " and\n   ");
        definition(oracle, object2, NULL);
    }

    deleteStack(stack1);
    deleteStack(stack2);
}

//-----------------------------------------------------------------------------
//! Creates a stack with the path from node to the root of tree.
//!
//! @param [in] tree
//! @param [in] node
//!
//! @warning Undefined behavior if node isn't in the tree.
//!
//! @return stack with top element being the root and bottom being node.
//-----------------------------------------------------------------------------
Stack* getPathFromRoot(BinaryTree* tree, BTNode* node)
{
    assert(tree != NULL);
    assert(node != NULL);

    Stack* stack = newStack();
    assert(stack != NULL);

    BTNode* currNode = node;
    while (getParent(currNode) != NULL)
    {
        stackPush(stack, currNode);
        currNode = getParent(currNode);
    }

    stackPush(stack, currNode); // root

    return stack;
}

void treeDiagram(Oracle* oracle)
{
    assert(oracle != NULL);
    assert(oracle->tree != NULL);
    
    FILE* file = fopen("tree_diagram.txt", "w");
    assert(file != NULL);

    fprintf(file,
            "digraph structs {\n"
            "\tnode [shape=\"rectangle\", style=\"filled\", fontcolor=\"#DCDCDC\", fillcolor=\"#2F4F4F\"];\n\n");

    if (getRoot(oracle->tree) == NULL)
    {
        fprintf(file,
                "\t\"ROOT\" [label = \"Empty database\"];\n");
    }
    else
    {
        subtreeDiagram(file, getRoot(oracle->tree));
    }

    fprintf(file, "}");

    fclose(file);

    system("dot -Tsvg tree_diagram.txt -o tree_diagram.svg");
    system("start tree_diagram.svg");    
}

void subtreeDiagram(FILE* file, BTNode* node)
{
    assert(file != NULL);
    
    if (node == NULL) { return; }

    fprintf(file, "\t\"%p\" [label=\"%s", node, getValue(node));

    if (getLeft(node) == NULL)
    {
        fprintf(file, "\", shape=\"hexagon\", peripheries = 2, fillcolor=\"#5F9EA0\", fontcolor=\"#F0FFFF\"");
    }
    else
    {
        fprintf(file, "?\"");
    }

    fprintf(file, "];\n");

    if (getParent(node) != NULL)
    {
        if (isLeft(node))
        {
            fprintf(file, "\t\"%p\":sw->\"%p\" [label=\"No\"];\n", getParent(node), node);
        }
        else
        {
            fprintf(file, "\t\"%p\":se->\"%p\" [label=\"Yes\"];\n", getParent(node), node);
        }
    }   

    subtreeDiagram(file, getLeft(node));
    subtreeDiagram(file, getRight(node));
}