#ifndef PARSING_H
#define PARSING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool parseORIG(char *source, int *minIndex, unsigned int *address)
{
    char *endPtr;

    while (isspace(source[*minIndex])) (*minIndex)++;
    if (source[*minIndex] != 'x' && source[*minIndex] != 'X') return false; // Ensure it starts with 'x'
    (*minIndex)++; // Skip 'x'

    // Convert the hexadecimal string to an unsigned int
    *address = strtoul(source + *minIndex, &endPtr, 16);

    *minIndex += (endPtr - (source + *minIndex));

    // If endPtr is not at a whitespace or end of string, parsing failed
    if (*endPtr != '\0' && !isspace(*endPtr)) 
    {
        return false; // Failed
    }

    return true; // Success
}

/* 
    ADD -> Validate
    if 
        3 parameters connected via ','
        cases:
            DR, SR1, SR2
            DR, SR1, Imm5
    else
        Invalid
*/
bool parseADD(char *source, int *minIndex, char *operandsOut) 
{
    int tokenCount;
    char operands[256] = "";
    
    for (tokenCount = 0; tokenCount < 3; tokenCount++) 
    {
        // Skip any leading whitespace
        while (isspace(peek(0, source, minIndex))) 
        {
            consume(source, minIndex);
        }

        char tokenBuffer[256];
        int tokenIndex = 0; // Initialize tokenIndex to 0
        while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != ',' && peek(0, source, minIndex) != '\0') 
        {
            tokenBuffer[tokenIndex] = consume(source, minIndex);
            tokenIndex++;
        }
        tokenBuffer[tokenIndex] = '\0';

        // Validate operands: the first two must be registers, and the third can be a register or an immediate value
        if (tokenCount < 2) 
        {
            // For the first two operands, check if they are valid registers
            if (!isRegister(tokenBuffer)) 
            {
                return false;
            }
        } 
        else 
        {
            // For the third operand, it can be either a register or an immediate value
            if (!(isRegister(tokenBuffer) || isImm5(tokenBuffer))) 
            {
                return false;
            }
        }

        if (tokenCount > 0) 
        {
            strcat(operands, ","); // Separate operands with commas
        }
        strcat(operands, tokenBuffer);

        // After the first two operands, expect a comma before the next operand
        if (tokenCount < 2) 
        {
            while (isspace(peek(0, source, minIndex))) consume(source, minIndex); // Consume spaces before checking for a comma
            if (peek(0, source, minIndex) != ',') 
            {
                return false;
            } 
            else 
            {
                consume(source, minIndex); // Consume the comma
            }
        }
    }

    strcpy(operandsOut, operands);
    return true;
}

/* 
    AND -> Validate
    if 
        3 parameters connected via ','
        cases:
            DR, SR1, SR2
            DR, SR1, Imm5
    else
        Invalid
*/
bool parseAND(char *source, int *minIndex, char *operandsOut) 
{
    int tokenCount;
    char operands[256] = "";

    for (tokenCount = 0; tokenCount < 3; tokenCount++) 
    {
        while (isspace(peek(0, source, minIndex))) 
        {
            consume(source, minIndex);
        }

        char tokenBuffer[256];
        int tokenIndex = 0; // Start at index 0
        while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != ',' && peek(0, source, minIndex) != '\0') 
        {
            tokenBuffer[tokenIndex] = consume(source, minIndex); // Assign before increment
            tokenIndex++;
        }
        tokenBuffer[tokenIndex] = '\0';

        // For the first two tokens, check if they are valid registers
        if (tokenCount < 2) 
        {
            if (!isRegister(tokenBuffer)) 
            {
                return false;
            }
        } 
        else 
        {
            // For the third token, it can be a register or an immediate value
            if (!(isRegister(tokenBuffer) || isImm5(tokenBuffer))) 
            {
                return false;
            }
        }

        if (tokenCount > 0) 
        {
            strcat(operands, ","); // Separate operands with commas
        }
        strcat(operands, tokenBuffer);

        // Consume a comma after the first two tokens if there are more tokens to read
        if (tokenCount < 2 && peek(0, source, minIndex) == ',') 
        {
            consume(source, minIndex);
        }
    }

    strcpy(operandsOut, operands);
    return true;
}

