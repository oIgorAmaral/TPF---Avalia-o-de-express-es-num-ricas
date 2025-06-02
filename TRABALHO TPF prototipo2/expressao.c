// expressao.c (Nomes de funcoes auxiliares revisados para clareza)
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
static int eh_char_digito(char c) // Anterior: meu_digito
{
    return (c >= '0' && c <= '9');
}

static int eh_char_letra(char c) // Anterior: minha_eh_alfa
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static int eh_char_espaco(char c) // Anterior: minha_eh_espaco
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

static void pilha_float_iniciar(PilhaFloat *p) { p->topo = -1; } // Anterior: inicializar_pilha_float
static int pilha_float_esta_vazia(PilhaFloat *p) { return p->topo == -1; } // Mantido
static int pilha_float_esta_cheia(PilhaFloat *p) { return p->topo == TAMANHO_MAXIMO_PILHA - 1; } // Mantido
static void pilha_float_empilhar(PilhaFloat *p, float valor) // Anterior: empurrar_pilha_float
{
    if (!pilha_float_esta_cheia(p))
    {
        p->itens[++p->topo] = valor;
    } // senao: estouro de pilha, nao tratado por brevidade
}
static float pilha_float_desempilhar(PilhaFloat *p) // Anterior: desempilhar_pilha_float
{
    if (!pilha_float_esta_vazia(p))
    {
        return p->itens[p->topo--];
    }
    return NAN; // Underflow de pilha
}

// --- Pilha para Tokens (Strings) ---
typedef struct
{
    char itens[TAMANHO_MAXIMO_PILHA][TAMANHO_MAXIMO_TOKEN]; // Armazena tokens como strings
    int topo;
} PilhaToken;

static void pilha_token_iniciar(PilhaToken *p) { p->topo = -1; } // Anterior: inicializar_pilha_token
static int pilha_token_esta_vazia(PilhaToken *p) { return p->topo == -1; } // Mantido
static int pilha_token_esta_cheia(PilhaToken *p) { return p->topo == TAMANHO_MAXIMO_PILHA - 1; } // Mantido
static void pilha_token_empilhar(PilhaToken *p, const char *token) // Anterior: empilhar_pilha_token
{
    if (!pilha_token_esta_cheia(p) && strlen(token) < TAMANHO_MAXIMO_TOKEN)
    {
        strcpy(p->itens[++p->topo], token);
    } // senao: estouro de pilha ou token muito longo
}
static char *pilha_token_desempilhar(PilhaToken *p, char *buffer_destino) // Anterior: desempilhar_pilha_token, buffer_destino adicionado para clareza
{
    if (!pilha_token_esta_vazia(p))
    {
        strcpy(buffer_destino, p->itens[p->topo--]);
        return buffer_destino;
    }
    buffer_destino[0] = '\0'; // Indica erro ou vazio
    return buffer_destino;
}
static char *pilha_token_consultar_topo(PilhaToken *p, char *buffer_destino) // Anterior: espiar_pilha_token, buffer_destino adicionado para clareza
{
    if (!pilha_token_esta_vazia(p))
    {
        strcpy(buffer_destino, p->itens[p->topo]);
        return buffer_destino;
    }
    buffer_destino[0] = '\0'; // Indica erro ou vazio
    return buffer_destino;
}

// --- Funcoes Auxiliares de Analise de Tokens ---
static int eh_token_operador(const char *token) // Anterior: operador
{
    return strcmp(token, "+") == 0 || strcmp(token, "-") == 0 ||
           strcmp(token, "*") == 0 || strcmp(token, "/") == 0 ||
           strcmp(token, "^") == 0;
}

static int eh_token_funcao(const char *token) // Anterior: funcao
{
    return strcmp(token, "raiz") == 0 || strcmp(token, "sen") == 0 ||
           strcmp(token, "cos") == 0 || strcmp(token, "tg") == 0 ||
           strcmp(token, "log") == 0;
}

static int obter_precedencia_token(const char *token_op) // Anterior: obter_precedencia
{
    if (eh_token_funcao(token_op)) // Usa a funcao auxiliar renomeada
        return 4; // Funcoes tem alta precedencia
    if (strcmp(token_op, "^") == 0)
        return 3;
    if (strcmp(token_op, "*") == 0 || strcmp(token_op, "/") == 0)
        return 2;
    if (strcmp(token_op, "+") == 0 || strcmp(token_op, "-") == 0)
        return 1;
    return 0; // Para parenteses ou desconhecido
}

static int token_eh_associativo_a_direita(const char *token_op) // Anterior: associativo_a_direita
{
    return strcmp(token_op, "^") == 0;
}

static double converter_graus_para_radianos(double graus) // Anterior: para_radianos
{
    return graus * (M_PI / 180.0);
}

// --- Funcoes Principais (Nomes mantidos conforme traducao original de expressao.h) ---

