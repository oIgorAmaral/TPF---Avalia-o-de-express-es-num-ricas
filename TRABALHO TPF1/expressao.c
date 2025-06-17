// expressao.c (Corrigido com strtok e Funções Auxiliares Traduzidas)
#include "expressao.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para strtok, strcmp, strlen, etc.
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Funções auxiliares traduzidas
int ehEspaco(char c) { return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v'); }
int ehDigito(char c) { return (c >= '0' && c <= '9'); }
int ehLetra(char c) { return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); }

#define CHAR_STACK_SIZE 256
typedef struct { char items[CHAR_STACK_SIZE]; int top; } PilhaChar;
void iniciarPilhaChar(PilhaChar *s) { s->top = -1; }
int pilhaCharVazia(PilhaChar *s) { return s->top == -1; }
void empilharChar(PilhaChar *s, char item) { if (s->top < CHAR_STACK_SIZE - 1) s->items[++(s->top)] = item; else fprintf(stderr, "Err:PilhaCharCheia\n"); }
char desempilharChar(PilhaChar *s) { if (!pilhaCharVazia(s)) return s->items[(s->top)--]; fprintf(stderr, "Err:PilhaCharVaziaPop\n"); return '\0'; }
char topoPilhaChar(PilhaChar *s) { if (!pilhaCharVazia(s)) return s->items[s->top]; return '\0'; }

#define FLOAT_STACK_SIZE 256
typedef struct { float items[FLOAT_STACK_SIZE]; int top; } PilhaFloat;
void iniciarPilhaFloat(PilhaFloat *s) { s->top = -1; }
int pilhaFloatVazia(PilhaFloat *s) { return s->top == -1; }
void empilharFloat(PilhaFloat *s, float item) { if (s->top < FLOAT_STACK_SIZE - 1) s->items[++(s->top)] = item; else fprintf(stderr, "Err:PilhaFloatCheia\n"); }
float desempilharFloat(PilhaFloat *s) { if (!pilhaFloatVazia(s)) return s->items[(s->top)--]; fprintf(stderr, "Err:PilhaFloatVaziaPop\n"); return NAN; }

#define STRING_STACK_SIZE 64
#define MAX_SUB_EXPR_LEN 128
typedef struct { char items[STRING_STACK_SIZE][MAX_SUB_EXPR_LEN]; int top; } PilhaString;
void iniciarPilhaString(PilhaString *s) { s->top = -1; }
int pilhaStringVazia(PilhaString *s) { return s->top == -1; }
void empilharString(PilhaString *s, const char *item) {
    if (s->top < STRING_STACK_SIZE - 1) {
        if (strlen(item) < MAX_SUB_EXPR_LEN) strcpy(s->items[++(s->top)], item);
        else fprintf(stderr, "Err:SubExprStringLonga\n");
    } else fprintf(stderr, "Err:PilhaStringCheia\n");
}
char* desempilharString(PilhaString *s) { if (!pilhaStringVazia(s)) return s->items[(s->top)--]; fprintf(stderr, "Err:PilhaStringVaziaPop\n"); return NULL; }

