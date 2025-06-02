// main.c (Versao Corrigida, Reduzida, Traduzida e Sem Acentos)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>    // Para fabs
#include "expressao.h" 

#define EPSILON_FLOAT 0.001f

// Estrutura para os casos de teste
typedef struct {
    const char* id;
    const char* infixaCruaPDF;
    const char* posfixaCruaPDFCorrigida; // Strings pos-fixadas corrigidas com '~' para todos os separadores
    const char* valorStrPDF;
    float valorFloatPDF;
} CasoTeste;

// Casos de teste com strings pos-fixadas ajustadas para correta tokenizacao
// O '$' no inicio e fim sera removido por prepararStringExpressao.
// '~' sera convertido para espaco.
CasoTeste casosDeTeste[] = {
    {"1", "$(3+4)*5$",                  "$3~4~+~5~*$", "35", 35.0f},
    {"2", "$7*2+4$",                    "$7~2~*~4~+$", "18", 18.0f},
    {"3", "$8+(5*(2+4))$",              "$8~5~2~4~+~*~+$", "38", 38.0f}, // PDF tinha "t" para "+", mantido como "+"
    {"4", "$(6/2+3)*4$",                "$6~2~/~3~+~4~*$", "24", 24.0f},
    // Teste 5: Infixa PDF (9+(5*(2+8*4))) avalia para 179.
    // Posfixa e Valor do PDF (correspondente a (9+(5*((2*8)+4)))) avaliam para 109.
    // A pos-fixa corrigida abaixo corresponde ao valor 109.
    {"5", "$9+(5*(2+8*4))$",          "$9~5~2~8~*~4~+~*~+$", "109", 109.0f},
    {"6", "$log(2+3)/5$",                "$2~3~+~log~5~/$", "Aprox. 0.14", 0.13979f},
    {"7", "$(log10)^3+2$",               "$10~log~3~^~2~+$", "3", 3.0f}, // (log 10)^3+2
    {"8", "$(45+60)*cos(30)$",           "$45~60~+~30~cos~*$", "Aprox. 90.93", 90.93267f},
    // Pos-fixa do PDF para Teste 9: "$0.5~45~sen~2^{\wedge}+$"
    // Para corresponder a sen(45)^2 + 0.5, a ordem na pos-fixa e "45 sen 2 ^ 0.5 +"
    {"9", "$sen(45)^2+0.5$",             "$45~sen~2~^~0.5~+$", "1", 1.0f}
};
int numCasosTeste = sizeof(casosDeTeste) / sizeof(casosDeTeste[0]);

// Funcao auxiliar para pre-processar strings: remove '$' do inicio/fim, converte '~' para espaco.
char* prepararStringExpressao(const char* strCrua) {
    if (!strCrua) return NULL;
    int lenCrua = strlen(strCrua);
    if (lenCrua == 0) {
        char* vazia = (char*)malloc(1);
        if (vazia) vazia[0] = '\0';
        return vazia;
    }

    char* processada = (char*)malloc(lenCrua + 1); // Aloca memoria suficiente
    if (!processada) {
        perror("Falha ao alocar memoria (prepararStringExpressao)");
        return NULL;
    }

    int inicio = 0;
    int fim = lenCrua;

    if (strCrua[0] == '$') {
        inicio = 1;
    }
    if (lenCrua > inicio && strCrua[lenCrua - 1] == '$') {
        fim = lenCrua - 1;
    }

    int j = 0; // Indice para a string processada
    for (int i = inicio; i < fim; i++) {
        if (strCrua[i] == '~') {
            processada[j++] = ' ';
        } else {
            processada[j++] = strCrua[i];
        }
    }
    processada[j] = '\0';
    return processada;
}

// Funcao auxiliar para comparar floats com tolerancia
int compararFloats(float f1, float f2) {
    if (isnan(f1) && isnan(f2)) return 1; // Ambos NaN e considerado "igual" para este teste
    if (isnan(f1) || isnan(f2)) return 0; // Um e NaN, o outro nao
    return fabs(f1 - f2) < EPSILON_FLOAT;
}

