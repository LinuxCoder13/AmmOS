# 0 "Aasm.c"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "Aasm.c"
# 56 "Aasm.c"
const char *CMDS[] = {"mov", "push", "pop", "syscall", "call", "jmp", "add", "sub",
               "mul", "div", "j==", "j!=", "j>", "j<", "j>=", "j<=", "j!",
               "xor", "or", "cmp", "inc", "dec", "nop", "ret", "leave", "test", "lea", "not",
               "and", "shl", "shr", NULL};

const char *HUMAN_AST[] = {"BYTE", "BYTE PTR", "QWORD", "QWORD PTR", "DWORD", "DWORD PTR", "LBYTE", "LBYTE PTR", NULL};
const char *HUMAN_TOKEN[] = {"inst", "label", "reg", "[reg]", "[reg +-*/ value]", "string", NULL};


const char *LET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";

const char *LETEXT = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxy0123456789-_.";
const char* DIG = "0123456789";
const char* DIGEXT = "0123456789abcdef";
const char* DIGBIN = "01";
const char* DIGOCT = "01234567";

const char *regs8[] = {"al", "ah", "bl", "bh", "cl", "ch", "dl", "dh", NULL};
const char* regs16[] = {"ax", "bx", "cx", "dx", "si", "di", "sp", "bp", NULL};
const char* regs32[] = {"eax", "ebx", "ecx", "edx", "esi", "edi", "esp", "ebp", NULL};
const char* regs64[] = {
    "rax", "rbx", "rcx", "rdx",
    "rsi", "rdi", "rsp", "rbp",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
    "rip", NULL
};

typedef struct {
    int type;
    char* value;
    int line;
} Token;

Token toks[1024];
int toks_count = 0;

typedef struct {
    const char* text;
    const char* filename;
    int length;
    char buf[256];
    Token* toks;
    char labelscope[256];
} Lexer;

typedef struct AST {
    int type;
    char cmd[256];

    union {
        struct { char operand1[64]; char operand2[64]; char operand3[64]; char operand4[64]; int oper_count} ins;
        struct { unsigned char data[256]; int size; } bytes;
        struct { unsigned char *data[256]; int size; } byteptrs;
        struct { short data[256]; int size; } qwords;
        struct { short *data[256]; int size; } qwordptrs;
        struct { int data[256]; int size; } dwords;
        struct { int *data[256]; int size; } dwordptrs;
        struct { long data[256]; int size; } lbytes;
        struct { long *data[256]; int size; } lbyteptrs;
        struct { unsigned long size; } resb;
        struct { unsigned long size; } resq;
        struct { unsigned long size; } resd;
        struct { unsigned long size; } resl;
        struct { char name[64]; int adress; } label;
        struct { char name[63]; } section;
    };
} AST;

AST ast[1024];
int ast_count = 0;

const char *p;

enum {
    CMD_MOV, CMD_PUSH, CMD_POP, CMD_SYSCALL,
    CMD_CALL, CMD_JMP, CMD_ADD, CMD_SUB,
    CMD_MUL, CMD_DIV, CMD_JE, CMD_JNE,
    CMD_JG, CMD_JL, CMD_JGE, CMD_JLE, CMD_JNULL,
    CMD_XOR, CMD_OR, CMD_CMP, CMD_INC, CMD_DEC, CMD_NOP,
    CMD_COUNT
};

int isin(char *str, char c){ for(int i=0; *str != '\0'; i++) if((*str + i) == c) return 1; return 0;}
int is2arrin(char **str, char *str2){ for(int i=0; *str != ((void*)0); ++i) if(astrcmp(str[i], str2) == 0) return 1; return 0;}


