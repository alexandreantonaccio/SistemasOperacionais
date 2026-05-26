/*
 * COMPILAÇÃO: gcc leitores_escritores_v3_sem_controle.c -o rw_v3 -lpthread
 * * VERSÃO 3: Sem nenhum controle de concorrência.
 * Demonstra a catástrofe que ocorre com acesso simultâneo. O saldo total será 
 * corrompido rapidamente devido a "Race Conditions" entre os próprios escritores.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_LEITORES 5
#define NUM_ESCRITORES 3
#define NUM_OPERACOES 4

int contaA = 500, contaB = 500;

void* escritor(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < NUM_OPERACOES; i++) {
        int valor = (rand() % 50) + 1;
        
        printf("[Escritor %d] Transferindo %d (SEM RC)...\n", id, valor);
        
        int tempA = contaA;
        usleep(rand() % 50000); // Dorme para aumentar chance de colisão
        contaA = tempA - valor;
        int tempB = contaB;
        usleep(rand() % 50000);
        contaB = tempB + valor;
        
        printf("[Escritor %d] Terminou de escrever.\n", id);
    }
    pthread_exit(NULL);
}

void* leitor(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < NUM_OPERACOES * 2; i++) {
        int a = contaA;
        int b = contaB;
        printf("[Leitor %d] Leu saldo -> ContaA: %d | ContaB: %d | Total: %d\n", id, a, b, a + b);
        usleep(50000);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t leitores[NUM_LEITORES], escritores[NUM_ESCRITORES];
    int ids[10];

    printf("Sistema Bancario Iniciado. Saldo Total Inicial: 1000.\n\n");

    for (int i = 0; i < NUM_ESCRITORES; i++) {
        ids[i] = i + 1;
        pthread_create(&escritores[i], NULL, escritor, &ids[i]);
    }
    for (int i = 0; i < NUM_LEITORES; i++) {
        ids[i+5] = i + 1;
        pthread_create(&leitores[i], NULL, leitor, &ids[i+5]);
    }

    for (int i = 0; i < NUM_ESCRITORES; i++) pthread_join(escritores[i], NULL);
    for (int i = 0; i < NUM_LEITORES; i++) pthread_join(leitores[i], NULL);

    printf("\nResultado Final (Deveria ser 1000): %d\n", contaA + contaB);
    return 0;
}