// expressao.c (strdup substituido por malloc/strcpy)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "expressao.h" // Conforme fornecido pelo usuario

// Define M_PI se nao estiver ja definido (exemplo: por _USE_MATH_DEFINES no Windows)
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#define TAMANHO_MAXIMO_PILHA 256
#define TAMANHO_MAXIMO_TOKEN 32 // Tamanho maximo para numeros, operadores ou nomes de funcoes

// --- Funcoes CTYPE personalizadas ---
static int char_digito(char c) 
{
    return (c >= '0' && c <= '9');
}

static int char_letra(char c) 
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static int char_espaco(char c) 
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v');
}

// --- Buffers Estaticos para Strings de Retorno ---
static char buffer_infixo_estatico[512];
static char buffer_posfixo_estatico[512];

// --- Pilha para Floats (para avaliacao posfixa) ---
typedef struct
{
    float itens[TAMANHO_MAXIMO_PILHA];
    int topo;
} PilhaFloat;

static void pilha_float_iniciar(PilhaFloat *p) { p->topo = -1; } 
static int pilha_float_esta_vazia(PilhaFloat *p) { return p->topo == -1; } 
static int pilha_float_esta_cheia(PilhaFloat *p) { return p->topo == TAMANHO_MAXIMO_PILHA - 1; } 
static void pilha_float_empilhar(PilhaFloat *p, float valor) 
{
    if (!pilha_float_esta_cheia(p))
    {
        p->itens[++p->topo] = valor;
    } 
}
static float pilha_float_desempilhar(PilhaFloat *p) 
{
    if (!pilha_float_esta_vazia(p))
    {
        return p->itens[p->topo--];
    }
    return NAN; 
}

// --- Pilha para Tokens (Strings) ---
typedef struct
{
    char itens[TAMANHO_MAXIMO_PILHA][TAMANHO_MAXIMO_TOKEN]; 
    int topo;
} PilhaToken;

static void pilha_token_iniciar(PilhaToken *p) { p->topo = -1; } 
static int pilha_token_esta_vazia(PilhaToken *p) { return p->topo == -1; } 
static int pilha_token_esta_cheia(PilhaToken *p) { return p->topo == TAMANHO_MAXIMO_PILHA - 1; } 
static void pilha_token_empilhar(PilhaToken *p, const char *token) 
{
    if (!pilha_token_esta_cheia(p) && strlen(token) < TAMANHO_MAXIMO_TOKEN)
    {
        strcpy(p->itens[++p->topo], token);
    } 
}
static char *pilha_token_desempilhar(PilhaToken *p, char *buffer_destino) 
{
    if (!pilha_token_esta_vazia(p))
    {
        strcpy(buffer_destino, p->itens[p->topo--]);
        return buffer_destino;
    }
    buffer_destino[0] = '\0'; 
    return buffer_destino;
}
static char *pilha_token_consultar_topo(PilhaToken *p, char *buffer_destino) 
{
    if (!pilha_token_esta_vazia(p))
    {
        strcpy(buffer_destino, p->itens[p->topo]);
        return buffer_destino;
    }
    buffer_destino[0] = '\0'; 
    return buffer_destino;
}

// --- Funcoes Auxiliares de Analise de Tokens ---
static int token_operador(const char *token) 
{
    return strcmp(token, "+") == 0 || strcmp(token, "-") == 0 ||
           strcmp(token, "*") == 0 || strcmp(token, "/") == 0 ||
           strcmp(token, "^") == 0;
}

static int token_funcao(const char *token) 
{
    return strcmp(token, "raiz") == 0 || strcmp(token, "sen") == 0 ||
           strcmp(token, "cos") == 0 || strcmp(token, "tg") == 0 ||
           strcmp(token, "log") == 0;
}

static int obter_precedencia_token(const char *token_op) 
{
    if (token_funcao(token_op)) 
        return 4; 
    if (strcmp(token_op, "^") == 0)
        return 3;
    if (strcmp(token_op, "*") == 0 || strcmp(token_op, "/") == 0)
        return 2;
    if (strcmp(token_op, "+") == 0 || strcmp(token_op, "-") == 0)
        return 1;
    return 0; 
}

static int token_associativo_a_direita(const char *token_op) 
{
    return strcmp(token_op, "^") == 0;
}

static double converter_graus_para_radianos(double graus) 
{
    return graus * (M_PI / 180.0);
}

