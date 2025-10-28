# 0 "Aasm.c"

/*
 * AmmOS - Minimal Modular Operating System
 * Copyright (C) 2025 Ammar Najafli
 *
 * This file is part of AmmOS.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/mman.h>
#include <time.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <stdint.h>

#define T_INS       0x00
#define T_INT       0x01
#define T_LAB       0x02
#define T_REG8      0x03
#define T_REG16     0x04
#define T_REG32     0x05
#define T_REG64     0x06
// #define T_GREG      0x07
// #define T_PTRID     0x08
#define T_ADDR_EXPR 0x09
#define T_ID        0x0a
#define T_STR       0x0b
#define T_EOL       0x0c
#define T_EOF       0x0d
#define T_SEC       0x0e

#define T_BYTE      0x0f
#define T_BYTEPTR   0x10
#define T_QWORD     0x11
#define T_QWORDPTR  0x12
#define T_DWORD     0x13
#define T_DWORDPTR  0x14
#define T_LBYTE     0x15
#define T_LBYTEPTR  0x16
#define T_CHAR      0x17
#define T_COMMA     0x18

#define T_RESB      0x19
#define T_RESQ      0x1a
#define T_RESD      0x1b
#define T_RESL      0x1c

// #define ASTINS   0x00
// #define ASTBYTES 0x01
// #define ASTRES   0x02
// #define ASTEOF   0x03

#define MAX_TOKENS 1024
                
// .rodata            
const char *CMDS[] = {"mov", "push", "pop", "syscall", "call", "jmp", "add", "sub", 
               "mul", "div", "je", "jne", "jg", "jl", "jge", "jle", "jz", "ja", "jb",
               "xor", "or", "cmp", "inc", "dec", "nop", "ret", "leave", "test", "lea", "not", 
               "and", "shl", "shr", NULL};

const char *HUMAN_AST[] = {
    "U8",     "U8PTR",
    "U64",    "U64PTR",
    "U32",    "U32PTR",
    "U16",    "U16PTR",
    NULL
}; 
const char *HUMAN_TOKEN[] = {"inst", "label", "reg", "[reg]", "[reg +-*/ value]", "string", NULL};

/* name of label can include only this chars */
const char *LET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";

const char *LETEXT = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxy0123456789-_.";
const char* DIG    = "0123456789";
const char* DIGEXT = "0123456789abcdef";
const char* DIGBIN = "01";
const char* DIGOCT = "01234567";

const char* regs8[] = {
    "al", "bl", "cl", "dl",
    "sil", "dil", "spl", "bpl",
    NULL
};

const char* regs8GP[] = {
    "r8b", "r9b", "r10b", "r11b",
    "r12b", "r13b", "r14b", "r15b",
    NULL
};

const char* regs16[] = {
    "ax", "bx", "cx", "dx",
    "si", "di", "sp", "bp",
    NULL
};

const char* regs16GP[] = {
    "r8w", "r9w", "r10w", "r11w",
    "r12w", "r13w", "r14w", "r15w",
    NULL
};

const char* regs32[] = {
    "eax", "ebx", "ecx", "edx",
    "esi", "edi", "esp", "ebp",
    NULL
};

const char* regs32GP[] = {
    "r8d", "r9d", "r10d", "r11d",
    "r12d", "r13d", "r14d", "r15d",
    NULL
};

const char* regs64[] = {
    "rax", "rbx", "rcx", "rdx",
    "rsi", "rdi", "rsp", "rbp",
    "rip", NULL // user can control inst-pointer
};

const char* regs64GP[] = {
    "r8", "r9", "r10", "r11",
    "r12", "r13", "r14", "r15",
    NULL
};

typedef struct {
    int   type;       
    char* value;      
    int   line;       // for errors
} Token;

Token toks[MAX_TOKENS];
int toks_count = 0;

typedef struct {   
    const char* filename;  
    char        buf[256]; 
    Token*      toks;
    char labelscope[256]; 
} Lexer;

const char *p;


int isin(const char *str, char c){ 
    for(int i = 0; str[i] != '\0'; i++)
        if(str[i] == c) return 1;
    return 0;
}

int is2arrin(const char *str[], char *str2){ 
    for(int i=0; str[i] != NULL; ++i) 
        if(strcasecmp(str[i], str2) == 0) 
            return 1; 
    return 0;
}

long parse_expr(); // for gcc to escape conflicting types

long parse_number() {
    while (isspace(*p)) p++;
    
    if (*p == '-') {
        p++;
        return -parse_number();
    }

    if (*p == '(') {
        p++; // skip (
        long val = parse_expr();
        if (*p == ')') p++; // skip )
        return val;
    }

    int res = 0;
    if(*p == '0'){
        char *end;
        if(*(p+1) == 'x' || *(p+1) == 'X'){ 
            res = strtol(p, &end, 16);
            p = end;
            return res;
        }
        else return 0;
    }
    while (isdigit(*p)) {
        res = res * 10 + (*p - '0');
        p++;
    }
    return res;
}


// level: *, /
long parse_term() {
    long left = parse_number();
    while (1) {
        while (isspace(*p)) p++;
        if (*p == '*') {
            p++;
            left *= parse_number();
        } else if (*p == '/') {
            p++;
            left /= parse_number();
        } else {
            break;
        }
    }
    return left;
}

// level: +, -
long parse_expr() {
    long left = parse_term();
    while (1) {
        while (isspace(*p)) p++;
        if (*p == '+') {
            p++;
            left += parse_term();
        } else if (*p == '-') {
            p++;
            left -= parse_term();
        } else {
            break;
        }
    }
    return left;
}

long eval_expr(const char *str) {
    p = str;
    return parse_expr();
}

void add_token(int type, char* value, int line){
    if(toks_count >= MAX_TOKENS){
        fprintf(stderr, "AmmAsm:\033[;31m Fatal: Too mant tokens (max 1024)\033[0m\n");
        exit(1);
    }
    toks[toks_count].line = line;
    toks[toks_count].type = type;
    toks[toks_count].value = strdup(value);
    toks_count++;
}

int is_inst(char* s){
    for(int i=0; CMDS[i]; ++i){
        if(strcasecmp(s, CMDS[i]) == 0){
            return 1;
        }
    }
    //else
    return 0;
}

void DEBUG_PRINT_TOKENS() {
    printf("=== TOKENS DUMP (%d tokens) ===\n", toks_count);
    for (int i = 0; i < toks_count; i++) {
        const char* type_str = "UNKNOWN";
        switch(toks[i].type) {
            case T_INS: type_str = "T_INS"; break;
            case T_INT: type_str = "T_INT"; break;
            case T_LAB: type_str = "T_LAB"; break;
            case T_REG8: type_str = "T_REG8"; break;
            case T_REG16: type_str = "T_REG16"; break;
            case T_REG32: type_str = "T_REG32"; break;
            case T_REG64: type_str = "T_REG64"; break;
            case T_ADDR_EXPR: type_str = "T_ADDR_EXPR"; break;
            case T_ID: type_str = "T_ID"; break;
            case T_STR: type_str = "T_STR"; break;
            case T_EOL: type_str = "T_EOL"; break;
            case T_EOF: type_str = "T_EOF"; break;
            case T_SEC: type_str = "T_SEC"; break;
            case T_BYTE: type_str = "T_BYTE"; break;
            case T_BYTEPTR: type_str = "T_BYTEPTR"; break;
            case T_QWORD: type_str = "T_QWORD"; break;
            case T_QWORDPTR: type_str = "T_QWORDPTR"; break;
            case T_DWORD: type_str = "T_DWORD"; break;
            case T_DWORDPTR: type_str = "T_DWORDPTR"; break;
            case T_LBYTE: type_str = "T_LBYTE"; break;
            case T_LBYTEPTR: type_str = "T_LBYTEPTR"; break;
            case T_CHAR: type_str = "T_CHAR"; break;
            case T_COMMA: type_str = "T_COMMA"; break;
            case T_RESB: type_str = "T_RESB"; break;
            case T_RESQ: type_str = "T_RESQ"; break;
            case T_RESD: type_str = "T_RESD"; break;
            case T_RESL: type_str = "T_RESL"; break;
        }
        
        printf("Token[%d]: type=%-12s value='%s' line=%d\n", 
               i, type_str, toks[i].value ? toks[i].value : "(null)", toks[i].line);
    }
    printf("=== END TOKENS ===\n\n");
}


int is_reg(const char *s) __attribute__((__nonnull__(1)));
int is_reg(const char *s){
    // 1 = reg8
    // 2 = reg16
    // 3 = reg32
    // 4 = reg64
    // 5 = reg64GP
    // 0 = undefined
    while(isspace(*s))s++;
    
    for(int i=0; regs8[i]; i++){
        if(strcasecmp(s, *(regs8+i)) == 0){
            return 1;
        }
    }
    for(int i=0; regs16[i]; i++){
        if(strcasecmp(s, *(regs16+i)) == 0){
            return 2;
        }
    }
    for(int i=0; regs32[i]; i++){
        if(strcasecmp(s, *(regs32+i)) == 0){
            return 3;
        }
    }
    for(int i=0; regs64[i]; i++){
        if(strcasecmp(s, *(regs64+i)) == 0){
            return 4;
        }
    }
    for(int i=0; regs64GP[i]; i++){
        if(strcasecmp(s, *(regs64GP+i)) == 0){
            return 5;
        }
    }
    return 0;
}

int is_literal(char* s) __attribute__((__nonnull__(1)));
int is_literal(char* s) {
    if ((s[0] >= '0' && s[0] <= '9') || s[0] == '-') {
        return 1;
    }
    return 0;
}



char *read_string(char *buff, char *dest, int line) {
    while (*buff != '\0' && *buff != '"') {
        if (*buff == '/') {
            switch (*(buff + 1)) {
                case 'n': *dest++ = '\n'; buff += 2; continue;
                case '"': *dest++ = '"'; buff += 2; continue;
                case '/': *dest++ = '/'; buff += 2; continue;
                case 't': *dest++ = '\t'; buff += 2; continue;
                case '0': *dest = '\0'; buff += 2; return buff; // nah who does care about it any way 
                default : fprintf(stderr, "AmmAsm: invalid escape sequence on line %d\n", line); exit(1);
            }
        }
        else {
            *dest++ = *buff++;
        }
    }
    return buff;
}