float obterValorPosFixa(char *string_posfixa)
{
    PilhaFloat pilha_operando;
    pilha_float_iniciar(&pilha_operando); // Renomeado
    char *copia_entrada = strdup(string_posfixa);
    if (!copia_entrada)
        return NAN;

    char *token = strtok(copia_entrada, " ");
    while (token != NULL)
    {
        if (eh_char_digito(token[0]) || // Renomeado
            (token[0] == '.' && token[1] != '\0' && eh_char_digito(token[1])) || // Renomeado
            (token[0] == '-' && token[1] != '\0' &&
             (eh_char_digito(token[1]) || (token[1] == '.' && token[2] != '\0' && eh_char_digito(token[2]))))) // Renomeado
        {
            pilha_float_empilhar(&pilha_operando, atof(token)); // Renomeado
        }
        else if (eh_token_operador(token)) // Renomeado
        {
            if (pilha_float_esta_vazia(&pilha_operando)) // Mantido
            {
                free(copia_entrada);
                return NAN;
            }
            float operando2 = pilha_float_desempilhar(&pilha_operando); // Renomeado
            if (pilha_float_esta_vazia(&pilha_operando)) // Mantido
            {
                free(copia_entrada);
                return NAN;
            }
            float operando1 = pilha_float_desempilhar(&pilha_operando); // Renomeado

            if (strcmp(token, "+") == 0)
                pilha_float_empilhar(&pilha_operando, operando1 + operando2); // Renomeado
            else if (strcmp(token, "-") == 0)
                pilha_float_empilhar(&pilha_operando, operando1 - operando2); // Renomeado
            else if (strcmp(token, "*") == 0)
                pilha_float_empilhar(&pilha_operando, operando1 * operando2); // Renomeado
            else if (strcmp(token, "/") == 0)
            {
                if (fabs(operando2) < 1e-9) // Verifica divisao por zero
                {
                    free(copia_entrada);
                    return NAN;
                }
                pilha_float_empilhar(&pilha_operando, operando1 / operando2); // Renomeado
            }
            else if (strcmp(token, "^") == 0)
                pilha_float_empilhar(&pilha_operando, powf(operando1, operando2)); // Renomeado
        }
        else if (eh_token_funcao(token)) // Renomeado
        {
            if (pilha_float_esta_vazia(&pilha_operando)) // Mantido
            {
                free(copia_entrada);
                return NAN;
            }
            float operando1 = pilha_float_desempilhar(&pilha_operando); // Renomeado
            if (strcmp(token, "log") == 0)
            {
                if (operando1 <= 0)
                {
                    free(copia_entrada);
                    return NAN;
                }
                pilha_float_empilhar(&pilha_operando, log10f(operando1)); // Renomeado
            }
            else if (strcmp(token, "sen") == 0)
                pilha_float_empilhar(&pilha_operando, sinf(converter_graus_para_radianos(operando1))); // Renomeado
            else if (strcmp(token, "cos") == 0)
                pilha_float_empilhar(&pilha_operando, cosf(converter_graus_para_radianos(operando1))); // Renomeado
            else if (strcmp(token, "tg") == 0)
            {
                double argumento_radiano = converter_graus_para_radianos(operando1); // Renomeado
                if (fabs(cos(argumento_radiano)) < 1e-9) // Verifica se cos(theta) eh proximo de 0
                {
                    free(copia_entrada);
                    return NAN;
                }
                pilha_float_empilhar(&pilha_operando, tanf(argumento_radiano)); // Renomeado
            }
            else if (strcmp(token, "raiz") == 0)
            {
                if (operando1 < 0)
                {
                    free(copia_entrada);
                    return NAN;
                }
                pilha_float_empilhar(&pilha_operando, sqrtf(operando1)); // Renomeado
            }
        }
        else // Token desconhecido
        {
            free(copia_entrada);
            return NAN;
        }
        token = strtok(NULL, " ");
    }
    free(copia_entrada);
    if (pilha_operando.topo == 0) // A pilha deve conter exatamente um resultado
    {
        return pilha_float_desempilhar(&pilha_operando); // Renomeado
    }
    return NAN; // Mais ou menos de um item na pilha indica erro
}

