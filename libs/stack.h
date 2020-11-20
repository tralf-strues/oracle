#pragma once

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#ifdef STACK_DEBUG_MODE
#define STACK_DEBUG_LVL3 
#endif

#if !defined(STACK_DEBUG_LVL1) && !defined(STACK_DEBUG_LVL2) && !defined(STACK_DEBUG_LVL3)
#define ASSERT_STACK_OK(stack) assert(stack != NULL);
#else
#define STACK_DEBUG_MODE
#endif

#ifdef STACK_DEBUG_LVL1
#define ASSERT_STACK_OK(stack) if(stack == NULL || !stackOk(stack)) { dump(stack); LG_Close(); assert(! "OK"); }
#define STACK_POISON           nan("")
#define IS_STACK_POISON(value) isnan(value)
#endif

#ifdef STACK_DEBUG_LVL2
#define ASSERT_STACK_OK(stack) if(stack == NULL || !stackOk(stack)) { dump(stack); LG_Close(); assert(! "OK"); }
#define STACK_POISON           nan("")
#define IS_STACK_POISON(value) isnan(value)

#define STACK_CANARIES_ENABLED
#endif

#ifdef STACK_DEBUG_LVL3
#define ASSERT_STACK_OK(stack) if(stack == NULL || !stackOk(stack)) { dump(stack); LG_Close(); assert(! "OK"); }
#define STACK_POISON           nan("")
#define IS_STACK_POISON(value) isnan(value)
#define STACK_CANARIES_ENABLED

#define STACK_ARRAY_HASHING
#endif

#ifdef STACK_CANARIES_ENABLED
static uint32_t STACK_ARRAY_CANARY_L  = 0xBADC0FFE;
static uint32_t STACK_ARRAY_CANARY_R  = 0xDEADBEEF;

static uint32_t STACK_STRUCT_CANARY_L = 0xDEDDED32;
static uint32_t STACK_STRUCT_CANARY_R = 0xFACEBEEF;
#endif

#ifdef STACK_DEBUG_MODE
#define stackConstruct(stack, capacity) fstackConstruct(stack, capacity, &#stack[1]);
#define stackDefaultConstruct(stack)    fstackConstruct(stack, &#stack[1]);
#else
#define stackConstruct(stack, capacity) fstackConstruct(stack, capacity);
#define stackDefaultConstruct(stack)    fstackConstruct(stack);
#endif

static const double STACK_EXPAND_MULTIPLIER = 1.8;
static const size_t DEFAULT_STACK_CAPACITY  = 10;
static const size_t MINIMAL_STACK_CAPACITY  = 3;

#ifdef STACK_DEBUG_MODE
static const char* DYNAMICALLY_CREATED_STACK_NAME = "no name, created dynamically";
#endif

enum StackError
{
    STACK_NO_ERROR            = 0x00,
    STACK_POP_FROM_EMPTY      = 0x01,
    STACK_TOP_FROM_EMPTY      = 0x02,
    STACK_CONSTRUCTION_FAILED = 0x04,
    STACK_REALLOCATION_FAILED = 0x08,
    STACK_NOT_CONSTRUCTED_USE = 0x10,
    STACK_DESTRUCTED_USE      = 0x20,
    STACK_MEMORY_CORRUPTION   = 0x40
};

enum StackStatus
{
    STACK_STATUS_NOT_CONSTRUCTED,
    STACK_STATUS_CONSTRUCTED,
    STACK_STATUS_DESTRUCTED
};

struct Stack
{
    #ifdef STACK_CANARIES_ENABLED
    uint32_t canaryL = STACK_STRUCT_CANARY_L;
    #endif

    #ifdef STACK_DEBUG_MODE
    const char* name = NULL;
    #endif

    size_t       size         = 0;
    size_t       capacity     = 0;
    elem_t*      dynamicArray = NULL;
    StackStatus  status       = STACK_STATUS_NOT_CONSTRUCTED;
    uint32_t     errorStatus  = STACK_NO_ERROR;

    #ifdef STACK_CANARIES_ENABLED
    uint32_t canaryR = STACK_STRUCT_CANARY_R;
    #endif
};