int strlchar(const char *s, int size, char c) {
    const char *p = s + size; 
    while (p-- != s) {        
        if (*p == c) 
            return p - s;     
    }
    return -1;
}


/* littel endian */

// 16-bit
void As16(uint16_t a, unsigned char out[2]) {
    out[0] = a & 0xff;
    out[1] = (a >> 8) & 0xff;
}

// 32-bit
void As32(uint32_t a, unsigned char out[4]) {
    out[0] = a & 0xff;
    out[1] = (a >> 8) & 0xff;
    out[2] = (a >> 16) & 0xff;
    out[3] = (a >> 24) & 0xff;
}

// 64-bit
void As64(uint64_t a, unsigned char out[8]) {
    out[0] = a & 0xff;
    out[1] = (a >> 8) & 0xff;
    out[2] = (a >> 16) & 0xff;
    out[3] = (a >> 24) & 0xff;
    out[4] = (a >> 32) & 0xff;
    out[5] = (a >> 40) & 0xff;
    out[6] = (a >> 48) & 0xff;
    out[7] = (a >> 56) & 0xff;
}


 
// lexer:
int LEXER(FILE* fl) {
    if (!fl) {
        fprintf(stderr, "AmmAsm:\033[;31m Fatal: no such file or dir\033[0m\n");
        exit(1);
    }
    Lexer lexer = {0};
    int line = 0;
    int in_comment = 0;

    while (fgets(lexer.buf, 256, fl) != NULL) {
        char* buff = lexer.buf;
        line++;
        printf("Processing line %d: %s", line, lexer.buf);
         fflush(stdout);

        while (*buff) {
            printf("Current char: '%c' (0x%02x)\n", *buff, *buff);
            fflush(stdout);
            while (isspace(*buff)) buff++;
            if (*buff == '\0' || *buff == '\n') break;
            if ((*buff == '/' && *(buff + 1) == '/') || *buff == ';') break; // skip comment till end of line
            if(!in_comment && *buff == '/' && *(buff+1) == '*'){
                in_comment = 1;
                buff += 2;
                continue;
            }
            else if(in_comment){
                if(*buff == '*' && *(buff+1) == '/'){
                    in_comment = 0;
                    buff += 2;
                } else buff++;
                continue;
            }
            else if (isin(LET, *buff)) { // global label
                char buf[256] = {0};
                int i = 0;
                while (isin(LETEXT, *buff)) {
                    buf[i++] = *buff++;
                }
                
                if (*buff == ':') {
                    buff++;
                    strncpy(lexer.labelscope, buf, 256);
                    add_token(T_LAB, buf, line);
                }
                else if (is2arrin(CMDS, buf)){
                    add_token(T_INS, buf, line);
                }
                else {
                    add_token(T_ID, buf, line);
                }
            }

            else if (*buff == '.') { // local label
                buff++;
                char buf[256] = {0};
                int i = 0;
                while (isin(LETEXT, *buff)) {
                    buf[i++] = *buff++;
                }

                char full[512] = {0};
                if(lexer.labelscope[0] == 0){fprintf(stderr, "AmmAsm: Used local label with out global label on line \"%d\"", line); exit(1);}
                snprintf(full, sizeof(full), "%s.%s", lexer.labelscope, buf); // labelscope + "." + buf

                if (*buff == ':') {
                    buff++;
                    add_token(T_LAB, full, line);
                } else {
                    if(is_inst(buf))add_token(T_INS, full, line);
                    else add_token(T_ID, full, line); // undefined 
                }
            }
            else if (*buff == '%') {
                buff++;
                char buf[12];
                int i = 0;
                while (!isspace(*buff) && *buff != '\0' && *buff != ',') buf[i++] = *buff++;
                buf[i] = '\0'; 

                int regtype = is_reg(buf);
                if (regtype == 1) add_token(T_REG8, buf, line);
                else if (regtype == 2) add_token(T_REG16, buf, line);
                else if (regtype == 3) add_token(T_REG32, buf, line);
                else if (regtype == 4) add_token(T_REG64, buf, line);
                else if (regtype == 5) add_token(T_REG64, buf, line); // same

                continue;
            }
            else if (*buff == '-' || *buff == '+' || (isdigit(*buff) && *buff != '0')) {
                char expr_buf[128] = {0};
                int i = 0;

                while (*buff && (isdigit(*buff) || *buff == '+' || *buff == '-' || *buff == '*' || *buff == '/' || isspace(*buff))) {
                    if (i < (int)(sizeof(expr_buf) - 1)) {
                        expr_buf[i++] = *buff;
                    }
                    buff++;
                }
                expr_buf[i] = '\0';

                long long value = (long)eval_expr(expr_buf);

                char val_str[32];
                snprintf(val_str, sizeof(val_str), "%lld", value);
                add_token(T_INT, val_str, line);
                continue; 
            }
            else if(*buff == '"'){
                buff++;

                char parsed_str[256] = {0};
                buff = read_string(buff, parsed_str, line);

                if(*buff == '"') buff++;
                else {
                    fprintf(stderr, "AmmAsm: unterminated string literal at line %d\n", line);
                    exit(1);
                }
                add_token(T_STR, parsed_str, line);
                continue;
            }
            else if (*buff == '\'') {
                char tmp;

                buff++; 

                if (*buff == '\\') {
                    switch (*(buff + 1)) {
                        case 'n': tmp = '\n'; break;
                        case 'r': tmp = '\r'; break;
                        case 't': tmp = '\t'; break;
                        case '0': tmp = '\0'; break;
                        case '\\': tmp = '\\'; break;
                        case '\"': tmp = '\"'; break;
                        case '\'': tmp = '\''; break;
                        default:
                            fprintf(stderr, "AmmAsm: Unknown escape sequence on line %d\n", line);
                            exit(1);
                    }
                    buff += 2;
                }
                else tmp = *buff;
                buff++;
                

                if (*buff != '\'') {
                    fprintf(stderr, "AmmAsm: Unterminated character literal on line %d\n", line);
                    exit(1);
                }
                /*else */buff++; 
                char tmp2[2] = {tmp, '\0'};
                add_token(T_CHAR, tmp2, line);
                continue;
            }
            else if(*buff == '0'){
                printf("START!\n");
                char tmp[72];
                long long c = 0;
                buff++; 
                
                if(*buff == 'x' || *buff == 'X'){ // hex
                    buff++;
                    while (isin(DIGEXT, *buff)) {
                        if (*buff >= '0' && *buff <= '9') 
                            c = c * 16 + (*buff - '0');
                        else if (*buff >= 'a' && *buff <= 'f') 
                            c = c * 16 + (*buff - 'a' + 10);
                        else if (*buff >= 'A' && *buff <= 'F') 
                            c = c * 16 + (*buff - 'A' + 10);
                        else break;
                        buff++; 
                    }
                    snprintf(tmp, sizeof(tmp), "%lld", c); 
                    add_token(T_INT, tmp, line);
                }
                else if(*buff == 'b'){ // binary
                    buff++; 
                    while(isin(DIGBIN, *buff))
                        c = c * 2 + (*buff++ - '0');
                    snprintf(tmp, sizeof(tmp), "%lld", c);
                    add_token(T_INT, tmp, line);
                }
                else if (*buff == 'o') { // octal
                    buff++; 
                    while (isin(DIGOCT, *buff)) 
                        c = c * 8 + (*buff++ - '0');
                    snprintf(tmp, sizeof(tmp), "%lld", c);
                    add_token(T_INT, tmp, line);
                }
                else {
                    add_token(T_INT, "0", line);
                    buff--; 
                }
                continue;
            }
            else if (*buff == '[') {
                buff++; 

                char expr_buf[128] = {0};
                int i = 0;

                while (*buff && *buff != ']') {
                    if (i < (int)(sizeof(expr_buf) - 1)) {
                        expr_buf[i++] = *buff++;
                    }
                }

                if (*buff == ']') buff++;
                else {fprintf(stderr, "AmmAsm: you fogot to close '[' at line %d", line); exit(1);;};;
                expr_buf[i] = '\0';


                char clean_expr[128] = {0};
                int k = 0;
                for (int j = 0; expr_buf[j]; j++) {
                    if (!isspace(expr_buf[j])) {
                        clean_expr[k++] = expr_buf[j];
                    }
               }
                
                add_token(T_ADDR_EXPR, clean_expr, line);
                continue;
            }
            else if (*buff == '!') {
                buff++; 
                if (strncasecmp(buff, "section", 7) == 0) {
                    buff += 7;
                    while (*buff == ' ' || *buff == '\t') buff++;

                    char section[12] = {0};
                    int i = 0;

                    while (*buff && *buff != '\n' && i < (int)(sizeof(section) - 1)) {
                        section[i++] = *buff++;
                    }

                    add_token(T_SEC, section, line);
                }
                continue;
            }
            else if(*buff == ','){
                buff++;
                while(isspace(*buff)) buff++;
                add_token(T_COMMA, ",", line);
                continue;
            } 
            else if(isin(LET, *buff)){
                char buffer[56] = {0};
                int i=0;
                while (isin(LET, *buff) && i < (int)(sizeof(buffer)-1)){
                    buffer[i] = *buff++;
                }
                if(strcasecmp(buffer, HUMAN_AST[0]) == 0) add_token(T_BYTE, buffer, line);
                else if(strcasecmp(buffer, HUMAN_AST[1]) == 0) add_token(T_BYTEPTR, buffer, line);
                else if(strcasecmp(buffer, HUMAN_AST[2]) == 0) add_token(T_QWORD, buffer, line);
                else if(strcasecmp(buffer, HUMAN_AST[3]) == 0) add_token(T_QWORDPTR, buffer, line);
                else if(strcasecmp(buffer, HUMAN_AST[4]) == 0) add_token(T_DWORD, buffer, line);
                else if(strcasecmp(buffer, HUMAN_AST[5]) == 0) add_token(T_DWORDPTR, buffer, line);
                else if(strcasecmp(buffer, HUMAN_AST[6]) == 0) add_token(T_LBYTE, buffer, line);
                else if(strcasecmp(buffer, HUMAN_AST[7]) == 0) add_token(T_LBYTEPTR, buffer, line);
                else add_token(T_ID, buffer, line);
                continue;             
            }
            else if (strncasecmp(buff, "resb", 4) == 0){ add_token(T_RESB, buff, line); buff += 4; while (isspace(*buff)) buff++; continue;}
            else if (strncasecmp(buff, "resq", 4) == 0){ add_token(T_RESQ, buff, line); buff += 4; while (isspace(*buff)) buff++; continue;}
            else if (strncasecmp(buff, "resd", 4) == 0){ add_token(T_RESD, buff, line); buff += 4; while (isspace(*buff)) buff++; continue;}
            else if (strncasecmp(buff, "resl", 4) == 0){ add_token(T_RESL, buff, line); buff += 4; while (isspace(*buff)) buff++; continue;}
                
            else{ 
                fprintf(stderr, "AmmAsm: unvalid syntax or char on line\n", line); 
                exit(1); 
            }
            buff++;
        }

        add_token(T_EOL, "", line);
    }
    add_token(T_EOF, "ELF", line); // LOL
    return line;
}


