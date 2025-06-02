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
static int minha_eh_digito(char c) {
    return (c >= '0' && c <= '9');
}

static int minha_eh_alfa(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static int minha_eh_espaco(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v');
}

// --- Buffers Estaticos para Strings de Retorno ---
static char buffer_infixo_estatico[512];
static char buffer_posfixo_estatico[512];

// --- Pilha para Floats (para avaliacao posfixa) ---
typedef struct {
    float itens[TAMANHO_MAXIMO_PILHA];
    int topo;
} PilhaFloat;

static void inicializar_pilha_float(PilhaFloat *p) { p->topo = -1; }
static int pilha_float_eh_vazia(PilhaFloat *p) { return p->topo == -1; }
static int pilha_float_eh_cheia(PilhaFloat *p) { return p->topo == TAMANHO_MAXIMO_PILHA - 1; }
static void empurrar_pilha_float(PilhaFloat *p, float valor) {
    if (!pilha_float_eh_cheia(p)) {
        p->itens[++p->topo] = valor;
    } // senao: estouro de pilha, nao tratado por brevidade
}
static float desempurrar_pilha_float(PilhaFloat *p) {
    if (!pilha_float_eh_vazia(p)) {
        return p->itens[p->topo--];
    }
    return NAN; // Underflow de pilha
}

// --- Pilha para Strings (operadores/funcoes em shunting-yard, operandos em posfixo para infixo) ---
typedef struct {
    char itens[TAMANHO_MAXIMO_PILHA][TAMANHO_MAXIMO_TOKEN]; // Armazena tokens como strings
    int topo;
} PilhaToken;

static void inicializar_pilha_token(PilhaToken *p) { p->topo = -1; }
static int pilha_token_eh_vazia(PilhaToken *p) { return p->topo == -1; }
static int pilha_token_eh_cheia(PilhaToken *p) { return p->topo == TAMANHO_MAXIMO_PILHA - 1; }
static void empurrar_pilha_token(PilhaToken *p, const char* token) {
    if (!pilha_token_eh_cheia(p) && strlen(token) < TAMANHO_MAXIMO_TOKEN) {
        strcpy(p->itens[++p->topo], token);
    } // senao: estouro de pilha ou token muito longo
}
static char* desempurrar_pilha_token(PilhaToken *p, char* buffer) {
    if (!pilha_token_eh_vazia(p)) {
        strcpy(buffer, p->itens[p->topo--]);
        return buffer;
    }
    buffer[0] = '\0'; // Indica erro ou vazio
    return buffer;
}
static char* espiar_pilha_token(PilhaToken *p, char* buffer) {
    if (!pilha_token_eh_vazia(p)) {
        strcpy(buffer, p->itens[p->topo]);
        return buffer;
    }
    buffer[0] = '\0'; // Indica erro ou vazio
    return buffer;
}


// --- Funcoes Auxiliares ---
static int eh_operador(const char* token) {
    return strcmp(token, "+") == 0 || strcmp(token, "-") == 0 ||
           strcmp(token, "*") == 0 || strcmp(token, "/") == 0 ||
           strcmp(token, "^") == 0;
}

static int eh_funcao(const char* token) {
    return strcmp(token, "raiz") == 0 || strcmp(token, "sen") == 0 ||
           strcmp(token, "cos") == 0 || strcmp(token, "tg") == 0 ||
           strcmp(token, "log") == 0;
}

static int obter_precedencia(const char* op) {
    if (eh_funcao(op)) return 4; // Funcoes tem alta precedencia
    if (strcmp(op, "^") == 0) return 3;
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0) return 2;
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 1;
    return 0; // Para parenteses ou desconhecido
}

static int eh_associativo_a_direita(const char* op) {
    return strcmp(op, "^") == 0;
}

static double para_radianos(double graus) {
    return graus * (M_PI / 180.0);
}

// --- Funcoes Principais ---

