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

typedef struct {
    char label[MAX_LABEL_LEN];
    int lineNum;
    int address;
} LabelInfo;

char peek(int offset, char *source, int *minIndex);
char consume(char *source, int *minIndex);
Tokens validateToken(const char *token);
RegisterTokens validateRegisterToken(const char *regstr);
bool isRegister(char *token);
bool isSoloLabel(const char* token);
bool isImm5(char *imm5);
bool isValidBranchCondition(char condition);
bool isValidLabel(char *label, char labels[][MAX_LABEL_LEN], int count);
bool isLabelDefinition(char *token);
bool addLabel(char labels[][MAX_LABEL_LEN], int* labelCount, const char* tokenBuffer);
bool isOffset6(char *offset);
bool isValidTrapVector(const char *offset);

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
void processOperands(const char *operandsBuffer, char *binaryOut, Tokens tokenType, LabelInfo labelInfos[], int labelCount, int currentLine);
void immToBinary(const char *immStr, char *binaryOut, int immediateSize);
void writeLineToBin(const char *opcode, const char *binaryOut, const char *comment, FILE *binFile); 
void hexToBinary(unsigned int hex, char *binary, int bits);
void convertLineNumToBin(int lineNum, char *binaryRepresentation, int bits);
int calculateOffset(const char* targetLabel, LabelInfo labels[], int labelCount, int currentLine);
void intToBinary(int value, char *binaryOut, int size);

