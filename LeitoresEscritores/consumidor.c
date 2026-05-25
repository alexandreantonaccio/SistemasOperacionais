/*
 * COMPILAÇÃO: gcc leitores_escritores_v1_leitura_suja.c -o rw_v1 -lpthread
 * * VERSÃO 1: Leitores e escritores sem "prioridade".
 * Ocorre "Leitura Suja". Os escritores bloqueiam uns aos outros (para não corromper 
 * totalmente os dados), mas os leitores NÃO usam bloqueio. Eles acessam os dados 
 * enquanto os escritores estão no meio de uma transferência, causando leitura de dados inconsistentes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_LEITORES 5
#define NUM_ESCRITORES 2
#define NUM_OPERACOES 3

// Variáveis Compartilhadas (O Banco)
int contaA = 500;
int contaB = 500;
pthread_mutex_t mutex_escrita; // Apenas para organizar os escritores entre si

void* escritor(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < NUM_OPERACOES; i++) {
        int valor = (rand() % 100) + 1; // Valor aleatório para transferir
        
        printf("[Escritor %d] Criado e tentando entrar na RC...\n", id);
        
        pthread_mutex_lock(&mutex_escrita); // Entra na Região Crítica
        printf("[Escritor %d] Entrou na RC. Iniciando transferencia de %d da Conta A para B.\n", id, valor);
        
        // Simula uma operação demorada para forçar a leitura suja
        contaA -= valor;
        usleep(100000); // Dorme por 100ms. AQUI OCORRE A INCONSISTÊNCIA!
        contaB += valor;
        
        printf("[Escritor %d] Transferencia concluida. (ContaA: %d, ContaB: %d, Total: %d). Saindo da RC.\n", 
               id, contaA, contaB, contaA + contaB);
        pthread_mutex_unlock(&mutex_escrita); // Sai da Região Crítica
        
        sleep(1); // Espera antes da proxima transferencia
    }
    printf("[Escritor %d] Finalizado.\n", id);
    pthread_exit(NULL);
}

void* leitor(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < NUM_OPERACOES * 2; i++) {
        printf("[Leitor %d] Lendo dados (SEM BLOQUEIO)...\n", id);
        
        // Lê os dados sem nenhum controle de concorrência (Leitura Suja)
        int a = contaA;
        int b = contaB;
        int total = a + b;
        
        if (total != 1000) {
            printf("\n---> [ALERTA LEITURA SUJA - Leitor %d] Total inconsistente! ContaA: %d, ContaB: %d, Total: %d\n\n", id, a, b, total);
        } else {
            printf("[Leitor %d] Leitura limpa. Total: %d\n", id, total);
        }
        
        usleep(50000); // Lê com mais frequência
    }
    printf("[Leitor %d] Finalizado.\n", id);
    pthread_exit(NULL);
}

int main() {
    pthread_t leitores[NUM_LEITORES], escritores[NUM_ESCRITORES];
    int id_leitores[NUM_LEITORES], id_escritores[NUM_ESCRITORES];

    srand(time(NULL));
    pthread_mutex_init(&mutex_escrita, NULL);

    printf("Sistema Bancario Iniciado. Saldo Total Inicial: %d\n\n", contaA + contaB);

    for (int i = 0; i < NUM_ESCRITORES; i++) {
        id_escritores[i] = i + 1;
        pthread_create(&escritores[i], NULL, escritor, &id_escritores[i]);
    }
    for (int i = 0; i < NUM_LEITORES; i++) {
        id_leitores[i] = i + 1;
        pthread_create(&leitores[i], NULL, leitor, &id_leitores[i]);
    }

    for (int i = 0; i < NUM_ESCRITORES; i++) pthread_join(escritores[i], NULL);
    for (int i = 0; i < NUM_LEITORES; i++) pthread_join(leitores[i], NULL);

    pthread_mutex_destroy(&mutex_escrita);
    printf("\nProcessamento concluido.\n");
    return 0;
}