float obterValorPosFixa(char *string_posfixa) {
    PilhaFloat pilha_operando;
    inicializar_pilha_float(&pilha_operando);
    char *copia_entrada = strdup(string_posfixa);
    if (!copia_entrada) return NAN;

    char *token = strtok(copia_entrada, " ");
    while (token != NULL) {
        // Verifica se o token eh um numero (positivo, negativo ou float)
        if (minha_eh_digito(token[0]) ||
            (token[0] == '.' && token[1] != '\0' && minha_eh_digito(token[1])) ||
            (token[0] == '-' && token[1] != '\0' &&
                (minha_eh_digito(token[1]) || (token[1] == '.' && token[2] != '\0' && minha_eh_digito(token[2])) )
            )
           ) {
            empurrar_pilha_float(&pilha_operando, atof(token));
        } else if (eh_operador(token)) {
            if (pilha_float_eh_vazia(&pilha_operando)) { free(copia_entrada); return NAN; }
            float operando2 = desempurrar_pilha_float(&pilha_operando);
            if (pilha_float_eh_vazia(&pilha_operando)) { free(copia_entrada); return NAN; }
            float operando1 = desempurrar_pilha_float(&pilha_operando);

            if (strcmp(token, "+") == 0) empurrar_pilha_float(&pilha_operando, operando1 + operando2);
            else if (strcmp(token, "-") == 0) empurrar_pilha_float(&pilha_operando, operando1 - operando2);
            else if (strcmp(token, "*") == 0) empurrar_pilha_float(&pilha_operando, operando1 * operando2);
            else if (strcmp(token, "/") == 0) {
                if (fabs(operando2) < 1e-9) { free(copia_entrada); return NAN; }
                empurrar_pilha_float(&pilha_operando, operando1 / operando2);
            } else if (strcmp(token, "^") == 0) empurrar_pilha_float(&pilha_operando, powf(operando1, operando2));
        } else if (eh_funcao(token)) {
            if (pilha_float_eh_vazia(&pilha_operando)) { free(copia_entrada); return NAN; }
            float operando1 = desempurrar_pilha_float(&pilha_operando);
            if (strcmp(token, "log") == 0) {
                if (operando1 <= 0) { free(copia_entrada); return NAN; }
                empurrar_pilha_float(&pilha_operando, log10f(operando1));
            } else if (strcmp(token, "sen") == 0) empurrar_pilha_float(&pilha_operando, sinf(para_radianos(operando1)));
            else if (strcmp(token, "cos") == 0) empurrar_pilha_float(&pilha_operando, cosf(para_radianos(operando1)));
            else if (strcmp(token, "tg") == 0) {
                double argumento_radiano = para_radianos(operando1);
                if (fabs(cos(argumento_radiano)) < 1e-9) {free(copia_entrada); return NAN;}
                empurrar_pilha_float(&pilha_operando, tanf(argumento_radiano));
            } else if (strcmp(token, "raiz") == 0) {
                if (operando1 < 0) { free(copia_entrada); return NAN; }
                empurrar_pilha_float(&pilha_operando, sqrtf(operando1));
            }
        } else {
            free(copia_entrada);
            return NAN;
        }
        token = strtok(NULL, " ");
    }
    free(copia_entrada);
    if (pilha_operando.topo == 0) {
        return desempurrar_pilha_float(&pilha_operando);
    }
    return NAN;
}

