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

typedef struct {
    BinOps binaryOps;
    const char *comment;
} CommentMap;

CommentMap commentMap[] = {
    {ADD_ONE_OP, "; ADD statement responsible for adding some SR1 and SR2, and placing the result in some DR."},
    {ADD_TWO_OP, "; ADD statement responsible for adding some SR1 and Imm5, and placing the result in some DR."},
    {AND_ONE_OP, "; AND statement responsible for anding some SR1 and SR2, and placing the result in some DR."},
    {AND_TWO_OP, "; AND statement responsible for anding some SR1 and Imm5, and placing the result in some DR."},
    {BR_OP, "; BR statement responsible for branching on some condition (n/z/p) to some defined LABEL"},
    {LD_OP, "; LD statement responsible for loading some defined LABEL into some DR"},
    {LDI_OP, "; LDI statement responsible for loading some defined LABEL indirectly into some DR"},
    {LDR_OP, "; LDR statement responsible for loading some SR1 into DR, with some offset6"},
    {LEA_OP, "; LEA statement responsible for loading the effective address of some defined LABEL into some DR"},
    {NOT_OP, "; NOT statement responsible for notting some defined SR1 and placing the result in some DR"},
    {ST_OP, "; ST statement responsible for storing some defined LABEL into some defined SR1"},
    {STI_OP, "; STI statement responsible for storing some defined LABEL indirectly into some defined SR1"},
    {STR_OP, "; STR statement responsible for storing some defined SR2 into some defined SR1, with some offset6"},
    {TRAP_OP, "; TRAP statement responsible for invoking exiting syscall"},
    {INVALID_OP, "; NULL"},
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
bool isValidTrapVector(const char *offset);

// Parsers for tokens
bool parseORIG(char *source, int *minIndex, unsigned int *address);
bool parseADD(char *source, int *minIndex, char *operandsOut); 
bool parseAND(char *source, int *minIndex, char *operandsOut);
bool parseBR(const char *instruction, char labels[][MAX_LABEL_LEN], int labelCount, char *labelOut);
bool isBRInstruction(char *token);
bool parseLD(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseLDI(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseLDR(char *source, int *minIndex, char *operandsBuffer);
bool parseLEA(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseNOT(char *source, int *minIndex, char *operandsOut);
bool parseST(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseSTI(char *source, int *minIndex, char labels[][MAX_LABEL_LEN], int labelCount);
bool parseSTR(char *source, int *minIndex, char *operandsOut);
bool parseTRAP(char *source, int *minIndex, int *trapVector);
bool parseSEMI(char *source, int *minIndex);
bool parseFILL(char *source, int *minIndex, int *immValue);
bool parseEND(char *source, int *minIndex);
bool parseBLKW(char *source, int *minIndex, int *blockSize);

const char *getOpcodeForToken(BinOps binaryOps);
const char *getBinValForRegister(RegisterTokens regTok);
const char *getCommentForInstruction(BinOps binaryOps);
BinOps tokenToBinaryOp(Tokens token, const char *operands);
void processOperands(const char *operandsBuffer, char *binaryOut, Tokens tokenType);
void immToBinary(const char *immStr, char *binaryOut, int immediateSize);
void writeLineToBin(const char *opcode, const char *binaryOut, const char *comment, FILE *binFile); 
void hexToBinary(unsigned int hex, char *binary, int bits);

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
        char* token = strtok(line, " \t\n"); // Tokenize the line

        while (token != NULL) 
        {
            // Check if the token starts with "BR" and contains condition codes
            if (strncmp(token, "BR", 2) == 0 && strlen(token) > 2) 
            {
                // Extract the condition code (n, z, p) from BR
                char conditionCode = token[2];

                // The next token should be the label after "BRn/z/p"
                token = strtok(NULL, " \t\n");
            }

            // Now 'token' should be a label or another instruction
            if (token != NULL && validateToken(token) == INVALID_TOKEN) 
            {
                // Check if it's a label and not previously added
                bool isLabelAlreadyAdded = false;
                int i;
                for (i = 0; i < labelCount; i++) 
                {
                    if (strcmp(labels[i], token) == 0) 
                    {
                        isLabelAlreadyAdded = true;
                        break;
                    }
                }

                if (!isLabelAlreadyAdded) 
                {
                    // Add the label to the labels array
                    strncpy(labels[labelCount], token, MAX_LABEL_LEN - 1);
                    labels[labelCount][MAX_LABEL_LEN - 1] = '\0'; // Ensure null termination
                    labelCount++;
                }
            }

            // Move to the next token
            token = strtok(NULL, " \t\n");
        }
    }
    
    rewind(file);

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

            if (line[minIndex] == '.') 
            {
                consume(line, &minIndex);

                while (!isspace(line[minIndex]) && line[minIndex] != '\0') 
                {
                    tokenBuffer[tokenIndex++] = line[minIndex++];
                }
                tokenBuffer[tokenIndex] = '\0';

                if (strcmp(tokenBuffer, "ORIG") == 0) 
                {
                    unsigned int address;
                    if (parseORIG(line, &minIndex, &address)) 
                    {
                        printf("Found .ORIG directive with address x%X (VALID)\n", address);

                        // Convert the address to binary
                        char binaryAddress[17]; // 16 bits + null terminator
                        hexToBinary(address, binaryAddress, 16);

                        // Write ".ORIG" and binary address to output.bin
                        fprintf(binFile, ".ORIG %s\n", binaryAddress);
                    } 
                    else 
                    {
                        printf("Failed to parse address for .ORIG directive.\n");
                    }
                }
                else if (strcmp(tokenBuffer, "FILL") == 0) 
                {
                    int immValue;
                    if (parseFILL(line, &minIndex, &immValue)) 
                    {
                        printf("Valid .FILL directive with value: %d.\n", immValue);

                        // Convert immValue to binary
                        char binaryValue[17] = {0}; // Initialize all elements to 0
                        int i;
                        for (i = 15; i >= 0; i--, immValue >>= 1) 
                        {
                            binaryValue[i] = (immValue & 1) + '0';
                        }

                        fprintf(binFile, ".FILL %s\n", binaryValue);
                    } 
                    else 
                    {
                        printf("Failed to parse or invalid operand for .FILL directive.\n");
                    }
                }
                else if (strcmp(tokenBuffer, "END") == 0) 
                {
                    if (parseEND(line, &minIndex)) 
                    {
                        // Since there's no binary equivalent for .END, we just append a comment noting the end of the program
                        printf("End of program found.\n");
                        fprintf(binFile, "; END OF PROGRAM\n");
                    } 
                    else 
                    {
                        printf("Invalid format for .END directive.\n");
                    }
                }
                else if (strcmp(tokenBuffer, "BLKW") == 0) 
                {
                    int blockSize;
                    if (parseBLKW(line, &minIndex, &blockSize)) 
                    {
                        printf("Valid .BLKW directive with block size: %d.\n", blockSize);

                        int i;
                        for (i = 0; i < blockSize; i++) 
                        {
                            fprintf(binFile, "; Reserved word %d of %d from .BLKW\n", i + 1, blockSize);
                        }
                    } 
                    else 
                    {
                        printf("Failed to parse or invalid block size for .BLKW directive.\n");
                    }
                }

                continue;
            }

            tokenBuffer[tokenIndex++] = consume(line, &minIndex);

            if (isspace(peek(0, line, &minIndex)) || peek(0, line, &minIndex) == '\0') 
            {
                tokenBuffer[tokenIndex] = '\0';
                tokenIndex = 0;

                if (strncmp(tokenBuffer, "BR", 2) == 0 && strlen(tokenBuffer) > 2) 
                {
                    // Prepare a buffer to hold the label part after "BR" instruction, excluding comments
                    char labelPart[256] = {0}; // Initialize the buffer to store the label

                    // Extract the label part from the rest of the line, ignoring comments
                    char *commentStart = strchr(line + minIndex, ';'); // Find the start of a comment
                    if (commentStart != NULL) 
                    {
                        *commentStart = '\0'; // Terminate the line at the start of the comment to exclude it
                    }

                    // Copy the label part, excluding any leading whitespace
                    sscanf(line + minIndex, "%s", labelPart); // This will skip leading whitespace and stop at the first whitespace after the label

                    // Validate the extracted label part
                    if (isValidLabel(labelPart, labels, labelCount)) 
                    {
                        printf("Valid BR instruction with label: %s\n", labelPart);
                    } 
                    else 
                    {
                        printf("Invalid BR instruction or label not found: %s\n", labelPart);
                    }
                }
                else 
                {
                    tokenType = validateToken(tokenBuffer);
                    if (firstToken && tokenType == INVALID_TOKEN) 
                    {
                        if (isLabelDefinition(tokenBuffer)) 
                        {
                            addLabel(labels, &labelCount, tokenBuffer);
                            printf("Label defined: %s\n", tokenBuffer);
                        } 
                        else 
                        {
                            printf("Invalid token or unrecognized label: %s\n", tokenBuffer);
                        }
                        firstToken = false;
                    }
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
                            printf("\nValid operands for ADD instruction.\n");
                            BinOps binaryAdd = tokenToBinaryOp(ADD, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryAdd);
                            const char *comment = getCommentForInstruction(binaryAdd);
                            printf("Opcode for ADD: %s\n", opcode);
                            printf("Operands: %s\n", operandsBuffer);
                            processOperands(operandsBuffer, binaryOut, ADD);
                            printf("Binary operands for ADD: %s\n", binaryOut);
                            printf("Comment for ADD: %s\n", comment);

                            writeLineToBin(opcode, binaryOut, comment, binFile);
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
                            printf("\nValid operands for AND instruction.\n");
                            BinOps binaryAnd = tokenToBinaryOp(AND, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryAnd);
                            const char *comment = getCommentForInstruction(binaryAnd);
                            printf("Opcode for AND: %s\n", opcode);
                            printf("Operands: %s\n", operandsBuffer);
                            processOperands(operandsBuffer, binaryOut, AND);
                            printf("Binary operands for AND: %s\n", binaryOut);
                            printf("Comment for AND: %s\n", comment);

                            writeLineToBin(opcode, binaryOut, comment, binFile);
                        }
                    }
                    else if (tokenType == BR)
                    {
                        char fullBRInstruction[256];
                        snprintf(fullBRInstruction, sizeof(fullBRInstruction), "%s%s", tokenBuffer, line + minIndex);

                        char remainingInstruction[256]; // Buffer to hold just the label after parsing

                        if (!parseBR(fullBRInstruction, labels, labelCount, remainingInstruction)) 
                        {
                            printf("Invalid BR instruction or label not found.\n");
                        } 
                        else 
                        {
                            printf("\nBR instruction with valid label: %s\n", remainingInstruction);
                            BinOps binaryBr = tokenToBinaryOp(BR, remainingInstruction); 
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
                            printf("\nValid operands for LD instruction.\n");
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
                            printf("\nValid operands for LDI instruction.\n");
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
                            printf("\nValid operands for LDR instruction.\n");
                            BinOps binaryLdr = tokenToBinaryOp(LDR, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryLdr);
                            const char *comment = getCommentForInstruction(binaryLdr);
                            printf("Opcode for LDR: %s\n", opcode);
                            printf("Operands: %s\n", operandsBuffer);
                            processOperands(operandsBuffer, binaryOut, LDR);
                            printf("Binary operands for LDR: %s\n", binaryOut);
                            printf("Comment for ADD: %s\n", comment);

                            writeLineToBin(opcode, binaryOut, comment, binFile);
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
                            printf("\nValid operands for LEA instruction.\n");
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
                            printf("\nValid operands for NOT instruction.\n");
                            BinOps binaryNot = tokenToBinaryOp(NOT, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryNot);
                            const char *comment = getCommentForInstruction(binaryNot);
                            printf("Opcode for NOT: %s\n", opcode);
                            printf("Operands: %s\n", operandsBuffer);
                            processOperands(operandsBuffer, binaryOut, NOT); 
                            printf("Binary operands for NOT: %s\n", binaryOut);
                            printf("Comment for NOT: %s\n", comment);

                            writeLineToBin(opcode, binaryOut, comment, binFile);
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
                            printf("\nValid operands for ST instruction.\n");
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
                            printf("\nValid operands for STI instruction.\n");
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
                            printf("\nValid operands for STR instruction.\n");
                            BinOps binaryStr = tokenToBinaryOp(STR, operandsBuffer);
                            const char *opcode = getOpcodeForToken(binaryStr);
                            const char *comment = getCommentForInstruction(binaryStr);
                            printf("Opcode for STR: %s\n", opcode);
                            printf("Operands: %s\n", operandsBuffer);
                            processOperands(operandsBuffer, binaryOut, STR);
                            printf("Binary operands for STR: %s\n", binaryOut);
                            printf("Comment for ADD: %s\n", comment);

                            writeLineToBin(opcode, binaryOut, comment, binFile);
                        }
                    }
                    else if (tokenType == TRAP) 
                    {
                        int trapVector;
                        if (parseTRAP(line, &minIndex, &trapVector)) 
                        {
                            BinOps binaryTrap = tokenToBinaryOp(TRAP, NULL);
                            const char *opcode = getOpcodeForToken(binaryTrap);
                            const char *comment = getCommentForInstruction(binaryTrap);
                            printf("Valid TRAP instruction.\n");

                            // Convert the trap vector to binary, ensuring it's 8 bits for the trap vector
                            char binaryTrapVector[9]; // 8 bits for the vector + null terminator
                            hexToBinary(trapVector, binaryTrapVector, 8);

                            char binaryOut[17]; // Full binary instruction + null terminator
                            snprintf(binaryOut, sizeof(binaryOut), "0000%s", binaryTrapVector); // Include padding and trap vector

                            writeLineToBin(opcode, binaryOut, comment, binFile);
                        } 
                        else 
                        {
                            printf("Failed to parse trap vector for TRAP directive.\n");
                        }
                    }
                }
            }
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
        printf("\nConsumed newline at index: %d, incrementing line count\n", *minIndex - 1);
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

bool addLabel(char labels[][MAX_LABEL_LEN], int* labelCount, const char* tokenBuffer) 
{
    // Check if the label already exists
    int i;
    for(i = 0; i < *labelCount; i++) 
    {
        if(strcmp(labels[i], tokenBuffer) == 0) 
        {
            return false; // Label already exists
        }
    }
    // Add the new label
    strcpy(labels[*labelCount], tokenBuffer);
    (*labelCount)++;
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

bool parseBR(const char *instruction, char labels[][MAX_LABEL_LEN], int labelCount, char *labelOut) 
{
    // Ensure instruction starts with "BR"
    if (strncmp(instruction, "BR", 2) != 0) 
    {
        return false;
    }

    int i = 2; // Start index after "BR"
    while (instruction[i] == 'n' || instruction[i] == 'z' || instruction[i] == 'p') 
    {
        i++;
    }

    // Initialize an index for labelOut
    int j = 0;
    // Skip whitespace after condition codes to start of label
    while (isspace(instruction[i])) i++;
    // Copy the label part into labelOut, stop at comment or end of line
    while (instruction[i] != '\0' && instruction[i] != ';' && !isspace(instruction[i])) 
    {
        labelOut[j++] = instruction[i++];
    }
    labelOut[j] = '\0'; // Null-terminate the label

    // Before validating, ensure labelOut is not empty
    if (strlen(labelOut) == 0) 
    {
        return false; // No label found
    }

    // Validate the extracted label
    return isValidLabel(labelOut, labels, labelCount);
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

bool isValidTrapVector(const char *offset) 
{
    unsigned int val;
    sscanf(offset, "%x", &val); // Convert hex string to unsigned int
    return val <= 0xFF; // Trap vectors are 8-bit, so valid if within this range
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

const char *getOpcodeForToken(BinOps binaryOps)
{
    int i;
    for (i = 0; instructionMap[i].binaryOps != INVALID_OP; i++)
    {
        if (instructionMap[i].binaryOps == binaryOps)
        {
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

const char *getCommentForInstruction(BinOps binaryOps)
{
    int i;
    for (i = 0; commentMap[i].binaryOps != INVALID_OP; i++)
    {
        if (commentMap[i].binaryOps == binaryOps)
        {
            return commentMap[i].comment;
        }
    }
    return NULL;
}

void processOperands(const char *operandsBuffer, char *binaryOut, Tokens tokenType) 
{
    binaryOut[0] = '\0'; // Initialize the binaryOut buffer to an empty string
    
    switch (tokenType) 
    {
        case AND: {
            char drBuffer[256], srBuffer[256], immediateBuffer[256];
            // Parse the operandsBuffer to extract DR, SR, and possibly an immediate value
            int operandCount = sscanf(operandsBuffer, "%[^,],%[^,],%s", drBuffer, srBuffer, immediateBuffer);
            
            // Convert DR and SR to their binary representations
            const char *binaryDR = getBinValForRegister(validateRegisterToken(drBuffer));
            const char *binarySR = getBinValForRegister(validateRegisterToken(srBuffer));

            // Begin constructing the binary output for AND instruction
            strcat(binaryOut, binaryDR); // Append DR's binary representation
            strcat(binaryOut, binarySR); // Append SR's binary representation

            if (operandCount == 3) 
            {
                if (immediateBuffer[0] == '#') 
                {
                    strcat(binaryOut, "1"); // Append "1" to indicate immediate mode

                    // Convert the immediate value to binary and append it
                    char binaryImm[16];
                    immToBinary(immediateBuffer, binaryImm, 5);
                    strcat(binaryOut, binaryImm);
                } 
                else 
                {
                    printf("Non-immediate third operand!\n");
                }
            } 
            else 
            {
                // If not using immediate mode, append "0" and five "0"s to complete the instruction format
                // This case handles AND instructions that do not use immediate values (register mode)
                strcat(binaryOut, "0"); // Append "0" to indicate register mode
                strcat(binaryOut, "00000"); // Append five "0"s for padding
            }
            break;
        }
        case ADD: {
            char drBuffer[256], sr1Buffer[256], secondOperandBuffer[256];
            // Parse the operandsBuffer to extract DR, SR1, and either SR2 or an immediate value
            int operandCount = sscanf(operandsBuffer, "%[^,],%[^,],%s", drBuffer, sr1Buffer, secondOperandBuffer);
            
            // Convert DR and SR1 to their binary representations
            const char *binaryDR = getBinValForRegister(validateRegisterToken(drBuffer));
            const char *binarySR1 = getBinValForRegister(validateRegisterToken(sr1Buffer));

            // Begin constructing the binary output for ADD instruction
            strcat(binaryOut, binaryDR); // Append DR's binary representation
            strcat(binaryOut, binarySR1); // Append SR1's binary representation

            if (operandCount == 3) 
            {
                if (secondOperandBuffer[0] == '#') 
                {
                    strcat(binaryOut, "1"); // Append "1" to indicate immediate mode

                    // Convert the immediate value to binary and append it
                    char binaryImm[16];
                    immToBinary(secondOperandBuffer, binaryImm, 5);
                    strcat(binaryOut, binaryImm);
                } 
                else 
                {
                    // Third operand is a register (SR2)
                    strcat(binaryOut, "000"); // Append "000" to indicate it's a register operation
                    const char *binarySR2 = getBinValForRegister(validateRegisterToken(secondOperandBuffer));
                    strcat(binaryOut, binarySR2); // Append SR2's binary representation
                }
            } 
            else 
            {
                printf("Operand count doesn't match expectations for ADD instruction.\n");
            }
            break;
        }
        case NOT: {
            char drBuffer[256], srBuffer[256];
            sscanf(operandsBuffer, "%[^,],%s", drBuffer, srBuffer); // Split operandsBuffer into DR and SR
            
            // Convert DR and SR to their binary representations
            const char *binaryDR = getBinValForRegister(validateRegisterToken(drBuffer));
            const char *binarySR = getBinValForRegister(validateRegisterToken(srBuffer));
            
            // Construct the binary instruction with "111111" appended
            sprintf(binaryOut, "%s%s111111", binaryDR, binarySR);
            break;
        }
        case LDR: {
            char drBuffer[256], baseRBuffer[256], offsetBuffer[256];
            // Parse the operandsBuffer to extract DR, BaseR, and offset
            int operandCount = sscanf(operandsBuffer, "%[^,],%[^,],%s", drBuffer, baseRBuffer, offsetBuffer);
            
            // Convert DR and BaseR to their binary representations
            const char *binaryDR = getBinValForRegister(validateRegisterToken(drBuffer));
            const char *binaryBaseR = getBinValForRegister(validateRegisterToken(baseRBuffer));

            // Begin constructing the binary output for LDR instruction
            strcat(binaryOut, binaryDR); // Append DR's binary representation
            strcat(binaryOut, binaryBaseR); // Append BaseR's binary representation

            if (operandCount == 3) 
            {
                // Convert the offset value to a 6-bit binary and append it
                char binaryOffset[16];
                immToBinary(offsetBuffer, binaryOffset, 6);
                strcat(binaryOut, binaryOffset);
            } 
            else 
            {
                printf("Operand count doesn't match expectations for LDR instruction.\n");
            }
            break;
        }
        case STR: {
            char srBuffer[256], baseRBuffer[256], offsetBuffer[256];
            // Parse the operandsBuffer to extract SR, BaseR, and offset
            int operandCount = sscanf(operandsBuffer, "%[^,],%[^,],%s", srBuffer, baseRBuffer, offsetBuffer);
            
            // Convert SR and BaseR to their binary representations
            const char *binarySR = getBinValForRegister(validateRegisterToken(srBuffer));
            const char *binaryBaseR = getBinValForRegister(validateRegisterToken(baseRBuffer));

            // Begin constructing the binary output for STR instruction
            strcat(binaryOut, binarySR); // Append SR's binary representation
            strcat(binaryOut, binaryBaseR); // Append BaseR's binary representation

            if (operandCount == 3) 
            {
                // Convert the offset value to a 6-bit binary and append it
                char binaryOffset[16];
                immToBinary(offsetBuffer, binaryOffset, 6);
                strcat(binaryOut, binaryOffset);
            } 
            else 
            {
                printf("Operand count doesn't match expectations for STR instruction.\n");
            }
            break;
        }
        default: {
            char operand[256];
            int opIndex = 0; // Index to build up each operand string
            int binIndex = 0; // Index for the binaryOut string
            
            int immediateSize = 5;

            int i;
            for (i = 0; operandsBuffer[i] != '\0'; i++) 
            {
                if (operandsBuffer[i] == ',' || operandsBuffer[i] == ' ') 
                {
                    if (opIndex > 0) 
                    {
                        operand[opIndex] = '\0'; // Null-terminate the current operand
                        if (operand[0] == 'R') 
                        {
                            // Operand is a register
                            const char *binaryReg = getBinValForRegister(validateRegisterToken(operand));
                            strcat(binaryOut, binaryReg);
                        } 
                        else if (operand[0] == '#') 
                        {
                            // Operand is an immediate value
                            char binaryImm[16];
                            immToBinary(operand, binaryImm, immediateSize);
                            strcat(binaryOut, binaryImm);
                        }
                        opIndex = 0; // Reset operand index for the next operand
                    }
                } 
                else 
                {
                    operand[opIndex++] = operandsBuffer[i];
                }
            }

            // Process the last operand, if any
            if (opIndex > 0) 
            {
                operand[opIndex] = '\0'; // Null-terminate the last operand
                if (operand[0] == 'R') 
                {
                    // Operand is a register
                    const char *binaryReg = getBinValForRegister(validateRegisterToken(operand));
                    strcat(binaryOut, binaryReg);
                } 
                else if (operand[0] == '#') 
                {
                    // Operand is an immediate value
                    char binaryImm[16];
                    immToBinary(operand, binaryImm, immediateSize);
                    strcat(binaryOut, binaryImm);
                }
            }
            break;
        }
    }
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
        strcpy(binaryOut, "ERROR");
        return;
    }

    // Adjust the value for negative numbers to fit into the specified bit size
    if (immVal < 0) 
    {
        immVal = (1 << immediateSize) + immVal;
    }

    // Convert to binary representation
    int i;
    for (i = immediateSize - 1; i >= 0; i--) 
    {
        binaryOut[i] = (immVal & 1) ? '1' : '0';
        immVal >>= 1;
    }
    binaryOut[immediateSize] = '\0';
}

void writeLineToBin(const char *opcode, const char *binaryOut, const char *comment, FILE *binFile) 
{
    int spaceNeeded = (strlen(binaryOut) > 0) ? 1 : 0;

    // Calculate the total length considering the opcode, binary output, comment, and newline.
    int totalLen = strlen(opcode) + strlen(binaryOut) + strlen(comment) + spaceNeeded + 2; // +2 for the newline and null terminator

    char *binaryLine = (char *)malloc(totalLen * sizeof(char));
    if (binaryLine == NULL) 
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    // Concatenate opcode, binary output (with a space if needed), and comment into one line
    snprintf(binaryLine, totalLen, "%s%s%s%s\n", opcode, binaryOut, spaceNeeded ? " " : "", comment);

    // Write the combined line to the binary file
    fwrite(binaryLine, sizeof(char), strlen(binaryLine), binFile);

    free(binaryLine);
}

void hexToBinary(unsigned int hex, char *binary, int bits) 
{
    binary[bits] = '\0';
    int i;
    for (i = bits - 1; i >= 0; i--) 
    {
        binary[i] = (hex & 1) ? '1' : '0';
        hex >>= 1;
    }
}