typedef enum {
    AST_INS,    
    AST_U8,
    AST_U8PTR,
    AST_U16,
    AST_U16PTR,
    AST_U32,
    AST_U32PTR,
    AST_U64,
    AST_U64PTR,
    AST_RESB,
    AST_RESQ,
    AST_RESD,
    AST_RESL,
    AST_LABEL,   
    AST_COMMENT,
    AST_SECTION,
    AST_COMMA,
    AST_ADDR_EXPR,
    AST_UNKNOWN
} ASTType;

typedef enum {
    O_NONE = 2,
    O_REG8,
    O_REG16,
    O_REG32,
    O_REG64,
    O_IMM,
    O_MEM,
    O_LABEL
} OperandType;


typedef struct AST {
    ASTType type;
    char cmd[256];  // mostly useing for ins

    union {     
        struct {uint8_t operands[4][35]; OperandType otype[4]; int oper_count; } ins;
        struct { unsigned char data[256]; int size; } u8;
        struct { unsigned char *data[256]; int size; } u8ptr;
        struct { unsigned char data[256]; int size; } u16;
        struct { unsigned char *data[256]; int size; } u16ptr;
        struct { unsigned char data[256]; int size; } u32;
        struct { unsigned char *data[256]; int size; } u32ptr;
        struct { unsigned char data[256]; int size; } u64;
        struct { unsigned char *data[256]; int size; } u64ptr;
        struct { long size; } resb;
        struct { long size; } resq;
        struct { long size; } resd;
        struct { long size; } resl;
        struct { char name[64]; long adress; } label; // 64 bit adress
        struct { char name[63]; } section;
    };
        uint8_t machine_code[256];
        int machine_code_size;
} AST;

AST ast[MAX_TOKENS];
int ast_count = 0;


void DEBUG_PRINT_AST() {
    printf("=== AST DUMP (%d nodes) ===\n", ast_count);
    for (int i = 0; i < ast_count; i++) {
        AST* node = &ast[i];
        const char* type_str = "UNKNOWN";
        
        switch(node->type) {
            case AST_INS: type_str = "AST_INS"; break;
            case AST_U8: type_str = "AST_U8"; break;
            case AST_U8PTR: type_str = "AST_U8PTR"; break;
            case AST_U16: type_str = "AST_U16"; break;
            case AST_U16PTR: type_str = "AST_U16PTR"; break;
            case AST_U32: type_str = "AST_U32"; break;
            case AST_U32PTR: type_str = "AST_U32PTR"; break;
            case AST_U64: type_str = "AST_U64"; break;
            case AST_U64PTR: type_str = "AST_U64PTR"; break;
            case AST_RESB: type_str = "AST_RESB"; break;
            case AST_RESQ: type_str = "AST_RESQ"; break;
            case AST_RESD: type_str = "AST_RESD"; break;
            case AST_RESL: type_str = "AST_RESL"; break;
            case AST_LABEL: type_str = "AST_LABEL"; break;
            case AST_SECTION: type_str = "AST_SECTION"; break;
            case AST_ADDR_EXPR: type_str = "AST_ADDR_EXPR"; break;
        }
        
        printf("AST[%d]: type=%-15s", i, type_str);
        
        switch(node->type) {
            case AST_INS:
                printf(" cmd='%s' operands=%d", node->cmd, node->ins.oper_count);
                for (int j = 0; j < node->ins.oper_count; j++) {
                    const char* otype_str = "?";
                    switch(node->ins.otype[j]) {
                        case O_NONE: otype_str = "NONE"; break;
                        case O_REG8: otype_str = "REG8"; break;
                        case O_REG16: otype_str = "REG16"; break;
                        case O_REG32: otype_str = "REG32"; break;
                        case O_REG64: otype_str = "REG64"; break;
                        case O_IMM: otype_str = "IMM"; break;
                        case O_MEM: otype_str = "MEM"; break;
                        case O_LABEL: otype_str = "LABEL"; break;
                    }
                    printf(" [%s:'%s']", otype_str, node->ins.operands[j]);
                }
                break;
                
            case AST_LABEL:
                printf(" name='%s' addr=0x%lx", node->label.name, node->label.adress);
                break;
                
            case AST_U8:
                printf(" size=%d data=[", node->u8.size);
                for (int j = 0; j < node->u8.size; j++) {
                    printf("%02x", node->u8.data[j]);
                    if (j < node->u8.size - 1) printf(" ");
                }
                printf("]");
                break;
                
            case AST_U16:
                printf(" size=%d data=[", node->u16.size);
                for (int j = 0; j < node->u16.size; j++) {
                    printf("%04x", node->u16.data[j]);
                    if (j < node->u16.size - 1) printf(" ");
                }
                printf("]");
                break;
                
            case AST_U32:
                printf(" size=%d data=[", node->u32.size);
                for (int j = 0; j < node->u32.size; j++) {
                    printf("%08x", node->u32.data[j]);
                    if (j < node->u32.size - 1) printf(" ");
                }
                printf("]");
                break;
                
            case AST_U64:
                printf(" size=%d data=[", node->u64.size);
                for (int j = 0; j < node->u64.size; j++) {
                    printf("%016llx", node->u64.data[j]);
                    if (j < node->u64.size - 1) printf(" ");
                }
                printf("]");
                break;
                
            case AST_RESB:
                printf(" size=%ld bytes", node->resb.size);
                break;
                
            case AST_RESQ:
                printf(" size=%ld qwords", node->resq.size);
                break;
                
            case AST_RESD:
                printf(" size=%ld dwords", node->resd.size);
                break;
                
            case AST_RESL:
                printf(" size=%ld lbytes", node->resl.size);
                break;
                
            case AST_SECTION:
                printf(" name='%s'", node->section.name);
                break;
        }
        
        if (node->machine_code_size > 0) {
            printf(" machine_code=[");
            for (int j = 0; j < node->machine_code_size; j++) {
                printf("%02x", node->machine_code[j]);
                if (j < node->machine_code_size - 1) printf(" ");
            }
            printf("]");
        }
        
        printf("\n");
    }
    printf("=== END AST ===\n\n");
}


