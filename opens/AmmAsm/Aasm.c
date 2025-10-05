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


AST ast[MAX_TOKENS];
int ast_count = 0;



const char *p;


int isin(char *str, char c){ for(int i=0; *str != '\0'; i++) if((*str + i) == c) return 1; return 0;}
int is2arrin(char *str[], char *str2){ for(int i=0; str[i] != NULL; ++i) if(strcasecmp(str[i], str2) == 0) return 1; return 0;}



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

// Compare n bytes of s and CMDS[i]
int is_inst_n(const char* s, int n){
    int i = 0;
    int j = n;
    for(i, j; *s && CMDS[i]; i++, ++j){

    }
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
void As16(short a, unsigned char out[2]) {
    out[0] = a & 0xff;
    out[1] = (a >> 8) & 0xff;
}

// 32-bit
void As32(int a, unsigned char out[4]) {
    out[0] = a & 0xff;
    out[1] = (a >> 8) & 0xff;
    out[2] = (a >> 16) & 0xff;
    out[3] = (a >> 24) & 0xff;
}

// 64-bit
void As64(long long a, unsigned char out[8]) {
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
// НЕ УМЕЕТ ПРАВИЛЬНО ПАРСИТЬ ИНСТРУКЦИИ!!!!!
    Lexer lexer = {0};
    int line = 0;
    int in_comment = 0;

    while (fgets(lexer.buf, 256, fl) != NULL) {
        char* buff = lexer.buf;
        line++;

        while (*buff) {
            while (isspace(*buff)) buff++;
            if (*buff == '\0' || *buff == '\n') break;
            if (*buff == '/' && *(buff + 1) == '/') break; // skip comment till end of line
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
                while (isin(*buff, LETEXT)) {
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
                while (isin(*buff, LETEXT)) {
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
                *buff++;
                char buf[12];
                int i = 0;
                while (!isspace(*buff) && *buff != '\0') buf[i++] = *buff++;
                buf[i] = '\0'; 

                int regtype = is_reg(buf);
                if (regtype == 1) add_token(T_REG8, buf, line);
                else if (regtype == 2) add_token(T_REG16, buf, line);
                else if (regtype == 3) add_token(T_REG32, buf, line);
                else if (regtype == 4) add_token(T_REG64, buf, line);
                else if (regtype == 5) add_token(T_REG64, buf, line); // same
            }
            else if (*buff == '-' || *buff == '+' || isdigit(*buff)) {
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
            }
            else if(*buff == '0'){
                char tmp[72];
                long long c = 0;
                buff++;
                if(*buff++ == 'x'){  // hex
                    while (isin(LETEXT, *buff)) {
                        if (*buff >= '0' && *buff <= '9') c = c * 16 + (*buff - '0');  
                        else if (*buff >= 'a' && *buff <= 'f') c = c * 16 + (*buff - 'a' + 10);                       
                        else if (*buff >= 'A' && *buff <= 'F') c = c * 16 + (*buff - 'A' + 10);                       
                        else break;
                    }
                    snprintf(tmp, sizeof(tmp), "%d", c);
                    add_token(T_INT, tmp, line);
                }
                else if(*buff == 'b'){  // binary
                    while(isin(DIGBIN, *buff))c = c * 2 + (*buff++ - '0');
                    snprintf(tmp, sizeof(tmp), "%lld", c);
                    add_token(T_INT, tmp, line);
                }
                else if (*buff == 'o') {  // octal
                    while (isin(DIGOCT, *buff)) c = c * 8 + (*buff++ - '0');
                    snprintf(tmp, sizeof(tmp), "%lld", c);
                    add_token(T_INT, tmp, line);
                }
                else {
                    snprintf(tmp, sizeof(tmp), "%lld", 0);
                    add_token(T_INT, tmp, line);
                }
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
                
                add_token(T_ADDR_EXPR, strdup(clean_expr), line);
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
            }
            else if(*buff == ','){
                buff++;
                while(isspace(*buff)) buff++;
                add_token(T_COMMA, ",", line);
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
            }
            else if (strncasecmp(buff, "resb", 4) == 0){ add_token(T_RESB, buff, line); buff += 4; while (isspace(*buff)) buff++;}
            else if (strncasecmp(buff, "resq", 4) == 0){ add_token(T_RESQ, buff, line); buff += 4; while (isspace(*buff)) buff++;}
            else if (strncasecmp(buff, "resd", 4) == 0){ add_token(T_RESD, buff, line); buff += 4; while (isspace(*buff)) buff++;}
            else if (strncasecmp(buff, "resl", 4) == 0){ add_token(T_RESL, buff, line); buff += 4; while (isspace(*buff)) buff++;}
                
            else{ fprintf(stderr, "AmmAsm: unvalid syntax or char on line", line); exit(1); }
            buff++;
        }

        add_token(T_EOL, "\\n", line);
    }
    add_token(T_EOF, "\x7FELF", line); // LOL
    return 0;
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

typedef struct AST {
    int type;
    char cmd[256];  // mostly useing for ins

    union {
        struct { char operand1[64]; char operand2[64]; char operand3[64]; char operand4[64]; int oper_count} ins;
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
        struct { char name[64]; int adress; } label;
        struct { char name[63]; } section;
        AddrExpr sib;
    };
        unsigned char machine_code[256];
        int machine_code_size;
} AST;

// parser:
AST* PARSE(){
    int pos = 0;
    int pc = 0;

    while(toks[pos].type != T_EOF){
        Token *tok = &toks[pos];
        if (tok->type == T_INS) {
            
        }

        if(tok->type == T_LAB){
            AST node = { .type = AST_LABEL };
            if(tok->type != T_EOF && tok->type != T_EOL){
                strncpy(node.label.name, toks[pos++].value, sizeof(node.label.name));
                strncpy(node.cmd, toks[pos++].value, sizeof(node.cmd));
            }
            ast[ast_count++] = node;
            ++pos;
            continue;
        }
        if(tok->type == T_BYTE && (pos == 0 || toks[pos-1].type == T_ID)){   
            AST node = { .type = AST_U8};
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
            AST node = { .type = AST_U8PTR };
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
                    fprintf(stderr, "AmmAsm: expected label name (T_ID) or 0 (NULL) in BYTEPTRS at line %d\n", toks[pos].line);
                    exit(1);
                }
            }


            ast[ast_count++] = node;
            if(pos < toks_count && toks[pos].type == T_EOL) ++pos;
            continue;
        }
        else if(tok->type == T_QWORD && (pos == 0 || toks[pos-1].type == T_ID)){
            AST node = { .type = AST_U16};
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
            AST node = { .type = AST_U16PTR };
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
            AST node = { .type = AST_U32 };
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
            AST node = { .type = AST_U32PTR };
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
            AST node = { .type = AST_U64 };
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
            AST node = { .type = AST_U64PTR };
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
            // else

            AST node = { 0 };

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
        // else if(tok->type == T_ADDR_EXPR){
        //     AST node = { 0 };
        //     node.ins.oper_count = 0;
        //     pos++;

        //     unsigned char *new[5];
        //     for(int i = pos, j = 0; i <= pos+5; i++, j++){
        //         if(toks[i].type == T_COMMA) continue;
        //         new[j] = toks[i].value;
        //     }
        //     strncpy(toks[])

            

        // }

        

    }
}

int indexof(char** regs, char* reg){

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

int find_reg64_index(const char* reg) {
    for (int i=0; i<8; i++) {
        if (strcasecasecmp(regs64[i], reg) == 0) return i;
    }
    for (int i=0; i<8; i++) {
        if (strcasecasecmp(regs64GP[i], reg) == 0) return i + 8;
    }
    return -1;
}
int find_reg32_index(const char* reg) {
    for (int i=0; i<8; i++) {
        if (strcasecasecmp(regs32[i], reg) == 0) return i;
    }
    for (int i=0; i<8; i++) {
        if (strcasecasecmp(regs32GP[i], reg) == 0) return i + 8;
    }
    return -1;
}
int find_reg16_index(const char* reg) {
    for (int i=0; i<8; i++) {
        if (strcasecasecmp(regs16[i], reg) == 0) return i;
    }
    for (int i=0; i<8; i++) {
        if (strcasecasecmp(regs16GP[i], reg) == 0) return i + 8;
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
Address Expression Parsing Rules (AmmAsm2 Custom x86-64 Syntax)
===============================================================================

We intentionally restrict the addressing syntax to avoid Intel/NASM chaos
and make parsing predictable. This is NOT NASM/MASM syntax — these are OUR rules.

Allowed form:
    [ BASE/Physical-Address +/- INDEX/DISP * SCALE ]

Where:
    BASE  - one 64-bit register (e.g., rax, rbx, rcx, r8, r15)
            or a physical address constant (e.g., 0xB8000)
    INDEX - one 64-bit register, used only if SCALE is present
    DISP  - integer displacement (decimal or hex)
    SCALE - must be 1, 2, 4, or 8 (default is 1 if omitted)

Examples (valid):
    [%%rax + 20]         ; base + displacement
    [%%r8 + 4 * 8]       ; base + (disp * scale)
    [%%rbx + %rcx * 2]   ; base + (index * scale)
    [0xB8000]            ; physical address (useful for kernel x86_16/32/64-bit mode)
    [%%rbx * 4]          ; index-only with scale
    [0xB8000 + 0xA]      ; physical address + displacement

Invalid forms:
    [SCALE * DISP + BASE]   ; wrong order
    [DISP + BASE]           ; displacement first = ambiguous
    [BASE + SCALE]          ; scale without index/disp
    [%%rax]                   ; bare register without displacement

Parsing logic:
    1) First token is either a BASE register or a numeric address.
    2) If '+' is present:
        - Left side is BASE/address
        - Right side can be:
            * DISP
            * INDEX
            * INDEX * SCALE
    3) If '*' is present:
        - Left side before '*' must be INDEX or DISP
        - Right side after '*' must be SCALE (1, 2, 4, or 8)
    4) Without '+' or '*', the only valid form is [0xADDR].

Why this “gospel” exists:
 - Simple to parse, no NASM schizophrenia
 - No guessing games about base vs index
 - Prevents parser complexity and weird corner cases
 - Still matches actual x86-64 SIB encoding rules
 - Easy to remember without Intel's 200-page addressing manual

Implementation note:
The parser:
   - Splits the expression by '+' and '*'
   - Identifies BASE, DISP, INDEX, and SCALE
   - Validates SCALE (1, 2, 4, 8 only)
   - Rejects ambiguous or invalid forms immediately
===============================================================================
*/

#define SKIP_SP while(isspace(*p++));

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
// parsing only LM registers
AddrExpr parse_addr_expr(const char* expr , int line) {
    AddrExpr new = { 0 };
    new.scale = 1; // base value
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

        /*
    Guide to forming MOV (register operands):

    1) MOV reg, imm64 (immediate):
    - Opcode: 0xB8 + register number (low 3 bits)
    - REX prefix:
        * 0x48 for rax..rdi (registers 0..7)
        * 0x49 for r8..r15 (registers 8..15)
    - imm64: 8-byte constant

    2) MOV reg, reg:
    - Opcode: 0x89 (MOV r/m64, r64)
    - REX prefix:
        * W bit = 1 — 64-bit operation (0x40 + 0x08)
        * R bit = 1 if source is r8..r15 (0x04)
        * B bit = 1 if destination is r8..r15 (0x01)
    - ModR/M byte:
        * mod = 11 (register addressing)
        * reg = source register
        * rm = destination register

    REX prefix format:
    0100WRXB (0x40 + W<<3 + R<<2 + X<<1 + B)

    ModR/M byte:
    mod reg rm
    - mod=11 (0b11) — both registers
    - reg — source register
    - rm — destination register

    Notes:
    - Registers numbered 8..15 require setting corresponding R or B bits in REX.
    - X bit = 0 here (no SIB used).
    */
    if (strcasecmp(node->cmd, "mov") == 0) {
        // MOV R64, IMMU64
        if ((is2arrin(regs64, node->ins.operand1) || is2arrin(regs64GP, node->ins.operand1)) 
            && is_literal(node->ins.operand2)) {

            uint8_t rex = REX_BASE; // base REX prefix: 0100 ----
            uint8_t opcode = 0xB8; // base MOV opcode
            long imm64 = (long)atoll(node->ins.operand2);

            int reg_num = -1;

            // Determine register number (0-7) and set REX.B
            if ((reg_num = indexof(regs64, node->ins.operand1)) != -1) {
                rex |= REX_W; // W = 1
                    
            } // no REX.B needed, reg_num in 0..7
            else if ((reg_num = indexof(regs64GP, node->ins.operand1)) != -1) {
                    rex |= REX_W; // W = 1
                    rex |= REX_B; // B = 1 (r8–r15 need this)
            } 
            else {
                return; 
            }

            opcode += reg_num; // B8 + reg_num (0..7) (nah does cares about byte code P.S hello python coder)

            // Encode
            node->machine_code[0] = rex;
            node->machine_code[1] = opcode;
            As64(imm64, &node->machine_code[2]);

            *s = 10;
            *pc += 10;
            node->machine_code_size = *s;
            return;
        }
        // MOV R64, R64 (r8..r15 is valid for dest and src)
        else if ((is2arrin(regs64, node->ins.operand1) || is2arrin(regs64GP, node->ins.operand1)) &&
                (is2arrin(regs64, node->ins.operand2) || is2arrin(regs64GP, node->ins.operand2))) {

            uint8_t rex = REX_BASE | REX_W; // 64-bit operation
            uint8_t opcode = 0x89; // MOV r/m64, r64
            uint8_t modrm = 0b11000000; // mod = 11 (register addressing mode)
// mod | src | dest
            int dst_index = indexof(regs64, node->ins.operand1);
            if (dst_index == -1) {
                dst_index = indexof(regs64GP, node->ins.operand1);
                rex |= REX_B; // if destination is r8–r15
            }

            int src_index = indexof(regs64, node->ins.operand2);
            if (src_index == -1) {
                src_index = indexof(regs64GP, node->ins.operand2);
                rex |= REX_R; // if source is r8–r15
            }

            // ModR/M format: mod(2) | reg(3) | rm(3)
            modrm |= (src_index << 3); // reg = source
            modrm |= dst_index;        // rm = destination

            node->machine_code[0] = rex;
            node->machine_code[1] = opcode;
            node->machine_code[2] = modrm;

            node->machine_code_size = 3;
            *pc += node->machine_code_size;
            *s = node->machine_code_size;
            return;
        }
        // MOV REG32, IMM32
        else if ((is2arrin(regs32, node->ins.operand1) || is2arrin(regs32GP, node->ins.operand1)) 
                && is_literal(node->ins.operand2)) {

            uint8_t rex = 0; // no REX by default
            uint32_t imm32 = atoi(node->ins.operand2);
            uint8_t opcode = 0xB8; // base opcode
            int reg_num = -1;

            // eax..edi
            if ((reg_num = indexof(regs32, node->ins.operand1)) != -1) {
                // no rex needed
            }
            // r8d..r15d
            else if ((reg_num = indexof(regs32GP, node->ins.operand1)) != -1) {
                rex = REX_BASE | REX_B; // only B = 1 bit for r8d..r15d
            }
            else return; // something went wrong

            opcode += reg_num; // add register number

            int pos = 0;
            if (rex) node->machine_code[pos++] = rex;
            node->machine_code[pos++] = opcode;
            As32(imm32, &node->machine_code[pos]);
            pos += 4;

            *s = pos;
            *pc += pos;
            node->machine_code_size = *s;
            return;
        }
        // MOV R32, R32 (r8d..r15d is valid for dest and src)
        else if ((is2arrin(regs32, node->ins.operand1) || is2arrin(regs32GP, node->ins.operand1)) &&
                (is2arrin(regs32, node->ins.operand2) || is2arrin(regs32GP, node->ins.operand2))) {

            uint8_t rex = 0; // (W = 0)
            uint8_t opcode = 0x89; // MOV r/m32, r32
            uint8_t modrm = 0b11000000; // mod = 11 (register addressing mode)

            int dst_index = indexof(regs32, node->ins.operand1);
            if (dst_index == -1) {
                dst_index = indexof(regs32GP, node->ins.operand1);
                if (dst_index == -1) return;   // invalid register
                rex |= REX_B; // if destination is r8d–r15d
            }

            int src_index = indexof(regs32, node->ins.operand2);
            if (src_index == -1) {
                src_index = indexof(regs32GP, node->ins.operand2);
                if (src_index == -1) return;   // guess what? .. invalid register
                rex |= REX_R; // if source is r8d–r15d
            }

            // build ModR/M: mod(2)=11 | reg(3)=src | rm(3)=dst
            modrm |= ((src_index & 0b00000111) << 3) | (dst_index & 0b00000111);

            int pos = 0;
            if (rex) node->machine_code[pos++] = (REX_BASE | (rex & (REX_R|REX_X|REX_B))); // using all flags in Modr/m
            // NOTE: here rex only contains REX_R|REX_B bits; add REX_BASE when emitting.

            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            node->machine_code_size = pos;
            *pc += node->machine_code_size;
            *s = node->machine_code_size;
            return;
        }
        // MOV REG16, IMM16
        else if ((is2arrin(regs32, node->ins.operand1) || is2arrin(regs32GP, node->ins.operand1)) 
                && is_literal(node->ins.operand2)) {

            uint8_t rex = 0; // no REX by default
            uint16_t imm16 = (uint16_t)atoi(node->ins.operand2);
            uint8_t opcode = 0xB8; // base opcode
            int reg_num = -1;

            // ax..dx, si, di, bp, sp
            if ((reg_num = indexof(regs16, node->ins.operand1)) != -1) {
                // no rex needed
            }
            // r8w..r15w
            else if ((reg_num = indexof(regs16GP, node->ins.operand1)) != -1) {
                rex = REX_BASE | REX_B; // only B = 1 bit for r8w..r15w
            }
            else return; // something went wrong

            opcode += reg_num; // add register number

            int pos = 0;
            node->machine_code[pos++] = 0x66; // 16-bit operand size prefix
            if (rex) node->machine_code[pos++] = rex;
            node->machine_code[pos++] = opcode;
            As16(imm16, &node->machine_code[pos]);
            pos += 2;

            *s = pos;
            *pc += pos;
            node->machine_code_size = *s;
            return;
        }
        // MOV R16, R16 (r8w..r15w is valid for dest and src)
        else if ((is2arrin(regs32, node->ins.operand1) || is2arrin(regs32GP, node->ins.operand1)) &&
                (is2arrin(regs32, node->ins.operand2) || is2arrin(regs32GP, node->ins.operand2))) {

            uint8_t rex = 0; // (W = 0)
            uint8_t opcode = 0x89; // MOV r/m16, r16
            uint8_t modrm = 0b11000000; // mod = 11 (register addressing mode)

            int dst_index = indexof(regs16, node->ins.operand1);
            if (dst_index == -1) {
                dst_index = indexof(regs16GP, node->ins.operand1);
                if (dst_index == -1) return;   // invalid register
                rex |= REX_B; // if destination is r8w–r15w
            }

            int src_index = indexof(regs16, node->ins.operand2);
            if (src_index == -1) {
                src_index = indexof(regs16GP, node->ins.operand2);
                if (src_index == -1) return;   // guess what? .. invalid register
                rex |= REX_R; // if source is r8w–r15w
            }

            // build ModR/M: mod(2)=11 | reg(3)=src | rm(3)=dst
            modrm |= ((src_index & 0b00000111) << 3) | (dst_index & 0b00000111);

            int pos = 0;
            node->machine_code[pos++] = 0x66;
            uint8_t rex_val = REX_BASE | (rex & (REX_R | REX_B));
            if (rex_val != REX_BASE) node->machine_code[pos++] = rex_val;
            // NOTE: here rex only contains REX_R|REX_B bits; add REX_BASE when emitting.
            
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            node->machine_code_size = pos;
            *pc += node->machine_code_size;
            *s = node->machine_code_size;
            return;
        }
        // MOV REG8, IMM8
        else if ((is2arrin(regs32, node->ins.operand1) || is2arrin(regs32GP, node->ins.operand1)) 
                && is_literal(node->ins.operand2)) {

            uint8_t rex = 0; // no REX by default
            uint8_t imm8 = (uint8_t)atoi(node->ins.operand2);
            uint8_t opcode = 0xB0; // base opcode for 8-bit registers
            int reg_num = -1;

            if ((reg_num = indexof(regs8, node->ins.operand1)) != -1) {
                // no rex needed
            }

            else if ((reg_num = indexof(regs8GP, node->ins.operand1)) != -1) {
                rex = REX_BASE | REX_B; // only B = 1 bit for r8b..r15b
            }
            else return; // something went wrong

            opcode += reg_num; // add register number

            int pos = 0;
            if (rex) node->machine_code[pos++] = rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = imm8;


            *s = pos;
            *pc += pos;
            node->machine_code_size = *s;
            return;
        }
        // MOV R8, R8 (r8b..r15b is valid for dest and src)
        else if ((is2arrin(regs32, node->ins.operand1) || is2arrin(regs32GP, node->ins.operand1)) &&
                (is2arrin(regs32, node->ins.operand2) || is2arrin(regs32GP, node->ins.operand2))) {

            uint8_t rex = 0; // (W = 0)
            uint8_t opcode = 0x88; // MOV r/m8, r8
            uint8_t modrm = 0b11000000; // mod = 11 (register addressing mode)

            int dst_index = indexof(regs8, node->ins.operand1);
            if (dst_index == -1) {
                dst_index = indexof(regs8GP, node->ins.operand1);
                if (dst_index == -1) return;   // invalid register
                rex |= REX_B; // if destination is r8b–r15b
            }

            int src_index = indexof(regs8, node->ins.operand2);
            if (src_index == -1) {
                src_index = indexof(regs8GP, node->ins.operand2);
                if (src_index == -1) return;   // guess what? .. invalid register
                rex |= REX_R; // if source is r8b–r15b
            }

            // build ModR/M: mod(2)=11 | reg(3)=src | rm(3)=dst
            modrm |= ((src_index & 0b00000111) << 3) | (dst_index & 0b00000111);

            int pos = 0;
            uint8_t rex_val = REX_BASE | (rex & (REX_R | REX_B));
            if (rex_val != REX_BASE) node->machine_code[pos++] = rex_val;
            // NOTE: here rex only contains REX_R|REX_B bits; add REX_BASE when emitting.
            
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;

            node->machine_code_size = pos;
            *pc += node->machine_code_size;
            *s = node->machine_code_size;
            return;
        }
        // MOV R64, [ADDR]
        else if ((is2arrin(regs64, node->ins.operand1) || is2arrin(regs64GP, node->ins.operand1)) &&
                node->type == AST_ADDR_EXPR) {

            AddrExpr parsed = parse_addr_expr(node->ins.operand2, line);
            uint8_t rex = REX_BASE | REX_W; // 64-bit
            uint8_t opcode = 0x8B; // MOV r64, r/m64
            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;


            int reg_index = find_reg64_index(node->ins.operand1);
            if (reg_index >= 8) rex |= REX_R; 

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;
            
            if (parsed.base_is_reg) {
                rm_bits = find_reg64_index(parsed.base_reg);
                
                // Special case: RBP/R13
                if (rm_bits == 5) { // RBP/R13
                    if (parsed.disp_val == 0) {
                        mod_bits = 0b01; // disp8 
                        disp_size = 1;
                    } 
                    else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                        mod_bits = 0b01; // disp8
                        disp_size = 1;
                    } 
                    else {
                        mod_bits = 0b10; // disp32
                        disp_size = 4;
                    }
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00; // no displacement
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01; // disp8
                    disp_size = 1;
                } 
                else {
                    mod_bits = 0b10; // disp32
                    disp_size = 4;
                }
            } 
            else {
                // [imm32]
                mod_bits = 0b00;
                rm_bits = 0b101; // means SIB with base=0 or pure disp32
                disp_size = 0b100;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
            } 
            else if (rm_bits == 0b100) { // RSP/R12 need SIB if useing index or scale
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

            node->machine_code[pos++] = rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;
            
            if (need_sib) node->machine_code[pos++] = sib;
            if (disp_size == 1) node->machine_code[pos++] = (uint8_t)parsed.disp_val;
             
            else if (disp_size == 0b100) {
                As32(parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }

            node->machine_code_size = pos;
            *pc += pos;
        }
        // MOV r/m64, r64
        else if ((is2arrin(regs64, node->ins.operand2) || is2arrin(regs64GP, node->ins.operand2)) &&
                node->type == AST_ADDR_EXPR) {

            AddrExpr parsed = parse_addr_expr(node->ins.operand2, line);
            uint8_t rex = REX_BASE | REX_W; // 64-bit
            uint8_t opcode = 0x89; // MOV r/m64, r64 
            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;


            int reg_index = find_reg64_index(node->ins.operand2);
            if (reg_index >= 8) rex |= REX_B; 

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;
            
            if (parsed.base_is_reg) {
                rm_bits = find_reg64_index(parsed.base_reg);
                
                // Special case: RBP/R13
                if (rm_bits == 5) { // RBP/R13
                    if (parsed.disp_val == 0) {
                        mod_bits = 0b01; // disp8 
                        disp_size = 1;
                    } 
                    else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                        mod_bits = 0b01; // disp8
                        disp_size = 1;
                    } 
                    else {
                        mod_bits = 0b10; // disp32
                        disp_size = 4;
                    }
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00; // no displacement
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01; // disp8
                    disp_size = 1;
                } 
                else {
                    mod_bits = 0b10; // disp32
                    disp_size = 4;
                }
            } 
            else {
                // [imm32]
                mod_bits = 0b00;
                rm_bits = 0b101; // means SIB with base=0 or pure disp32
                disp_size = 0b100;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
            } 
            else if (rm_bits == 0b100) { // RSP/R12 need SIB if useing index or scale
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

            node->machine_code[pos++] = rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;
            
            if (need_sib) node->machine_code[pos++] = sib;
            if (disp_size == 1) node->machine_code[pos++] = (uint8_t)parsed.disp_val;
             
            else if (disp_size == 0b100) {
                As32(parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }

            node->machine_code_size = pos;
            *pc += pos;
        }
        // mov r32, r/m
        else if(node->type == AST_ADDR_EXPR && 
                (is2arrin(regs32, node->ins.operand1) || is2arrin(regs32GP, node->ins.operand1))){
            AddrExpr parsed = parse_addr_expr(node->ins.operand2, line);
            uint8_t rex = REX_BASE; // 32-bit
            uint8_t opcode = 0x8B; // MOV r/m, r32  
            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;


            int reg_index = find_reg32_index(node->ins.operand2);
            if (reg_index >= 8) rex |= REX_B; 

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;
            
            if (parsed.base_is_reg) {
                rm_bits = find_reg32_index(parsed.base_reg);
                
                // Special case: RBP/R13
                if (rm_bits == 5) { // RBP/R13
                    if (parsed.disp_val == 0) {
                        mod_bits = 0b01; // disp8 
                        disp_size = 1;
                    } 
                    else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                        mod_bits = 0b01; // disp8
                        disp_size = 1;
                    } 
                    else {
                        mod_bits = 0b10; // disp32
                        disp_size = 4;
                    }
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00; // no displacement
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01; // disp8
                    disp_size = 1;
                } 
                else {
                    mod_bits = 0b10; // disp32
                    disp_size = 4;
                }
            } 
            else {
                // [imm32]
                mod_bits = 0b00;
                rm_bits = 0b101; // means SIB with base=0 or pure disp32
                disp_size = 0b100;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
            } 
            else if (rm_bits == 0b100) { // RSP/R12 need SIB if useing index or scale
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

            node->machine_code[pos++] = rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;
            
            if (need_sib) node->machine_code[pos++] = sib;
            if (disp_size == 1) node->machine_code[pos++] = (uint8_t)parsed.disp_val;
             
            else if (disp_size == 0b100) {
                As32(parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }

            node->machine_code_size = pos;
            *pc += pos;
        }
        // mov r16, r/m
        else if(node->type == AST_ADDR_EXPR && 
                (is2arrin(regs16, node->ins.operand1) || is2arrin(regs16GP, node->ins.operand1))){
            AddrExpr parsed = parse_addr_expr(node->ins.operand2, line);
            uint8_t rex = REX_BASE;
            uint8_t prefix = 0x66;
            uint8_t opcode = 0x8B; // MOV r/m, r32  
            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;


            int reg_index = find_reg16_index(node->ins.operand2);
            if (reg_index >= 8) rex |= REX_B; 

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;
            
            if (parsed.base_is_reg) {
                rm_bits = find_reg16_index(parsed.base_reg);
                
                // Special case: RBP/R13
                if (rm_bits == 5) { // RBP/R13
                    if (parsed.disp_val == 0) {
                        mod_bits = 0b01; // disp8 
                        disp_size = 1;
                    } 
                    else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                        mod_bits = 0b01; // disp8
                        disp_size = 1;
                    } 
                    else {
                        mod_bits = 0b10; // disp32
                        disp_size = 4;
                    }
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00; // no displacement
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01; // disp8
                    disp_size = 1;
                } 
                else {
                    mod_bits = 0b10; // disp32
                    disp_size = 4;
                }
            } 
            else {
                // [imm32]
                mod_bits = 0b00;
                rm_bits = 0b101; // means SIB with base=0 or pure disp32
                disp_size = 0b100;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
            } 
            else if (rm_bits == 0b100) { // RSP/R12 need SIB if useing index or scale
                need_sib = 1;
            } 
            else if (!parsed.base_is_reg) { 
                need_sib = 1;
            }

            if (need_sib) {
                sib = gen_SIB_byte(parsed);
                rm_bits = 0b100; 
            }


            if (need_sib) rm_bits = 0b100;

            modrm = (mod_bits << 6) | ((reg_index & 7) << 3) | (rm_bits & 7);

            node->machine_code[pos++] = prefix;   // 0x66 for r16
            node->machine_code[pos++] = rex;      // REX 
            node->machine_code[pos++] = opcode;   // 0x8B

            node->machine_code[pos++] = modrm;    
            if (need_sib) node->machine_code[pos++] = sib; 

            if (disp_size == 1)
                node->machine_code[pos++] = (uint8_t)parsed.disp_val;
            else if (disp_size == 0b100) {
                As32(parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }
        }
        // mov r8, r/m
        else if(node->type == AST_ADDR_EXPR && 
                (is2arrin(regs8, node->ins.operand1) || is2arrin(regs8GP, node->ins.operand1))){
            AddrExpr parsed = parse_addr_expr(node->ins.operand2, line);
            uint8_t rex = REX_BASE; // 32-bit
            uint8_t opcode = 0x8A; // MOV r/m, r8
            uint8_t modrm = 0;
            uint8_t sib = 0;
            int disp_size = 0;
            int pos = 0;


            int reg_index = find_reg8_index(node->ins.operand2);
            if (reg_index >= 8) rex |= REX_B; 

            uint8_t mod_bits = 0;
            uint8_t rm_bits = 0;
            
            if (parsed.base_is_reg) {
                rm_bits = find_reg8_index(parsed.base_reg);
                
                // Special case: RBP/R13
                if (rm_bits == 5) { // RBP/R13
                    if (parsed.disp_val == 0) {
                        mod_bits = 0b01; // disp8 
                        disp_size = 1;
                    } 
                    else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                        mod_bits = 0b01; // disp8
                        disp_size = 1;
                    } 
                    else {
                        mod_bits = 0b10; // disp32
                        disp_size = 4;
                    }
                }
                else if (parsed.disp_val == 0 && !parsed.index_exists) {
                    mod_bits = 0b00; // no displacement
                }
                else if (parsed.disp_val >= -128 && parsed.disp_val <= 127) {
                    mod_bits = 0b01; // disp8
                    disp_size = 1;
                } 
                else {
                    mod_bits = 0b10; // disp32
                    disp_size = 4;
                }
            } 
            else {
                // [imm32]
                mod_bits = 0b00;
                rm_bits = 0b101; // means SIB with base=0 or pure disp32
                disp_size = 0b100;
            }

            int need_sib = 0;
            if (parsed.index_exists) {
                need_sib = 1;
            } 
            else if (rm_bits == 0b100) { // RSP/R12 need SIB if useing index or scale
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

            node->machine_code[pos++] = rex;
            node->machine_code[pos++] = opcode;
            node->machine_code[pos++] = modrm;
            
            if (need_sib) node->machine_code[pos++] = sib;
            if (disp_size == 1) node->machine_code[pos++] = (uint8_t)parsed.disp_val;
             
            else if (disp_size == 0b100) {
                As32(parsed.disp_val, &node->machine_code[pos]);
                pos += 4;
            }

            node->machine_code_size = pos;
            *pc += pos;
        }
        // MOV r/m, immN
        else if(node->type == AST_ADDR_EXPR && is_literal(node->ins.operand2)){
            uint8_t scale = 0b00;
            uint64_t imm; 
            if(node->ins.operand2[0] == '0' && node->ins.operand2[1] == 'x') imm = strtol(&node->ins.operand2[2], NULL, 16);
            if(imm <= 0xFF)                    scale = 0b00; // 1
            else if(imm <= 0xFFFF)             scale = 0b01; // 2
            else if(imm <= 0xFFFFFFFF)         scale = 0b10; // 4
            else if(imm <= 0xFFFFFFFFFFFFFFFF) scale = 0b11; // 8
            uint8_t MODrm = 0;
            uint8_t SIB;

        }
    }

}