/* 
    BR -> Validate
    if 
        3 possible parameters (At least one, at most 3)
        params:
            n => negative
            z => zero
            p => positive
        only case
    else
        Invalid
*/
bool parseBR(const char *instruction, char labels[][MAX_LABEL_LEN], int labelCount, char *labelOut) 
{
    // Check if instruction starts with "BR"
    if (strncmp(instruction, "BR", 2) != 0) 
    {
        return false;
    }

    int i = 2; // Start index after "BR"
    // Skip condition codes
    while (instruction[i] == 'n' || instruction[i] == 'z' || instruction[i] == 'p') 
    {
        i++;
    }

    // Move past any whitespace after condition codes
    while (isspace(instruction[i])) i++;

    // Copy label to labelOut, ensuring null termination
    int j = 0;
    while (instruction[i] != '\0' && instruction[i] != ';' && !isspace(instruction[i])) 
    {
        labelOut[j++] = instruction[i++];
    }
    labelOut[j] = '\0'; // Ensure null termination

    // Validate extracted label is not empty and exists
    return strlen(labelOut) > 0 && isValidLabel(labelOut, labels, labelCount);
}

/* 
    LD -> Validate
    if 
        2 parameters seperated by ','
        case:
            DR, LABEL
    else
        Invalid
*/
bool parseLD(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount) 
{
    // Skip whitespace before DR
    while (isspace(peek(0, source, minIndex))) 
    {
        consume(source, minIndex);
    }

    char tokenBuffer[256];
    int tokenIndex = 0;
    // Parse DR
    while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != ',' && peek(0, source, minIndex) != '\0') 
    {
        tokenBuffer[tokenIndex] = consume(source, minIndex);
        tokenIndex++;
    }
    tokenBuffer[tokenIndex] = '\0'; 

    if (!isRegister(tokenBuffer)) 
    {
        return false;
    }

    // Skip whitespace (and comma if present) before the label
    while (isspace(peek(0, source, minIndex)) || peek(0, source, minIndex) == ',') 
    {
        consume(source, minIndex);
    }

    // Reset for label parsing
    tokenIndex = 0;
    // Parse LABEL
    while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != '\0') 
    {
        tokenBuffer[tokenIndex] = consume(source, minIndex);
        tokenIndex++;
    }
    tokenBuffer[tokenIndex] = '\0';

    // Check if label is valid
    return isValidLabel(tokenBuffer, labels, labelCount);
}

/* 
    LDI -> Validate
    if 
        2 parameters seperated by ','
        case:
            DR, LABEL
    else
        Invalid
*/
bool parseLDI(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount) 
{
    // Skip any whitespace after the "LDI" instruction
    while (isspace(peek(0, source, minIndex))) 
    {
        consume(source, minIndex);
    }

    // Parse and validate the destination register (DR)
    char registerBuffer[256];
    int registerIndex = 0;
    while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != ',' && peek(0, source, minIndex) != '\0') 
    {
        registerBuffer[registerIndex++] = consume(source, minIndex);
    }
    registerBuffer[registerIndex] = '\0'; // Null-terminate the register part

    // Check if the first operand (DR) is a valid register
    if (!isRegister(registerBuffer)) 
    {
        printf("Register not valid: %s\n", registerBuffer);
        return false;
    }

    // Skip the comma and any whitespace before the label
    if (peek(0, source, minIndex) == ',') 
    {
        consume(source, minIndex); // Consume the comma
    }
    while (isspace(peek(0, source, minIndex))) 
    {
        consume(source, minIndex);
    }

    // Parse the label
    char labelBuffer[256];
    int labelIndex = 0;
    while (peek(0, source, minIndex) != '\0' && peek(0, source, minIndex) != ';' && !isspace(peek(0, source, minIndex))) 
    {
        labelBuffer[labelIndex++] = consume(source, minIndex);
    }
    labelBuffer[labelIndex] = '\0'; // Null-terminate the label

    // Validate the extracted label
    if (isValidLabel(labelBuffer, labels, labelCount)) 
    {
        // Successfully parsed and validated the label
        printf("Valid label for LDI: %s\n", labelBuffer);
        return true;
    } 
    else 
    {
        // Label validation failed
        printf("Label not valid or not found for LDI: %s\n", labelBuffer);
        return false;
    }
}

