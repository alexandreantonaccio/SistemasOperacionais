/*
 * COMPILAÇÃO: gcc leitores_escritores_v2_prioridade_escritor.c -o rw_v2 -lpthread
 * * VERSÃO 2: Escritores com prioridade sobre leitores.
 * Solução clássica "Writers-Preference". Novos leitores não podem acessar os 
 * dados se houver algum escritor esperando. Não ocorre leitura suja.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_LEITORES 5
#define NUM_ESCRITORES 2
#define NUM_OPERACOES 3

int contaA = 500, contaB = 500;

// Controles de Concorrência
int read_count = 0, write_count = 0;
pthread_mutex_t rmutex, wmutex;
sem_t recurso, read_try;

void* escritor(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < NUM_OPERACOES; i++) {
        int valor = (rand() % 100) + 1;
        
        printf("[Escritor %d] Criado. Tentando obter prioridade...\n", id);
        
        // Protocolo de Entrada - Escritor
        pthread_mutex_lock(&wmutex);
        write_count++;
        if (write_count == 1) sem_wait(&read_try); // Bloqueia novos leitores
        pthread_mutex_unlock(&wmutex);
        
        sem_wait(&recurso); // Entra na RC
        
        printf("[Escritor %d] Entrou na RC. Bloqueou leitores. Transferindo %d...\n", id, valor);
        contaA -= valor;
        usleep(100000); // Simulando lentidão
        contaB += valor;
        printf("[Escritor %d] Transferencia finalizada. (Total: %d). Saindo da RC.\n", id, contaA + contaB);
        
        sem_post(&recurso); // Sai da RC
        
        // Protocolo de Saída - Escritor
        pthread_mutex_lock(&wmutex);
        write_count--;
        if (write_count == 0) sem_post(&read_try); // Libera leitores se não houver mais escritores
        pthread_mutex_unlock(&wmutex);
        
        sleep(1);
    }
    printf("[Escritor %d] Finalizado.\n", id);
    pthread_exit(NULL);
}

void* leitor(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < NUM_OPERACOES * 2; i++) {
        printf("[Leitor %d] Tentando ler...\n", id);
        
        // Protocolo de Entrada - Leitor
        sem_wait(&read_try); // Espera se houver escritores ativos/aguardando
        pthread_mutex_lock(&rmutex);
        read_count++;
        if (read_count == 1) sem_wait(&recurso); // Primeiro leitor bloqueia o recurso
        pthread_mutex_unlock(&rmutex);
        sem_post(&read_try);
        
        // Região Crítica (Leitura Limpa garantida)
        int total = contaA + contaB;
        printf("[Leitor %d] Lendo na RC. Total na conta: %d\n", id, total);
        usleep(50000);
        
        // Protocolo de Saída - Leitor
        pthread_mutex_lock(&rmutex);
        read_count--;
        if (read_count == 0) sem_post(&recurso); // Último leitor libera o recurso
        pthread_mutex_unlock(&rmutex);
        
        sleep(1);
    }
    printf("[Leitor %d] Finalizado.\n", id);
    pthread_exit(NULL);
}

int main() {
    pthread_t leitores[NUM_LEITORES], escritores[NUM_ESCRITORES];
    int id_leitores[NUM_LEITORES], id_escritores[NUM_ESCRITORES];

    pthread_mutex_init(&rmutex, NULL);
    pthread_mutex_init(&wmutex, NULL);
    sem_init(&recurso, 0, 1);
    sem_init(&read_try, 0, 1);

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

    return 0;
}