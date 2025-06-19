
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>   
#include "expressao.h" 

#define EPSILON_FLOAT 0.001f


typedef struct {
    const char* id;
    const char* infixa;
    const char* posfixa;
    const char* valorStr;
    float valorFloat;
} CasoTeste;

// Casos de teste originais do PDF, dadas do Documento!
CasoTeste casosDeTestePDF[] = {
    {"1", "(3+4)*5",         "3 4 + 5 *",       "35",          35.0f},
    {"2", "7*2+4",           "7 2 * 4 +",       "18",          18.0f},
    {"3", "8+(5*(2+4))",     "8 5 2 4 + * +",   "38",          38.0f},
    {"4", "(6/2+3)*4",       "6 2 / 3 + 4 *",   "24",          24.0f},
    {"5", "9+(5*(2+8*4))",   "9 5 2 8 * 4 + * +", "109",         109.0f},
    {"6", "log(2+3)/5",      "2 3 + log 5 /",   "Aprox. 0.14", 0.13979f},
    {"7", "(log(10))^3+2",  "10 log 3 ^ 2 +",  "3",           3.0f},
    {"8", "(45+60)*cos(30)", "45 60 + 30 cos *", "Aprox. 90.93", 90.93267f},
    {"9", "sen(45)^2+0.5",   "45 sen 2 ^ 0.5 +", "1",           1.0f}
};
int numCasosTestePDF = sizeof(casosDeTestePDF) / sizeof(casosDeTestePDF[0]);

// --- Novos Casos de Teste Adicionais, como o senhor pediu!
CasoTeste casosDeTesteAdicionais[] = {
    // ID, Infixa, Postfixa Esperada, Valor String, Valor Float
    {"10", "10 % 3", "10 3 %", "1.0", 1.0f},
    {"11", "raiz(16)", "16 raiz", "4.0", 4.0f},
    {"12", "tg(45)", "45 tg", "1.0", 1.0f},
    {"14", "10 * raiz(4)", "10 4 raiz *", "20.0", 20.0f},
    {"15", "5 - 10 * 2", "5 10 2 * -", "-15.0", -15.0f}

};
int numCasosTesteAdicionais = sizeof(casosDeTesteAdicionais) / sizeof(casosDeTesteAdicionais[0]);


// Funcao auxiliar para comparar floats com tolerancia
int compararFloats(float f1, float f2) {
    if (isnan(f1) && isnan(f2)) return 1; 
    if (isnan(f1) || isnan(f2)) return 0; 
    return fabs(f1 - f2) < EPSILON_FLOAT;
}


void executarTeste(CasoTeste c) {
    printf("--- Teste %s: Infixa \"%s\" ---\n", c.id, c.infixa);

    // 1. Teste: getFormaPosFixa()
    char* posfixaGerada = getFormaPosFixa(c.infixa);
    printf("  1. Conversao Infixa -> Posfixa:\n");
    printf("     - Gerada:   \"%s\"\n", posfixaGerada ? posfixaGerada : "ERRO_CONVERSAO");
    printf("     - Esperada: \"%s\"\n", c.posfixa);
    printf("     - Resultado: %s\n", (posfixaGerada && strcmp(posfixaGerada, c.posfixa) == 0) ? "SUCESSO" : "FALHA");

    // 2. Teste: getValorInFixa()
    float valorDeInfixa = getValorInFixa(c.infixa);
    printf("  2. Avaliacao da Expressao Infixa:\n");
    printf("     - Gerado:   %.4f\n", valorDeInfixa);
    printf("     - Esperado: %s (%.4f)\n", c.valorStr, c.valorFloat);
    printf("     - Resultado: %s\n", compararFloats(valorDeInfixa, c.valorFloat) ? "SUCESSO" : "FALHA");

    // 3. Teste: getValorPosFixa()
    float valorDePosfixa = getValorPosFixa(c.posfixa);
    printf("  3. Avaliacao da Expressao Posfixa:\n");
    printf("     - Gerado:   %.4f\n", valorDePosfixa);
    printf("     - Esperado: %s (%.4f)\n", c.valorStr, c.valorFloat);
    printf("     - Resultado: %s\n", compararFloats(valorDePosfixa, c.valorFloat) ? "SUCESSO" : "FALHA");

    // 4. Teste: getFormaInFixa()

    char* infixaGerada = getFormaInFixa(c.posfixa);
    printf("  4. Conversao Posfixa -> Infixa:\n");
    printf("     - Gerada: \"%s\" (para verificacao manual)\n", infixaGerada ? infixaGerada : "ERRO_CONVERSAO");

    if (posfixaGerada) free(posfixaGerada);
    if (infixaGerada) free(infixaGerada);
    printf("\n");
}

int main() {
    printf("============================================\n");
    printf("  Avaliador de Expressoes Numericas - Testes\n");
    printf("============================================\n\n");

    printf("--------------------------------------------\n");
    printf("      Testes Originais do Enunciado\n");
    printf("--------------------------------------------\n");
    for (int i = 0; i < numCasosTestePDF; ++i) {
        executarTeste(casosDeTestePDF[i]);
    }

    printf("--------------------------------------------\n");
    printf("         Novos Testes Adicionais\n");
    printf("--------------------------------------------\n");
    for (int i = 0; i < numCasosTesteAdicionais; ++i) {
        executarTeste(casosDeTesteAdicionais[i]);
    }
    printf("Testes concluidos.\n");


    return 0;
}