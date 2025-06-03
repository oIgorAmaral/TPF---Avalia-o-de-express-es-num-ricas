#include <stdio.h>
#include <string.h> // Para strcpy, strcmp
#include <math.h>   // Para isnan, fabs, roundf
#include "expressao.h"

void imprimir_valor(float valor_calculado, float valor_esperado_tabela) {
    char buffer[64];

    if (isnan(valor_calculado)) {
        sprintf(buffer, "Erro (NAN)");
    } else {
        // Verifica se o valor esta muito proximo de um inteiro para impressao mais limpa
        if (fabs(valor_calculado - roundf(valor_calculado)) < 0.0001) {
            sprintf(buffer, "%.0f", valor_calculado);
        } else { // Float com casas decimais
            // Usa um numero razoavel de casas decimais, ex: 4 para precisao
            sprintf(buffer, "%.4f", valor_calculado);
        }
    }
    printf("%s", buffer);

    // Compara com o valor numerico esperado da tabela
    // Usando uma pequena tolerancia para comparacoes de ponto flutuante
    if ((fabs(valor_calculado - valor_esperado_tabela) < 0.01 && !isnan(valor_calculado) && !isnan(valor_esperado_tabela)) ||
        (isnan(valor_calculado) && isnan(valor_esperado_tabela))) {
        printf(" (CORRESPONDE ao valor numerico esperado: ~%.2f)\n", valor_esperado_tabela);
    } else {
        if (isnan(valor_esperado_tabela) && !isnan(valor_calculado)) {
            printf(" (DIFERENTE da tabela, que indicava erro/NAN mas foi calculado %s)\n", buffer);
        } else if (!isnan(valor_esperado_tabela) && isnan(valor_calculado)) {
            printf(" (DIFERENTE da tabela, que esperava ~%.2f mas foi calculado Erro/NAN)\n", valor_esperado_tabela);
        } else {
            printf(" (DIFERENTE do valor numerico esperado: ~%.2f)\n", valor_esperado_tabela);
        }
    }
}

void executar_teste_professor(int numero_teste, const char* tabela_infixa, const char* tabela_posfixa, const char* tabela_valor_str, float valor_esperado_float) {
    char string_gerada[512];
    float valor_calculado;

    printf("--- Teste %d ---\n", numero_teste);
    printf("Tabela Infixa:     %s\n", tabela_infixa);
    printf("Tabela PosFixa:    %s\n", tabela_posfixa);
    printf("Tabela Valor:      %s\n\n", tabela_valor_str);

    // A) Traduzir Infixa para PosFixa
    printf("A) Infixa -> PosFixa:\n");
    printf("  Processando Infixa da Tabela: \"%s\"\n", tabela_infixa);
    strcpy(string_gerada, getFormaPosFixa((char*)tabela_infixa)); // Cast para char* conforme assinatura da funcao
    printf("  PosFixa Gerada pelo Codigo:   \"%s\"\n", string_gerada);
    if (strcmp(string_gerada, tabela_posfixa) == 0) {
        printf("  Comparacao com Tabela PosFixa: CORRETO (Strings identicas)\n");
    } else {
        printf("  Comparacao com Tabela PosFixa: DIFERENTE (Tabela: \"%s\")\n", tabela_posfixa);
        // Nota: Uma diferenca na string nem sempre significa incorreto se matematicamente equivalente
        // mas para este exercicio, a correspondencia exata com a string posfixa da tabela eh frequentemente esperada.
    }
    printf("\n");

    // B) Traduzir PosFixa para Infixa
    printf("B) PosFixa -> Infixa:\n");
    printf("  Processando PosFixa da Tabela: \"%s\"\n", tabela_posfixa);
    strcpy(string_gerada, getFormaInFixa((char*)tabela_posfixa)); // Cast para char*
    printf("  Infixa Gerada pelo Codigo:     \"%s\"\n", string_gerada);
    // Para Infixa, a equivalencia matematica eh mais importante que a correspondencia exata da string devido aos parenteses.
    // Verificamos isso comparando a avaliacao da infixa da tabela e da infixa gerada.
    float valor_da_tabela_infixa_para_B = getValorInFixa((char*)tabela_infixa);
    float valor_da_infixa_gerada_para_B = getValorInFixa(string_gerada);

    if ((fabs(valor_da_tabela_infixa_para_B - valor_da_infixa_gerada_para_B) < 0.001 && !isnan(valor_da_tabela_infixa_para_B) && !isnan(valor_da_infixa_gerada_para_B)) ||
        (isnan(valor_da_tabela_infixa_para_B) && isnan(valor_da_infixa_gerada_para_B)) ) {
          printf("  Verificacao de Equivalencia (Valor): CORRETA (Infixa gerada eh matematicamente equivalente a Infixa da tabela)\n");
    } else {
          printf("  Verificacao de Equivalencia (Valor): INCORRETA ou DIVERGENTE\n");
    }
    printf("\n");

    // G) Avaliar expressoes
    printf("G) Avaliacao de Expressoes:\n");
    // G.1 Avaliar Infixa da Tabela
    printf("  Avaliando Infixa da Tabela (\"%s\"): ", tabela_infixa);
    valor_calculado = getValorInFixa((char*)tabela_infixa);
    imprimir_valor(valor_calculado, valor_esperado_float);

    // G.2 Avaliar PosFixa da Tabela
    printf("  Avaliando PosFixa da Tabela (\"%s\"): ", tabela_posfixa);
    valor_calculado = getValorPosFixa((char*)tabela_posfixa);
    imprimir_valor(valor_calculado, valor_esperado_float);

    printf("--------------------------------------------------\n\n");
}