#ifdef STACK_DEBUG_MODE
Stack*       fstackConstruct  (Stack* stack, size_t capacity, const char* stackName);
Stack*       fstackConstruct  (Stack* stack, const char* stackName);
#else
Stack*       fstackConstruct  (Stack* stack, size_t capacity);
Stack*       fstackConstruct  (Stack* stack);
#endif

Stack*       newStack         (size_t capacity);
Stack*       newStack         ();
void         stackDestruct    (Stack* stack);
void         deleteStack      (Stack* stack);

size_t       stackSize        (Stack* stack);
size_t       stackCapacity    (Stack* stack);
uint32_t     stackErrorStatus (Stack* stack);
const char*  stackGetErrorStr (StackError error);
                              
StackError   stackPush        (Stack* stack, elem_t value);
elem_t       stackPop         (Stack* stack);
elem_t       stackTop         (Stack* stack);
void         stackClear       (Stack* stack);
bool         stackShrinkToFit (Stack* stack);

bool         stackOk          (Stack* stack);    
void         dump             (Stack* stack);

const unsigned char STACK_MAX_ERRORS_COUNT = 16;

void stackSetError(Stack* stack, StackError error)
{
    assert(stack != NULL);

    stack->errorStatus = stack->errorStatus | error;
}

#ifdef STACK_POISON
    #define PUT_POISON(begin, end) putPoison(begin, end)

//-----------------------------------------------------------------------------
//! Sets values [begin, end) to POISON.
//!
//! @param [in]  begin  
//! @param [in]  end   
//-----------------------------------------------------------------------------
void putPoison(elem_t* begin, elem_t* end)
{
    assert(begin != NULL);
    assert(end   != NULL);

    for (; begin < end; begin++)
    {
        *begin = STACK_POISON;
    }
}

//-----------------------------------------------------------------------------
//! Checks whether or not stack's dynamicArray has POISON in unused space and 
//! sets stack's errorStatus to MEMORY_CORRUPTION if there's an unused element
//! that doesn't have POISON.
//!
//! @param [in]  stack    
//!
//! @return whether or not stack's dynamicArray has POISON in unused space.
//-----------------------------------------------------------------------------
bool stackCheckPoison(Stack* stack)
{
    for (size_t i = 0; i < stack->size; i++)
    {
        if (IS_STACK_POISON(stack->dynamicArray[i]))
        {
            stackSetError(stack, STACK_MEMORY_CORRUPTION);
            return false;
        }
    }

    for (size_t i = stack->size; i < stack->capacity; i++)
    {
        if (!IS_STACK_POISON(stack->dynamicArray[i]))
        {
            stackSetError(stack, STACK_MEMORY_CORRUPTION);
            return false;
        }
    }

    return true;
}

#else
    #define PUT_POISON(begin, end) 
#endif

#ifdef STACK_CANARIES_ENABLED
    #define GET_CANARY(memBlock, memBlockSize, side)               getCanary  (memBlock, memBlockSize, side)
    #define SET_CANARY(memBlock, memBlockSize, canary, side)       setCanary  (memBlock, memBlockSize, canary,  side)
    #define SET_CANARIES(memBlock, memBlockSize, canaryL, canaryR) setCanaries(memBlock, memBlockSize, canaryL, canaryR)

//-----------------------------------------------------------------------------
//! Returns left (side = 'l') or right (side = 'r') canary of the memBlock. 
//! Undefined behavior in case there's no canary protection for memBlock.
//!
//! @param [in]  memBlock  
//! @param [in]  memBlockSize   
//! @param [in]  side          indicates which canary to return - 'l' for
//!                            left and 'r' for right
//!
//! @note dependent on whether or not STACK_ARRAY_HASHING is defined (hash is
//!       located between memBlock and right canary).
//!
//! @return canary value or 0 if side isn't 'l' or 'r'
//-----------------------------------------------------------------------------
uint32_t getCanary(void* memBlock, size_t memBlockSize, char side)
{
    if (side == 'l')
    {
        return *(uint32_t*)((char*) memBlock - sizeof(uint32_t));
    }
    else if (side == 'r')
    {
        return *(uint32_t*)((char*) memBlock + memBlockSize 
                            
                            #ifdef STACK_ARRAY_HASHING
                            + sizeof(uint32_t)
                            #endif

                            );
    }

    return 0;
}