// parser:
AST* PARSE(){
    int pos = 0;
    uint64_t pc = 0;

    while(pos < toks_count && toks[pos].type != T_EOF){
        Token *tok = &toks[pos];
        AST node = { 0 };
        if (tok->type == T_INS) {
            node.type = AST_INS;
            node.ins.oper_count = 0;
            strncpy(node.cmd, toks[pos].value, sizeof node.cmd);
            pos++;

            while(pos < toks_count){
                if(toks[pos].type == T_COMMA){ pos++; continue;}
                else if(toks[pos].type == T_REG8 || toks[pos].type == T_REG16 ||
                        toks[pos].type == T_REG32 || toks[pos].type == T_REG64){
                    int tt = toks[pos].type;
                    strncpy(node.ins.operands[node.ins.oper_count], toks[pos++].value, 36);
                    switch (tt) {
                        case T_REG8:  node.ins.otype[node.ins.oper_count++] = O_REG8;  break;
                        case T_REG16: node.ins.otype[node.ins.oper_count++] = O_REG16; break;
                        case T_REG32: node.ins.otype[node.ins.oper_count++] = O_REG32; break;
                        case T_REG64: node.ins.otype[node.ins.oper_count++] = O_REG64; break;
                        default:      node.ins.otype[node.ins.oper_count++] = O_NONE;  break;
                    }
                }
                else if(toks[pos].type == T_ADDR_EXPR){
                    strncpy(node.ins.operands[node.ins.oper_count], toks[pos].value, 35);
                    pos++;
                    node.ins.otype[node.ins.oper_count++] = O_MEM;
                    continue;
                }
                else if(toks[pos].type == T_LAB){
                    strncpy(node.ins.operands[node.ins.oper_count], toks[pos].value, 35);
                    pos++;
                    node.ins.otype[node.ins.oper_count++] = O_LABEL;
                    continue;
                }
                else if(toks[pos].type == T_INT){
                    strncpy(node.ins.operands[node.ins.oper_count], toks[pos].value, 35);
                    pos++;
                    node.ins.otype[node.ins.oper_count++] = O_IMM;
                    continue;
                }
                else if(toks[pos].type == T_EOL || toks[pos].type == T_EOF){
                    break;
                }
                else {
                    fprintf(stderr, "AmmAsm: Unexpected token type %d at line %d\n", 
                            toks[pos].type, toks[pos].line);
                    pos++; 
                }
            }
            ast[ast_count++] = node;
            if (pos < toks_count && toks[pos].type == T_EOL) ++pos;
            continue;
        }

        if(tok->type == T_LAB){
            node.type = AST_LABEL;
            if(tok->type != T_EOF && tok->type != T_EOL){
                strncpy(node.label.name, toks[pos++].value, sizeof(node.label.name));
                node.label.adress = pc; // then we will add 0x40000 when I will add support of ELF64.. but I am toooo lazy
            }
            ast[ast_count++] = node;
            ++pos;
            continue;
        }
        if(tok->type == T_BYTE && (pos == 0 || toks[pos-1].type == T_ID)){   
            node.type = AST_U8;
            node.u8.size = 0;
            while(pos < toks_count){  
                if (toks[pos].type == T_EOL || toks[pos].type == T_EOF) break;
                if(toks[pos].type == T_COMMA){ pos++; continue;} // skip this shit
                if (node.u8.size >= 256) { fprintf(stderr, "AmmAsm: Too many bytes in string literal at line %d\n", toks[pos].line); exit(1); }
                if(toks[pos].type == T_CHAR){ 
                    node.u8.data[node.u8.size++] = (unsigned char)toks[pos].value[0];
                    ++pc;
                    pos++;
                }
                else if(toks[pos].type == T_INT){   // can be hex, octal, des see line 417
                    node.u8.data[node.u8.size++] = (unsigned char)atoi(toks[pos].value);
                    ++pc;
                    pos++;
                }
                else if(toks[pos].type == T_STR){
                    char *s = toks[pos].value;
                    while (*s) {
                    if (node.u8.size >= sizeof(node.u8.data)){ fprintf(stderr, "AmmAsm: String too long in U8 directive at line %d\n", toks[pos].line); exit(1);}
                        node.u8.data[node.u8.size++] = (unsigned char)*s++;
                        ++pc;
                    }
                    pos++;
                }
                else{
                    fprintf(stderr, "AmmAsm: Bytes are too cooked at line %d\n", toks[pos].line);
                    exit(1);
                }

            }
            ast[ast_count++] = node;
            if (pos < toks_count && toks[pos].type == T_EOL) ++pos;
            continue;
        }
        else if (tok->type == T_BYTEPTR && (pos == 0 || toks[pos-1].type == T_ID)) {
            node.type = AST_U8PTR;
            node.u8ptr.size = 0;
            pos++;

            while (pos < toks_count) {
                if (toks[pos].type == T_EOL || toks[pos].type == T_EOF) break;
                
                if (toks[pos].type == T_COMMA) { pos++; continue; }
                if (toks[pos].type == T_ID) {
                    if (node.u8ptr.size >= 256) {
                        fprintf(stderr, "AmmAsm: too many u8ptr (max. 256)\n");
                        exit(1);
                    }
                    node.u8ptr.data[node.u8ptr.size++] = (char *)toks[pos].value;
                    
                    pc += sizeof(char *);
                    pos++;
                }
                else if (toks[pos].type == T_INT && atoi(toks[pos].value) == 0) {
                    if (node.u8ptr.size >= 256) {
                        fprintf(stderr, "AmmAsm: too many u8ptr (max. 256)\n");
                        exit(1);
                    }
                    node.u8ptr.data[node.u8ptr.size++] = ((void*)0);
                    pc += sizeof(char *);
                    pos++;
                }
                else {
                    fprintf(stderr, "AmmAsm: expected label name (T_ID\\T_LAB) or 0 (NULL) in U8PTR at line %d\n", toks[pos].line);
                    exit(1);
                }
            }


            ast[ast_count++] = node;
            if(pos < toks_count && toks[pos].type == T_EOL) ++pos;
            continue;
        }
        else if(tok->type == T_QWORD && (pos == 0 || toks[pos-1].type == T_ID)){
            node.type = AST_U16;
            node.u16.size = 0;
            while(pos < toks_count){
                if (toks[pos].type == T_EOL || toks[pos].type == T_EOF) break;
                if(toks[pos].type == T_COMMA){ pos++; continue;} 
                if(node.u16.size >= 256) fprintf(stderr, "AmmAsm: to many u16x leterals at line \"%d\"", toks[pos].line);
                
                if(toks[pos].type == T_INT){
                    node.u16.data[node.u16.size++] = (short)atoi(toks[pos].value);
                    pc += sizeof(short);
                    ++pos;
                }
                else {
                    fprintf(stderr, "AmmAsm: unexpected token in u16 at line %d: '%s'\n", toks[pos].line, toks[pos].value);
                    exit(1);
                }
            }
            ast[ast_count++] = node;
            if(pos < toks_count && toks[pos].type == T_EOL) ++pos;
            continue;
        }
        else if(tok->type == T_QWORDPTR && (pos == 0 || toks[pos-1].type == T_ID)){
            node.type = AST_U16PTR;
            node.u16ptr.size = 0;
            ++pos; // skip cmd

            while (pos < toks_count){
                if(toks[pos].type == T_EOL || toks[pos].type == T_EOF)break;
                if(toks[pos].type == T_COMMA){ ++pos; continue; }
                if(node.u16ptr.size >= 256){ fprintf(stderr, "AmmAsm: Too many u16ptr at line \"%d\"", toks[pos].line); exit(1);}

                if(toks[pos].type == T_ID){
                    node.u16ptr.data[node.u16ptr.size++] = toks[pos].value;
                    pc += 8;
                    ++pos;
                }
                else if(toks[pos].type == T_INT && atoi(toks[pos].value) == 0){
                    node.u16ptr.data[node.u16ptr.size] = ((void*)0);
                    pc += 8;
                    pos++;
                }
                else {
                    fprintf(stderr, "AmmAsm: expected label name (T_ID) or 0 (NULL) in u16ptr at line %d\n", toks[pos].line);
                    exit(1);
                }
            }
            ast[ast_count++] = node;
            if(pos < toks_count && toks[pos].type == T_EOL) ++pos;
            continue;            
        }
        else if (tok->type == T_DWORD && (pos == 0 || toks[pos-1].type == T_ID)) {
            node.type = AST_U32;
            node.u32.size = 0;
            pos++; 

            while (pos < toks_count && toks[pos].type != T_EOL && toks[pos].type != T_EOF) {
                if (toks[pos].type == T_COMMA){ pos++; continue; }
                if (node.u32.size >= 256){ fprintf(stderr, "AmmAsm: Too many u32 at line %d\n", toks[pos].line); exit(1);}
                if (toks[pos].type == T_INT) {
                    node.u32.data[node.u32.size++] = atoi(toks[pos].value);
                    pc += sizeof(int);
                    pos++;
                } 
                else {
                    fprintf(stderr, "AmmAsm: unexpected token in u32 at line %d: '%s'\n", toks[pos].line, toks[pos].value);
                    exit(1);
                }
            }

            ast[ast_count++] = node;
            if (pos < toks_count && toks[pos].type == T_EOL) pos++;
        }
        else if(tok->type == T_DWORDPTR && (pos == 0 || toks[pos-1].type == T_ID)){
            node.type = AST_U32PTR;
            node.u32ptr.size = 0;
            ++pos; // skip cmd

            while (pos < toks_count){
                if(toks[pos].type == T_EOL || toks[pos].type == T_EOF)break;
                if(toks[pos].type == T_COMMA){ ++pos; continue; }
                if(node.u32ptr.size >= 256){ fprintf(stderr, "AmmAsm: Too many u32ptr at line \"%d\"", toks[pos].line); exit(1);}

                if(toks[pos].type == T_ID){
                    node.u32ptr.data[node.u32ptr.size++] = toks[pos].value;
                    pc += 8;
                    ++pos;
                }
                else if(toks[pos].type == T_INT && atoi(toks[pos].value) == 0){
                    node.u32ptr.data[node.u32ptr.size] = ((void*)0);
                    pc += 8;
                    pos++;
                }
                else {
                    fprintf(stderr, "AmmAsm: expected label name (T_ID) or 0 (NULL) in u32ptr at line %d\n", toks[pos].line);
                    exit(1);
                }
            }
            ast[ast_count++] = node;
            if(pos < toks_count && toks[pos].type == T_EOL) ++pos;
            continue;            
        }
        else if (tok->type == T_LBYTE && (pos == 0 || toks[pos-1].type == T_ID)) {
            node.type = AST_U64;
            node.u64.size = 0;
            pos++; 

            while (pos < toks_count && toks[pos].type != T_EOL && toks[pos].type != T_EOF) {
                if (toks[pos].type == T_COMMA){ pos++; continue; }
                if (node.u64.size >= 256){ fprintf(stderr, "AmmAsm: Too many u64 at line %d\n", toks[pos].line); exit(1);}
                if (toks[pos].type == T_INT) {
                    node.u64.data[node.u64.size++] = atoi(toks[pos].value);
                    pc += sizeof(long);
                    pos++;
                } 
                else {
                    fprintf(stderr, "AmmAsm: unexpected token in u64 at line %d: '%s'\n", toks[pos].line, toks[pos].value);
                    exit(1);
                }
            }

            ast[ast_count++] = node;
            if (pos < toks_count && toks[pos].type == T_EOL) pos++;
        }
        else if(tok->type == T_LBYTEPTR && (pos == 0 || toks[pos-1].type == T_ID)){
            node.type = AST_U64PTR;
            node.u64ptr.size = 0;
            ++pos; // skip cmd

            while (pos < toks_count){
                if(toks[pos].type == T_EOL || toks[pos].type == T_EOF)break;
                if(toks[pos].type == T_COMMA){ ++pos; continue; }
                if(node.u64ptr.size >= 256){ fprintf(stderr, "AmmAsm: Too many u64ptr at line \"%d\"", toks[pos].line); exit(1);}

                if(toks[pos].type == T_ID){
                    node.u64ptr.data[node.u64ptr.size++] = toks[pos].value;
                    pc += 8;
                    ++pos;
                }
                else if(toks[pos].type == T_INT && atoi(toks[pos].value) == 0){
                    node.u64ptr.data[node.u64ptr.size] = ((void*)0);
                    pc += 8;
                    pos++;
                }
                else {
                    fprintf(stderr, "AmmAsm: expected label name (T_ID) or 0 (NULL) in u64ptr at line %d\n", toks[pos].line);
                    exit(1);
                }
            }
            ast[ast_count++] = node;
            if(pos < toks_count && toks[pos].type == T_EOL) ++pos;
            continue;            
        }
        // bss
        else if(tok->type == T_RESB || tok->type == T_RESQ || tok->type == T_RESD || tok->type == T_RESL){
            Token *pp = tok;
            if(toks[(pos-1)].type != T_ID || toks[(pos+1)].type != T_INT){ 
                fprintf(stderr, "AmmAsm: syntax erorr. expected identifier before \"%s\" and decimal number after\n", 
                    (pp->type == T_RESB) ? "resb" : 
                    (pp->type == T_RESQ) ? "resq" : 
                    (pp->type == T_RESD) ? "resd" : 
                    (pp->type == T_RESL) ? "resl" : "?");                    
                exit(1);    
            }

            switch (tok->type){
            case T_RESB: node.type = AST_RESB; node.resb.size = (long)eval_expr(toks[pos].value); break;
            case T_RESQ: node.type = AST_RESQ; node.resq.size = (long)eval_expr(toks[pos].value); break;
            case T_RESD: node.type = AST_RESD; node.resd.size = (long)eval_expr(toks[pos].value); break;
            case T_RESL: node.type = AST_RESL; node.resl.size = (long)eval_expr(toks[pos].value); break;
            default: break; // well .. this will never be executed
            }
            pos += 1;

            ast[ast_count++] = node;
            if(pos < toks_count && toks[pos].type == T_EOL) ++pos;
            continue;              
        }
        if (pos < toks_count && toks[pos].type != T_EOF) {
            pos++;
        }
    }
    return ast; // for debug
}
// How cpu see regs
int indexof(const char** regs, char* reg){

    if(!is2arrin(regs, reg)) return -1;

    return 
    (!strcasecmp(reg, "rax") || !strcasecmp(reg, "r8") || !strcasecmp(reg, "eax") || !strcasecmp(reg, "ax") || !strcasecmp(reg, "al") || !strcasecmp(reg, "r8d") || !strcasecmp(reg, "r8w") || !strcasecmp(reg, "r8b"))      ? 0b000 :
    (!strcasecmp(reg, "rcx") || !strcasecmp(reg, "r9") || !strcasecmp(reg, "ecx") || !strcasecmp(reg, "cx") || !strcasecmp(reg, "cl") || !strcasecmp(reg, "r9d") || !strcasecmp(reg, "r9w") || !strcasecmp(reg, "r9b"))      ? 0b001 :
    (!strcasecmp(reg, "rdx") || !strcasecmp(reg, "r10") || !strcasecmp(reg, "edx") || !strcasecmp(reg, "dx") || !strcasecmp(reg, "dl") || !strcasecmp(reg, "r10d") || !strcasecmp(reg, "r10w") || !strcasecmp(reg, "r10b"))  ? 0b010 :
    (!strcasecmp(reg, "rbx") || !strcasecmp(reg, "r11") || !strcasecmp(reg, "ebx") || !strcasecmp(reg, "bx") || !strcasecmp(reg, "bl") || !strcasecmp(reg, "r11d") || !strcasecmp(reg, "r11w") || !strcasecmp(reg, "r11b"))  ? 0b011 :
    (!strcasecmp(reg, "rsp") || !strcasecmp(reg, "r12") || !strcasecmp(reg, "esp") || !strcasecmp(reg, "sp") || !strcasecmp(reg, "spl") || !strcasecmp(reg, "r12d") || !strcasecmp(reg, "r12w") || !strcasecmp(reg, "r12b")) ? 0b100 :
    (!strcasecmp(reg, "rbp") || !strcasecmp(reg, "r13") || !strcasecmp(reg, "ebp") || !strcasecmp(reg, "bp") || !strcasecmp(reg, "bpl") || !strcasecmp(reg, "r13d") || !strcasecmp(reg, "r13w") || !strcasecmp(reg, "r13b")) ? 0b101 :
    (!strcasecmp(reg, "rsi") || !strcasecmp(reg, "r14") || !strcasecmp(reg, "esi") || !strcasecmp(reg, "si") || !strcasecmp(reg, "sil") || !strcasecmp(reg, "r14d") || !strcasecmp(reg, "r14w") || !strcasecmp(reg, "r14b")) ? 0b110 :
    (!strcasecmp(reg, "rdi") || !strcasecmp(reg, "r15") || !strcasecmp(reg, "edi") || !strcasecmp(reg, "di") || !strcasecmp(reg, "dil") || !strcasecmp(reg, "r15d") || !strcasecmp(reg, "r15w") || !strcasecmp(reg, "r15b")) ? 0b111 : -1;
}