char *obterFormaInFixa(char *string_posfixa)
{
    buffer_infixo_estatico[0] = '\0'; // Limpa buffer estatico de retorno
    PilhaToken pilha_operando;
    pilha_token_iniciar(&pilha_operando); // Renomeado

    char *copia_entrada = strdup(string_posfixa);
    if (!copia_entrada)
    {
        strcpy(buffer_infixo_estatico, "Erro: Falha na alocacao de memoria");
        return buffer_infixo_estatico;
    }

    char *token = strtok(copia_entrada, " ");
    char buffer_expressao[512]; // Buffer para construir sub-expressoes
    char string_operando1[TAMANHO_MAXIMO_TOKEN * 4 + 10]; // Buffers para operandos desempilhados
    char string_operando2[TAMANHO_MAXIMO_TOKEN * 4 + 10];
    char string_argumento[TAMANHO_MAXIMO_TOKEN * 4 + 10];


    while (token != NULL)
    {
        if (eh_char_digito(token[0]) || // Renomeado
            (token[0] == '.' && token[1] != '\0' && eh_char_digito(token[1])) || // Renomeado
            (token[0] == '-' && token[1] != '\0' &&
             (eh_char_digito(token[1]) || (token[1] == '.' && token[2] != '\0' && eh_char_digito(token[2]))))) // Renomeado
        {
            pilha_token_empilhar(&pilha_operando, token); // Renomeado
        }
        else if (eh_token_operador(token)) // Renomeado
        {
            if (pilha_token_esta_vazia(&pilha_operando)) // Mantido
            {
                free(copia_entrada);
                strcpy(buffer_infixo_estatico, "Erro: Operandos insuficientes para operador");
                return buffer_infixo_estatico;
            }
            pilha_token_desempilhar(&pilha_operando, string_operando2); // Renomeado
            if (pilha_token_esta_vazia(&pilha_operando)) // Mantido
            {
                free(copia_entrada);
                strcpy(buffer_infixo_estatico, "Erro: Operandos insuficientes para operador");
                return buffer_infixo_estatico;
            }
            pilha_token_desempilhar(&pilha_operando, string_operando1); // Renomeado

            snprintf(buffer_expressao, sizeof(buffer_expressao), "(%s %s %s)", string_operando1, token, string_operando2);
            pilha_token_empilhar(&pilha_operando, buffer_expressao); // Renomeado
        }
        else if (eh_token_funcao(token)) // Renomeado
        {
            if (pilha_token_esta_vazia(&pilha_operando)) // Mantido
            {
                free(copia_entrada);
                strcpy(buffer_infixo_estatico, "Erro: Argumento insuficiente para funcao");
                return buffer_infixo_estatico;
            }
            pilha_token_desempilhar(&pilha_operando, string_argumento); // Renomeado

            snprintf(buffer_expressao, sizeof(buffer_expressao), "%s(%s)", token, string_argumento);
            pilha_token_empilhar(&pilha_operando, buffer_expressao); // Renomeado
        }
        else // Token desconhecido
        {
            free(copia_entrada);
            snprintf(buffer_infixo_estatico, sizeof(buffer_infixo_estatico), "Erro: Token desconhecido '%s'", token);
            return buffer_infixo_estatico;
        }
        token = strtok(NULL, " ");
    }
    free(copia_entrada);

    if (pilha_operando.topo == 0) // Deve haver exatamente uma expressao na pilha
    {
        char buffer_expressao_final[sizeof(buffer_infixo_estatico)]; 
        pilha_token_desempilhar(&pilha_operando, buffer_expressao_final); // Renomeado
        strncpy(buffer_infixo_estatico, buffer_expressao_final, sizeof(buffer_infixo_estatico) - 1);
        buffer_infixo_estatico[sizeof(buffer_infixo_estatico) - 1] = '\0'; // Garante terminacao nula
    }
    else
    {
        strcpy(buffer_infixo_estatico, "Erro: Expressao posfixa mal formada");
    }
    return buffer_infixo_estatico;
}

