#ifndef AMMASM_H
#define AMMASM_H 2

char** file;

extern int parse_num();
extern int parse_term();
extern int parse_expr();
extern int eval_expr(const char *str);



#endif