//-----------------------------------------------------------------------------
//! Sets left (side = 'l') or right (side = 'r') canary of the memBlock. 
//! Does nothing if side isn't 'l' or 'r'. Undefined behavior in case there's 
//! no canary protection for memBlock.
//!
//! @param [in]  memBlock  
//! @param [in]  memBlockSize   
//! @param [in]  canary   
//! @param [in]  side          indicates which canary to return - 'l' for
//!                            left and 'r' for right
//!
//! @note dependent on whether or not STACK_ARRAY_HASHING is defined (hash is
//!       located between memBlock and right canary).
//-----------------------------------------------------------------------------
void setCanary(void* memBlock, size_t memBlockSize, uint32_t canary, char side)
{
    if (side == 'l')
    {
        *(uint32_t*)((char*) memBlock - sizeof(uint32_t)) = canary;
    }
    else if (side == 'r')
    {
        *(uint32_t*)((char*) memBlock + memBlockSize
                     
                     #ifdef STACK_ARRAY_HASHING
                     + sizeof(uint32_t)
                     #endif
                     
                     )  = canary;
    }
}

//-----------------------------------------------------------------------------
//! Sets left and right canaries of the memBlock. Undefined behavior in case 
//! there's no canary protection for memBlock.
//!
//! @param [in]  memBlock  
//! @param [in]  memBlockSize   
//! @param [in]  canaryL   
//! @param [in]  canaryR   
//!
//! @note dependent on whether or not STACK_ARRAY_HASHING is defined (hash is
//!       located between memBlock and right canary).
//-----------------------------------------------------------------------------
void setCanaries(void* memBlock, size_t memBlockSize, uint32_t canaryL, uint32_t canaryR)
{
    setCanary(memBlock, memBlockSize, canaryL, 'l');
    setCanary(memBlock, memBlockSize, canaryR, 'r');
}

//-----------------------------------------------------------------------------
//! Checks whether or not stack's canaries have correct values and sets stack's
//! errorStatus to MEMORY_CORRUPTION if they don't.
//!
//! @param [in]  stack    
//!
//! @return whether or not stack's canaries have correct values.
//-----------------------------------------------------------------------------
bool stackCheckCanaries(Stack* stack)
{

    if (getCanary((void*)stack->dynamicArray, stack->capacity * sizeof(elem_t), 'l') != STACK_ARRAY_CANARY_L  || 
        getCanary((void*)stack->dynamicArray, stack->capacity * sizeof(elem_t), 'r') != STACK_ARRAY_CANARY_R  ||
        stack->canaryL                                                               != STACK_STRUCT_CANARY_L ||
        stack->canaryR                                                               != STACK_STRUCT_CANARY_R)
    {
        stackSetError(stack, STACK_MEMORY_CORRUPTION);
        return false;
    }

    return true;
}

#else
    #define GET_CANARY(memBlock, memBlockSize, side)               
    #define SET_CANARY(memBlock, memBlockSize, canary, side)       
    #define SET_CANARIES(memBlock, memBlockSize, canaryL, canaryR) 
#endif

#ifdef STACK_ARRAY_HASHING
    #define STACK_UPDATE_HASH(stack) stackUpdateHash(stack)

//-----------------------------------------------------------------------------
//! Updates hash value. Computes XOR for rotated right hash and current byte 
//! (does this for each byte of memBlock).
//!
//! @param [in]  memBlock  
//! @param [in]  memBlockSize   
//! @param [out] hash   
//-----------------------------------------------------------------------------
void updateHash(void* memBlock, size_t memBlockSize, uint32_t* hash, uint32_t baseHashValue)
{
    *hash = baseHashValue;
    for (char* currByte = (char*) memBlock; currByte < (char*)memBlock + memBlockSize; currByte++)
    {
        *hash = ((*hash << 1) + ((*hash >> (8 * sizeof(*hash) - 1)) & 1)) ^ *currByte;
    }
}

void stackUpdateHash(Stack* stack)
{
    updateHash((void*)stack->dynamicArray, 
               stack->capacity * sizeof(elem_t), 
               (uint32_t*) &stack->dynamicArray[stack->capacity], 
               2 * (stack->size << 1) + 3 * (stack->capacity >> 1));
}