char *obterFormaPosFixa(char *string_infixa)
{
    buffer_posfixo_estatico[0] = '\0'; // Limpa buffer estatico
    char *ponteiro_posfixo = buffer_posfixo_estatico;

    PilhaToken pilha_operador;
    pilha_token_iniciar(&pilha_operador); // Renomeado

    char token_atual[TAMANHO_MAXIMO_TOKEN];
    char buffer_temporario_operador_pilha[TAMANHO_MAXIMO_TOKEN]; // Para resultado de consultar_topo

    int i = 0;
    while (string_infixa[i] != '\0')
    {
        if (eh_char_espaco(string_infixa[i])) // Renomeado
        {
            i++;
            continue;
        }

        if (eh_char_digito(string_infixa[i]) || (string_infixa[i] == '.' && string_infixa[i + 1] != '\0' && eh_char_digito(string_infixa[i + 1]))) // Renomeado
        {
            int k = 0;
            while (string_infixa[i] != '\0' && (eh_char_digito(string_infixa[i]) || string_infixa[i] == '.')) // Renomeado
            {
                if (k < TAMANHO_MAXIMO_TOKEN - 1)
                    token_atual[k++] = string_infixa[i];
                i++;
            }
            token_atual[k] = '\0';
            ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", token_atual);
        }
        else if (eh_char_letra(string_infixa[i])) // Renomeado
        {
            int k = 0;
            while (string_infixa[i] != '\0' && eh_char_letra(string_infixa[i])) // Renomeado
            {
                if (k < TAMANHO_MAXIMO_TOKEN - 1)
                    token_atual[k++] = string_infixa[i];
                i++;
            }
            token_atual[k] = '\0';
            if (eh_token_funcao(token_atual)) // Renomeado
            {
                pilha_token_empilhar(&pilha_operador, token_atual); // Renomeado
            }
            else
            {
                snprintf(buffer_posfixo_estatico, sizeof(buffer_posfixo_estatico), "Erro: Identificador desconhecido '%s'", token_atual);
                return buffer_posfixo_estatico;
            }
        }
        else if (strchr("+-*/^", string_infixa[i])) // Operador de um caractere
        {
            token_atual[0] = string_infixa[i];
            token_atual[1] = '\0';

            while (!pilha_token_esta_vazia(&pilha_operador) && // Mantido
                   strcmp(pilha_token_consultar_topo(&pilha_operador, buffer_temporario_operador_pilha), "(") != 0 && // Renomeado
                   (obter_precedencia_token(buffer_temporario_operador_pilha) > obter_precedencia_token(token_atual) || // Renomeado
                    (obter_precedencia_token(buffer_temporario_operador_pilha) == obter_precedencia_token(token_atual) && !token_eh_associativo_a_direita(token_atual)))) // Renomeado
            {
                pilha_token_desempilhar(&pilha_operador, buffer_temporario_operador_pilha); // Renomeado
                ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", buffer_temporario_operador_pilha);
            }
            pilha_token_empilhar(&pilha_operador, token_atual); // Renomeado
            i++;
        }
        else if (string_infixa[i] == '(')
        {
            pilha_token_empilhar(&pilha_operador, "("); // Renomeado
            i++;
        }
        else if (string_infixa[i] == ')')
        {
            while (!pilha_token_esta_vazia(&pilha_operador) && strcmp(pilha_token_consultar_topo(&pilha_operador, buffer_temporario_operador_pilha), "(") != 0) // Renomeado e Mantido
            {
                pilha_token_desempilhar(&pilha_operador, buffer_temporario_operador_pilha); // Renomeado
                ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", buffer_temporario_operador_pilha);
            }
            if (pilha_token_esta_vazia(&pilha_operador) || strcmp(pilha_token_consultar_topo(&pilha_operador, buffer_temporario_operador_pilha), "(") != 0) // Mantido e Renomeado
            {
                strcpy(buffer_posfixo_estatico, "Erro: Parenteses desbalanceados");
                return buffer_posfixo_estatico;
            }
            pilha_token_desempilhar(&pilha_operador, buffer_temporario_operador_pilha); // Remove '(', Renomeado

            if (!pilha_token_esta_vazia(&pilha_operador) && eh_token_funcao(pilha_token_consultar_topo(&pilha_operador, buffer_temporario_operador_pilha))) // Mantido, Renomeado e Renomeado
            {
                pilha_token_desempilhar(&pilha_operador, buffer_temporario_operador_pilha); // Renomeado
                ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", buffer_temporario_operador_pilha);
            }
            i++;
        }
        else // Caractere desconhecido
        {
            snprintf(buffer_posfixo_estatico, sizeof(buffer_posfixo_estatico), "Erro: Caractere desconhecido '%c'", string_infixa[i]);
            return buffer_posfixo_estatico;
        }
    }

    while (!pilha_token_esta_vazia(&pilha_operador)) // Mantido
    {
        pilha_token_desempilhar(&pilha_operador, buffer_temporario_operador_pilha); // Renomeado
        if (strcmp(buffer_temporario_operador_pilha, "(") == 0)
        {
            strcpy(buffer_posfixo_estatico, "Erro: Parenteses desbalanceados");
            return buffer_posfixo_estatico;
        }
        ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", buffer_temporario_operador_pilha);
    }

    // Remove espaco extra no final, se houver
    if (ponteiro_posfixo > buffer_posfixo_estatico && *(ponteiro_posfixo - 1) == ' ')
    {
        *(ponteiro_posfixo - 1) = '\0';
    }
    else
    {
        *ponteiro_posfixo = '\0'; // Garante terminacao nula se a string estiver vazia
    }

    return buffer_posfixo_estatico;
}

float obterValorInFixa(char *string_infixa)
{
    char *expressao_posfixa = obterFormaPosFixa(string_infixa);
    // Verifica se obterFormaPosFixa retornou uma string de erro
    if (strncmp(expressao_posfixa, "Erro:", 5) == 0) // Comparar os primeiros 5 caracteres
    {
        return NAN; 
    }
    if (expressao_posfixa == NULL || expressao_posfixa[0] == '\0') // Checagem adicional de seguranca
    {
        return NAN;
    }
    return obterValorPosFixa(expressao_posfixa);
}