int main() {
    printf("Avaliador de Expressoes Numericas - Testes\n");
    printf("--------------------------------------------\n");

    for (int i = 0; i < numCasosTeste; ++i) {
        CasoTeste c = casosDeTeste[i];
        printf("Teste %s:\n", c.id);

        char* infixaOriginalFormatada = prepararStringExpressao(c.infixaCruaPDF);
        char* posfixaEsperadaFormatada = prepararStringExpressao(c.posfixaCruaPDFCorrigida);

        if (!infixaOriginalFormatada || !posfixaEsperadaFormatada) {
            printf("  Erro ao preparar strings para Teste %s. Pulando.\n\n", c.id);
            free(infixaOriginalFormatada); 
            free(posfixaEsperadaFormatada);
            continue;
        }

        // --- 1. Testando com a Infixa do PDF ---
        printf("  Infixa Fornecida (crua): \"%s\"\n", c.infixaCruaPDF);
        
        char* posfixaGerada = getFormaPosFixa(infixaOriginalFormatada);
        printf("    Pos-fixa -> Gerada:\"%s\", Esperada (formatada):\"%s\" (%s)\n",
               posfixaGerada ? posfixaGerada : "ERRO_CONVERSAO",
               posfixaEsperadaFormatada,
               (posfixaGerada && strcmp(posfixaGerada, posfixaEsperadaFormatada) == 0) ? "SUCESSO" : "FALHA/VERIFICAR");

        float valorDeInfixa = getValorInFixa(infixaOriginalFormatada);
        int sucessoValorInfixa = compararFloats(valorDeInfixa, c.valorFloatPDF);
        
        if (strcmp(c.id, "5") == 0 && !sucessoValorInfixa && compararFloats(valorDeInfixa, 179.0f)) {
             printf("    Valor Infixa -> Gerado:%.4f (Nota: Infixa PDF resulta em 179.0), Esperado (PDF):%s (%.4f) (AVISO: Inconsistencia PDF)\n",
                   valorDeInfixa, c.valorStrPDF, c.valorFloatPDF);
        } else {
            printf("    Valor Infixa -> Gerado:%.4f, Esperado:%s (%.4f) (%s)\n",
                   valorDeInfixa, c.valorStrPDF, c.valorFloatPDF,
                   sucessoValorInfixa ? "SUCESSO" : "FALHA");
        }
        if(posfixaGerada) free(posfixaGerada);

        // --- 2. Testando com a Posfixa do PDF (agora usando a corrigida) ---
        printf("  Posfixa Fornecida (crua): \"%s\"\n", c.posfixaCruaPDFCorrigida);

        char* infixaGerada = getFormaInFixa(posfixaEsperadaFormatada); // Usa a pos-fixa ja formatada
        // Para comparacao, a infixaOriginalFormatada e a referencia "limpa" da infixa do PDF
        printf("    Infixa -> Gerada:\"%s\", Esperada (formatada):\"%s\" (%s)\n",
               infixaGerada ? infixaGerada : "ERRO_CONVERSAO",
               infixaOriginalFormatada, 
               (infixaGerada && strcmp(infixaGerada, infixaOriginalFormatada) == 0) ? "SUCESSO" : "FALHA/VERIFICAR_FORMATO");
        
        float valorDePosfixa = getValorPosFixa(posfixaEsperadaFormatada); // Usa a pos-fixa ja formatada
        printf("    Valor Posfixa -> Gerado:%.4f, Esperado:%s (%.4f) (%s)\n",
               valorDePosfixa, c.valorStrPDF, c.valorFloatPDF,
               compararFloats(valorDePosfixa, c.valorFloatPDF) ? "SUCESSO" : "FALHA");

        if(infixaGerada) free(infixaGerada);
        
        free(infixaOriginalFormatada);
        free(posfixaEsperadaFormatada);
        printf("\n");
    }

    printf("--------------------------------------------\n");
    printf("Testes concluidos.\n");
    printf("Lembre-se de compilar com: gcc main.c expressao.c -o nome_do_executavel -lm\n");

    return 0;
}