long parse_number() {
    while (isspace(*p)) p++;

    if (*p == '-') {
        p++;
        return -parse_number();
    }

    if (*p == '(') {
        p++;
        long val = parse_expr();
        if (*p == ')') p++;
        return val;
    }

    int res = 0;
    while (isdigit(*p)) {
        res = res * 10 + (*p - '0');
        p++;
    }
    return res;
}



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
    if(toks_count >= 1024){
        fprintf(stderr, "AmmAsm:\033[;31m Fatal: Too mant tokens (max 1024)\033[0m\n");
        exit(1);
    }
    toks[toks_count].line = line;
    toks[toks_count].type = type;
    toks[toks_count].value = astrdup(value);
    toks_count++;
}

int is_inst(char* s){
    for(int i=0; CMDS[i]; ++i){
        if(strcmp(s, CMDS[i]) == 0){
            return 1;
        }
    }

    return 0;
}

int is_reg(char *s) __attribute__((__nonnull__(1)));
int is_reg(char *s){






    for(int i=0; regs8[i]; i++){
        if(strcmp(s, *(regs8+i)) == 0){
            return 1;
        }
    }
    for(int i=0; regs16[i]; i++){
        if(strcmp(s, *(regs16+i)) == 0){
            return 2;
        }
    }
    for(int i=0; regs32[i]; i++){
        if(strcmp(s, *(regs32+i)) == 0){
            return 3;
        }
    }
    for(int i=0; regs64[i]; i++){
        if(strcmp(s, *(regs64+i)) == 0){
            return 4;
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
                case '0': *dest = '\0'; buff += 2; return buff;
                default : fprintf(stderr, "AmmAsm: invalid escape sequence on line %d\n", line); exit(1);
            }
        }
        else {
            *dest++ = *buff++;
        }
    }
    return buff;
}


int parseinst(const char *instruction, const char *operand1, const char *operand2, int *size) {
    if (strcmp(instruction, "mov") == 0){
        int reg1 = is_reg(operand1);
        int reg2 = is_reg(operand2);

        if (reg1 && reg2) {
            if(reg1 == 1 && reg2 == 1){

            }
            if(reg1 == 4 && reg2 == 4){
                if(strcmp(operand1, "rax") == 0 && strcmp(operand2, "rbx") == 0) *size = 2;
                else if(strcmp(operand2, "rax") == 0 && strcmp(operand1, "rbx") == 0) *size = 2;
                else if(strcmp(operand1, "rax") == 0 && strcmp(operand2, "rdi") == 0) *size = 2;
                *size = 3;
                return 0;
            }
            else if(reg1 == 3 && reg2 == 3){
                *size = 2;
                return 0;
            }
        }
        else if (reg1 && is_literal(operand2)) {
            *size = 5;
            return 0;
        }

    }
    return -1;
}




void As16(short a, char out[2]) {
    out[0] = a & 0xff;
    out[1] = (a >> 8) & 0xff;
}
short a = 'Ф';

void As32(int a, char out[4]) {
    out[0] = a & 0xff;
    out[1] = (a >> 8) & 0xff;
    out[2] = (a >> 16) & 0xff;
    out[3] = (a >> 24) & 0xff;
}


