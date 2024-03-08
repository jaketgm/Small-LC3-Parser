#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LABELS 100
#define MAX_LABEL_LEN 20
#define IMMEDIATE_SIZE_ADD_AND 5
#define IMMEDIATE_SIZE_LDR_STR 6

typedef enum {
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
    SEMI,
    BLKW,
    FILL,
    INVALID_TOKEN
} Tokens;

typedef enum {
    ADD_ONE_OP,
    ADD_TWO_OP,
    AND_ONE_OP,
    AND_TWO_OP,
    BR_OP,
    LD_OP,
    LDI_OP,
    LDR_OP,
    LEA_OP,
    NOT_OP,
    ST_OP,
    STI_OP,
    STR_OP,
    TRAP_OP,
    INVALID_OP
} BinOps;

typedef struct {
    BinOps binaryOps;
    const char *opcode;
} InstructionMap;

InstructionMap instructionMap[] = {
    {ADD_ONE_OP, "0001"},
    {ADD_TWO_OP, "0001"},
    {AND_ONE_OP, "0101"},
    {AND_TWO_OP, "0101"},
    {BR_OP, "0000"},
    {LD_OP, "0010"},
    {LDI_OP, "1010"},
    {LDR_OP, "0110"},
    {LEA_OP, "1110"},
    {NOT_OP, "1001"},
    {ST_OP, "0011"},
    {STI_OP, "1011"},
    {STR_OP, "0111"},
    {TRAP_OP, "1111"},
    {INVALID_OP, "NULL"},
};

typedef enum {
    R0,
    R1,
    R2, 
    R3, 
    R4,
    R5,
    R6,
    R7,
    INVALID_REGISTER
} RegisterTokens;

typedef struct {
    RegisterTokens regTok;
    const char *binVal;
} RegisterMap;

RegisterMap registerMap[] = {
    {R0, "000"},
    {R1, "001"},
    {R2, "010"},
    {R3, "011"},
    {R4, "100"},
    {R5, "101"},
    {R6, "110"},
    {R7, "111"},
    {INVALID_REGISTER, "NULL"},
};

char peek(int offset, char *source, int *minIndex);
char consume(char *source, int *minIndex);
Tokens validateToken(const char *token);
RegisterTokens validateRegisterToken(const char *regstr);
bool isRegister(char *token);
bool isImm5(char *imm5);
bool isValidBranchCondition(char condition);
bool isValidLabel(char *label, char labels[][MAX_LABEL_LEN], int count);
bool isLabelDefinition(char *token);
bool addLabel(char labels[][MAX_LABEL_LEN], int* labelCount, const char* tokenBuffer);
bool isOffset6(char *offset);
bool isValidTrapVector(char *offset);