// For human readable
int find_reg64_index(const char* reg) {
    for (int i=0; i<8; i++) {
        if (strcasecmp(regs64[i], reg) == 0) return i;
    }
    for (int i=0; i<8; i++) {
        if (strcasecmp(regs64GP[i], reg) == 0) return i + 8;
    }
    return -1;
}
int find_reg32_index(const char* reg) {
    for (int i=0; i<8; i++) {
        if (strcasecmp(regs32[i], reg) == 0) return i;
    }
    for (int i=0; i<8; i++) {
        if (strcasecmp(regs32GP[i], reg) == 0) return i + 8;
    }
    return -1;
}
int find_reg16_index(const char* reg) {
    for (int i=0; i<8; i++) {
        if (strcasecmp(regs16[i], reg) == 0) return i;
    }
    for (int i=0; i<8; i++) {
        if (strcasecmp(regs16GP[i], reg) == 0) return i + 8;
    }
    return -1;
}
int find_reg8_index(const char* reg) {
    for (int i=0; i<8; i++) {
        if (strcasecmp(regs8[i], reg) == 0) return i;
    }
    for (int i=0; i<8; i++) {
        if (strcasecmp(regs8GP[i], reg) == 0) return i + 8;
    }
    return -1;
}

/*
===============================================================================
Address Expression Parsing Rules — AmmAsm2 (x86-64)
===============================================================================

Unlike NASM/MASM, AmmAsm2 uses a *key-based format* for clarity and predictable parsing.  
No brackets, no “pretty” syntax — everything is explicitly labeled:
we parse expressions like:

    b=rbx, i=rcx, s=4, d=0x10

instead of NASM-like `[rbx + rcx*4 + 0x10]`.

This makes parsing trivial, avoids ambiguity, and still maps 1:1
to actual x86-64 SIB/modRM encoding logic.

===============================================================================
Allowed Keys:
===============================================================================

    b=REG or b=ADDR      → base register (e.g. rax) or numeric address (0xB8000)
    i=REG                → index register (e.g.rcx)
    s=NUM                → scale (1, 2, 4, 8 only)
    d=NUM or HEX or BIN  → displacement (e.g. 16, 0x10, 0b1010)

Each key is optional, but you must specify at least one of:
    base (b), index (i), or displacement (d)

===============================================================================
Examples (valid):
===============================================================================

    b=rax, d=32             ; [rax + 32]
    b=rbx, i=rcx, s=4       ; [rbx + rcx*4]
    b=r8, i=r9, s=8, d=0x10 ; [r8 + r9*8 + 0x10]
    d=0xB8000               ; [0xB8000]
    i=rbx, s=2              ; [rbx*2]

===============================================================================
Invalid (rejected at parse time):
===============================================================================

    s=2                ; scale without index
    i=rsp              ; rsp cannot be used as index
    s=3                ; invalid scale (must be 1,2,4,8)
    (empty)            ; nothing specified
    b=r14abcdef        ; invalid register name

===============================================================================
Parsing Algorithm
===============================================================================

1) Split expression by commas → read tokens like `b=rax`, `i=rcx`, `s=4`, `d=0x10`
2) For each token:
      - skip spaces
      - identify the key (b, i, s, d)
      - parse right-hand side:
            * registers: stored as strings (`rax`)
            * numbers: auto-detected base (0x, 0b, 0o, decimal)
3) Validate:
      - if `scale` exists → `index` must exist
      - if `index == rsp` → error
      - if `scale` not in {1,2,4,8} → error
      - if nothing set → error
4) Store result in `AddrExpr`:
      base_reg, index_reg, disp_val, scale, base_is_reg, index_exists

===============================================================================
Design Purpose
===============================================================================

No ambiguity — each field explicit (`b=`, `i=`, `s=`, `d=`)  
Easy to parse using simple `if(*p == 'b')` etc.  
Simple validation without complex expression grammar  
Maps directly to SIB/modRM logic for encoder  
Perfect for JIT/compilers — minimal string parsing

===============================================================================
Example:
Input:
    "b=rbx, i=rcx, s=4, d=0x10"

Output:
    AddrExpr {
        base_reg = "rbx",
        base_is_reg = 1,
        index_reg = "rcx",
        index_exists = 1,
        disp_val = 0x10,
        scale = 4
    }

===============================================================================
*/


#define SKIP_SP while(isspace(*p)) p++;

typedef struct {
// mov [rbx + rcx * 8 + 0x10], rax 

    // base reg conf
    int base_is_reg;
    long base_val;
    char base_reg[8];

    // index conf
    int index_exists;
    long disp_val;  // if index != reg  
    char index_reg[8];

    int scale;       // 1, 2, 4, 8 (x86_64)
} AddrExpr;


// this function will return 1 fully parsed addexpr for sib_gen_byte() 
// parsing only GPR registers
AddrExpr parse_addr_expr(const char* expr , int line) {
    AddrExpr new = { 0 };
    new.scale = 1; // base value
    new.disp_val = 0; // base value
    const char *p = expr;
    uint8_t find_b = 0, find_i = 0, find_s = 0, find_d = 0;

    while (*p) {
        SKIP_SP

        if (*p == 'b') {
            find_b = 1;
            p++; SKIP_SP
            if (*p == '=') {
                p++; SKIP_SP
                int i = 0;
                while (*p && *p != ',' && !isspace(*p)) {
                    new.base_reg[i++] = *p++;
                }
                new.base_reg[i] = '\0';
                new.base_is_reg = 1;
            }
        }
        else if (*p == 'i') {
            find_i = 1;
            p++; SKIP_SP
            if (*p == '=') {
                p++; SKIP_SP
                int i = 0;
                while (*p && *p != ',' && !isspace(*p)) {
                    new.index_reg[i++] = *p++;
                }
                new.index_reg[i] = '\0';
                new.index_exists = 1;
            }
        }
        else if (*p == 's') {
            find_s = 1;
            p++; SKIP_SP
            if (*p == '=') {
                p++; SKIP_SP
                new.scale = atoi(p);
                while (*p && isdigit(*p)) p++;
            }
        }
        else if (*p == 'd') {
            find_d = 1;
            p++; SKIP_SP
            if (*p == '=') {
                p++; SKIP_SP
                if (*p == '0' && *(p+1) == 'x') {
                    new.disp_val = strtol(p, (char**)&p, 16);
                }
                else if (*p == '0' && *(p+1) == 'o') {
                    new.disp_val = strtol(p+2, (char**)&p, 8);
                }
                else if (*p == '0' && *(p+1) == 'b') {
                    new.disp_val = strtol(p+2, (char**)&p, 2);
                }
                else {
                    new.disp_val = strtol(p, (char**)&p, 10);
                }
            }
        }

        ;p++;
    }

    if (new.scale && !new.index_exists) {
        fprintf(stderr, "AmmAsm: Line %d: scale used without index register\n", line);
        exit(1);
    }

    if (new.index_exists && strcasecmp(new.index_reg, "rsp") == 0) {
        fprintf(stderr, "AmmAsm: Line %d: rsp cannot be used as index register\n", line);
        exit(1);
    }

    if (new.scale && !(new.scale == 1 || new.scale == 2 || new.scale == 4 || new.scale == 8)) {
        fprintf(stderr, "AmmAsm: Line %d: invalid scale factor %d (must be 1,2,4,8)\n", line, new.scale);
        exit(1);
    }

    if (!new.base_is_reg && !new.index_exists && !find_d) {
        fprintf(stderr, "AmmAsm: Line %d: addressing mode must have at least base, index or displacement\n", line);
        exit(1);
    }

    if (new.base_is_reg && strlen(new.base_reg) > 4) {
        fprintf(stderr, "AmmAsm: Line %d: invalid base register name '%s'\n", line, new.base_reg);
        exit(1);
    }

    if (new.index_exists && strlen(new.index_reg) > 4) {
        fprintf(stderr, "AmmAsm: Line %d: invalid index register name '%s'\n", line, new.index_reg);
        exit(1);
    }


    return new;
}