void As64(long long a, char out[8]) {
    out[0] = a & 0xff;
    out[1] = (a >> 8) & 0xff;
    out[2] = (a >> 16) & 0xff;
    out[3] = (a >> 24) & 0xff;
    out[4] = (a >> 32) & 0xff;
    out[5] = (a >> 40) & 0xff;
    out[6] = (a >> 48) & 0xff;
    out[7] = (a >> 56) & 0xff;
}



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

        while (*buff) {
            while (isspace(*buff)) buff++;
            if (*buff == '\0' || *buff == '\n') break;
            if (*buff == '/' && *(buff + 1) == '/') break;
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
            else if (isin(LET, *buff)) {
                char buf[256] = {0};
                int i = 0;
                while (isin(*buff, LETEXT)) {
                    buf[i++] = *buff++;
                }

                if (*buff == ':') {
                    buff++;
                    strcpy(lexer.labelscope, buf);
                    add_token(0x02, buf, line);
                } else {
                    add_token(0x0a, buf, line);
                }
            }

            else if (*buff == '.') {
                buff++;
                char buf[256] = {0};
                int i = 0;
                while (isin(*buff, LETEXT)) {
                    buf[i++] = *buff++;
                }

                char full[512] = {0};
                snprintf(full, sizeof(full), "%s.%s", lexer.labelscope, buf);

                if (*buff == ':') {
                    buff++;
                    add_token(0x02, full, line);
                } else {
                    if(is_inst(buf))add_token(0x00, full, line);
                    else add_token(0x0a, full, line);
                }
            }
            else if (is_reg(buff) != 0) {
                char buf[12];
                int i = 0;
                while (!isspace(*buff) && *buff != '\0') buf[i++] = *buff++;
                buf[i] = '\0';

                int regtype = is_reg(buf);
                if (regtype == 1) add_token(0x03, buf, line);
                else if (regtype == 2) add_token(0x04, buf, line);
                else if (regtype == 3) add_token(0x05, buf, line);
                else if (regtype == 4) add_token(0x06, buf, line);
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
                add_token(0x01, strdup(val_str), line);
            }
            else if(*buff == '"'){
                buff++;

                char parsed_str[256] = {0};
                buff = read_string(buff, parsed_str, line);

                if(*buff == '"') *buff++;
                else {
                    fprintf(stderr, "AmmAsm: unterminated string literal at line %d\n", line);
                    exit(1);
                }
                add_token(0x0b, parsed_str, line);
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
                         buff++;
                char tmp2[2] = {tmp, '\0'};
                add_token(0x17, tmp2, line);
            }
            else if(*buff == '0'){
                char tmp[72];
                long long c = 0;
                buff++;
                if(*buff == 'x'){
                    buff++;
                    while (isin(LETEXT, *buff)) {
                        if (*buff >= '0' && *buff <= '9') c = c * 16 + (*buff - '0');
                        else if (*buff >= 'a' && *buff <= 'f') c = c * 16 + (*buff - 'a' + 10);
                        else if (*buff >= 'A' && *buff <= 'F') c = c * 16 + (*buff - 'A' + 10);
                        else break;

                        buff++;
                    }
                    snprintf(tmp, sizeof(tmp), "%d", c);
                    add_token(0x01, tmp, line);
                }
                else if(*buff == 'b'){
                    while(isin(DIGBIN, *buff)){
                        c = c * 2 + (*buff - '0');
                        buff++;
                    }
                    snprintf(tmp, sizeof(tmp), "%lld", c);
                    add_token(0x01, tmp, line);
                }
                else if (*buff == 'o') {
                    while (isin(DIGOCT, *buff)) {
                        c = c * 8 + (*buff - '0');
                        buff++;
                    }
                    snprintf(tmp, sizeof(tmp), "%lld", c);
                    add_token(0x01, tmp, line);
                }
                else {
                    snprintf(tmp, sizeof(tmp), "%lld", 0);
                    add_token(0x01, tmp, line);
                }
            }
            else if (*buff == '[') {
                buff++;

                char expr_buf[128] = {0};
                int i = 0;

                while (*buff && *buff != ']') {
                    if (i < (int)(sizeof(expr_buf) - 1)) {
                        expr_buf[i++] = *buff;
                    }
                    buff++;
                }

                if (*buff == ']') buff++;
                expr_buf[i] = '\0';


                char clean_expr[128] = {0};
                int k = 0;
                for (int j = 0; expr_buf[j]; j++) {
                    if (!isspace(expr_buf[j])) {
                        clean_expr[k++] = expr_buf[j];
                    }
                }

                add_token(0x09, strdup(clean_expr), line);
            }
            else if (*buff == '!') {
                buff++;
                if (strncmp(buff, "section", 7) == 0) {
                    buff += 7;
                    while (*buff == ' ' || *buff == '\t') buff++;

                    char section[12] = {0};
                    int i = 0;

                    while (*buff && *buff != '\n' && i < (int)(sizeof(section) - 1)) {
                        section[i++] = *buff++;
                    }

                    add_token(0x0e, section, line);
                }
            }
            else if(*buff == ','){
                buff++;
                while(isspace(*buff)) buff++;
                add_token(0x18, ",", line);
            }
            else if(isin(LET, *buff)){
                char buffer[56] = {0};
                int i=0;
                while (isin(LET, *buff) && i < (int)(sizeof(buffer)-1)){
                    buffer[i] = *buff++;
                }
                if(strcmp(buffer, HUMAN_AST[0]) == 0) add_token(0x0f, buffer, line);
                else if(strcmp(buffer, HUMAN_AST[1]) == 0) add_token(0x10, buffer, line);
                else if(strcmp(buffer, HUMAN_AST[2]) == 0) add_token(0x11, buffer, line);
                else if(strcmp(buffer, HUMAN_AST[3]) == 0) add_token(0x12, buffer, line);
                else if(strcmp(buffer, HUMAN_AST[4]) == 0) add_token(0x13, buffer, line);
                else if(strcmp(buffer, HUMAN_AST[5]) == 0) add_token(0x14, buffer, line);
                else if(strcmp(buffer, HUMAN_AST[6]) == 0) add_token(0x15, buffer, line);
                else if(strcmp(buffer, HUMAN_AST[7]) == 0) add_token(0x16, buffer, line);
                else add_token(0x0a, buffer, line);
            }
            else if (strncmp(buff, "resb", 4) == 0){ add_token(0x19, buff, line); buff += 4; while (isspace(*buff)) buff++;}
            else if (strncmp(buff, "resq", 4) == 0){ add_token(0x1a, buff, line); buff += 4; while (isspace(*buff)) buff++;}
            else if (strncmp(buff, "resd", 4) == 0){ add_token(0x1b, buff, line); buff += 4; while (isspace(*buff)) buff++;}
            else if (strncmp(buff, "resl", 4) == 0){ add_token(0x1c, buff, line); buff += 4; while (isspace(*buff)) buff++;}

            else{ fprintf(stderr, "AmmAsm: unvalid syntax or char on line", line); exit(1); }
            buff++;
        }

        add_token(0x0c, "\\n", line);
    }
    add_token(0x0d, "\x7FELF", line);
    return 0;
}