int ehOperador(char c) { return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^'; }
int obterPrecedencia(const char *op) {
    if (strlen(op) > 1) return 4; 
    char o = op[0];
    if (o == '^') return 3;
    if (o == '*' || o == '/' || o == '%') return 2;
    if (o == '+' || o == '-') return 1;
    return 0;
}
int ehAssociativoEsquerda(const char *op) { 
    // Para o Shunting-Yard, a associatividade do operador no topo da pilha é relevante.
    // '^' é geralmente à direita, o resto à esquerda.
    if (strlen(op) == 1 && op[0] == '^') return 0; // Potência é associativa à direita
    return 1; // Outros operadores (+, -, *, /, %) são associativos à esquerda
}
double grausParaRadianos(double d) { return d * (M_PI / 180.0); }

const char* traduzirCharFuncParaNome(char fc, char* nome_buf, size_t buf_size) {
    const char* nome = NULL;
    if (fc == 'S') nome = "sen"; else if (fc == 'C') nome = "cos";
    else if (fc == 'T') nome = "tg"; else if (fc == 'L') nome = "log";
    else if (fc == 'R') nome = "raiz";
    if (nome) { strncpy(nome_buf, nome, buf_size -1); nome_buf[buf_size-1] = '\0'; return nome_buf; }
    return NULL;
}

int identificarFuncaoUnaria(const char* token, char* nome_func_saida, int max_len) {
    const char* funcoes[] = {"sen", "cos", "tg", "log", "raiz"};
    for (int i = 0; i < 5; ++i) {
        if (strcmp(token, funcoes[i]) == 0) {
            if (nome_func_saida) { strncpy(nome_func_saida, token, max_len - 1); nome_func_saida[max_len - 1] = '\0';}
            return 1;
        }
    }
    return 0;
}

char *getFormaPosFixa(char *StrInfixa) {
    if (!StrInfixa) return NULL;
    char *base_posfixa = (char *)malloc(strlen(StrInfixa) * 2 + 2);
    if (!base_posfixa) return NULL;
    char *p_saida = base_posfixa; *p_saida = '\0';

    PilhaChar pilha_op; iniciarPilhaChar(&pilha_op);
    char token[64], str_op_temp[10], buf_nome_func[10];
    int k = 0;

    while (StrInfixa[k] != '\0') {
        if (ehEspaco(StrInfixa[k])) { k++; continue; }

        if (ehDigito(StrInfixa[k]) || (StrInfixa[k] == '.' && ehDigito(StrInfixa[k+1]))) {
            int t = 0;
            while (ehDigito(StrInfixa[k]) || StrInfixa[k] == '.') token[t++] = StrInfixa[k++];
            token[t] = '\0';
            p_saida += sprintf(p_saida, "%s ", token);
        } else if (ehLetra(StrInfixa[k])) {
            int t = 0;
            while (ehLetra(StrInfixa[k])) token[t++] = StrInfixa[k++];
            token[t] = '\0';
            char char_rep_func = 0;
            if (strcmp(token, "sen") == 0) char_rep_func = 'S'; else if (strcmp(token, "cos") == 0) char_rep_func = 'C';
            else if (strcmp(token, "tg") == 0) char_rep_func = 'T'; else if (strcmp(token, "log") == 0) char_rep_func = 'L';
            else if (strcmp(token, "raiz") == 0) char_rep_func = 'R';
            
            if (char_rep_func) empilharChar(&pilha_op, char_rep_func);
            else { fprintf(stderr, "Err:FuncInvalida:%s\n", token); free(base_posfixa); return NULL; }
        } else if (ehOperador(StrInfixa[k])) {
            str_op_temp[0] = StrInfixa[k]; str_op_temp[1] = '\0'; // Operador atual da entrada
            while (!pilhaCharVazia(&pilha_op) && topoPilhaChar(&pilha_op) != '(') {
                char char_op_topo_pilha = topoPilhaChar(&pilha_op);
                char buf_str_op_topo[10] = {0}; 
                const char* repr_op_topo_pilha = buf_str_op_topo; 
                
                if (traduzirCharFuncParaNome(char_op_topo_pilha, buf_str_op_topo, sizeof(buf_str_op_topo))) {
                    // repr_op_topo_pilha já aponta para buf_str_op_topo que contém o nome da função
                } else { // É um operador simples
                    buf_str_op_topo[0] = char_op_topo_pilha; 
                    buf_str_op_topo[1] = '\0';
                }

                // Compara precedência do operador no topo da pilha com o operador atual da entrada
                // Se o do topo tem maior precedência, OU
                // Se têm igual precedência E o do topo é associativo à esquerda, desempilha.
                if ( (obterPrecedencia(repr_op_topo_pilha) > obterPrecedencia(str_op_temp)) ||
                     (obterPrecedencia(repr_op_topo_pilha) == obterPrecedencia(str_op_temp) && ehAssociativoEsquerda(repr_op_topo_pilha)) 
                   ) {
                    char op_da_pilha = desempilharChar(&pilha_op);
                    if (traduzirCharFuncParaNome(op_da_pilha, buf_nome_func, sizeof(buf_nome_func))) {
                        p_saida += sprintf(p_saida, "%s ", buf_nome_func);
                    } else {
                        p_saida += sprintf(p_saida, "%c ", op_da_pilha);
                    }
                } else break; // Operador atual tem maior precedência ou o do topo é associativo à direita com mesma precedência
            }
            empilharChar(&pilha_op, StrInfixa[k++]); // Empilha o operador atual da entrada
        } else if (StrInfixa[k] == '(') {
            empilharChar(&pilha_op, StrInfixa[k++]);
        } else if (StrInfixa[k] == ')') {
            while (!pilhaCharVazia(&pilha_op) && topoPilhaChar(&pilha_op) != '(') {
                char op_da_pilha = desempilharChar(&pilha_op);
                 if (traduzirCharFuncParaNome(op_da_pilha, buf_nome_func, sizeof(buf_nome_func))) {
                    p_saida += sprintf(p_saida, "%s ", buf_nome_func);
                } else {
                    p_saida += sprintf(p_saida, "%c ", op_da_pilha);
                }
            }
            if (pilhaCharVazia(&pilha_op) || desempilharChar(&pilha_op) != '(') { // Desempilha '('
                fprintf(stderr, "Err:ParenDesbalanceado\n"); free(base_posfixa); return NULL;
            }
            // Se o token no topo da pilha (após o '(') for uma função, desempilha
            if(!pilhaCharVazia(&pilha_op) && traduzirCharFuncParaNome(topoPilhaChar(&pilha_op), buf_nome_func, sizeof(buf_nome_func))) {
                 p_saida += sprintf(p_saida, "%s ", buf_nome_func);
                 desempilharChar(&pilha_op);
            }
            k++;
        } else { fprintf(stderr, "Err:CharInvalido:%c\n", StrInfixa[k]); free(base_posfixa); return NULL; }
    }

    while (!pilhaCharVazia(&pilha_op)) {
        char op = desempilharChar(&pilha_op);
        if (op == '(') { fprintf(stderr, "Err:ParenDesbalanceado\n"); free(base_posfixa); return NULL; }
        if (traduzirCharFuncParaNome(op, buf_nome_func, sizeof(buf_nome_func))) {
             p_saida += sprintf(p_saida, "%s ", buf_nome_func);
        } else {
             p_saida += sprintf(p_saida, "%c ", op);
        }
    }
    if (p_saida > base_posfixa && *(p_saida-1) == ' ') *(p_saida-1) = '\0'; 
    else *p_saida = '\0';
    return base_posfixa;
}

char *getFormaInFixa(char *StrPosFixa) {
    if (!StrPosFixa) return NULL;
    PilhaString pilha; iniciarPilhaString(&pilha);
    // strtok modifica a string original, então trabalhamos com uma cópia.
    char *copia_str = strdup(StrPosFixa); 
    if (!copia_str) { perror("Erro ao alocar memória para copia_str em getFormaInFixa"); return NULL; }
    
    char *token;
    char expr_temp[MAX_SUB_EXPR_LEN], nome_func[10];

    // Primeira chamada a strtok
    token = strtok(copia_str, " ");
    while (token != NULL) {
        if (ehDigito(token[0]) || (token[0] == '-' && strlen(token) > 1 && ehDigito(token[1])) || token[0] == '.') {
            empilharString(&pilha, token);
        } else if (identificarFuncaoUnaria(token, nome_func, sizeof(nome_func))) {
            if (pilhaStringVazia(&pilha)) { fprintf(stderr, "Err:SintaxePosfixa(func):%s\n", token); free(copia_str); return NULL; }
            snprintf(expr_temp, MAX_SUB_EXPR_LEN, "%s(%s)", nome_func, desempilharString(&pilha));
            empilharString(&pilha, expr_temp);
        } else if (strlen(token) == 1 && ehOperador(token[0])) {
            if (pilhaStringVazia(&pilha)) { fprintf(stderr, "Err:SintaxePosfixa(op2):%c\n", token[0]); free(copia_str); return NULL; }
            char *op2 = desempilharString(&pilha);
            if (pilhaStringVazia(&pilha)) { fprintf(stderr, "Err:SintaxePosfixa(op1):%c\n", token[0]); free(copia_str); return NULL; }
            char *op1 = desempilharString(&pilha);
            snprintf(expr_temp, MAX_SUB_EXPR_LEN, "(%s %c %s)", op1, token[0], op2);
            empilharString(&pilha, expr_temp);
        } else { fprintf(stderr, "Err:TokenPosfixaInv:%s\n", token); free(copia_str); return NULL; }
        // Próximas chamadas a strtok para a mesma string original
        token = strtok(NULL, " ");
    }

    if (pilha.top != 0) { fprintf(stderr, "Err:PosfixaMalFormada\n"); free(copia_str); return NULL; }
    
    char* resultado = strdup(pilha.items[0]); 
    free(copia_str); // Libera a cópia da string que foi modificada por strtok
    return resultado;
}

float getValorPosFixa(char *StrPosFixa) {
    if (!StrPosFixa) return NAN;
    PilhaFloat pilha; iniciarPilhaFloat(&pilha);
    // strtok modifica a string original, então trabalhamos com uma cópia.
    char *copia_str = strdup(StrPosFixa);
    if (!copia_str) { perror("Erro ao alocar memória para copia_str em getValorPosFixa"); return NAN; }

    char *token;
    char nome_func[10]; 
    float v1, v2, res;

    // Primeira chamada a strtok
    token = strtok(copia_str, " ");
    while (token != NULL) {
        if (ehDigito(token[0]) || (token[0] == '-' && strlen(token) > 1 && ehDigito(token[1])) || token[0] == '.') {
            empilharFloat(&pilha, atof(token));
        } else if (identificarFuncaoUnaria(token, nome_func, sizeof(nome_func))) {
            if (pilhaFloatVazia(&pilha)) { fprintf(stderr, "Err:FaltaOperando(func):%s\n", token); free(copia_str); return NAN; }
            v1 = desempilharFloat(&pilha);
            if (strcmp(nome_func,"log")==0) res=(v1<=0)?(fprintf(stderr,"Err:LogNumNaoPos\n"),NAN):log10f(v1);
            else if (strcmp(nome_func,"sen")==0) res=sinf(grausParaRadianos(v1));
            else if (strcmp(nome_func,"cos")==0) res=cosf(grausParaRadianos(v1));
            else if (strcmp(nome_func,"tg")==0) res=(cosf(grausParaRadianos(v1))==0.0f)?(fprintf(stderr,"Err:TgIndef\n"),NAN):tanf(grausParaRadianos(v1));
            else if (strcmp(nome_func,"raiz")==0) res=(v1<0)?(fprintf(stderr,"Err:RaizNumNeg\n"),NAN):sqrtf(v1);
            else { fprintf(stderr, "Err:FuncEvalInv:%s\n", nome_func); free(copia_str); return NAN; }
            empilharFloat(&pilha, res);
        } else if (strlen(token) == 1 && ehOperador(token[0])) {
            if (pilhaFloatVazia(&pilha)) { fprintf(stderr, "Err:FaltaOperando(op2):%c\n",token[0]); free(copia_str); return NAN; } v2 = desempilharFloat(&pilha);
            if (pilhaFloatVazia(&pilha)) { fprintf(stderr, "Err:FaltaOperando(op1):%c\n",token[0]); free(copia_str); return NAN; } v1 = desempilharFloat(&pilha);
            switch (token[0]) {
                case '+': res=v1+v2; break; case '-': res=v1-v2; break; case '*': res=v1*v2; break;
                case '/': res=(v2==0.0f)?(fprintf(stderr,"Err:DivPorZero\n"),NAN):v1/v2; break;
                case '%': res=(v2==0.0f)?(fprintf(stderr,"Err:ModPorZero\n"),NAN):fmodf(v1,v2); break;
                case '^': res=powf(v1,v2); break;
                default: fprintf(stderr, "Err:OpEvalInv:%c\n",token[0]); free(copia_str); return NAN;
            }
            empilharFloat(&pilha, res);
        } else { fprintf(stderr, "Err:TokenEvalInv:%s\n", token); free(copia_str); return NAN; }
        // Próximas chamadas a strtok para a mesma string original
        token = strtok(NULL, " ");
    }

    if (pilha.top != 0) { fprintf(stderr, "Err:PosfixaEvalMalFormada\n"); free(copia_str); return NAN; }
    
    res = desempilharFloat(&pilha); 
    free(copia_str); // Libera a cópia da string que foi modificada por strtok
    return res;
}

float getValorInFixa(char *StrInFixa) {
    char *posFixa = getFormaPosFixa(StrInFixa);
    if (!posFixa) { fprintf(stderr, "Err:ConvInfixaParaPosfixaEval\n"); return NAN; }
    float valor = getValorPosFixa(posFixa);
    free(posFixa);
    return valor;
}