uint8_t gen_SIB_byte(AddrExpr input) {
    uint8_t scale_bits = 0b00;
    uint8_t index_bits = 4; // 0b100 = no index(rsp/r12)
    uint8_t base_bits  = 5; // 0b101 = disp32 only

    // 1. Scale
    switch (input.scale) {
        case 1: scale_bits = 0b00; break;
        case 2: scale_bits = 0b01; break;
        case 4: scale_bits = 0b10; break;
        case 8: scale_bits = 0b11; break;
    }

    // 2. Index
    if (input.index_exists) {
        index_bits = find_reg64_index(input.index_reg);

    } 

    // 3. Base
    if (input.base_is_reg) {
        base_bits = find_reg64_index(input.base_reg);
    } 
    else if (input.base_val) {
        base_bits = 5; // disp32 only
    }

    // sib byte
    return (scale_bits << 6) | (index_bits << 3) | base_bits;
}

#define REX_BASE  0x40
#define REX_W     0b00001000
#define REX_R     0b00000100
#define REX_X     0b00000010
#define REX_B     0b00000001

void parseInst(AST* node, int* pc, int line) {
    if (node->type != AST_INS) return;

    memset(node->machine_code, 0, sizeof(node->machine_code));
    int* s = &node->machine_code_size;
    *s = 0;

    // ============================================================================
    // |                "MOV" INSTRUCTION ENCODING REFERENCE                      |
    // ============================================================================
    // 
    // REX prefix (0x40-0x4F):
    //   REX.W (bit 3) = 1 for 64-bit operands
    //   REX.R (bit 2) = 1 if reg field uses r8-r15
    //   REX.X (bit 1) = 1 if SIB index uses r8-r15  
    //   REX.B (bit 0) = 1 if r/m or SIB base uses r8-r15
    //
    // ModR/M byte format: [mod:2bit][reg:3bit][r/m:3bit]
    //   mod = 00: [base] (no disp, except RBP/R13 needs disp8)
    //   mod = 01: [base + disp8]
    //   mod = 10: [base + disp32]
    //   mod = 11: register direct (no memory)
    //
    // SIB byte format: [scale:2bit][index:3bit][base:3bit]
    //   scale: 00=*1, 01=*2, 10=*4, 11=*8
    //
    // Special cases:
    //   - RBP/R13 (r/m=101): ALWAYS needs displacement (min disp8)
    //   - RSP/R12 (r/m=100): ALWAYS needs SIB byte
    //   - No imm64 to memory (use reg as intermediate)
    //
    // ============================================================================

    if (strcasecmp(node->cmd, "mov") == 0) {
    
        // ========================================================================
        // 1. MOV REG, IMM (reg ← const)
        // ========================================================================
        
        // MOV R64, IMM64
        if ((is2arrin(regs64, node->ins.operands[0]) || is2arrin(regs64GP, node->ins.operands[0])) 
            && node->ins.otype[1] == O_IMM) {

            uint8_t rex = REX_BASE | REX_W; // 0x48 or 0x49
            uint8_t opcode = 0xB8;
            long imm64 = (long)atoll(node->ins.operands[1]);
            int reg_num = -1;

            if ((reg_num = indexof(regs64, node->ins.operands[0])) != -1) {
                // rax..rdi: rex = 0x48
            } 
            else if ((reg_num = indexof(regs64GP, node->ins.operands[0])) != -1) {
                rex |= REX_B; // r8..r15: rex = 0x49
            } 
            else return;

            opcode += reg_num;

            node->machine_code[0] = rex;
            node->machine_code[1] = opcode;
            As64((uint64_t)imm64, &node->machine_code[2]);

            *s = 10;
            *pc += 10;
            return;
        }

        // MOV R32, IMM32
        else if ((is2arrin(regs32, node->ins.operands[0]) || is2arrin(regs32GP, node->ins.operands[0])) 
                && node->ins.otype[1] == O_IMM) {

            uint8_t rex = 0;
            uint32_t imm32 = (uint32_t)atoi(node->ins.operands[1]);
            uint8_t opcode = 0xB8;
            int reg_num = -1;

            if ((reg_num = indexof(regs32, node->ins.operands[0])) != -1) {
                // eax..edi: no REX
            }
            else if ((reg_num = indexof(regs32GP, node->ins.operands[0])) != -1) {
                rex = REX_BASE | REX_B; // r8d..r15d
            }
            else return;

            opcode += reg_num;

            int pos = 0;
            if (rex) node->machine_code[pos++] = rex;
            node->machine_code[pos++] = opcode;
            As32((uint32_t)imm32, &node->machine_code[pos]);
            pos += 4;

            *s = pos;
            *pc += pos;
            return;
        }

        // MOV R16, IMM16
        else if ((is2arrin(regs16, node->ins.operands[0]) || is2arrin(regs16GP, node->ins.operands[0])) 
                && node->ins.otype[1] == O_IMM) {

            uint8_t rex = 0;
            uint16_t imm16 = (uint16_t)atoi(node->ins.operands[1]);
            uint8_t opcode = 0xB8;
            int reg_num = -1;

            if ((reg_num = indexof(regs16, node->ins.operands[0])) != -1) {
                // ax..di: no REX
            }
            else if ((reg_num = indexof(regs16GP, node->ins.operands[0])) != -1) {
                rex = REX_BASE | REX_B; // r8w..r15w
            }
            else return;

            opcode += reg_num;

            int pos = 0;
            node->machine_code[pos++] = 0x66; // 16-bit prefix
            if (rex) node->machine_code[pos++] = rex;
            node->machine_code[pos++] = opcode;
            As16((uint16_t)imm16, &node->machine_code[pos]);
            pos += 2;

            *s = pos;
            *pc += pos;
            return;
        }

        // MOV R8, IMM8
        else if ((is2arrin(regs8, node->ins.operands[0]) || is2arrin(regs8GP, node->ins.operands[0])) 
                && node->ins.otype[1] == O_IMM) {

            uint8_t rex = 0;
            uint8_t imm8 = (uint8_t)atoi(node->ins.operands[1]);
            uint8_t opcode = 0xB0;
            int reg_num = -1;

            if ((reg_num = indexof(regs8, node->ins.operands[0])) != -1) {
                // al..dil: no REX (or REX if using sil/dil/bpl/spl)
            }
            else if ((reg_num = indexof(regs8GP, node->ins.operands[0])) != -1) {
                rex = REX_BASE | REX_B; // r8b..r15b
            }
            else return;

            opcode += reg_num;

            int pos = 0;
            if (rex) node->machine_code[pos++] = rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = imm8;

            *s = pos;
            *pc += pos;
            return;
        }

        // ========================================================================
        // 2. MOV REG, REG (reg ← reg) (mod = 11)
        // ========================================================================

        // MOV R64, R64
        else if ((is2arrin(regs64, node->ins.operands[0]) || is2arrin(regs64GP, node->ins.operands[0])) &&
                (is2arrin(regs64, node->ins.operands[1]) || is2arrin(regs64GP, node->ins.operands[1]))) {

            uint8_t rex = REX_BASE | REX_W;
            uint8_t opcode = 0x89; // MOV r/m64, r64
            uint8_t modrm = 0b11000000;

            int dst_index = indexof(regs64, node->ins.operands[0]);
            if (dst_index == -1) {
                dst_index = indexof(regs64GP, node->ins.operands[0]);
                rex |= REX_B;
            }

            int src_index = indexof(regs64, node->ins.operands[1]);
            if (src_index == -1) {
                src_index = indexof(regs64GP, node->ins.operands[1]);
                rex |= REX_R;
            }

            modrm |= (src_index << 3) | dst_index;

            node->machine_code[0] = rex;
            node->machine_code[1] = opcode;
            node->machine_code[2] = modrm;

            *s = 3;
            *pc += 3;
            return;
        }

        // MOV R32, R32
        else if ((is2arrin(regs32, node->ins.operands[0]) || is2arrin(regs32GP, node->ins.operands[0])) &&
                (is2arrin(regs32, node->ins.operands[1]) || is2arrin(regs32GP, node->ins.operands[1]))) {

            uint8_t rex = 0;
            uint8_t opcode = 0x89;
            uint8_t modrm = 0b11000000;

            int dst_index = indexof(regs32, node->ins.operands[0]);
            if (dst_index == -1) {
                dst_index = indexof(regs32GP, node->ins.operands[0]);
                if (dst_index == -1) return;
                rex |= REX_B;
            }

            int src_index = indexof(regs32, node->ins.operands[1]);
            if (src_index == -1) {
                src_index = indexof(regs32GP, node->ins.operands[1]);
                if (src_index == -1) return;
                rex |= REX_R;
            }

            modrm |= ((src_index & 7) << 3) | (dst_index & 7);

            int pos = 0;
            if (rex) node->machine_code[pos++] = REX_BASE | rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            *s = pos;
            *pc += pos;
            return;
        }

        // MOV R16, R16
        else if ((is2arrin(regs16, node->ins.operands[0]) || is2arrin(regs16GP, node->ins.operands[0])) &&
                (is2arrin(regs16, node->ins.operands[1]) || is2arrin(regs16GP, node->ins.operands[1]))) {

            uint8_t rex = 0;
            uint8_t opcode = 0x89;
            uint8_t modrm = 0b11000000;

            int dst_index = indexof(regs16, node->ins.operands[0]);
            if (dst_index == -1) {
                dst_index = indexof(regs16GP, node->ins.operands[0]);
                if (dst_index == -1) return;
                rex |= REX_B;
            }

            int src_index = indexof(regs16, node->ins.operands[1]);
            if (src_index == -1) {
                src_index = indexof(regs16GP, node->ins.operands[1]);
                if (src_index == -1) return;
                rex |= REX_R;
            }

            modrm |= ((src_index & 7) << 3) | (dst_index & 7);

            int pos = 0;
            node->machine_code[pos++] = 0x66;
            if (rex) node->machine_code[pos++] = REX_BASE | rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            *s = pos;
            *pc += pos;
            return;
        }

        // MOV R8, R8
        else if ((is2arrin(regs8, node->ins.operands[0]) || is2arrin(regs8GP, node->ins.operands[0])) &&
                (is2arrin(regs8, node->ins.operands[1]) || is2arrin(regs8GP, node->ins.operands[1]))) {

            uint8_t rex = 0;
            uint8_t opcode = 0x88;
            uint8_t modrm = 0b11000000;

            int dst_index = indexof(regs8, node->ins.operands[0]);
            if (dst_index == -1) {
                dst_index = indexof(regs8GP, node->ins.operands[0]);
                if (dst_index == -1) return;
                rex |= REX_B;
            }

            int src_index = indexof(regs8, node->ins.operands[1]);
            if (src_index == -1) {
                src_index = indexof(regs8GP, node->ins.operands[1]);
                if (src_index == -1) return;
                rex |= REX_R;
            }

            modrm |= ((src_index & 7) << 3) | (dst_index & 7);

            int pos = 0;
            if (rex) node->machine_code[pos++] = REX_BASE | rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            *s = pos;
            *pc += pos;
            return;
        }

        // ========================================================================
        // 3. MOV REG, [ADDR] (mem ← reg)
        // ========================================================================

        // MOV R64, [ADDR]
        else if ((is2arrin(regs64, node->ins.operands[0]) || is2arrin(regs64GP, node->ins.operands[0])) &&
                node->ins.otype[1] == O_MEM) {

            AddrExpr parsed = parse_addr_expr(node->ins.operands[1], line);
            uint8_t rex = REX_BASE | REX_W;
            uint8_t opcode = 0x8B; // MOV r64, r/m64
            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;

            int reg_index = find_reg64_index(node->ins.operands[0]);
            if (reg_index >= 8) rex |= REX_R;

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;

            if (parsed.base_is_reg) {
                rm_bits = find_reg64_index(parsed.base_reg);
                if (rm_bits >= 8) rex |= REX_B;

                // Special case: RBP/R13
                if ((rm_bits & 7) == 5) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00;
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else {
                    mod_bits = 0b10;
                    disp_size = 4;
                }
            }
            else {
                // [imm32]
                mod_bits = 0b00;
                rm_bits = 0b101;
                disp_size = 4;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
                int idx = find_reg64_index(parsed.index_reg);
                if (idx >= 8) rex |= REX_X;
            }
            else if ((rm_bits & 7) == 0b100) {
                need_sib = 1;
            }
            else if (!parsed.base_is_reg) {
                need_sib = 1;
            }

            if (need_sib) {
                sib = gen_SIB_byte(parsed);
                rm_bits = 0b100;
            }

            modrm = (mod_bits << 6) | ((reg_index & 7) << 3) | (rm_bits & 7);

            if (rex != REX_BASE)
                node->machine_code[pos++] = rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            if (need_sib) node->machine_code[pos++] = sib;
            if (disp_size == 1)
                node->machine_code[pos++] = (uint8_t)parsed.disp_val;
            else if (disp_size == 4) {
                As32((uint32_t)parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }

            *s = pos;
            *pc += pos;
        }

        // MOV R32, [ADDR]
        else if ((is2arrin(regs32, node->ins.operands[0]) || is2arrin(regs32GP, node->ins.operands[0])) &&
                node->ins.otype[1] == O_MEM) {

            AddrExpr parsed = parse_addr_expr(node->ins.operands[1], line);
            uint8_t rex = 0;
            uint8_t opcode = 0x8B;
            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;

            int reg_index = find_reg32_index(node->ins.operands[0]);
            if (reg_index >= 8) rex |= REX_R;

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;

            if (parsed.base_is_reg) {
                rm_bits = find_reg32_index(parsed.base_reg);
                if (rm_bits >= 8) rex |= REX_B;

                if ((rm_bits & 7) == 5) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00;
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else {
                    mod_bits = 0b10;
                    disp_size = 4;
                }
            }
            else {
                mod_bits = 0b00;
                rm_bits = 0b101;
                disp_size = 4;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
                int idx = find_reg32_index(parsed.index_reg);
                if (idx >= 8) rex |= REX_X;
            }
            else if ((rm_bits & 7) == 0b100) {
                need_sib = 1;
            }
            else if (!parsed.base_is_reg) {
                need_sib = 1;
            }

            if (need_sib) {
                sib = gen_SIB_byte(parsed);
                rm_bits = 0b100;
            }

            modrm = (mod_bits << 6) | ((reg_index & 7) << 3) | (rm_bits & 7);

            if (rex != 0) node->machine_code[pos++] = REX_BASE | rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            if (need_sib) node->machine_code[pos++] = sib;
            if (disp_size == 1)
                node->machine_code[pos++] = (uint8_t)parsed.disp_val;
            else if (disp_size == 4) {
                As32((uint32_t)parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }

            *s = pos;
            *pc += pos;
        }

        // MOV R16, [ADDR]
        else if ((is2arrin(regs16, node->ins.operands[0]) || is2arrin(regs16GP, node->ins.operands[0])) &&
                node->ins.otype[1] == O_MEM) {

            AddrExpr parsed = parse_addr_expr(node->ins.operands[1], line);
            uint8_t rex = 0;
            uint8_t opcode = 0x8B;
            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;

            int reg_index = find_reg16_index(node->ins.operands[0]);
            if (reg_index >= 8) rex |= REX_R;

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;

            if (parsed.base_is_reg) {
                rm_bits = find_reg16_index(parsed.base_reg);
                if (rm_bits >= 8) rex |= REX_B;

                if ((rm_bits & 7) == 5) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00;
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else {
                    mod_bits = 0b10;
                    disp_size = 4;
                }
            }
            else {
                mod_bits = 0b00;
                rm_bits = 0b101;
                disp_size = 4;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
                int idx = find_reg16_index(parsed.index_reg);
                if (idx >= 8) rex |= REX_X;
            }
            else if ((rm_bits & 7) == 0b100) {
                need_sib = 1;
            }
            else if (!parsed.base_is_reg) {
                need_sib = 1;
            }

            if (need_sib) {
                sib = gen_SIB_byte(parsed);
                rm_bits = 0b100;
            }

            modrm = (mod_bits << 6) | ((reg_index & 7) << 3) | (rm_bits & 7);

            node->machine_code[pos++] = 0x66;
            if (rex != 0) node->machine_code[pos++] = REX_BASE | rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            if (need_sib) node->machine_code[pos++] = sib;
            if (disp_size == 1)
                node->machine_code[pos++] = (uint8_t)parsed.disp_val;
            else if (disp_size == 4) {
                As32((uint32_t)parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }

            *s = pos;
            *pc += pos;
        }

        // MOV R8, [ADDR]
        else if ((is2arrin(regs8, node->ins.operands[0]) || is2arrin(regs8GP, node->ins.operands[0])) &&
                node->ins.otype[1] == O_MEM) {

            AddrExpr parsed = parse_addr_expr(node->ins.operands[1], line);
            uint8_t rex = 0;
            uint8_t opcode = 0x8A;
            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;

            int reg_index = find_reg8_index(node->ins.operands[0]);
            if (reg_index >= 8) rex |= REX_R;

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;

            if (parsed.base_is_reg) {
                rm_bits = find_reg8_index(parsed.base_reg);
                if (rm_bits >= 8) rex |= REX_B;

                if ((rm_bits & 7) == 5) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00;
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else {
                    mod_bits = 0b10;
                    disp_size = 4;
                }
            }
            else {
                mod_bits = 0b00;
                rm_bits = 0b101;
                disp_size = 4;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
                int idx = find_reg8_index(parsed.index_reg);
                if (idx >= 8) rex |= REX_X;
            }
            else if ((rm_bits & 7) == 0b100) {
                need_sib = 1;
            }
            else if (!parsed.base_is_reg) {
                need_sib = 1;
            }

            if (need_sib) {
                sib = gen_SIB_byte(parsed);
                rm_bits = 0b100;
            }

            modrm = (mod_bits << 6) | ((reg_index & 7) << 3) | (rm_bits & 7);

            if (rex != 0) node->machine_code[pos++] = REX_BASE | rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            if (need_sib) node->machine_code[pos++] = sib;
            if (disp_size == 1)
                node->machine_code[pos++] = (uint8_t)parsed.disp_val;
            else if (disp_size == 4) {
                As32((uint32_t)parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }

            *s = pos;
            *pc += pos;
        }

        // ========================================================================
        // 4. MOV [ADDR], REG (mem ← reg)
        // ========================================================================

        // MOV [ADDR], R64
        else if ((is2arrin(regs64, node->ins.operands[1]) || is2arrin(regs64GP, node->ins.operands[1])) &&
                node->ins.otype[0] == O_MEM) {

            AddrExpr parsed = parse_addr_expr(node->ins.operands[0], line);
            uint8_t rex = REX_BASE | REX_W;
            uint8_t opcode = 0x89;
            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;

            int reg_index = find_reg64_index(node->ins.operands[1]);
            if (reg_index >= 8) rex |= REX_R;

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;

            if (parsed.base_is_reg) {
                rm_bits = find_reg64_index(parsed.base_reg);
                if (rm_bits >= 8) rex |= REX_B;

                if ((rm_bits & 7) == 5) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00;
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else {
                    mod_bits = 0b10;
                    disp_size = 4;
                }
            }
            else {
                mod_bits = 0b00;
                rm_bits = 0b101;
                disp_size = 4;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
                int idx = find_reg64_index(parsed.index_reg);
                if (idx >= 8) rex |= REX_X;
            }
            else if ((rm_bits & 7) == 0b100) {
                need_sib = 1;
            }
            else if (!parsed.base_is_reg) {
                need_sib = 1;
            }

            if (need_sib) {
                sib = gen_SIB_byte(parsed);
                rm_bits = 0b100;
            }

            modrm = (mod_bits << 6) | ((reg_index & 7) << 3) | (rm_bits & 7);

            if (rex != REX_BASE)
                node->machine_code[pos++] = rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            if (need_sib) node->machine_code[pos++] = sib;
            if (disp_size == 1)
                node->machine_code[pos++] = (uint8_t)parsed.disp_val;
            else if (disp_size == 4) {
                As32((uint32_t)parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }

            *s = pos;
            *pc += pos;
        }

        // MOV [ADDR], R32
        else if ((is2arrin(regs32, node->ins.operands[1]) || is2arrin(regs32GP, node->ins.operands[1])) &&
                node->ins.otype[0] == O_MEM) {

            AddrExpr parsed = parse_addr_expr(node->ins.operands[0], line);
            uint8_t rex = 0;
            uint8_t opcode = 0x89;
            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;

            int reg_index = find_reg32_index(node->ins.operands[1]);
            if (reg_index >= 8) rex |= REX_R;

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;

            if (parsed.base_is_reg) {
                rm_bits = find_reg32_index(parsed.base_reg);
                if (rm_bits >= 8) rex |= REX_B;

                if ((rm_bits & 7) == 5) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00;
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else {
                    mod_bits = 0b10;
                    disp_size = 4;
                }
            }
            else {
                mod_bits = 0b00;
                rm_bits = 0b101;
                disp_size = 4;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
                int idx = find_reg32_index(parsed.index_reg);
                if (idx >= 8) rex |= REX_X;
            }
            else if ((rm_bits & 7) == 0b100) {
                need_sib = 1;
            }
            else if (!parsed.base_is_reg) {
                need_sib = 1;
            }

            if (need_sib) {
                sib = gen_SIB_byte(parsed);
                rm_bits = 0b100;
            }

            modrm = (mod_bits << 6) | ((reg_index & 7) << 3) | (rm_bits & 7);

            if (rex != 0) node->machine_code[pos++] = REX_BASE | rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            if (need_sib) node->machine_code[pos++] = sib;
            if (disp_size == 1)
                node->machine_code[pos++] = (uint8_t)parsed.disp_val;
            else if (disp_size == 4) {
                As32((uint32_t)parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }

            *s = pos;
            *pc += pos;
        }

        // MOV [ADDR], R16
        else if ((is2arrin(regs16, node->ins.operands[1]) || is2arrin(regs16GP, node->ins.operands[1])) &&
                node->ins.otype[0] == O_MEM) {

            AddrExpr parsed = parse_addr_expr(node->ins.operands[0], line);
            uint8_t rex = 0;
            uint8_t opcode = 0x89;
            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;

            int reg_index = find_reg16_index(node->ins.operands[1]);
            if (reg_index >= 8) rex |= REX_R;

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;

            if (parsed.base_is_reg) {
                rm_bits = find_reg16_index(parsed.base_reg);
                if (rm_bits >= 8) rex |= REX_B;

                if ((rm_bits & 7) == 5) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00;
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else {
                    mod_bits = 0b10;
                    disp_size = 4;
                }
            }
            else {
                mod_bits = 0b00;
                rm_bits = 0b101;
                disp_size = 4;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
                int idx = find_reg16_index(parsed.index_reg);
                if (idx >= 8) rex |= REX_X;
            }
            else if ((rm_bits & 7) == 0b100) {
                need_sib = 1;
            }
            else if (!parsed.base_is_reg) {
                need_sib = 1;
            }

            if (need_sib) {
                sib = gen_SIB_byte(parsed);
                rm_bits = 0b100;
            }

            modrm = (mod_bits << 6) | ((reg_index & 7) << 3) | (rm_bits & 7);

            node->machine_code[pos++] = 0x66;
            if (rex != 0) node->machine_code[pos++] = REX_BASE | rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            if (need_sib) node->machine_code[pos++] = sib;
            if (disp_size == 1)
                node->machine_code[pos++] = (uint8_t)parsed.disp_val;
            else if (disp_size == 4) {
                As32((uint32_t)parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }

            *s = pos;
            *pc += pos;
        }

        // MOV [ADDR], R8
        else if ((is2arrin(regs8, node->ins.operands[1]) || is2arrin(regs8GP, node->ins.operands[1])) &&
                node->ins.otype[0] == O_MEM) {

            AddrExpr parsed = parse_addr_expr(node->ins.operands[0], line);
            uint8_t rex = 0;
            uint8_t opcode = 0x88;
            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;

            int reg_index = find_reg8_index(node->ins.operands[1]);
            if (reg_index >= 8) rex |= REX_R;

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;

            if (parsed.base_is_reg) {
                rm_bits = find_reg8_index(parsed.base_reg);
                if (rm_bits >= 8) rex |= REX_B;

                if ((rm_bits & 7) == 5) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00;
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else {
                    mod_bits = 0b10;
                    disp_size = 4;
                }
            }
            else {
                mod_bits = 0b00;
                rm_bits = 0b101;
                disp_size = 4;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
                int idx = find_reg8_index(parsed.index_reg);
                if (idx >= 8) rex |= REX_X;
            }
            else if ((rm_bits & 7) == 0b100) {
                need_sib = 1;
            }
            else if (!parsed.base_is_reg) {
                need_sib = 1;
            }

            if (need_sib) {
                sib = gen_SIB_byte(parsed);
                rm_bits = 0b100;
            }

            modrm = (mod_bits << 6) | ((reg_index & 7) << 3) | (rm_bits & 7);

            if (rex != 0) node->machine_code[pos++] = REX_BASE | rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            if (need_sib) node->machine_code[pos++] = sib;
            if (disp_size == 1)
                node->machine_code[pos++] = (uint8_t)parsed.disp_val;
            else if (disp_size == 4) {
                As32((uint32_t)parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }

            *s = pos;
            *pc += pos;
        }

        // ========================================================================
        // 5. MOV [ADDR], IMM (mem ← imm)
        // ========================================================================

        // MOV [ADDR], IMM32 (for 64-bit) 
        else if (node->ins.otype[0] == O_MEM && node->ins.otype[1] == O_IMM) {
            AddrExpr parsed = parse_addr_expr(node->ins.operands[0], line);
            int imm_size = 4; // default: 32-bit
            uint8_t opcode = 0xC7; // MOV r/m32, imm32
            uint8_t rex = 0;
            uint8_t prefix = 0;
            
            long imm_val = (long)atoll(node->ins.operands[1]);
            
            if (imm_val > 0x7FFFFFFF || imm_val < (long)0xFFFFFFFF80000000LL) {
                fprintf(stderr, "AmmAsm: Cannot MOV imm64 to memory directly (no such opcode). On line '%d'\n", line);
                exit(1);
            }

            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;

            // reg in ModR/M = 0 for MOV [addr], imm
            int reg_field = 0;

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;

            if (parsed.base_is_reg) {
                rm_bits = find_reg64_index(parsed.base_reg);
                if (rm_bits >= 8) rex |= REX_B;

                if ((rm_bits & 7) == 5) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00;
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01;
                    disp_size = 1;
                }
                else {
                    mod_bits = 0b10;
                    disp_size = 4;
                }
            }
            else {
                mod_bits = 0b00;
                rm_bits = 0b101;
                disp_size = 4;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
                int idx = find_reg64_index(parsed.index_reg);
                if (idx >= 8) rex |= REX_X;
            }
            else if ((rm_bits & 7) == 0b100) {
                need_sib = 1;
            }
            else if (!parsed.base_is_reg) {
                need_sib = 1;
            }

            if (need_sib) {
                sib = gen_SIB_byte(parsed);
                rm_bits = 0b100;
            }

            modrm = (mod_bits << 6) | (reg_field << 3) | (rm_bits & 7);

            if (rex != 0) node->machine_code[pos++] = REX_BASE | rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            if (need_sib) node->machine_code[pos++] = sib;
            
            if (disp_size == 1)
                node->machine_code[pos++] = (uint8_t)parsed.disp_val;
            else if (disp_size == 4) {
                As32((uint32_t)parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }

            if (imm_size == 4) {
                As32((int32_t)imm_val, &node->machine_code[pos]);
                pos += 4;
            }
            else if (imm_size == 2) {
                As16((int16_t)imm_val, &node->machine_code[pos]);
                pos += 2;
            }
            else if (imm_size == 1) {
                node->machine_code[pos++] = (uint8_t)imm_val;
            }

            *s = pos;
            *pc += pos;
        }
    }
}

void Ammcompiler(FILE *out) {
    if (!out) return;

    for (int i = 0; i < ast_count; ++i) {
        for(int j = 0; j < ast[i].machine_code_size; ++j)
            fputc(ast[i].machine_code[j], out);
    }
    fflush(out);
}


int main(int argc, char **argv){
    if(argc < 2){
        printf("AmmAsm v1.0 \nUsage: ./aasm <input.asm> \n");
        return 1;
    }
    int pc = 0, line = 0;
    FILE *input = fopen(argv[1], "r");

    line = LEXER(input);
    fclose(input); 
    DEBUG_PRINT_TOKENS();
    
    PARSE();
    DEBUG_PRINT_AST();

    printf("starting compile\n");
    for(int i = 0; i < ast_count; i++)
        parseInst(&ast[i], &pc, line);
    printf("end compile\n");

    FILE *output = fopen("a.bin", "wb");
    if (!output) {
        fprintf(stderr, "AmmAsm: Failed to create output.bin\n");
        return 1;
    }
    Ammcompiler(output);
    fclose(output);
    
    printf("AmmAsm: Compiled successfully! Output: output.bin (%d bytes)\n", pc);
    return 0;
}