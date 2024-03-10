        .ORIG x3000
            LD R1, NUMX ; Load x into R1
            AND R3, R3, #0 ; Clear R3
            ADD R2, R1, #-1 ; Set the counter
            LDR R4, R4, #5
            NOT R3, R4
            STR R2, R2, #6
            LEA R0, DATA 
            STR R0, R1, #0
            LDI R2, POINTER
            STI R2, VAL
            ST R3, STORLOC

LOOPADD
            ADD R3, R3, R1 ; Add: R3+R1->R3
            ADD R2, R2, #-1 ; Decrement counter
            BRp LOOPADD ; Loop if positive
            TRAP x22

HALT
        .BLKW 254 ; (254 because x3100 - x3001 (LD command for R1) - 1 (To account for PC) = 254 in decimal)
NUMX    .FILL #11 ; x=11
DATA    .FILL #6
POINTER .FILL #5
VAL     .FILL #1
STORLOC .FILL #4
        .END