//-----------------------------------------------------------------------------
//! Checks whether or not stack's hash has correct value.
//!
//! @param [in]  stack    
//!
//! @return whether or not stack's hash has correct value.
//-----------------------------------------------------------------------------
bool stackCheckHash(Stack* stack)
{
    uint32_t prevHash = *(uint32_t*) &stack->dynamicArray[stack->capacity];
    STACK_UPDATE_HASH(stack);
    uint32_t newHash  = *(uint32_t*) &stack->dynamicArray[stack->capacity];

    if (prevHash != newHash)
    {
        stackSetError(stack, STACK_MEMORY_CORRUPTION);
        return false;
    }

    return true;
}

#else
    #define STACK_UPDATE_HASH(stack) 
#endif

//-----------------------------------------------------------------------------
//! Stack's constructor. Allocates max(capacity, MINIMAL_STACK_CAPACITY) 
//! objects of type elem_t.
//!
//! @param [out]  stack  
//! @param [in]   capacity   
//!
//! @note if calloc returned NULL then sets stack's errorStatus to 
//!       INITIALIZATION_FAILED.
//!
//! @return stack if constructed successfully or NULL otherwise.
//-----------------------------------------------------------------------------
#ifdef STACK_DEBUG_MODE
Stack* fstackConstruct(Stack* stack, size_t capacity, const char* stackName)
#else
Stack* fstackConstruct(Stack* stack, size_t capacity)
#endif
{
    assert(stack != NULL);
    assert(capacity > 0);

    #ifdef STACK_DEBUG_MODE
    stack->name = stackName;
    #endif

    stack->size         = 0;
    stack->capacity     = capacity > MINIMAL_STACK_CAPACITY ? capacity : MINIMAL_STACK_CAPACITY;

    stack->dynamicArray = (elem_t*) ((char*)calloc(1, stack->capacity * sizeof(elem_t) 

                                                   #ifdef STACK_CANARIES_ENABLED
                                                   + sizeof(STACK_ARRAY_CANARY_L) 
                                                   + sizeof(STACK_ARRAY_CANARY_R) 
                                                   #endif
                                               
                                                   #ifdef STACK_ARRAY_HASHING
                                                   + sizeof(uint32_t)
                                                   #endif
                                                   )
                                     #ifdef STACK_CANARIES_ENABLED
                                     + sizeof(STACK_ARRAY_CANARY_L)
                                     #endif 
                                    );

    if (stack->dynamicArray == NULL) 
    {
        stackSetError(stack, STACK_CONSTRUCTION_FAILED);
        ASSERT_STACK_OK(stack);
        return NULL;
    }

    SET_CANARIES((void*)stack->dynamicArray, stack->capacity * sizeof(elem_t), STACK_ARRAY_CANARY_L, STACK_ARRAY_CANARY_R);
    
    PUT_POISON(stack->dynamicArray, stack->dynamicArray + stack->capacity);
    STACK_UPDATE_HASH(stack);

    stack->status = STACK_STATUS_CONSTRUCTED;
    ASSERT_STACK_OK(stack);

    return stack;
}

//-----------------------------------------------------------------------------
//! Stack's constructor. Allocates DEFAULT_STACK_CAPACITY objects of type 
//! elem_t.
//!
//! @param [out]  stack   
//!
//! @note if calloc returned NULL then sets stack's errorStatus to 
//!       INITIALIZATION_FAILED.
//!
//! @return stack if constructed successfully or NULL otherwise.
//-----------------------------------------------------------------------------
#ifdef STACK_DEBUG_MODE
Stack* fstackConstruct(Stack* stack, const char* stackName)
#else
Stack* fstackConstruct(Stack* stack)
#endif
{
    #ifdef STACK_DEBUG_MODE
    stack = fstackConstruct(stack, DEFAULT_STACK_CAPACITY, stackName);
    #else
    stack = fstackConstruct(stack, DEFAULT_STACK_CAPACITY);
    #endif

    ASSERT_STACK_OK(stack);

    return stack;
}