// --- Funcoes Principais (Nomes mantidos conforme traducao original de expressao.h) ---

float obterValorPosFixa(char *string_posfixa)
{
    PilhaFloat pilha_operando;
    pilha_float_iniciar(&pilha_operando); 
    
    char *copia_entrada = (char *)malloc(strlen(string_posfixa) + 1);
    if (!copia_entrada) {
        // Opcional: fprintf(stderr, "Erro: Falha na alocacao de memoria em obterValorPosFixa\n");
        return NAN; // Indica falha de alocacao
    }
    strcpy(copia_entrada, string_posfixa);

    char *token = strtok(copia_entrada, " ");
    while (token != NULL)
    {
        if (char_digito(token[0]) || 
            (token[0] == '.' && token[1] != '\0' && char_digito(token[1])) || 
            (token[0] == '-' && token[1] != '\0' &&
             (char_digito(token[1]) || (token[1] == '.' && token[2] != '\0' && char_digito(token[2]))))) 
        {
            pilha_float_empilhar(&pilha_operando, atof(token)); 
        }
        else if (token_operador(token)) 
        {
            if (pilha_float_esta_vazia(&pilha_operando)) 
            {
                free(copia_entrada);
                return NAN;
            }
            float operando2 = pilha_float_desempilhar(&pilha_operando); 
            if (pilha_float_esta_vazia(&pilha_operando)) 
            {
                free(copia_entrada);
                return NAN;
            }
            float operando1 = pilha_float_desempilhar(&pilha_operando); 

            if (strcmp(token, "+") == 0)
                pilha_float_empilhar(&pilha_operando, operando1 + operando2); 
            else if (strcmp(token, "-") == 0)
                pilha_float_empilhar(&pilha_operando, operando1 - operando2); 
            else if (strcmp(token, "*") == 0)
                pilha_float_empilhar(&pilha_operando, operando1 * operando2); 
            else if (strcmp(token, "/") == 0)
            {
                if (fabs(operando2) < 1e-9) 
                {
                    free(copia_entrada);
                    return NAN;
                }
                pilha_float_empilhar(&pilha_operando, operando1 / operando2); 
            }
            else if (strcmp(token, "^") == 0)
                pilha_float_empilhar(&pilha_operando, powf(operando1, operando2)); 
        }
        else if (token_funcao(token)) 
        {
            if (pilha_float_esta_vazia(&pilha_operando)) 
            {
                free(copia_entrada);
                return NAN;
            }
            float operando1 = pilha_float_desempilhar(&pilha_operando); 
            if (strcmp(token, "log") == 0)
            {
                if (operando1 <= 0)
                {
                    free(copia_entrada);
                    return NAN;
                }
                pilha_float_empilhar(&pilha_operando, log10f(operando1)); 
            }
            else if (strcmp(token, "sen") == 0)
                pilha_float_empilhar(&pilha_operando, sinf(converter_graus_para_radianos(operando1))); 
            else if (strcmp(token, "cos") == 0)
                pilha_float_empilhar(&pilha_operando, cosf(converter_graus_para_radianos(operando1))); 
            else if (strcmp(token, "tg") == 0)
            {
                double argumento_radiano = converter_graus_para_radianos(operando1); 
                if (fabs(cos(argumento_radiano)) < 1e-9) 
                {
                    free(copia_entrada);
                    return NAN;
                }
                pilha_float_empilhar(&pilha_operando, tanf(argumento_radiano)); 
            }
            else if (strcmp(token, "raiz") == 0)
            {
                if (operando1 < 0)
                {
                    free(copia_entrada);
                    return NAN;
                }
                pilha_float_empilhar(&pilha_operando, sqrtf(operando1)); 
            }
        }
        else 
        {
            free(copia_entrada);
            return NAN;
        }
        token = strtok(NULL, " ");
    }
    free(copia_entrada);
    if (pilha_operando.topo == 0) 
    {
        return pilha_float_desempilhar(&pilha_operando); 
    }
    return NAN; 
}

