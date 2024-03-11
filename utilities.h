#ifndef UTILITIES_H
#define UTILITIES_H

#include "validations.h"
#include "parsing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

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

void processOperands(const char *operandsBuffer, char *binaryOut, Tokens tokenType, LabelInfo labelInfos[], int labelCount, int currentLine)
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
        case BR: {
            char conditionCodes[4] = {0};
            sscanf(operandsBuffer, "%3s", conditionCodes); // Extract condition codes
            
            char label[256];
            // Assuming the label immediately follows the condition codes
            strcpy(label, operandsBuffer + 4); // +4 to skip over the condition codes and space

            // Translate condition codes to binary
            char conditionBinary[4] = "000"; // Default to 000 for BR instruction
            if (strchr(conditionCodes, 'n')) conditionBinary[0] = '1';
            if (strchr(conditionCodes, 'z')) conditionBinary[1] = '1';
            if (strchr(conditionCodes, 'p')) conditionBinary[2] = '1';

            // Find the label's line number
            int labelLineNum = -1;
            for (int i = 0; i < labelCount; i++) {
                if (strcmp(labelInfos[i].label, label) == 0) {
                    labelLineNum = labelInfos[i].lineNum;
                    break;
                }
            }

            if (labelLineNum == -1) {
                printf("Label '%s' not found.\n", label);
                return; // Exit if label is not found
            }

            // Convert the line number difference to binary
            char lineNumBinary[10]; // 9 bits + null terminator for the offset
            convertLineNumToBin(labelLineNum - currentLine, lineNumBinary, 9); // Assuming convertLineNumberToBinary is defined

            // Construct the full binary instruction for BR
            snprintf(binaryOut, 17, "0000%s%s", conditionBinary, lineNumBinary);
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

void convertLineNumToBin(int lineNum, char *binaryRepresentation, int bits) 
{
    // Initialize all bits to '0'
    for (int i = 0; i < bits; i++) {
        binaryRepresentation[i] = '0';
    }
    binaryRepresentation[bits] = '\0'; // Null-terminate the string

    // Convert lineNum to binary representation
    for (int i = bits - 1; i >= 0 && lineNum > 0; i--) {
        binaryRepresentation[i] = (lineNum % 2) + '0';
        lineNum /= 2;
    }
}

#endif