char *obterFormaInFixa(char *string_posfixa) {
    buffer_infixo_estatico[0] = '\0';
    PilhaToken pilha_operando;
    inicializar_pilha_token(&pilha_operando);

    char *copia_entrada = strdup(string_posfixa);
    if (!copia_entrada) {
        strcpy(buffer_infixo_estatico, "Erro: Falha na alocacao de memoria");
        return buffer_infixo_estatico;
    }

    char *token = strtok(copia_entrada, " ");
    char buffer_expressao[512];

    while (token != NULL) {
        // Verifica se o token eh um numero (positivo, negativo ou float)
        if (minha_eh_digito(token[0]) ||
            (token[0] == '.' && token[1] != '\0' && minha_eh_digito(token[1])) ||
            (token[0] == '-' && token[1] != '\0' &&
                (minha_eh_digito(token[1]) || (token[1] == '.' && token[2] != '\0' && minha_eh_digito(token[2])) )
            )
           ) {
            empurrar_pilha_token(&pilha_operando, token);
        } else if (eh_operador(token)) {
            char string_operando2[TAMANHO_MAXIMO_TOKEN * 4 + 10];
            char string_operando1[TAMANHO_MAXIMO_TOKEN * 4 + 10];

            if (pilha_token_eh_vazia(&pilha_operando)) { free(copia_entrada); strcpy(buffer_infixo_estatico, "Erro: Operandos insuficientes"); return buffer_infixo_estatico;}
            desempurrar_pilha_token(&pilha_operando, string_operando2);
            if (pilha_token_eh_vazia(&pilha_operando)) { free(copia_entrada); strcpy(buffer_infixo_estatico, "Erro: Operandos insuficientes"); return buffer_infixo_estatico;}
            desempurrar_pilha_token(&pilha_operando, string_operando1);

            snprintf(buffer_expressao, sizeof(buffer_expressao), "(%s %s %s)", string_operando1, token, string_operando2);
            empurrar_pilha_token(&pilha_operando, buffer_expressao);
        } else if (eh_funcao(token)) {
            char string_argumento[TAMANHO_MAXIMO_TOKEN * 4 + 10];
            if (pilha_token_eh_vazia(&pilha_operando)) { free(copia_entrada); strcpy(buffer_infixo_estatico, "Erro: Operandos insuficientes para a funcao"); return buffer_infixo_estatico;}
            desempurrar_pilha_token(&pilha_operando, string_argumento);

            snprintf(buffer_expressao, sizeof(buffer_expressao), "%s(%s)", token, string_argumento);
            empurrar_pilha_token(&pilha_operando, buffer_expressao);
        } else {
            free(copia_entrada);
            snprintf(buffer_infixo_estatico, sizeof(buffer_infixo_estatico), "Erro: Token desconhecido '%s'", token);
            return buffer_infixo_estatico;
        }
        token = strtok(NULL, " ");
    }
    free(copia_entrada);

    if (pilha_operando.topo == 0) {
        char buffer_expressao_final[(TAMANHO_MAXIMO_TOKEN * 4) + 50]; // Garante que o buffer eh grande o suficiente
        desempurrar_pilha_token(&pilha_operando, buffer_expressao_final);
        strncpy(buffer_infixo_estatico, buffer_expressao_final, sizeof(buffer_infixo_estatico) -1);
        buffer_infixo_estatico[sizeof(buffer_infixo_estatico)-1] = '\0';
    } else {
        strcpy(buffer_infixo_estatico, "Erro: Expressao posfixa mal formada");
    }
    return buffer_infixo_estatico;
}


