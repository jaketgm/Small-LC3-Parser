#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LABELS 100
#define MAX_LABEL_LEN 20

enum Tokens {
    ADD, 
    AND, 
    BR,
    LD,
    LDI,
    LDR,
    LEA, 
    NOT,
    ST,
    STI,
    STR,
    TRAP,
    ORIG,
    END,
    HASH,
    LABEL,
    INVALID_TOKEN
};

enum registerTokens {
    R0,
    R1,
    R2, 
    R3, 
    R4,
    R5,
    R6,
    R7,
    INVALID_REGISTER
};

char peek(int offset, char *source, int *minIndex);
char consume(char *source, int *minIndex);
enum Tokens validateToken(const char *token);
enum registerTokens validateRegisterToken(const char *regstr);
bool isRegister(char *token);
bool isImm5(char *imm5);
bool isValidBranchCondition(char condition);
bool isValidLabel(char *label, char labels[][MAX_LABEL_LEN], int count);
bool isOffset6(char *offset);
bool isValidTrapVector(char *offset);

// Parsers for tokens
bool parseADD(char *source, int *minIndex);
bool parseAND(char *source, int *minIndex);
bool parseBR(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseLD(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseLDI(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseLDR(char *source, int *minIndex);
bool parseLEA(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseNOT(char *source, int *minIndex);
bool parseST(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseSTI(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseSTR(char *source, int *minIndex);
bool parseTRAP(char *source, int *minIndex);

int main() 
{
    FILE *file;
    if ((file = fopen("file.asm", "r")) == NULL) 
    {
        printf("Error opening file!\n");
        exit(EXIT_FAILURE);
    }

    char line[256];
    bool firstToken = true;
    bool validStart = false;

    while (fgets(line, sizeof(line), file))
    {
        int minIndex = 0;
        char tokenBuffer[256];
        int tokenIndex = 0;

        char labels[MAX_LABELS][MAX_LABEL_LEN];
        int labelCount = 0;

        enum Tokens tokenType = INVALID_TOKEN;

        while (minIndex < strlen(line))
        {
            char ch = peek(0, line, &minIndex);
            if (ch == '\0' || ch == '\n' || ch == '\t' || ch == ' ')
            {
                if (tokenIndex > 0)
                {
                    tokenBuffer[tokenIndex] = '\0';
                    if (firstToken && tokenBuffer[0] == '.')
                    {
                        tokenType = validateToken(tokenBuffer + 1);
                        if (tokenType == ORIG)
                        {
                            printf("Valid start to the program: %s\n", tokenBuffer);
                            validStart = true;
                        }
                        else 
                        {
                            printf("Invalid start to the program: %s\n", tokenBuffer);
                            validStart = false;
                        }
                        firstToken = false;
                    }
                    else 
                    {
                        tokenType = validateToken(tokenBuffer);
                        if (tokenType != INVALID_TOKEN)
                        {
                            printf("Valid token: %s\n", tokenBuffer);

                            if (tokenType == ADD)
                            {
                                if (!parseADD(line, &minIndex))
                                {
                                    printf("Invalid operands for ADD instruction.\n");
                                }
                                else 
                                {
                                    printf("Valid operands for ADD instruction.\n");
                                }
                            }
                            else if (tokenType == AND)
                            {
                                if (!parseAND(line, &minIndex))
                                {
                                    printf("Invalid operands for AND instruction.\n");
                                }
                                else 
                                {
                                    printf("Valid operands for AND instruction.\n");
                                }
                            }
                            else if (tokenType == LABEL)
                            {
                                if (labelCount < MAX_LABELS)
                                {
                                    strncpy(labels[labelCount], tokenBuffer, MAX_LABEL_LEN);
                                    labels[labelCount][MAX_LABEL_LEN - 1] = '\0';
                                    labelCount++;
                                }
                                else 
                                {
                                    // TODO conditional here
                                }
                            }
                            else if (tokenType == BR)
                            {
                                if (!parseBR(line, &minIndex, labels, labelCount))
                                {
                                    printf("Invalid operands for BR instruction.\n");
                                }
                                else 
                                {
                                    printf("Valid operands for BR instruction.\n");
                                }
                            }
                            else if (tokenType == LD)
                            {
                                if (!parseLD(line, &minIndex, labels, labelCount))
                                {
                                    printf("Invalid operands for LD instruction.\n");
                                }
                                else 
                                {
                                    printf("Valid operands for LD instruction.\n");
                                }
                            }
                            else if (tokenType == LDI)
                            {
                                if (!parseLDI(line, &minIndex, labels, labelCount))
                                {
                                    printf("Invalid operands for LDI instruction.\n");
                                }
                                else 
                                {
                                    printf("Valid operands for LDI instruction.\n");
                                }
                            }
                            else if (tokenType == LDR)
                            {
                                if (!parseLDR(line, &minIndex))
                                {
                                    printf("Invalid operands for LDR instruction.\n");
                                }
                                else 
                                {
                                    printf("Valid operands for LDR instruction.\n");
                                }
                            }
                            else if (tokenType == LEA)
                            {
                                if (!parseLEA(line, &minIndex, labels, labelCount))
                                {
                                    printf("Invalid operands for LEA instruction.\n");
                                }
                                else 
                                {
                                    printf("Valid operands for LEA instruction.\n");
                                }
                            }
                            else if (tokenType == NOT)
                            {
                                if (!parseNOT(line, &minIndex))
                                {
                                    printf("Invalid operands for NOT instruction.\n");
                                }
                                else 
                                {
                                    printf("Valid operands for NOT instruction.\n");
                                }
                            }
                            else if (tokenType == ST)
                            {
                                if (!parseST(line, &minIndex, labels, labelCount))
                                {
                                    printf("Invalid operands for ST instruction.\n");
                                }
                                else 
                                {
                                    printf("Valid operands for ST instruction.\n");
                                }
                            }
                            else if (tokenType == STI)
                            {
                                if (!parseST(line, &minIndex, labels, labelCount))
                                {
                                    printf("Invalid operands for STI instruction.\n");
                                }
                                else 
                                {
                                    printf("Valid operands for STI instruction.\n");
                                }
                            }
                            else if (tokenType == STR)
                            {
                                if (!parseSTR(line, &minIndex))
                                {
                                    printf("Invalid operands for STR instruction.\n");
                                }
                                else 
                                {
                                    printf("Valid operands for STR instruction.\n");
                                }
                            }
                        }
                        else 
                        {
                            printf("Invalid token: %s\n", tokenBuffer);
                        }
                    }
                    tokenIndex = 0;
                }
                consume(line, &minIndex);
            } 
            else 
            {
                tokenBuffer[tokenIndex] = consume(line, &minIndex);
                tokenIndex++;
            }
        }

        if (!validStart)
        {
            printf("Not a valid start to the program.\n");
        }
    }
    
    fclose(file);
    return 0;
}

char peek(int offset, char *source, int *minIndex) 
{
    int length = strlen(source);
    if (*minIndex + offset >= length) 
    {
        return '\0';
    }
    return source[*minIndex + offset];
}

char consume(char *source, int *minIndex) 
{
    char ch = source[(*minIndex)++];
    if (ch == '\n') 
    {
        printf("Consumed newline at index: %d, incrementing line count\n", *minIndex - 1);
    } 
    else 
    {
        printf("Consumed char: %c, at index: %d\n", ch, *minIndex - 1);
    }
    return ch;
}

enum Tokens validateToken(const char *token) 
{
    if (strcmp(token, "ADD") == 0)
    {
        return ADD;
    }
    else if (strcmp(token, "AND") == 0)
    {
        return AND;
    }
    else if (strcmp(token, "BR") == 0)
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
    else 
    {
        return INVALID_TOKEN;
    }
}

enum registerTokens validateRegisterToken(const char *regstr)
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

bool isImm5(char *imm5)
{
    int val = atoi(imm5);
    if (val >= -16 && val <= 15)
    {
        return true;
    }
    return false;
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
bool parseADD(char *source, int *minIndex)
{
    int tokenCount;
    for (tokenCount = 0; tokenCount < 3; tokenCount++)
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

        if (tokenCount < 2)
        {
            if (!isRegister(tokenBuffer))
            {
                return false;
            }
        }
        else 
        {
            if (!isRegister(tokenBuffer) && !isImm5(tokenBuffer))
            {
                return false;
            }
        }

        if (tokenCount < 2 && consume(source, minIndex) != ',')
        {
            return false;
        }
    }

    return true;
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
bool parseAND(char *source, int *minIndex)
{
    int tokenCount;
    for (tokenCount = 0; tokenCount < 3; tokenCount++)
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

        if (tokenCount < 2)
        {
            if (!isRegister(tokenBuffer))
            {
                return false;
            }
        }
        else 
        {
            if (!isRegister(tokenBuffer) && !isImm5(tokenBuffer))
            {
                return false;
            }
        }

        if (tokenCount < 2 && consume(source, minIndex) != ',')
        {
            return false;
        }
    }

    return true;
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
    int i;
    for (i = 0; i < count; i++)
    {
        if (strcmp(label, labels[i]) == 0)
        {
            return true;
        }
    }
    return false;
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
bool parseBR(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount)
{
    bool n = false, z = false, p = false;
    char tokenBuffer[256];
    int tokenIndex = 0;

    while (isValidBranchCondition(peek(0, source, minIndex)))
    {
        char condition = consume(source, minIndex);
        switch (condition)
        {
            case 'n':
                if (n)
                {
                    return false;
                }
                else 
                {
                    n = true;
                }
                break;
            case 'z': 
                if (z)
                {
                    return false;
                }
                else 
                {
                    z = true;
                }
                break;
            case 'p': 
                if (p)
                {
                    return false;
                }
                else 
                {
                    p = true;
                }
                break;
            default:
                break;
        }
    }

    while (isspace(peek(0, source, minIndex)))
    {
        consume(source, minIndex);
    }

    while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != '\0')
    {
        tokenIndex++;
        tokenBuffer[tokenIndex] = consume(source, minIndex);
    }
    tokenBuffer[tokenIndex] = '\0';

    // Analyze label
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

bool isOffset6(char *offset)
{
    int val = atoi(offset);
    if (val >= -32 && val <= 31)
    {
        return true;
    }
    return false;
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
bool parseLDR(char *source, int *minIndex)
{
    int tokenCount;
    for (tokenCount = 0; tokenCount < 3; tokenCount++)
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

        if (tokenCount < 2)
        {
            if (!isRegister(tokenBuffer))
            {
                return false;
            }
        }
        else 
        {
            if (!isOffset6(tokenBuffer))
            {
                return false;
            }
        }

        if (tokenCount < 2 && consume(source, minIndex) != ',')
        {
            return false;
        }
    }

    return true;
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
    NOT -> Validate
    if 
        2 parameters seperated by ','
        case:
            DR, SR1
    else
        Invalid
*/
bool parseNOT(char *source, int *minIndex)
{
    while (isspace(peek(0, source, minIndex))) 
    {
        consume(source, minIndex);
    }

    char tokenBuffer[256];
    int tokenIndex = 0;
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

    while (isspace(peek(0, source, minIndex)) || peek(0, source, minIndex) == ',') 
    {
        consume(source, minIndex);
    }

    tokenIndex = 0;
    while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != '\0') 
    {
        tokenBuffer[tokenIndex] = consume(source, minIndex);
        tokenIndex++;
    }
    tokenBuffer[tokenIndex] = '\0';

    if (!isRegister(tokenBuffer)) 
    {
        return false;
    }
    
    return true;
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
bool parseSTR(char *source, int *minIndex)
{
    int tokenCount;
    for (tokenCount = 0; tokenCount < 3; tokenCount++)
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

        if (tokenCount < 2)
        {
            if (!isRegister(tokenBuffer))
            {
                return false;
            }
        }
        else 
        {
            if (!isOffset6(tokenBuffer))
            {
                return false;
            }
        }

        if (tokenCount < 2 && consume(source, minIndex) != ',')
        {
            return false;
        }
    }

    return true;
}

bool isValidTrapVector(char *offset)
{
    int val = atoi(offset);
    if (val >= 0 && val <= 255)
    {
        return true;
    }
    return false;
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
bool parseTRAP(char *source, int *minIndex)
{
    while (isspace(peek(0, source, minIndex)))
    {
        consume(source, minIndex);
    }

    char tokenBuffer[256];
    int tokenIndex = 0;
    while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != '\0')
    {
        tokenIndex++;
        tokenBuffer[tokenIndex] = consume(source, minIndex);
    }
    tokenBuffer[tokenIndex] = '\0';

    if (isValidTrapVector(tokenBuffer))
    {
        return true;
    }
    else 
    {
        return false;
    }
}

// NEED to check in imm5 that # is placed before