//-----------------------------------------------------------------------------
//! Allocates a Stack, calls constructor and returns the pointer to this Stack.
//!
//! @param [in]  capacity   
//!
//! @note if calloc returned NULL then sets stack's errorStatus to 
//!       INITIALIZATION_FAILED.
//! @note if capacity is less than MINIMAL_STACK_CAPACITY, than sets capacity
//!       to MINIMAL_STACK_CAPACITY.
//!
//! @return stack if constructed successfully or NULL otherwise.
//-----------------------------------------------------------------------------
Stack* newStack(size_t capacity)
{
    assert(capacity > 0);

    Stack* newStack = (Stack*) calloc(1, sizeof(Stack));

    #ifdef STACK_DEBUG_MODE
    fstackConstruct(newStack, DYNAMICALLY_CREATED_STACK_NAME);
    #else
    fstackConstruct(newStack);
    #endif

    return newStack;
}

//-----------------------------------------------------------------------------
//! Allocates a Stack, calls constructor and returns the pointer to this Stack.
//!
//! @note if calloc returned NULL then sets stack's errorStatus to 
//!       INITIALIZATION_FAILED.
//! @note starting capacity is DEFAULT_STACK_CAPACITY
//!
//! @return stack if constructed successfully or NULL otherwise.
//-----------------------------------------------------------------------------
Stack* newStack()
{
    return newStack(DEFAULT_STACK_CAPACITY);
}

//-----------------------------------------------------------------------------
//! Stack's destructor. Calls free for stack's dynamicArray.
//!
//! @param [out]  stack   
//-----------------------------------------------------------------------------
void stackDestruct(Stack* stack)
{
    ASSERT_STACK_OK(stack);
    PUT_POISON(stack->dynamicArray, stack->dynamicArray + stack->capacity);

    #ifdef STACK_CANARIES_ENABLED
    free((char*)stack->dynamicArray - sizeof(STACK_ARRAY_CANARY_L));
    #else
    free(stack->dynamicArray);
    #endif

    stack->size         = 0;
    stack->capacity     = 0;
    stack->dynamicArray = NULL;

    stack->status = STACK_STATUS_DESTRUCTED;
}

//-----------------------------------------------------------------------------
//! Calls destructor of stack and free of stack. Undefined behavior if stack
//! wasn't created dynamically using newStack() or an allocator.
//!
//! @param [out]  stack   
//-----------------------------------------------------------------------------
void deleteStack(Stack* stack)
{
    ASSERT_STACK_OK(stack);

    stackDestruct(stack);

    free(stack);
}

//-----------------------------------------------------------------------------
//! @param [in]  stack   
//!
//! @return current size of stack.
//-----------------------------------------------------------------------------
size_t stackSize(Stack* stack)
{
    ASSERT_STACK_OK(stack);

    return stack->size;
}

//-----------------------------------------------------------------------------
//! @param [in]  stack   
//!
//! @return current capacity of stack.
//-----------------------------------------------------------------------------
size_t stackCapacity(Stack* stack)
{
    ASSERT_STACK_OK(stack);

    return stack->capacity;
}

//-----------------------------------------------------------------------------
//! @param [in] stack   
//!
//! @return current stack's errorStatus.
//-----------------------------------------------------------------------------
uint32_t stackErrorStatus(Stack* stack)
{
    assert(stack != NULL);

    return stack->errorStatus;
}

//-----------------------------------------------------------------------------
//! @param [in] error   
//!
//! @return string representation of error.
//-----------------------------------------------------------------------------
const char* stackGetErrorStr(StackError error)
{
    #define TO_STR(value) #value

    switch (error)
    {
        case STACK_NO_ERROR:            return TO_STR(STACK_NO_ERROR);
        case STACK_POP_FROM_EMPTY:      return TO_STR(STACK_POP_FROM_EMPTY);
        case STACK_TOP_FROM_EMPTY:      return TO_STR(STACK_TOP_FROM_EMPTY);
        case STACK_CONSTRUCTION_FAILED: return TO_STR(STACK_CONSTRUCTION_FAILED);
        case STACK_REALLOCATION_FAILED: return TO_STR(STACK_REALLOCATION_FAILED);
        case STACK_NOT_CONSTRUCTED_USE: return TO_STR(STACK_NOT_CONSTRUCTED_USE);
        case STACK_DESTRUCTED_USE:      return TO_STR(STACK_DESTRUCTED_USE);
        case STACK_MEMORY_CORRUPTION:   return TO_STR(STACK_MEMORY_CORRUPTION);

        default:                        return NULL;
    }

    #undef TO_STR
}

