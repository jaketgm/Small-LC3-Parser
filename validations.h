#ifndef VALIDATIONS_H
#define VALIDATIONS_H

#include "parsing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

Tokens validateToken(const char *token) 
{
    if (strcmp(token, "ADD") == 0)
    {
        return ADD;
    }
    else if (strcmp(token, "AND") == 0)
    {
        return AND;
    }
    else if (strncmp(token, "BR", 2) == 0 && strlen(token) == 2) 
    {
        return BR;
    }
    else if (strcmp(token, "LD") == 0)
    {
        return LD;
    }
    else if (strcmp(token, "LDI") == 0)
    {
        return LDI;
    }
    else if (strcmp(token, "LDR") == 0)
    {
        return LDR;
    }
    else if (strcmp(token, "LEA") == 0)
    {
        return LEA;
    }
    else if (strcmp(token, "NOT") == 0)
    {
        return NOT;
    }
    else if (strcmp(token, "ST") == 0)
    {
        return ST;
    }
    else if (strcmp(token, "STI") == 0)
    {
        return STI;
    }
    else if (strcmp(token, "STR") == 0)
    {
        return STR;
    }
    else if (strcmp(token, "TRAP") == 0)
    {
        return TRAP;
    }
    else if (strcmp(token, "ORIG") == 0)
    {
        return ORIG;
    }
    else if (strcmp(token, "END") == 0)
    {
        return END;
    }
    else if (strcmp(token, "#") == 0)
    {
        return HASH;
    }
    else if (strcmp(token, ";") == 0)
    {
        return SEMI;
    }
    else if (strcmp(token, "BLKW") == 0)
    {
        return BLKW;
    }
    else if (strcmp(token, ".FILL") == 0)
    {
        return FILL;
    }
    else 
    {
        return INVALID_TOKEN;
    }
}

RegisterTokens validateRegisterToken(const char *regstr)
{
    if (strcmp(regstr, "R0") == 0)
    {
        return R0;
    }
    else if (strcmp(regstr, "R1") == 0)
    {
        return R1;
    }
    else if (strcmp(regstr, "R2") == 0)
    {
        return R2;
    }
    else if (strcmp(regstr, "R3") == 0)
    {
        return R3;
    }
    else if (strcmp(regstr, "R4") == 0)
    {
        return R4;
    }
    else if (strcmp(regstr, "R5") == 0)
    {
        return R5;
    }
    else if (strcmp(regstr, "R6") == 0)
    {
        return R6;
    }
    else if (strcmp(regstr, "R7") == 0)
    {
        return R7;
    }
    else
    {
        return INVALID_REGISTER;
    }
}

bool isRegister(char *token)
{
    if (validateRegisterToken(token) != INVALID_REGISTER)
    {
        return true;
    }
    return false;
}

bool isSoloLabel(const char* token)
{
    char* colonPos = strchr(token, ':');
    if (colonPos != NULL && *(colonPos + 1) == '\0') 
    {
        // Check if the rest of the token is a valid label (you need to define what's valid)
        size_t labelLength = colonPos - token;
        return true; // For now, we assume all labels ending with a colon are valid.
    }
    return false;
}

bool isImm5(char *imm5) 
{
    if (imm5[0] != '#') 
    {
        return false;
    }

    int val = atoi(imm5 + 1);
    if (val >= -32768 && val <= 32767) 
    {
        return true;
    }
    return false;
}

bool isValidBranchCondition(char condition)
{
    if (condition == 'n' || condition == 'z' || condition == 'p')
    {
        return true;
    }
    return false;
}

bool isValidLabel(char *label, char labels[][MAX_LABEL_LEN], int count) 
{
    printf("Validating label: %s\n", label);
    int i;
    for (i = 0; i < count; i++) 
    {
        if (strcmp(label, labels[i]) == 0) 
        {
            printf("Label found and valid: %s\n", label);
            return true;
        }
    }
    printf("Label not found: %s\n", label);
    return false;
}

bool isLabelDefinition(char *token)
{
    if (validateToken(token) == INVALID_TOKEN)
    {
        return true;
    }
    return false;
}

bool isBRInstruction(char *token) 
{
    // Check if token starts with "BR" and optionally followed by any of 'n', 'z', 'p'.
    if (strncmp(token, "BR", 2) == 0) 
    {
        int i;
        for (i = 2; token[i] != '\0'; i++) 
        {
            if (token[i] != 'n' && token[i] != 'z' && token[i] != 'p') 
            {
                return false; // Extra characters that are not 'n', 'z', or 'p'.
            }
        }
        return true; // Token is a valid BR instruction.
    }
    return false; // Token does not start with "BR".
}

bool isOffset6(char *offset) 
{
    if (offset[0] != '#') 
    {
        return false;
    }

    int val = atoi(offset + 1); 
    if (val >= -32 && val <= 31) 
    {
        return true;
    }
    return false;
}

bool isValidTrapVector(const char *offset) 
{
    unsigned int val;
    sscanf(offset, "%x", &val); // Convert hex string to unsigned int
    return val <= 0xFF; // Trap vectors are 8-bit, so valid if within this range
}

#endif