typedef enum {
    AST_INS,
    AST_BYTES,
    AST_BYTEPTRS,
    AST_QWORDS,
    AST_QWORDPTRS,
    AST_DWORDS,
    AST_DWORDPTRS,
    AST_LBYTES,
    AST_LBYTEPTRS,
    AST_RESB,
    AST_RESQ,
    AST_RESD,
    AST_RESL,
    AST_LABEL,
    AST_COMMENT,
    AST_SECTION,
    AST_COMMA,
    AST_UNKNOWN
} ASTType;

typedef struct AST {
    int type;
    char cmd[256];

    union {
        struct { char operand1[64]; char operand2[64]; char operand3[64]; char operand4[64]; int oper_count} ins;
        struct { unsigned char data[256]; int size; } bytes;
        struct { unsigned char *data[256]; int size; } byteptrs;
        struct { short data[256]; int size; } qwords;
        struct { short *data[256]; int size; } qwordptrs;
        struct { int data[256]; int size; } dwords;
        struct { int *data[256]; int size; } dwordptrs;
        struct { long data[256]; int size; } lbytes;
        struct { long *data[256]; int size; } lbyteptrs;
        struct { long size; } resb;
        struct { long size; } resq;
        struct { long size; } resd;
        struct { long size; } resl;
        struct { char name[64]; int adress; } label;
        struct { char name[63]; } section;
    };
} AST;