#include "utilities.h"
#include "validations.h"
#include "parsing.h"

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

    LabelInfo labelInfos[MAX_LABELS];
    LabelInfo dummyLabels[1];
    char label[MAX_LABEL_LEN];
    int labelLines[MAX_LABELS];
    int labelAddresses[MAX_LABELS];
    int lineNum = 0;
    int dummyLabelCount = 0;
    int currentAddress = 0;
    bool isLabel;
    bool startAddressSet = false;
    bool isDirectiveThatConsumesSpace = false;
    int blockSize = 0;
    
    while (fgets(line, sizeof(line), file)) 
    {
        // Ignore lines that are empty or start with a comment
        if (line[0] == ';' || line[0] == '\n' || line[0] == '\r' || line[0] == '\0') 
        {
            lineNum++;
            continue;
        }

        char* token = strtok(line, " \t\n");
        bool isLabelLine = false;
        int minIndex = 0;

        if (token && strcmp(token, ".ORIG") == 0)
        {
            token = strtok(NULL, " \t\n");
            if (token)
            {
                sscanf(token, "x%X", &currentAddress);
                printf("Starting Address: x%X\n", currentAddress);
                startAddressSet = true;
            }
        }
        else if (strcmp(token, ".FILL") == 0)
        {
            currentAddress += 2;
            isDirectiveThatConsumesSpace = true;
        }
        else if (strcmp(token, ".BLKW") == 0)
        {
            int blockSize = 0;
            if (parseBLKW(line, &minIndex, &blockSize))
            {
                printf("Valid .BLKW directive with block size: %d.\n", blockSize);
                currentAddress += 2 * blockSize;
                isDirectiveThatConsumesSpace = true;
            }
        }
        else if (!isLabelLine) 
        {
            currentAddress += 2;
        }

        if (token && validateToken(token) == INVALID_TOKEN) 
        {
            // Check the next token to ensure this isn't just an unknown instruction
            char *nextToken = strtok(NULL, " \t\n");
            if (nextToken && validateToken(nextToken) != INVALID_TOKEN) 
            {
                isLabelLine = true;
            }
            else if (isSoloLabel(token)) 
            {
                isLabelLine = true;
            }
        }

        // If we found a label, store it with its line number
        if (isLabelLine) 
        {
            size_t tokenLen = strlen(token);
            if (token[tokenLen - 1] == ':') 
            {
                token[tokenLen - 1] = '\0';
            }
            strncpy(labels[labelCount], token, MAX_LABEL_LEN - 1);
            labels[labelCount][MAX_LABEL_LEN - 1] = '\0'; // Ensure null-termination
            labelLines[labelCount] = lineNum;
            labelAddresses[labelCount] = currentAddress;
            labelCount++;
        }

        lineNum++;
        isDirectiveThatConsumesSpace = false;
    }

    for (int i = 0; i < labelCount; i++) 
    {
        strcpy(labelInfos[i].label, labels[i]);
        labelInfos[i].lineNum = labelLines[i] + 1;
        labelInfos[i].address = labelAddresses[i];
    }

    printf("Total Labels: %d\n", labelCount);
    for (int i = 0; i < labelCount; i++) 
    {
        printf("Label: %s, Line Number: %d\n", labels[i], labelLines[i] + 1);
    }
    printf("\n");

    for (int i = 0; i < labelCount; i++) 
    {
        printf("Label: %s, Line Number: %d, Address: x%X\n", labelInfos[i].label, labelInfos[i].lineNum, labelInfos[i].address);
    }

    rewind(file);
    lineNum = 0;
    currentAddress = 0x3000;

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

                if (strncmp(tokenBuffer, "BR", 2) == 0) 
                {
                    char conditionCodes[4] = {0};
                    strncpy(conditionCodes, tokenBuffer + 2, 3); // Extract condition codes (n, z, p)
                    
                    // Consume whitespace and extract the label
                    while (isspace(peek(0, line, &minIndex))) consume(line, &minIndex);

                    char label[256];
                    int labelIndex = 0;
                    while (!isspace(peek(0, line, &minIndex)) && peek(0, line, &minIndex) != '\0') 
                    {
                        label[labelIndex++] = consume(line, &minIndex);
                    }
                    label[labelIndex] = '\0';

                    if (isValidLabel(label, labels, labelCount)) 
                    {
                        char conditionBinary[4] = {'0', '0', '0', '\0'};
                        if (strchr(conditionCodes, 'n')) conditionBinary[0] = '1';
                        if (strchr(conditionCodes, 'z')) conditionBinary[1] = '1';
                        if (strchr(conditionCodes, 'p')) conditionBinary[2] = '1';

                        int offset = calculateOffset(label, labelInfos, labelCount, currentAddress + 4);
                        if (offset == INT_MIN) 
                        {
                            printf("Error: Label '%s' not found.\n", label);
                        } 
                        else 
                        {
                            printf("Offset: %d\n", offset);

                            char offsetBinary[10];
                            intToBinary(offset, offsetBinary, 9); // Convert offset to binary

                            char binaryInstruction[17];
                            snprintf(binaryInstruction, sizeof(binaryInstruction), "0000%s%s", conditionBinary, offsetBinary);

                            printf("BR instruction binary: %s\n", binaryInstruction);

                            BinOps binaryBr = tokenToBinaryOp(BR, conditionCodes);
                            const char *opcode = getOpcodeForToken(binaryBr);
                            const char *comment = getCommentForInstruction(binaryBr);

                            writeLineToBin("", binaryInstruction, comment, binFile);
                        }
                    } 
                    else 
                    {
                        printf("Invalid BR instruction or label not found: %s\n", label);
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
                            processOperands(operandsBuffer, binaryOut, ADD, dummyLabels, dummyLabelCount, lineNum);
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
                            processOperands(operandsBuffer, binaryOut, AND, dummyLabels, dummyLabelCount, lineNum);
                            printf("Binary operands for AND: %s\n", binaryOut);
                            printf("Comment for AND: %s\n", comment);

                            writeLineToBin(opcode, binaryOut, comment, binFile);
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
                            processOperands(operandsBuffer, binaryOut, LDR, dummyLabels, dummyLabelCount, lineNum);
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
                            processOperands(operandsBuffer, binaryOut, NOT, dummyLabels, dummyLabelCount, lineNum); 
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
                            processOperands(operandsBuffer, binaryOut, STR, dummyLabels, dummyLabelCount, lineNum);
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

        if (tokenType != INVALID_TOKEN && tokenType != LABEL) 
        {
            currentAddress += 2;
        }

        lineNum++;
    }

    fclose(file);
    fclose(binFile);
    printf("Successfully converted the LC-3 ASM file to binary!");

    return 0;
}