//-----------------------------------------------------------------------------
//! Resizes stack's array to newCapacity. If reallocation was unsuccessful, 
//! returns NULL, sets stack's errorStatus to REALLOCATION_FAILED, but current
//! elements of the stack won't be removed.
//!
//! @param [out]  stack   
//! @param [in]   newCapacity   
//!
//! @note if newCapacity is less than stack's size then there will be DATA 
//!       LOSS!
//!
//! @return pointer to the new stack's array if reallocation was successful 
//!         or NULL otherwise.
//-----------------------------------------------------------------------------
elem_t* resizeArray(Stack* stack, size_t newCapacity)
{
    elem_t* newDynamicArray = NULL;
    ASSERT_STACK_OK(stack);

    newDynamicArray = (elem_t*) ((char*) realloc((char*) stack->dynamicArray 
                                                 
                                                 #ifdef STACK_CANARIES_ENABLED
                                                 - sizeof(STACK_ARRAY_CANARY_L)
                                                 #endif
                                                 , 
                                                 newCapacity * sizeof(elem_t) 

                                                 #ifdef STACK_CANARIES_ENABLED
                                                 + sizeof(STACK_ARRAY_CANARY_L) 
                                                 + sizeof(STACK_ARRAY_CANARY_R)
                                                 #endif

                                                 #ifdef STACK_ARRAY_HASHING
                                                 + sizeof(uint32_t)
                                                 #endif
                                                ) 

                                 #ifdef STACK_CANARIES_ENABLED
                                 + sizeof(STACK_ARRAY_CANARY_L)
                                 #endif  
                                 );

    if (newDynamicArray == NULL)
    {
        stackSetError(stack, STACK_REALLOCATION_FAILED);
        ASSERT_STACK_OK(stack);
    }
    else
    {
        stack->dynamicArray = newDynamicArray;
        stack->capacity     = newCapacity;

        PUT_POISON(stack->dynamicArray + stack->size, stack->dynamicArray + stack->capacity);
        SET_CANARIES((void*)stack->dynamicArray, stack->capacity * sizeof(elem_t), STACK_ARRAY_CANARY_L, STACK_ARRAY_CANARY_R);
        STACK_UPDATE_HASH(stack);
    }

    return newDynamicArray;
}

//-----------------------------------------------------------------------------
//! Push value to stack.
//!
//! @param [out]  stack   
//! @param [in]   value   
//!
//! @note if realloc returned NULL then sets stack's errorStatus to 
//!       REALLOCATION_FAILED.
//!
//! @return NO_ERROR if pushed successfully or some STACK_ERRORS code 
//!         otherwise.
//-----------------------------------------------------------------------------
StackError stackPush(Stack* stack, elem_t value)
{
    ASSERT_STACK_OK(stack);

    if (stack->size == stack->capacity)
    {
        elem_t* newDynamicArray = resizeArray(stack, stack->capacity * STACK_EXPAND_MULTIPLIER);

        if (newDynamicArray == NULL)
        {
            return STACK_REALLOCATION_FAILED;
        }
    }

    stack->dynamicArray[stack->size] = value;
    stack->size++;

    STACK_UPDATE_HASH(stack);
    ASSERT_STACK_OK(stack);

    return STACK_NO_ERROR;
}

//-----------------------------------------------------------------------------
//! Removes the element on top of the stack and returns it. Undefined behavior
//! if the stack is empty!
//!
//! @param [out]  stack    
//!
//! @note if pop is called from an empty stack then sets the stack's 
//!       errorStatus to POP_FROM_EMPTY.
//!
//! @return the element on top of the stack.
//-----------------------------------------------------------------------------
elem_t stackPop(Stack* stack)
{
    ASSERT_STACK_OK(stack);
 
    if (stack->size == 0)
    {
        stackSetError(stack, STACK_POP_FROM_EMPTY);
        ASSERT_STACK_OK(stack);
    }

    stack->size--;
    elem_t returnValue = stack->dynamicArray[stack->size];

    PUT_POISON(stack->dynamicArray + stack->size, stack->dynamicArray + stack->size + 1);
    STACK_UPDATE_HASH(stack);
    ASSERT_STACK_OK(stack);

    return returnValue;
}