/* 
    LDR -> Validate
    if 
        3 parameters seperated by ','
        case:
            DR, SR1, offset6
    else
        Invalid
*/
bool parseLDR(char *source, int *minIndex, char *operandsBuffer) 
{
    int tokenCount;
    operandsBuffer[0] = '\0'; // Ensure the buffer starts empty.

    for (tokenCount = 0; tokenCount < 3; tokenCount++) 
    {
        // Skip whitespace
        while (isspace(peek(0, source, minIndex))) 
        {
            consume(source, minIndex);
        }

        char tokenBuffer[256];
        int tokenIndex = 0; // Initialize tokenIndex to 0
        while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != ',' && peek(0, source, minIndex) != '\0') 
        {
            tokenBuffer[tokenIndex++] = consume(source, minIndex); // Use tokenIndex++ to increment after use
        }
        tokenBuffer[tokenIndex] = '\0'; // Null-terminate the token

        // Append the token to the operands buffer, separating operands with a comma
        if (tokenCount > 0) 
        {
            strncat(operandsBuffer, ",", 255 - strlen(operandsBuffer));
        }
        strncat(operandsBuffer, tokenBuffer, 255 - strlen(operandsBuffer));

        if (tokenCount < 2) 
        {
            if (!isRegister(tokenBuffer)) 
            {
                return false; // The first two tokens must be registers
            }
        } 
        else 
        {
            if (!isOffset6(tokenBuffer)) 
            {
                return false; // The last token must be an offset6
            }
        }

        // Consume the comma after the first two operands but not after the third
        if (tokenCount < 2) 
        {
            if (peek(0, source, minIndex) != ',') 
            {
                return false; // Expecting a comma here
            } 
            else 
            {
                consume(source, minIndex); // Consume the comma
            }
        }
    }

    return true; // If all tokens are parsed successfully, return true
}

/* 
    LEA -> Validate
    if 
        2 parameters seperated by ','
        case:
            DR, LABEL
    else
        Invalid
*/
bool parseLEA(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount) 
{
    while (isspace(peek(0, source, minIndex))) 
    {
        consume(source, minIndex);
    }

    // DR part
    char registerBuffer[256];
    int registerIndex = 0;
    while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != ',' && peek(0, source, minIndex) != '\0') 
    {
        registerBuffer[registerIndex++] = consume(source, minIndex);
    }
    registerBuffer[registerIndex] = '\0'; // Null-terminate the register part

    // Check if the first operand (DR) is a valid register
    if (!isRegister(registerBuffer)) 
    {
        return false;
    }

    // Skip the comma after the DR part
    if (peek(0, source, minIndex) == ',') 
    {
        consume(source, minIndex); // Consume the comma
    }

    // Skip any whitespace before the label
    while (isspace(peek(0, source, minIndex))) 
    {
        consume(source, minIndex);
    }

    // Label part
    char labelBuffer[256];
    int labelIndex = 0;
    while (peek(0, source, minIndex) != '\0' && peek(0, source, minIndex) != ';' && !isspace(peek(0, source, minIndex))) 
    {
        labelBuffer[labelIndex++] = consume(source, minIndex); // Increment and assign
    }
    labelBuffer[labelIndex] = '\0'; // Null-terminate the label part

    // Validate the extracted label
    return isValidLabel(labelBuffer, labels, labelCount);
}

/* 
    NOT -> Validate
    if 
        2 parameters seperated by ','
        case:
            DR, SR1
    else
        Invalid
*/
bool parseNOT(char *source, int *minIndex, char *operandsOut) 
{
    char drBuffer[256], srBuffer[256];
    int drIndex = 0, srIndex = 0;
    bool commaEncountered = false;

    // Initialize the operands output string.
    operandsOut[0] = '\0';

    // Skip whitespace before the first register (DR)
    while (isspace(peek(0, source, minIndex))) 
    {
        consume(source, minIndex);
    }

    // Collect characters for DR until a comma or whitespace is encountered
    while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != ',' && peek(0, source, minIndex) != '\0') 
    {
        drBuffer[drIndex++] = consume(source, minIndex);
    }
    drBuffer[drIndex] = '\0'; // Null-terminate the DR operand

    // Skip over comma and whitespace to reach the second register (SR)
    while (peek(0, source, minIndex) == ',' || isspace(peek(0, source, minIndex))) 
    {
        consume(source, minIndex);
        commaEncountered = true; // Ensure a comma has been encountered to expect SR
    }

    // Only proceed to parse SR if a comma was encountered after DR
    if (commaEncountered) 
    {
        // Collect characters for SR
        while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != '\0') 
        {
            srBuffer[srIndex++] = consume(source, minIndex);
        }
        srBuffer[srIndex] = '\0'; // Null-terminate the SR operand

        // Validate both operands are valid registers
        if (isRegister(drBuffer) && isRegister(srBuffer)) 
        {
            // Format operandsOut as "DR,SR"
            sprintf(operandsOut, "%s,%s", drBuffer, srBuffer);
            return true; // Successfully parsed both registers
        }
    }

    return false; // Failed to parse valid registers for NOT instruction
}