char *obterFormaInFixa(char *string_posfixa)
{
    buffer_infixo_estatico[0] = '\0'; 
    PilhaToken pilha_operando;
    pilha_token_iniciar(&pilha_operando); 

    char *copia_entrada = (char *)malloc(strlen(string_posfixa) + 1);
    if (!copia_entrada)
    {
        strcpy(buffer_infixo_estatico, "Erro: Falha na alocacao de memoria");
        return buffer_infixo_estatico;
    }
    strcpy(copia_entrada, string_posfixa);

    char *token = strtok(copia_entrada, " ");
    char buffer_expressao[512]; 
    char string_operando1[TAMANHO_MAXIMO_TOKEN * 4 + 10]; 
    char string_operando2[TAMANHO_MAXIMO_TOKEN * 4 + 10];
    char string_argumento[TAMANHO_MAXIMO_TOKEN * 4 + 10];


    while (token != NULL)
    {
        if (char_digito(token[0]) || 
            (token[0] == '.' && token[1] != '\0' && char_digito(token[1])) || 
            (token[0] == '-' && token[1] != '\0' &&
             (char_digito(token[1]) || (token[1] == '.' && token[2] != '\0' && char_digito(token[2]))))) 
        {
            pilha_token_empilhar(&pilha_operando, token); 
        }
        else if (token_operador(token)) 
        {
            if (pilha_token_esta_vazia(&pilha_operando)) 
            {
                free(copia_entrada);
                strcpy(buffer_infixo_estatico, "Erro: Operandos insuficientes para operador");
                return buffer_infixo_estatico;
            }
            pilha_token_desempilhar(&pilha_operando, string_operando2); 
            if (pilha_token_esta_vazia(&pilha_operando)) 
            {
                free(copia_entrada);
                strcpy(buffer_infixo_estatico, "Erro: Operandos insuficientes para operador");
                return buffer_infixo_estatico;
            }
            pilha_token_desempilhar(&pilha_operando, string_operando1); 

            snprintf(buffer_expressao, sizeof(buffer_expressao), "(%s %s %s)", string_operando1, token, string_operando2);
            pilha_token_empilhar(&pilha_operando, buffer_expressao); 
        }
        else if (token_funcao(token)) 
        {
            if (pilha_token_esta_vazia(&pilha_operando)) 
            {
                free(copia_entrada);
                strcpy(buffer_infixo_estatico, "Erro: Argumento insuficiente para funcao");
                return buffer_infixo_estatico;
            }
            pilha_token_desempilhar(&pilha_operando, string_argumento); 

            snprintf(buffer_expressao, sizeof(buffer_expressao), "%s(%s)", token, string_argumento);
            pilha_token_empilhar(&pilha_operando, buffer_expressao); 
        }
        else 
        {
            free(copia_entrada);
            snprintf(buffer_infixo_estatico, sizeof(buffer_infixo_estatico), "Erro: Token desconhecido '%s'", token);
            return buffer_infixo_estatico;
        }
        token = strtok(NULL, " ");
    }
    free(copia_entrada);

    if (pilha_operando.topo == 0) 
    {
        char buffer_expressao_final[sizeof(buffer_infixo_estatico)]; 
        pilha_token_desempilhar(&pilha_operando, buffer_expressao_final); 
        strncpy(buffer_infixo_estatico, buffer_expressao_final, sizeof(buffer_infixo_estatico) - 1);
        buffer_infixo_estatico[sizeof(buffer_infixo_estatico) - 1] = '\0'; 
    }
    else
    {
        strcpy(buffer_infixo_estatico, "Erro: Expressao posfixa mal formada");
    }
    return buffer_infixo_estatico;
}