int main() {
    printf("Avaliacao Conforme Tabela do Professor:\n");
    printf("==================================================\n\n");

    // Dados de teste da tabela:
    // Teste | Notacao Posfixa           | Notacao Infixa             | Valor           | Numerico Esperado
    executar_teste_professor(1, "(3+4)*5",               "3 4 + 5 *",                 "35",             35.0f);
    executar_teste_professor(2, "7*2+4",                 "7 2 * 4 +",                 "18",             18.0f);

    // Teste 3: O "Valor" da tabela (38) eh inconsistente com a Infixa fornecida (avalia para 88) e Posfixa (avalia para 104).
    // Usaremos 88.0f (da Infixa "8*(5+(2+4))") como o valor esperado para fins de comparacao para este caso de teste.
    executar_teste_professor(3, "8*(5+(2+4))",           "8 5 2 4 * + *",             "38 (Nota: Valor da tabela inconsistente)", 88.0f);

    executar_teste_professor(4, "(6/2+3)*4",               "6 2 / 3 + 4 *",             "24",             24.0f);

    // Teste 5: O "Valor" da tabela (109) eh inconsistente com a Infixa/Posfixa fornecida (avalia para 179).
    // Usaremos 179.0f (de "9+(5*(2+8*4))") como o valor esperado.
    executar_teste_professor(5, "9+(5*(2+8*4))",         "9 5 2 8 4 * + * +",         "109 (Nota: Valor da tabela inconsistente)", 179.0f);

    executar_teste_professor(6, "log(2+3)/5",             "2 3 + log 5 /",             "Aprox. 0.14",    0.139794f); // log10(5)/5

    // Teste 7: Tabela Infixa "(log10)^3+2". Interpretado como "(log(10))^3+2" para o parser.
    executar_teste_professor(7, "(log(10))^3+2",           "10 log 3 ^ 2 +",            "3",              3.0f);

    executar_teste_professor(8, "(45+60)*cos(30)",         "45 60 + 30 cos *",          "Aprox. 90.93",   90.932625f); // 105 * cos(30deg)

    // Teste 9: A imagem mostra Posfixa "0.5 45 sen 2 ^ +".
    executar_teste_professor(9, "sen(45)^2+0.5",           "0.5 45 sen 2 ^ +",          "1",              1.0f);

    printf("Fim da Avaliacao Conforme Tabela.\n");
    return 0;
}