/* 
    ST -> Validate
    if 
        2 parameters seperated by ','
        case:
            SR1, LABEL
    else
        Invalid
*/
bool parseST(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount) 
{
    while (isspace(peek(0, source, minIndex))) 
    {
        consume(source, minIndex);
    }

    // SR part
    char registerBuffer[256];
    int registerIndex = -1;
    while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != ',' && peek(0, source, minIndex) != '\0') 
    {
        registerBuffer[++registerIndex] = consume(source, minIndex);
    }
    registerBuffer[registerIndex + 1] = '\0'; // Null-terminate the register part

    // Check if the first operand (SR) is a valid register
    if (!isRegister(registerBuffer)) 
    {
        return false;
    }

    // Skip over the comma (if present) and any whitespace after the register
    if (peek(0, source, minIndex) == ',') 
    {
        consume(source, minIndex); // Skip the comma
    }
    while (isspace(peek(0, source, minIndex))) 
    {
        consume(source, minIndex); // Skip any whitespace
    }

    // Label part
    char labelBuffer[256];
    int labelIndex = -1;
    while (peek(0, source, minIndex) != '\0' && peek(0, source, minIndex) != ';') 
    {
        if (isspace(peek(0, source, minIndex))) 
        {
            // Stop at the first whitespace after the label
            break;
        }
        labelBuffer[++labelIndex] = consume(source, minIndex); // Pre-increment index
    }
    labelBuffer[labelIndex + 1] = '\0'; // Null-terminate the label part

    // Validate the extracted label
    return isValidLabel(labelBuffer, labels, labelCount);
}

/* 
    STI -> Validate
    if 
        2 parameters seperated by ','
        case:
            SR1, LABEL
    else
        Invalid
*/
bool parseSTI(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount)
{
    while (isspace(peek(0, source, minIndex)))
    {
        consume(source, minIndex);
    }

    char tokenBuffer[256];
    int tokenIndex = 0;
    while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != ',' && peek(0, source, minIndex) != '\0')
    {
        tokenIndex++;
        tokenBuffer[tokenIndex] = consume(source, minIndex);
    }
    tokenBuffer[tokenIndex] = '\0';

    if (!isRegister(tokenBuffer))
    {
        return false;
    }

    while (isspace(peek(0, source, minIndex)))
    {
        consume(source, minIndex);
    }

    tokenIndex = 0;
    while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != '\0')
    {
        tokenIndex++;
        tokenBuffer[tokenIndex] = consume(source, minIndex);
    }
    tokenBuffer[tokenIndex] = '\0';

    if (isValidLabel(tokenBuffer, labels, labelCount))
    {
        return true;
    }
    else 
    {
        return false;
    }
}

/* 
    STR -> Validate
    if 
        3 parameters seperated by ','
        case:
            SR1, SR2, offset6
    else
        Invalid
*/
bool parseSTR(char *source, int *minIndex, char *operandsOut) 
{
    int tokenCount;
    operandsOut[0] = '\0';  // Initialize the operands output string.

    for (tokenCount = 0; tokenCount < 3; tokenCount++) 
    {
        // Skip whitespace
        while (isspace(peek(0, source, minIndex))) 
        {
            consume(source, minIndex);
        }

        char tokenBuffer[256];
        int tokenIndex = 0; // Initialize tokenIndex to 0
        while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != ',' && peek(0, source, minIndex) != '\0') 
        {
            tokenBuffer[tokenIndex++] = consume(source, minIndex); // Corrected to tokenIndex++
        }
        tokenBuffer[tokenIndex] = '\0'; // Null-terminate the token

        // Append the token to the operands output
        strcat(operandsOut, tokenBuffer);
        if (tokenCount < 2) 
        {
            strcat(operandsOut, ",");  // Add a comma after the first two operands
        }

        if (tokenCount < 2) 
        {
            // Consume the comma after the first two operands
            if (peek(0, source, minIndex) != ',') 
            {
                return false;
            } 
            else 
            {
                consume(source, minIndex);
            }
        }
    }

    return true; // Return true if all operands are valid and processed
}