char *obterFormaPosFixa(char *string_infixa)
{
    buffer_posfixo_estatico[0] = '\0'; 
    char *ponteiro_posfixo = buffer_posfixo_estatico;

    PilhaToken pilha_operador;
    pilha_token_iniciar(&pilha_operador); 

    char token_atual[TAMANHO_MAXIMO_TOKEN];
    char buffer_temporario_operador_pilha[TAMANHO_MAXIMO_TOKEN]; 

    int i = 0;
    while (string_infixa[i] != '\0')
    {
        if (char_espaco(string_infixa[i])) 
        {
            i++;
            continue;
        }

        if (char_digito(string_infixa[i]) || (string_infixa[i] == '.' && string_infixa[i + 1] != '\0' && char_digito(string_infixa[i + 1]))) 
        {
            int k = 0;
            while (string_infixa[i] != '\0' && (char_digito(string_infixa[i]) || string_infixa[i] == '.')) 
            {
                if (k < TAMANHO_MAXIMO_TOKEN - 1)
                    token_atual[k++] = string_infixa[i];
                i++;
            }
            token_atual[k] = '\0';
            ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", token_atual);
        }
        else if (char_letra(string_infixa[i])) 
        {
            int k = 0;
            while (string_infixa[i] != '\0' && char_letra(string_infixa[i])) 
            {
                if (k < TAMANHO_MAXIMO_TOKEN - 1)
                    token_atual[k++] = string_infixa[i];
                i++;
            }
            token_atual[k] = '\0';
            if (token_funcao(token_atual)) 
            {
                pilha_token_empilhar(&pilha_operador, token_atual); 
            }
            else
            {
                snprintf(buffer_posfixo_estatico, sizeof(buffer_posfixo_estatico), "Erro: Identificador desconhecido '%s'", token_atual);
                return buffer_posfixo_estatico;
            }
        }
        else if (strchr("+-*/^", string_infixa[i])) 
        {
            token_atual[0] = string_infixa[i];
            token_atual[1] = '\0';

            while (!pilha_token_esta_vazia(&pilha_operador) && 
                   strcmp(pilha_token_consultar_topo(&pilha_operador, buffer_temporario_operador_pilha), "(") != 0 && 
                   (obter_precedencia_token(buffer_temporario_operador_pilha) > obter_precedencia_token(token_atual) || 
                    (obter_precedencia_token(buffer_temporario_operador_pilha) == obter_precedencia_token(token_atual) && !token_associativo_a_direita(token_atual)))) 
            {
                pilha_token_desempilhar(&pilha_operador, buffer_temporario_operador_pilha); 
                ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", buffer_temporario_operador_pilha);
            }
            pilha_token_empilhar(&pilha_operador, token_atual); 
            i++;
        }
        else if (string_infixa[i] == '(')
        {
            pilha_token_empilhar(&pilha_operador, "("); 
            i++;
        }
        else if (string_infixa[i] == ')')
        {
            while (!pilha_token_esta_vazia(&pilha_operador) && strcmp(pilha_token_consultar_topo(&pilha_operador, buffer_temporario_operador_pilha), "(") != 0) 
            {
                pilha_token_desempilhar(&pilha_operador, buffer_temporario_operador_pilha); 
                ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", buffer_temporario_operador_pilha);
            }
            if (pilha_token_esta_vazia(&pilha_operador) || strcmp(pilha_token_consultar_topo(&pilha_operador, buffer_temporario_operador_pilha), "(") != 0) 
            {
                strcpy(buffer_posfixo_estatico, "Erro: Parenteses desbalanceados");
                return buffer_posfixo_estatico;
            }
            pilha_token_desempilhar(&pilha_operador, buffer_temporario_operador_pilha); 

            if (!pilha_token_esta_vazia(&pilha_operador) && token_funcao(pilha_token_consultar_topo(&pilha_operador, buffer_temporario_operador_pilha))) 
            {
                pilha_token_desempilhar(&pilha_operador, buffer_temporario_operador_pilha); 
                ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", buffer_temporario_operador_pilha);
            }
            i++;
        }
        else 
        {
            snprintf(buffer_posfixo_estatico, sizeof(buffer_posfixo_estatico), "Erro: Caractere desconhecido '%c'", string_infixa[i]);
            return buffer_posfixo_estatico;
        }
    }

    while (!pilha_token_esta_vazia(&pilha_operador)) 
    {
        pilha_token_desempilhar(&pilha_operador, buffer_temporario_operador_pilha); 
        if (strcmp(buffer_temporario_operador_pilha, "(") == 0)
        {
            strcpy(buffer_posfixo_estatico, "Erro: Parenteses desbalanceados");
            return buffer_posfixo_estatico;
        }
        ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", buffer_temporario_operador_pilha);
    }

    
    if (ponteiro_posfixo > buffer_posfixo_estatico && *(ponteiro_posfixo - 1) == ' ')
    {
        *(ponteiro_posfixo - 1) = '\0';
    }
    else
    {
        *ponteiro_posfixo = '\0'; 
    }

    return buffer_posfixo_estatico;
}

float obterValorInFixa(char *string_infixa)
{
    char *expressao_posfixa = obterFormaPosFixa(string_infixa);
    
    if (strncmp(expressao_posfixa, "Erro:", 5) == 0) 
    {
        return NAN; 
    }
    if (expressao_posfixa == NULL || expressao_posfixa[0] == '\0') 
    {
        return NAN;
    }
    return obterValorPosFixa(expressao_posfixa);
}