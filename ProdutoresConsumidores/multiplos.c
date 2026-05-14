/*
 * COMPILAÇÃO: gcc produtor_consumidor_v2_n_consumidores.c -o pc_v2 -lpthread
 * * VERSÃO 2: Vários Produtores e Vários Consumidores.
 * A lógica dos semáforos é robusta o suficiente para escalar naturalmente 
 * para N produtores e M consumidores sem alterações na função das threads.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TAM_BUFFER 5
#define NUM_PRODUTORES 3
#define NUM_CONSUMIDORES 2
#define ITENS_POR_PRODUTOR 4

int buffer[TAM_BUFFER];
int in = 0, out = 0;
int itens_consumidos = 0; // Para controle de parada dos consumidores

sem_t empty; 
sem_t full;  
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
        int item = (id * 10) + i; 
        
        sem_wait(&empty); 
        pthread_mutex_lock(&mutex_buffer); 
        
        printf("[Produtor %d] Status: PRODUZINDO item %d.\n", id, item);
        buffer[in] = item;
        in = (in + 1) % TAM_BUFFER;
        imprimir_buffer();
        
        pthread_mutex_unlock(&mutex_buffer); 
        sem_post(&full); 
        
        usleep(500000); // 0.5 seg
    }
    pthread_exit(NULL);
}

void* consumidor(void* arg) {
    int id = *((int*)arg);
    int total_gerado = NUM_PRODUTORES * ITENS_POR_PRODUTOR;
    
    while(1) {
        sem_wait(&full); 
        pthread_mutex_lock(&mutex_buffer); 
        
        if (itens_consumidos >= total_gerado) {
            // Se já consumiu tudo, sai e libera para outros sairem
            pthread_mutex_unlock(&mutex_buffer);
            sem_post(&full); 
            break;
        }

        int item = buffer[out];
        buffer[out] = 0; 
        out = (out + 1) % TAM_BUFFER;
        itens_consumidos++;
        
        printf("[Consumidor %d] Status: CONSUMINDO item %d.\n", id, item);
        imprimir_buffer();
        
        pthread_mutex_unlock(&mutex_buffer); 
        sem_post(&empty); 
        
        sleep(1); // Consumidor mais lento
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t prods[NUM_PRODUTORES], cons[NUM_CONSUMIDORES];
    int ids[10];

    for(int i=0; i<TAM_BUFFER; i++) buffer[i] = 0;
    sem_init(&empty, 0, TAM_BUFFER);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex_buffer, NULL);

    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        ids[i] = i + 1;
        pthread_create(&cons[i], NULL, consumidor, &ids[i]);
    }
    for (int i = 0; i < NUM_PRODUTORES; i++) {
        ids[i+5] = i + 1;
        pthread_create(&prods[i], NULL, produtor, &ids[i+5]);
    }

    for (int i = 0; i < NUM_PRODUTORES; i++) pthread_join(prods[i], NULL);
    for (int i = 0; i < NUM_CONSUMIDORES; i++) pthread_join(cons[i], NULL);

    return 0;
}