AST* PARSE(){
    int pos = 0;
    int pc = 0;

    while(toks[pos].type != 0x0d){
        Token *tok = &toks[pos];
        if (tok->type == 0x00) {

        }

        if(tok->type == 0x02){
            AST node = { .type = AST_LABEL };
            if(tok->type != 0x0d && tok->type != 0x0c){
                strncpy(node.label.name, toks[pos++].value, sizeof(node.label.name));
                strncpy(node.cmd, toks[pos++].value, sizeof(node.cmd));
            }
            ast[ast_count++] = node;
            ++pos;
            continue;
        }
        if(tok->type == 0x0f && (pos == 0 || toks[pos-1].type == 0x0a)){
            AST node = { .type = AST_BYTES};
            node.bytes.size = 0;
            while(pos < toks_count){
                if (toks[pos].type == 0x0c || toks[pos].type == 0x0d) break;
                if(toks[pos].type == 0x18){ pos++; continue;}
                if (node.bytes.size >= 256) { fprintf(stderr, "AmmAsm: Too many bytes in string literal at line %d\n", toks[pos].line); exit(1); }
                if(toks[pos].type == 0x17){
                    node.bytes.data[node.bytes.size++] = (unsigned char)toks[pos].value[0];
                    ++pc;
                    pos++;
                }
                else if(toks[pos].type == 0x01){
                    node.bytes.data[node.bytes.size++] = (unsigned char)atoi(toks[pos].value);
                    ++pc;
                    pos++;
                }
                else if(toks[pos].type == 0x0b){
                    char *s = toks[pos].value;
                    while (*s) {
                    if (node.bytes.size >= sizeof(node.bytes.data)){ fprintf(stderr, "AmmAsm: String too long in BYTE directive at line %d\n", toks[pos].line); exit(1);}
                        node.bytes.data[node.bytes.size++] = (unsigned char)*s++;
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
            if (pos < toks_count && toks[pos].type == 0x0c) ++pos;
            continue;
        }
        else if (tok->type == 0x10 && (pos == 0 || toks[pos-1].type == 0x0a)) {
            AST node = { .type = AST_BYTEPTRS };
            node.byteptrs.size = 0;
            pos++;

            while (pos < toks_count) {
                if (toks[pos].type == 0x0c || toks[pos].type == 0x0d) break;

                if (toks[pos].type == 0x18) { pos++; continue; }
                if (toks[pos].type == 0x0a) {
                    if (node.byteptrs.size >= 256) {
                        fprintf(stderr, "AmmAsm: too many BYTEPTRS (max. 256)\n");
                        exit(1);
                    }
                    node.byteptrs.data[node.byteptrs.size++] = (char *)toks[pos].value;

                    pc += sizeof(char *);
                    pos++;
                }
                else if (toks[pos].type == 0x01 && atoi(toks[pos].value) == 0) {
                    if (node.byteptrs.size >= 256) {
                        fprintf(stderr, "AmmAsm: too many BYTEPTRS (max. 256)\n");
                        exit(1);
                    }
                    node.byteptrs.data[node.byteptrs.size++] = ((void*)0);
                    pc += sizeof(char *);
                    pos++;
                }
                else {
                    fprintf(stderr, "AmmAsm: expected label name (T_ID) or 0 (NULL) in BYTEPTRS at line %d\n", toks[pos].line);
                    exit(1);
                }
            }


            ast[ast_count++] = node;
            if(pos < toks_count && toks[pos].type == 0x0c) ++pos;
            continue;
        }
        else if(tok->type == 0x11 && (pos == 0 || toks[pos-1].type == 0x0a)){
            AST node = { .type = AST_QWORDS};
            node.qwords.size = 0;
            while(pos < toks_count){
                if (toks[pos].type == 0x0c || toks[pos].type == 0x0d) break;
                if(toks[pos].type == 0x18){ pos++; continue;}
                if(node.qwords.size >= 256) fprintf(stderr, "AmmAsm: to many qword leterals at line \"%d\"", toks[pos].line);

                if(toks[pos].type == 0x01){
                    node.qwords.data[node.qwords.size++] = (short)atoi(toks[pos].value);
                    pc += sizeof(short);
                    ++pos;
                }
                else {
                    fprintf(stderr, "AmmAsm: unexpected token in QWORD at line %d: '%s'\n", toks[pos].line, toks[pos].value);
                    exit(1);
                }
            }
            ast[ast_count++] = node;
            if(pos < toks_count && toks[pos].type == 0x0c) ++pos;
            continue;
        }
        else if(tok->type == 0x12 && (pos == 0 || toks[pos-1].type == 0x0a)){
            AST node = { .type = AST_QWORDPTRS };
            node.qwordptrs.size = 0;
            ++pos;

            while (pos < toks_count){
                if(toks[pos].type == 0x0c || toks[pos].type == 0x0d)break;
                if(toks[pos].type == 0x18){ ++pos; continue; }
                if(node.qwordptrs.size >= 256){ fprintf(stderr, "AmmAsm: Too many qwordptrs at line \"%d\"", toks[pos].line); exit(1);}

                if(toks[pos].type == 0x0a){
                    node.qwordptrs.data[node.qwordptrs.size++] = toks[pos].value;
                    pc += 8;
                    ++pos;
                }
                else if(toks[pos].type == 0x01 && atoi(toks[pos].value) == 0){
                    node.qwordptrs.data[node.qwordptrs.size] = ((void*)0);
                    pc += 8;
                    pos++;
                }
                else {
                    fprintf(stderr, "AmmAsm: expected label name (T_ID) or 0 (NULL) in QWORDPTRS at line %d\n", toks[pos].line);
                    exit(1);
                }
            }
            ast[ast_count++] = node;
            if(pos < toks_count && toks[pos].type == 0x0c) ++pos;
            continue;
        }
        else if (tok->type == 0x13 && (pos == 0 || toks[pos-1].type == 0x0a)) {
            AST node = { .type = AST_DWORDS };
            node.dwords.size = 0;
            pos++;

            while (pos < toks_count && toks[pos].type != 0x0c && toks[pos].type != 0x0d) {
                if (toks[pos].type == 0x18){ pos++; continue; }
                if (node.dwords.size >= 256){ fprintf(stderr, "AmmAsm: Too many dwords at line %d\n", toks[pos].line); exit(1);}
                if (toks[pos].type == 0x01) {
                    node.dwords.data[node.dwords.size++] = atoi(toks[pos].value);
                    pc += sizeof(int);
                    pos++;
                }
                else {
                    fprintf(stderr, "AmmAsm: unexpected token in DWORD at line %d: '%s'\n", toks[pos].line, toks[pos].value);
                    exit(1);
                }
            }

            ast[ast_count++] = node;
            if (pos < toks_count && toks[pos].type == 0x0c) pos++;
        }
        else if(tok->type == 0x14 && (pos == 0 || toks[pos-1].type == 0x0a)){
            AST node = { .type = AST_DWORDPTRS };
            node.dwordptrs.size = 0;
            ++pos;

            while (pos < toks_count){
                if(toks[pos].type == 0x0c || toks[pos].type == 0x0d)break;
                if(toks[pos].type == 0x18){ ++pos; continue; }
                if(node.dwordptrs.size >= 256){ fprintf(stderr, "AmmAsm: Too many dwordptrs at line \"%d\"", toks[pos].line); exit(1);}

                if(toks[pos].type == 0x0a){
                    node.dwordptrs.data[node.dwordptrs.size++] = toks[pos].value;
                    pc += 8;
                    ++pos;
                }
                else if(toks[pos].type == 0x01 && atoi(toks[pos].value) == 0){
                    node.dwordptrs.data[node.dwordptrs.size] = ((void*)0);
                    pc += 8;
                    pos++;
                }
                else {
                    fprintf(stderr, "AmmAsm: expected label name (T_ID) or 0 (NULL) in DWORDPTRS at line %d\n", toks[pos].line);
                    exit(1);
                }
            }
            ast[ast_count++] = node;
            if(pos < toks_count && toks[pos].type == 0x0c) ++pos;
            continue;
        }
        else if (tok->type == 0x15 && (pos == 0 || toks[pos-1].type == 0x0a)) {
            AST node = { .type = AST_LBYTES };
            node.lbytes.size = 0;
            pos++;

            while (pos < toks_count && toks[pos].type != 0x0c && toks[pos].type != 0x0d) {
                if (toks[pos].type == 0x18){ pos++; continue; }
                if (node.lbytes.size >= 256){ fprintf(stderr, "AmmAsm: Too many lbytes at line %d\n", toks[pos].line); exit(1);}
                if (toks[pos].type == 0x01) {
                    node.lbytes.data[node.lbytes.size++] = atoi(toks[pos].value);
                    pc += sizeof(long);
                    pos++;
                }
                else {
                    fprintf(stderr, "AmmAsm: unexpected token in LBYTE at line %d: '%s'\n", toks[pos].line, toks[pos].value);
                    exit(1);
                }
            }

            ast[ast_count++] = node;
            if (pos < toks_count && toks[pos].type == 0x0c) pos++;
        }
        else if(tok->type == 0x16 && (pos == 0 || toks[pos-1].type == 0x0a)){
            AST node = { .type = AST_LBYTEPTRS };
            node.lbyteptrs.size = 0;
            ++pos;

            while (pos < toks_count){
                if(toks[pos].type == 0x0c || toks[pos].type == 0x0d)break;
                if(toks[pos].type == 0x18){ ++pos; continue; }
                if(node.lbyteptrs.size >= 256){ fprintf(stderr, "AmmAsm: Too many lbyteptrs at line \"%d\"", toks[pos].line); exit(1);}

                if(toks[pos].type == 0x0a){
                    node.lbyteptrs.data[node.lbyteptrs.size++] = toks[pos].value;
                    pc += 8;
                    ++pos;
                }
                else if(toks[pos].type == 0x01 && atoi(toks[pos].value) == 0){
                    node.lbyteptrs.data[node.lbyteptrs.size] = ((void*)0);
                    pc += 8;
                    pos++;
                }
                else {
                    fprintf(stderr, "AmmAsm: expected label name (T_ID) or 0 (NULL) in LBYTEPTRS at line %d\n", toks[pos].line);
                    exit(1);
                }
            }
            ast[ast_count++] = node;
            if(pos < toks_count && toks[pos].type == 0x0c) ++pos;
            continue;
        }

        else if(tok->type == 0x19 || tok->type == 0x1a || tok->type == 0x1b || tok->type == 0x1c){
            Token *pp = tok;
            if(toks[(pos-1)].type != 0x0a || toks[(pos+1)].type != 0x01){
                fprintf(stderr, "AmmAsm: syntax erorr. expected identifier before \"%s\" and decimal number after\n",
                    (pp->type == 0x19) ? "resb" :
                    (pp->type == 0x1a) ? "resq" :
                    (pp->type == 0x1b) ? "resd" :
                    (pp->type == 0x1c) ? "resl" : "?");
                exit(1);
            }


            AST node = { 0 };

            switch (tok->type){
            case 0x19: node.type = AST_RESB; node.resb.size = (long)eval_expr(toks[pos].value); break;
            case 0x1a: node.type = AST_RESQ; node.resq.size = (long)eval_expr(toks[pos].value); break;
            case 0x1b: node.type = AST_RESD; node.resd.size = (long)eval_expr(toks[pos].value); break;
            case 0x1c: node.type = AST_RESL; node.resl.size = (long)eval_expr(toks[pos].value); break;
            default: break;
            }
            pos += 1;

            ast[ast_count++] = node;
            if(pos < toks_count && toks[pos].type == 0x0c) ++pos;
            continue;
        }


    }
}
