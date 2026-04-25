#pragma ones
 
#include <stdint.h>
#include <stdio.h>
 
/* == forward declarations == */
typedef enum {
    AST_INT,
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
    AST_CHAR,
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
    O_LABEL,
    O_CHAR,
    O_ID
} OperandType;

// will be added more in future version
typedef enum {
    SEC_DATA,
    SEC_TEXT
} SECtype;

typedef struct {
    // must have
    uint8_t base;
    uint8_t index;
    uint8_t scale;
    int32_t disp;

    uint8_t have_base;
    uint8_t have_index;
    uint8_t have_disp;

    uint8_t is_rip_rel;
    uint8_t label[64];

} AddrExpr;


typedef struct AST {
    ASTType type;
    char cmd[256];  // mostly useing for ins

    union {     
        struct { uint8_t operands[4][35]; OperandType otype[4]; int oper_count; int resolved; AddrExpr expr;} ins; // resolved -> for linking symbols. expr -> for sib
        struct { uint8_t data[256]; int size; } u8;
        struct { uint8_t *data[256]; int size; } u8ptr;
        struct { uint16_t data[256]; int size; } u16;
        struct { uint8_t *data[256]; int size; } u16ptr;
        struct { uint32_t data[256]; int size; } u32;
        struct { uint8_t *data[256]; int size; } u32ptr;
        struct { uint64_t data[256]; int size; } u64;
        struct { uint8_t *data[256]; int size; } u64ptr;
        struct { uint64_t size; } resb;
        struct { uint64_t size; } resq;
        struct { uint64_t size; } resd;
        struct { uint64_t size; } resl;
        struct { uint8_t c;     } chr;
        struct { uint8_t name[64]; uint64_t adress; uint64_t vadress; uint8_t defined;} label; // only can be in .text section
        struct { uint8_t name[64]; uint64_t offset; uint32_t size; SECtype type;} section;
    };
        uint8_t machine_code[64];
        uint8_t machine_code_size;
} AST;



typedef struct {
    int   type;       
    char* value;      
    int   line;       // for errors
} Token;

typedef struct {   
    const char* filename;  
    char        buf[256]; 
    Token*      toks;
    char labelscope[256]; 
} Lexer;

/* == utility == */
int    isin(const char *str, char c);
int    is2arrin(const char *str[], char *str2);
 
/* == expression evaluator == */
long   parse_number(void);
long   parse_term(void);
long   parse_expr(void);
long   eval_expr(const uint8_t *str);
 
/* == string / char helpers == */
char  *read_string(char *buff, char *dest, int line);
 
/* == little-endian serialisers == */
void   As16(uint16_t a, unsigned char out[2]);
void   As32(uint32_t a, unsigned char out[4]);
void   As64(uint64_t a, unsigned char out[8]);
 
/* == token management == */
void   add_token(int type, char *value, int line);
void   del_all_toks(void);
void   DEBUG_PRINT_TOKENS(void);
 
/* == register helpers == */
uint8_t find_reg64_index(const char *reg);
uint8_t find_reg32_index(const char *reg);
uint8_t find_reg16_index(const char *reg);
uint8_t find_reg8_index(const char *reg);
 
/* == address-expression parser == */
AddrExpr parse_addr_expr(const uint8_t *expr);
 
/* == instruction encoders == */
uint8_t encode_mov_reg_imm(uint8_t *mash_code, uint8_t reg_idx, uint64_t imm, uint8_t sz);
uint8_t encode_mov_reg_reg(uint8_t *mash_code, uint8_t dest_idx, uint8_t src_idx, uint8_t sz);
uint8_t encode_inst_rm_rm(uint8_t *mash_code, uint8_t reg_idx, AddrExpr *expr, uint8_t sz, uint8_t opcode);
uint8_t encode_add_reg_reg(uint8_t *mash_code, uint8_t dest, uint8_t src, uint8_t sz);

/* == lexer == */
int    LEXER(FILE *fl);
 
/* == parser == */
AST   *PARSE(void);
 
/* == label resolver / linker == */
uint64_t collect_labels(void);
void     resolve_labels(void);
 
/* == code-gen passes == */
void parseInst(AST *node);
void parse_size_directives(AST *node);
 
/* == ELF writer == */
void ELFgenfile(FILE *fl, uint64_t e_entry, uint8_t *text_code, uint64_t text_size);
 
/* == top-level pipeline == */
void compiler(uint8_t *text, int *textsize, uint64_t *e_entry);
void handl_pipeline(int argc, char **argv);
 
/* == debug == */
void DEBUG_PRINT_AST(void);
 

 