// Parsers for tokens
bool parseADD(char *source, int *minIndex, char *operandsOut); 
bool parseAND(char *source, int *minIndex, char *operandsOut);
bool parseBR(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount, char *instruction);
bool parseLD(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseLDI(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseLDR(char *source, int *minIndex, char *operandsBuffer);
bool parseLEA(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseNOT(char *source, int *minIndex, char *operandsOut);
bool parseST(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseSTI(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseSTR(char *source, int *minIndex, char *operandsOut);
bool parseTRAP(char *source, int *minIndex);
bool parseSEMI(char *source, int *minIndex);
bool parseFILL(char *source, int *minIndex);

const char *getOpcodeForToken(BinOps binaryOps);
BinOps tokenToBinaryOp(Tokens token, const char *operands);
void processOperands(const char *operandsBuffer, char *binaryOut, Tokens tokenType);
void immToBinary(const char *immStr, char *binaryOut, int immediateSize);
void writeLineToBin(const char *opcode, const char *binaryOut);

int main() 
{
    FILE *file;
    if ((file = fopen("file.asm", "r")) == NULL) 
    {
        printf("Error opening file!\n");
        exit(EXIT_FAILURE);
    }

    FILE *binFile = fopen("output.bin", "wb");
    if (binFile == NULL)
    {
        fprintf(stderr, "Error opening file.\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    char line[256];
    bool validStart = false;

    char labels[MAX_LABELS][MAX_LABEL_LEN];
    int labelCount = 0;

    while (fgets(line, sizeof(line), file))
    {
        int minIndex = 0;
        char tokenBuffer[256];
        int tokenIndex = 0;
        bool firstToken = true;
        Tokens tokenType = INVALID_TOKEN;

        while (minIndex < strlen(line))
        {
            char ch = peek(0, line, &minIndex);

            if (isspace(ch))
            {
                consume(line, &minIndex);
                continue;
            }

            if (ch == ';' || ch == '\0' || ch == '\n') 
            {
                break;
            }

            tokenBuffer[tokenIndex++] = consume(line, &minIndex);

            if (isspace(peek(0, line, &minIndex)) || peek(0, line, &minIndex) == '\0')
            {
                tokenBuffer[tokenIndex] = '\0';

                tokenIndex = 0;

                tokenType = validateToken(tokenBuffer);

                if (firstToken && tokenBuffer[0] == '.')
                {
                    tokenType = validateToken(tokenBuffer + 1);
                    if (tokenType == ORIG)
                    {
                        printf("Valid start to the program: %s\n", tokenBuffer);
                        validStart = true;
                    }
                    else if (tokenType == BLKW)
                    {
                        printf("Valid Block Word: %s\n", tokenBuffer);
                    }
                    else if (tokenType == END)
                    {
                        printf("Valid End to Program: %s\n", tokenBuffer);
                    }
                    else if (tokenType == FILL)
                    {
                        printf("Valid Fill statement: %s\n", tokenBuffer);
                    }
                    else 
                    {
                        printf("Invalid start to the program: %s\n", tokenBuffer);
                        validStart = false;
                    }
                    firstToken = false;
                }
                else if (firstToken && tokenType == INVALID_TOKEN)
                {
                    addLabel(labels, &labelCount, tokenBuffer);
                    firstToken = false;
                }
                else if (tokenType != INVALID_TOKEN)
                {
                    printf("Valid token: %s\n", tokenBuffer);

                    if (tokenType == ADD)
                    {
                        char operandsBuffer[256];
                        char binaryOut[256];
                        if (!parseADD(line, &minIndex, operandsBuffer))
                        {
                            printf("Invalid operands for ADD instruction.\n");
                        }
                        else 
                        {
                            printf("Valid operands for ADD instruction.\n");
                            BinOps binaryAdd = tokenToBinaryOp(ADD, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryAdd);
                            printf("Opcode for ADD: %s\n", opcode);
                            printf("Operands: %s\n", operandsBuffer);
                            processOperands(operandsBuffer, binaryOut, ADD);
                            printf("Binary operands for ADD: %s\n", binaryOut);

                            writeLineToBin(opcode, binaryOut);
                        }
                    }
                    else if (tokenType == AND)
                    {
                        char operandsBuffer[256];
                        char binaryOut[256];
                        if (!parseAND(line, &minIndex, operandsBuffer))
                        {
                            printf("Invalid operands for AND instruction.\n");
                        }
                        else 
                        {
                            printf("Valid operands for AND instruction.\n");
                            BinOps binaryAnd = tokenToBinaryOp(AND, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryAnd);
                            printf("Opcode for AND: %s\n", opcode);
                            printf("Operands: %s\n", operandsBuffer);
                            processOperands(operandsBuffer, binaryOut, AND);
                            printf("Binary operands for AND: %s\n", binaryOut);

                            writeLineToBin(opcode, binaryOut);
                        }
                    }
                    else if (strncmp(tokenBuffer, "BR", 2) == 0) 
                    {
                        char operandsBuffer[256];
                        if (!parseBR(line, &minIndex, labels, labelCount, tokenBuffer)) 
                        {
                            printf("Invalid operands for BR instruction.\n");
                        } 
                        else 
                        {
                            printf("Valid operands for BR instruction.\n");
                            BinOps binaryBr = tokenToBinaryOp(BR, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryBr);
                            printf("Opcode for BR: %s\n", opcode);
                        }
                    }
                    else if (tokenType == LD)
                    {
                        char operandsBuffer[256];
                        if (!parseLD(line, &minIndex, labels, labelCount))
                        {
                            printf("Invalid operands for LD instruction.\n");
                        }
                        else 
                        {
                            printf("Valid operands for LD instruction.\n");
                            BinOps binaryLd = tokenToBinaryOp(LD, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryLd);
                            printf("Opcode for LD: %s\n", opcode);
                        }
                    }
                    else if (tokenType == LDI)
                    {
                        char operandsBuffer[256];
                        if (!parseLDI(line, &minIndex, labels, labelCount))
                        {
                            printf("Invalid operands for LDI instruction.\n");
                        }
                        else 
                        {
                            printf("Valid operands for LDI instruction.\n");
                            BinOps binaryLdi = tokenToBinaryOp(LDI, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryLdi);
                            printf("Opcode for LDI: %s\n", opcode);
                        }
                    }
                    else if (tokenType == LDR)
                    {
                        char operandsBuffer[256];
                        char binaryOut[256];
                        if (!parseLDR(line, &minIndex, operandsBuffer))
                        {
                            printf("Invalid operands for LDR instruction.\n");
                        }
                        else 
                        {
                            printf("Valid operands for LDR instruction.\n");
                            BinOps binaryLdr = tokenToBinaryOp(LDR, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryLdr);
                            printf("Opcode for LDR: %s\n", opcode);
                            printf("Operands: %s\n", operandsBuffer);
                            processOperands(operandsBuffer, binaryOut, LDR);
                            printf("Binary operands for LDR: %s\n", binaryOut);

                            writeLineToBin(opcode, binaryOut);
                        }
                    }
                    else if (tokenType == LEA)
                    {
                        char operandsBuffer[256];
                        if (!parseLEA(line, &minIndex, labels, labelCount))
                        {
                            printf("Invalid operands for LEA instruction.\n");
                        }
                        else 
                        {
                            printf("Valid operands for LEA instruction.\n");
                            BinOps binaryLea = tokenToBinaryOp(LEA, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryLea);
                            printf("Opcode for LEA: %s\n", opcode);
                        }
                    }
                    else if (tokenType == NOT)
                    {
                        char operandsBuffer[256];
                        char binaryOut[256];
                        if (!parseNOT(line, &minIndex, operandsBuffer))
                        {
                            printf("Invalid operands for NOT instruction.\n");
                        }
                        else 
                        {
                            printf("Valid operands for NOT instruction.\n");
                            BinOps binaryNot = tokenToBinaryOp(NOT, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryNot);
                            printf("Opcode for NOT: %s\n", opcode);
                            printf("Operands: %s\n", operandsBuffer);
                            processOperands(operandsBuffer, binaryOut, NOT);
                            printf("Binary operands for NOT: %s\n", binaryOut);
                        }
                    }
                    else if (tokenType == ST)
                    {
                        char operandsBuffer[256];
                        if (!parseST(line, &minIndex, labels, labelCount))
                        {
                            printf("Invalid operands for ST instruction.\n");
                        }
                        else 
                        {
                            printf("Valid operands for ST instruction.\n");
                            BinOps binarySt = tokenToBinaryOp(ST, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binarySt);
                            printf("Opcode for ST: %s\n", opcode);
                        }
                    }
                    else if (tokenType == STI)
                    {
                        char operandsBuffer[256];
                        if (!parseST(line, &minIndex, labels, labelCount))
                        {
                            printf("Invalid operands for STI instruction.\n");
                        }
                        else 
                        {
                            printf("Valid operands for STI instruction.\n");
                            BinOps binarySti = tokenToBinaryOp(STI, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binarySti);
                            printf("Opcode for STI: %s\n", opcode);
                        }
                    }
                    else if (tokenType == STR)
                    {
                        char operandsBuffer[256];
                        char binaryOut[256];
                        if (!parseSTR(line, &minIndex, operandsBuffer))
                        {
                            printf("Invalid operands for STR instruction.\n");
                        }
                        else 
                        {
                            printf("Valid operands for STR instruction.\n");
                            BinOps binaryStr = tokenToBinaryOp(STR, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryStr);
                            printf("Opcode for STR: %s\n", opcode);
                            printf("Operands: %s\n", operandsBuffer);
                            processOperands(operandsBuffer, binaryOut, STR);
                            printf("Binary operands for STR: %s\n", binaryOut);

                            writeLineToBin(opcode, binaryOut);
                        }
                    }
                    else if (tokenType == FILL)
                    {
                        printf("Valid Fill statement: %s\n", tokenBuffer);
                        if (!parseFILL(line, &minIndex)) // Ensure minIndex points to the start of the operand.
                        {
                            printf("Invalid operands for FILL instruction.\n");
                        }
                        else 
                        {
                            printf("Valid operands for FILL instruction.\n");
                        }
                    }
                }
                else 
                {
                    if (!firstToken)
                    {
                        printf("Invalid token or unrecognized label: %s\n", tokenBuffer);
                    }
                }
            }
            
            if (peek(0, line, &minIndex) == ';')
            {
                break;
            }
        }

        if (!validStart && firstToken) 
        {
            printf("Not a valid start to the program or empty line.\n");
        }
    }
    
    fclose(file);
    fclose(binFile);
    printf("Successfully converted the LC-3 ASM file to binary!");

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

BinOps tokenToBinaryOp(Tokens token, const char *operands) 
{
    switch (token) 
    {
        case ADD:
            if (strstr(operands, "#") != NULL)
            {
                return ADD_TWO_OP;
            }
            else 
            {
                return ADD_ONE_OP;
            }
            break;
        case AND:
            if (strstr(operands, "#") != NULL)
            {
                return AND_TWO_OP;
            }
            else 
            {
                return AND_ONE_OP;
            }
            break;
        case BR: 
            return BR_OP;
        case LD: 
            return LD_OP;
        case LDI: 
            return LDI_OP;
        case LDR: 
            return LDR_OP;
        case LEA: 
            return LEA_OP;
        case NOT: 
            return NOT_OP;
        case ST: 
            return ST_OP;
        case STI: 
            return STI_OP;
        case STR: 
            return STR_OP;
        case TRAP: 
            return TRAP_OP;
        default: 
            return INVALID_OP;
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
    ADD -> Validate
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

bool isLabelDefinition(char *token)
{
    if (validateToken(token) == INVALID_TOKEN)
    {
        return true;
    }
    return false;
}

bool addLabel(char labels[][MAX_LABEL_LEN], int* labelCount, const char* tokenBuffer)
{
    int i;
    for (i = 0; i < *labelCount; i++)
    {
        if (strcmp(labels[i], tokenBuffer) == 0)
        {
            return false;
        }
    }

    if (*labelCount < MAX_LABELS)
    {
        strncpy(labels[*labelCount], tokenBuffer, MAX_LABEL_LEN);
        labels[*labelCount][MAX_LABEL_LEN - 1] = '\0';
        (*labelCount)++;
        return true;
    }
    else 
    {
        return false;
    }
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
bool parseBR(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount, char *instruction) 
{
    // Initial condition flags
    bool n = false, z = false, p = false;
    int i = 2;

    // Loop through the instruction string starting from conditions
    while (instruction[i] && !isspace(instruction[i]) && instruction[i] != ';') 
    {
        if (instruction[i] == 'n') n = true;
        else if (instruction[i] == 'z') z = true;
        else if (instruction[i] == 'p') p = true;
        i++;
    }

    // Skip any whitespace after condition codes to find the start of the label
    while (isspace(instruction[i])) i++;

    // Assuming label starts right after condition codes and whitespace
    char *label = &instruction[i];

    // Validate the label
    return isValidLabel(label, labels, labelCount);
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
bool parseNOT(char *source, int *minIndex, char *operandsOut) 
{
    operandsOut[0] = '\0'; // Initialize the operands output string.
    char tokenBuffer[256];

    // The NOT instruction expects exactly 2 register operands.
    for (int operandsFound = 0; operandsFound < 2; operandsFound++) 
    {
        // Skip whitespace before each operand
        while (isspace(peek(0, source, minIndex))) 
        {
            consume(source, minIndex);
        }

        int tokenIndex = 0;
        // Collect characters for one operand (register)
        while (!isspace(peek(0, source, minIndex)) && peek(0, source, minIndex) != '\0') 
        {
            tokenBuffer[tokenIndex++] = consume(source, minIndex);
        }
        tokenBuffer[tokenIndex] = '\0'; // Null-terminate the current operand

        // Validate the current operand is a valid register
        if (!isRegister(tokenBuffer)) 
        {
            return false; // Return false if the operand is not a valid register
        }

        // Append the operand and a separating comma only for the first operand
        if (operandsFound > 0) 
        {
            strcat(operandsOut, ","); // Add a comma only after the first operand
        }
        strcat(operandsOut, tokenBuffer); // Append the operand to the output string
    }

    return true; // Both operands processed successfully
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

bool parseFILL(char *source, int *minIndex) 
{
    // Skip whitespace
    while (isspace(source[*minIndex])) (*minIndex)++;

    // Extract the operand
    char imm5[256]; // This buffer may need to be adjusted based on your actual constraints
    int i = 0;
    while (source[*minIndex] != '\0' && source[*minIndex] != '\n' && !isspace(source[*minIndex])) 
    {
        imm5[i++] = source[*minIndex];
        (*minIndex)++;
    }
    imm5[i] = '\0'; // Null-terminate the string

    // Validate the operand
    return isImm5(imm5);
}

const char *getOpcodeForToken(BinOps binaryOps)
{
    int i;
    for (i = 0; instructionMap[i].binaryOps != INVALID_OP; i++)
    {
        if (instructionMap[i].binaryOps == binaryOps)
        {
            // printf("Instruction op: %s", instructionMap[i].opcode);
            return instructionMap[i].opcode;
        }
    }
    return NULL;
}


const char *getBinValForRegister(RegisterTokens regTok)
{
    int i;
    for (i = 0; registerMap[i].regTok != INVALID_REGISTER; i++)
    {
        if (registerMap[i].regTok == regTok)
        {
            return registerMap[i].binVal;
        }
    }
    return NULL;
}

void processOperands(const char *operandsBuffer, char *binaryOut, Tokens tokenType) 
{
    char operand[256];
    int opIndex = 0;
    int binIndex = 0;

    // Initialize the binaryOut buffer to empty string
    binaryOut[0] = '\0';

    int immediateSize = (tokenType == ADD || tokenType == AND) ? IMMEDIATE_SIZE_ADD_AND : IMMEDIATE_SIZE_LDR_STR;

    int i;
    for (i = 0; operandsBuffer[i] != '\0'; i++) 
    {
        if (operandsBuffer[i] == ',' || isspace(operandsBuffer[i])) 
        {
            if (opIndex > 0) 
            {
                operand[opIndex] = '\0'; // Null-terminate the current operand
                if (operand[0] == 'R') 
                {
                    RegisterTokens regToken = validateRegisterToken(operand);
                    const char *binary = getBinValForRegister(regToken);
                    if (binary) 
                    {
                        strcpy(&binaryOut[binIndex], binary);
                        binIndex += strlen(binary);
                    }
                } 
                else if (operand[0] == '#') 
                {
                    immToBinary(operand, &binaryOut[binIndex], immediateSize);
                    binIndex += immediateSize;
                }
                opIndex = 0; // Reset the index for the next operand
            }
            continue; // Skip the comma or whitespace
        }
        operand[opIndex++] = operandsBuffer[i]; // Add the character to the current operand
    }

    // Check if there's a last operand to process after the loop
    if (opIndex > 0) 
    {
        operand[opIndex] = '\0'; // Null-terminate the last operand
        if (operand[0] == 'R') 
        { 
            RegisterTokens regToken = validateRegisterToken(operand);
            const char *binary = getBinValForRegister(regToken);
            if (binary) 
            {
                strcpy(&binaryOut[binIndex], binary);
                binIndex += strlen(binary);
            }
        } 
        else if (operand[0] == '#') 
        {
            immToBinary(operand, &binaryOut[binIndex], immediateSize);
            binIndex += immediateSize;
        }
    }
    binaryOut[binIndex] = '\0'; // Null-terminate the binary output string
}

void immToBinary(const char *immStr, char *binaryOut, int immediateSize) 
{
    // Skip the '#' character to get the integer value
    int immVal = atoi(immStr + 1);

    // Calculate the maximum value based on the immediate size
    int maxVal = (1 << (immediateSize - 1)) - 1;
    int minVal = -maxVal - 1;

    // Check if the value is within the valid range
    if (immVal < minVal || immVal > maxVal) 
    {
        // Handle the error appropriately
        strcpy(binaryOut, "ERROR");
        return;
    }

    // Adjust the value for negative numbers to fit into the specified bit size
    if (immVal < 0) 
    {
        immVal = (1 << immediateSize) + immVal;
    }

    // Convert to binary representation
    for (int i = immediateSize - 1; i >= 0; i--) 
    {
        binaryOut[i] = (immVal & 1) ? '1' : '0';
        immVal >>= 1;
    }
    binaryOut[immediateSize] = '\0';
}

void writeLineToBin(const char *opcode, const char *binaryOut)
{
    int totalLen = strlen(opcode) + strlen(binaryOut) + 2;

    char *binaryLine = (char *)malloc(totalLen * sizeof(char));
    if (binaryLine == NULL) 
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(binaryLine, opcode);
    strcat(binaryLine, binaryOut);
    binaryLine[totalLen - 2] = '\n'; 
    binaryLine[totalLen - 1] = '\0'; 

    FILE *binFile = fopen("output.bin", "ab");
    if (binFile == NULL) 
    {
        fprintf(stderr, "Error opening file.\n");
        free(binaryLine);
        exit(EXIT_FAILURE);
    }

    fwrite(binaryLine, sizeof(char), totalLen - 1, binFile);

    fclose(binFile);
    free(binaryLine);
    printf("Successfully wrote to output.bin\n");
}