/* 
    TRAP -> Validate
    if 
        1 parameter seperated by ','
        case:
            trapvector8
    else
        Invalid
*/
bool parseTRAP(char *source, int *minIndex, int *trapVector) 
{
    while (isspace(source[*minIndex])) (*minIndex)++;
    if (source[*minIndex] != 'x' && source[*minIndex] != 'X') return false;

    (*minIndex)++; // Skip 'x' or 'X'
    char trapVectorStr[5]; // Enough to hold 4 hex digits and a null terminator
    int i = 0;
    while (isxdigit(source[*minIndex]) && i < 4) 
    {
        trapVectorStr[i++] = source[(*minIndex)++];
    }
    trapVectorStr[i] = '\0'; // Null-terminate the string

    if (!isValidTrapVector(trapVectorStr)) return false;

    *trapVector = strtol(trapVectorStr, NULL, 16); // Convert hexadecimal string to integer

    return true; // Successfully parsed trap vector
}

bool parseSEMI(char *source, int *minIndex)
{
    if (peek(0, source, minIndex) != ';')
    {
        return true;
    }

    while (peek(0, source, minIndex) != '\n' && peek(0, source, minIndex) != '\0')
    {
        consume(source, minIndex);
    }

    return true;
}

bool parseFILL(char *source, int *minIndex, int *immValue) 
{
    // Skip whitespace
    while (isspace(source[*minIndex])) (*minIndex)++;

    // Extract the operand
    char operandBuffer[256];
    int i = 0;
    while (source[*minIndex] != '\0' && source[*minIndex] != '\n' && !isspace(source[*minIndex])) 
    {
        operandBuffer[i++] = source[*minIndex];
        (*minIndex)++;
    }
    operandBuffer[i] = '\0'; // Null-terminate the string

    // Validate the operand
    if (!isImm5(operandBuffer)) 
    {
        return false;
    }

    // Convert the immediate value to an integer
    if (operandBuffer[0] == '#') 
    {
        *immValue = atoi(operandBuffer + 1); // Skip the '#' and convert
    } 
    else 
    {
        *immValue = atoi(operandBuffer); // Directly convert if no '#' prefix
    }

    return true;
}

bool parseEND(char *source, int *minIndex) 
{
    // Move past any whitespace before checking for .END
    while (isspace(source[*minIndex])) 
    {
        (*minIndex)++;
    }

    // Check if the rest of the line is just comments or whitespace
    while (source[*minIndex] != '\0' && source[*minIndex] != '\n') 
    {
        if (source[*minIndex] == ';') 
        {
            // .END is valid, there's a comment following it
            return true;
        } 
        else if (!isspace(source[*minIndex])) 
        {
            // Found a non-space character before a comment or end of line, .END is not valid
            return false;
        }
        (*minIndex)++;
    }

    // Reached the end of the line without finding non-whitespace characters before any comment
    return true;
}

bool parseBLKW(char *source, int *minIndex, int *blockSize) 
{
    while (isspace(source[*minIndex])) 
    {
        (*minIndex)++;
    }

    char blockSizeStr[10];
    int i = 0;
    while (isdigit(source[*minIndex]) && i < (sizeof(blockSizeStr) - 1)) 
    {
        blockSizeStr[i++] = source[(*minIndex)++];
    }
    blockSizeStr[i] = '\0'; // Null-terminate the string

    if (i == 0) 
    {
        // No digits were found, indicating an invalid block size
        return false;
    }

    *blockSize = atoi(blockSizeStr); // Convert the block size string to an integer

    if (*blockSize < 0) 
    {
        // Block size is invalid if it's negative
        return false;
    }

    return true; // Successfully parsed block size
}

#endif