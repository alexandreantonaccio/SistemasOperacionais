/*
 * COMPILAÇÃO: gcc produtor_consumidor_v1_1_consumidor.c -o pc_v1 -lpthread
 * * VERSÃO 1: Vários Produtores e 1 Consumidor.
 * Controle com Mutex para acesso ao vetor e Semáforos para contagem (vazio/cheio).
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TAM_BUFFER 5
#define NUM_PRODUTORES 3
#define NUM_CONSUMIDORES 1
#define ITENS_POR_PRODUTOR 4

int buffer[TAM_BUFFER];
int in = 0, out = 0;

// Primitivas de sincronização
sem_t empty; // Vagas livres
sem_t full;  // Vagas ocupadas
pthread_mutex_t mutex_buffer;

void imprimir_buffer() {
    printf("BUFFER: [ ");
    for(int i=0; i<TAM_BUFFER; i++) {
        if(buffer[i] == 0) printf("- ");
        else printf("%d ", buffer[i]);
    }
    printf("]\n");
}

void* produtor(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < ITENS_POR_PRODUTOR; i++) {
        int item = (id * 100) + i; // Gera um dado (ex: produtor 2 gera 200, 201...)
        
        printf("[Produtor %d] Produziu o item %d. Dormindo/Aguardando vaga...\n", id, item);
        
        sem_wait(&empty); // Se buffer cheio, bloqueia aqui ("dorme")
        pthread_mutex_lock(&mutex_buffer); // Entra RC
        
        printf("[Produtor %d] Entrou na RC.\n", id);
        buffer[in] = item;
        in = (in + 1) % TAM_BUFFER;
        imprimir_buffer();
        printf("[Produtor %d] Saiu da RC.\n", id);
        
        pthread_mutex_unlock(&mutex_buffer); // Sai RC
        sem_post(&full); // Avisa que tem novo item
        
        usleep((rand() % 500) * 1000); // Atraso aleatório
    }
    pthread_exit(NULL);
}

void* consumidor(void* arg) {
    int id = *((int*)arg);
    int total_consumir = NUM_PRODUTORES * ITENS_POR_PRODUTOR;
    
    for (int i = 0; i < total_consumir; i++) {
        printf("[Consumidor %d] Aguardando item no buffer (Dormindo se vazio)...\n", id);
        
        sem_wait(&full); // Se vazio, bloqueia ("dorme")
        pthread_mutex_lock(&mutex_buffer); // Entra RC
        
        int item = buffer[out];
        buffer[out] = 0; // Limpa para visualização
        out = (out + 1) % TAM_BUFFER;
        
        printf("[Consumidor %d] Entrou na RC. Consumiu o item %d.\n", id, item);
        imprimir_buffer();
        printf("[Consumidor %d] Saiu da RC.\n", id);
        
        pthread_mutex_unlock(&mutex_buffer); // Sai RC
        sem_post(&empty); // Avisa que liberou vaga
        
        sleep(1); // Consumidor mais lento
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t prods[NUM_PRODUTORES], cons[NUM_CONSUMIDORES];
    int id_prods[NUM_PRODUTORES], id_cons[NUM_CONSUMIDORES];

    // Inicialização
    for(int i=0; i<TAM_BUFFER; i++) buffer[i] = 0;
    sem_init(&empty, 0, TAM_BUFFER);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex_buffer, NULL);

    // Criação de threads
    id_cons[0] = 1;
    pthread_create(&cons[0], NULL, consumidor, &id_cons[0]);

    for (int i = 0; i < NUM_PRODUTORES; i++) {
        id_prods[i] = i + 1;
        pthread_create(&prods[i], NULL, produtor, &id_prods[i]);
    }

    // Join
    for (int i = 0; i < NUM_PRODUTORES; i++) pthread_join(prods[i], NULL);
    pthread_join(cons[0], NULL);

    return 0;
}