//-----------------------------------------------------------------------------
//! Returns the element on top of the stack. Undefined behavior if the stack 
//! is empty!
//!
//! @param [out]  stack    
//!
//! @note if top is called from an empty stack then sets the stack's 
//!       errorStatus to TOP_FROM_EMPTY.
//!
//! @return the element on top of the stack.
//-----------------------------------------------------------------------------
elem_t stackTop(Stack* stack)
{
    ASSERT_STACK_OK(stack);

    if (stack->size == 0)
    {
        stackSetError(stack, STACK_TOP_FROM_EMPTY);
        ASSERT_STACK_OK(stack);
    }

    return stack->dynamicArray[stack->size - 1];
}

//-----------------------------------------------------------------------------
//! Empties stack.
//!
//! @param [out]  stack    
//-----------------------------------------------------------------------------
void stackClear(Stack* stack)
{
    ASSERT_STACK_OK(stack);

    stack->size = 0;

    if (!stackShrinkToFit(stack))
    {
        stackSetError(stack, STACK_REALLOCATION_FAILED);
        ASSERT_STACK_OK(stack);
    }

    PUT_POISON(stack->dynamicArray, stack->dynamicArray + stack->capacity);
    STACK_UPDATE_HASH(stack);
    ASSERT_STACK_OK(stack);
}

//-----------------------------------------------------------------------------
//! Resizes stack's array to max(stack's size, MINIMAL_STACK_CAPACITY). If 
//! reallocation was unsuccessful, returns false, sets stack's errorStatus to 
//! REALLOCATION_FAILED, but current elements of the stack won't be removed.
//!
//! @param [out]  stack   
//!
//! @return whether or not shrinking was successful.
//-----------------------------------------------------------------------------
bool stackShrinkToFit(Stack* stack)
{
    ASSERT_STACK_OK(stack);

    if (stack->capacity <= MINIMAL_STACK_CAPACITY)
    {
        return false;
    }

    elem_t* newDynamicArray = resizeArray(stack, stack->size);
    if (newDynamicArray == NULL)
    {
        stackSetError(stack, STACK_REALLOCATION_FAILED);
        ASSERT_STACK_OK(stack);
        return false;
    }

    STACK_UPDATE_HASH(stack);
    ASSERT_STACK_OK(stack);

    return true;
}

//-----------------------------------------------------------------------------
//! Checks whether or not stack is working correctly.
//!
//! @param [out]  stack   
//!
//! @return whether or not stack is working correctly.
//-----------------------------------------------------------------------------
bool stackOk(Stack* stack)
{
    assert(stack != NULL);

    if (stack == NULL)
    {
        return false;
    }

    if (stack->errorStatus != STACK_NO_ERROR)
    {
        return false;
    }

    if (stack->status == STACK_STATUS_NOT_CONSTRUCTED)
    {
        stackSetError(stack, STACK_NOT_CONSTRUCTED_USE);
        return false;
    }

    if (stack->status == STACK_STATUS_DESTRUCTED)
    {
        stackSetError(stack, STACK_DESTRUCTED_USE);
        return false;
    }

    if (stack->size < 0 || stack->size > stack->capacity)
    {
        stackSetError(stack, STACK_MEMORY_CORRUPTION);
        return false;
    }

    if (stack->dynamicArray == NULL)
    {
        stackSetError(stack, STACK_MEMORY_CORRUPTION);
        return false;
    }

    #ifdef STACK_POISON
    if (!stackCheckPoison(stack))
    {
        return false;
    }
    #endif

    #ifdef STACK_CANARIES_ENABLED
    if (!stackCheckCanaries(stack))
    {
        return false;
    }
    #endif

    #ifdef STACK_ARRAY_HASHING
    if (!stackCheckHash(stack))
    {
        return false;
    }
    #endif

    return true;
}