char *obterFormaPosFixa(char *string_infixa) {
    buffer_posfixo_estatico[0] = '\0';
    char *ponteiro_posfixo = buffer_posfixo_estatico;

    PilhaToken pilha_operador;
    inicializar_pilha_token(&pilha_operador);

    char token_atual[TAMANHO_MAXIMO_TOKEN];
    char buffer_temporario_operador[TAMANHO_MAXIMO_TOKEN];

    int i = 0;
    while (string_infixa[i] != '\0') {
        if (minha_eh_espaco(string_infixa[i])) {
            i++;
            continue;
        }

        // Numeros (inteiro ou float, nao trata menos unario no inicio aqui)
        // Assume que numeros sao positivos ou comecam com '.' como ".5"
        if (minha_eh_digito(string_infixa[i]) || (string_infixa[i] == '.' && string_infixa[i+1] != '\0' && minha_eh_digito(string_infixa[i+1]))) {
            int k = 0;
            while (string_infixa[i] != '\0' && (minha_eh_digito(string_infixa[i]) || string_infixa[i] == '.')) {
                if (k < TAMANHO_MAXIMO_TOKEN - 1) token_atual[k++] = string_infixa[i];
                i++;
            }
            token_atual[k] = '\0';
            ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", token_atual);
        }
        // Funcoes ou identificadores (exemplo: "sen", "cos", "log")
        else if (minha_eh_alfa(string_infixa[i])) {
            int k = 0;
            while (string_infixa[i] != '\0' && minha_eh_alfa(string_infixa[i])) {
                if (k < TAMANHO_MAXIMO_TOKEN - 1) token_atual[k++] = string_infixa[i];
                i++;
            }
            token_atual[k] = '\0';
            if (eh_funcao(token_atual)) {
                empurrar_pilha_token(&pilha_operador, token_atual);
            } else {
                snprintf(buffer_posfixo_estatico, sizeof(buffer_posfixo_estatico), "Erro: Identificador desconhecido '%s'", token_atual);
                return buffer_posfixo_estatico;
            }
        }
        // Operadores (+, -, *, /, ^)
        else if (strchr("+-*/^", string_infixa[i])) {
            token_atual[0] = string_infixa[i];
            token_atual[1] = '\0';

            while (!pilha_token_eh_vazia(&pilha_operador) &&
                   strcmp(espiar_pilha_token(&pilha_operador, buffer_temporario_operador), "(") != 0 &&
                   (obter_precedencia(buffer_temporario_operador) > obter_precedencia(token_atual) ||
                    (obter_precedencia(buffer_temporario_operador) == obter_precedencia(token_atual) && !eh_associativo_a_direita(token_atual)))) {
                desempurrar_pilha_token(&pilha_operador, buffer_temporario_operador);
                ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", buffer_temporario_operador);
            }
            empurrar_pilha_token(&pilha_operador, token_atual);
            i++;
        }
        // Parentese Esquerdo
        else if (string_infixa[i] == '(') {
            empurrar_pilha_token(&pilha_operador, "(");
            i++;
        }
        // Parentese Direito
        else if (string_infixa[i] == ')') {
            while (!pilha_token_eh_vazia(&pilha_operador) && strcmp(espiar_pilha_token(&pilha_operador, buffer_temporario_operador), "(") != 0) {
                desempurrar_pilha_token(&pilha_operador, buffer_temporario_operador);
                ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", buffer_temporario_operador);
            }
            if (pilha_token_eh_vazia(&pilha_operador) || strcmp(espiar_pilha_token(&pilha_operador, buffer_temporario_operador), "(") != 0) {
                strcpy(buffer_posfixo_estatico, "Erro: Parenteses desbalanceados");
                return buffer_posfixo_estatico;
            }
            desempurrar_pilha_token(&pilha_operador, buffer_temporario_operador); // Remove '('

            if (!pilha_token_eh_vazia(&pilha_operador) && eh_funcao(espiar_pilha_token(&pilha_operador, buffer_temporario_operador))) {
                desempurrar_pilha_token(&pilha_operador, buffer_temporario_operador);
                ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", buffer_temporario_operador);
            }
            i++;
        } else {
            snprintf(buffer_posfixo_estatico, sizeof(buffer_posfixo_estatico), "Erro: Caractere desconhecido '%c'", string_infixa[i]);
            return buffer_posfixo_estatico;
        }
    }

    while (!pilha_token_eh_vazia(&pilha_operador)) {
        desempurrar_pilha_token(&pilha_operador, buffer_temporario_operador);
        if (strcmp(buffer_temporario_operador, "(") == 0) {
            strcpy(buffer_posfixo_estatico, "Erro: Parenteses desbalanceados");
            return buffer_posfixo_estatico;
        }
        ponteiro_posfixo += sprintf(ponteiro_posfixo, "%s ", buffer_temporario_operador);
    }

    if (ponteiro_posfixo > buffer_posfixo_estatico && *(ponteiro_posfixo - 1) == ' ') {
        *(ponteiro_posfixo - 1) = '\0';
    } else {
        *ponteiro_posfixo = '\0';
    }

    return buffer_posfixo_estatico;
}


float obterValorInFixa(char *string_infixa) {
    char *expressao_posfixa = obterFormaPosFixa(string_infixa);
    if (strncmp(expressao_posfixa, "Erro:", 6) == 0) {
        return NAN;
    }
    if (expressao_posfixa == NULL || expressao_posfixa[0] == '\0') {
        return NAN;
    }
    return obterValorPosFixa(expressao_posfixa);
}