void dumpPrintErrors(Stack* stack, const char* indentation)
{
    assert(stack != NULL);

    if (stack->errorStatus == STACK_NO_ERROR)
    {
        LG_Write(stackGetErrorStr(STACK_NO_ERROR), LG_STYLE_CLASS_GOOD);
    }

    unsigned char errorsCount = 0;
    uint32_t      currBit     = 0;
    for (unsigned char i = 0; i < STACK_MAX_ERRORS_COUNT; i++)
    {
        currBit = (stack->errorStatus >> i) & 0x01;

        if (currBit == 1)
        {
            errorsCount++;
        }
    }

    unsigned char errorsPrinted = 0;
    for (unsigned char i = 0; i < STACK_MAX_ERRORS_COUNT; i++)
    {
        currBit = (stack->errorStatus >> i) & 0x01;

        if (currBit == 1)
        {
            currBit = currBit << i;

            if (errorsPrinted > 0 && indentation != NULL)
            {
                LG_Write(",\n%s", indentation);
            } 
            else if (errorsPrinted > 0)
            {
                LG_Write(",");
            }

            LG_Write("ERROR %u: %s", LG_STYLE_CLASS_ERROR, currBit, stackGetErrorStr((StackError) currBit));

            if (errorsCount == 1) { return; }

            errorsPrinted++;
        }
    }
}

//-----------------------------------------------------------------------------
//! Uses logGenerator to dump stack to html log file. 
//!
//! @param [out]  stack   
//-----------------------------------------------------------------------------
void dump(Stack* stack)
{
    assert(stack != NULL);

    if (!LG_IsInitialized())
    {
        LG_Init();
    }
    
    LG_WriteMessageStart(LG_COLOR_BLACK);
    LG_Write("Stack (");
    dumpPrintErrors(stack, "       ");

    if ((stack->errorStatus & STACK_NOT_CONSTRUCTED_USE) != 0 || (stack->errorStatus & STACK_DESTRUCTED_USE) != 0)
    {
        LG_Write(") [0x%X] \n", stack);
    }
    else
    {
        LG_Write(") [0x%X] "

                 #ifdef STACK_DEBUG_MODE
                 "\"%s\""
                 #endif

                 "\n"
                 "{\n"  

                 #ifdef STACK_CANARIES_ENABLED
                 "    canaryL: 0x%lX | must be 0x%lX\n"
                 "    canaryR: 0x%lX | must be 0x%lX\n"
                 #endif

                 "    size         = %lu\n"
                 "    capacity     = %lu\n"
                 "    dynamicArray [0x%X]\n"
                 "    {\n"

                 #ifdef STACK_CANARIES_ENABLED
                 "        canaryL: 0x%lX | must be 0x%lX\n"
                 "        canaryR: 0x%lX | must be 0x%lX\n"
                 #endif

                 #ifdef STACK_ARRAY_HASHING
                 "        hash:    0x%lX (decimal = %lu)\n"
                 #endif

                 ,
                 stack, 

                 #ifdef STACK_DEBUG_MODE
                 stack->name, 
                 #endif

                 #ifdef STACK_CANARIES_ENABLED
                 stack->canaryL, STACK_STRUCT_CANARY_L,
                 stack->canaryR, STACK_STRUCT_CANARY_R,
                 #endif

                 stack->size, 
                 stack->capacity, 
                 stack->dynamicArray
                 
                 #ifdef STACK_CANARIES_ENABLED
                 ,getCanary((void*)stack->dynamicArray, stack->capacity * sizeof(elem_t), 'l'),
                  STACK_ARRAY_CANARY_L,
                  getCanary((void*)stack->dynamicArray, stack->capacity * sizeof(elem_t), 'r'),
                  STACK_ARRAY_CANARY_R
                 #endif  

                 #ifdef STACK_ARRAY_HASHING
                 ,(uint32_t*) &stack->dynamicArray[stack->capacity],
                  (uint32_t*) &stack->dynamicArray[stack->capacity]
                 #endif
        );

        for (size_t i = 0; i < stack->capacity; i++)
        {
            if (i < stack->size)
            {
                LG_Write("        *[%lu]\t= %lg ", 
                         i, stack->dynamicArray[i]);
            }
            else
            {
                LG_Write("         [%lu]\t= %lg ", 
                         i, stack->dynamicArray[i]);
            }

            #ifdef STACK_POISON
            if (IS_STACK_POISON(stack->dynamicArray[i]))
            {
                LG_Write("(POISON!)");
            }
            #endif

            LG_Write("\n");
        }

        LG_Write("    }\n"
                 "}\n");

    }

    LG_WriteMessageEnd();
}

