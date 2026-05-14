/*
 * COMPILAÇÃO: gcc produtor_consumidor_v3_sem_controle.c -o pc_v3 -lpthread
 * * VERSÃO 3: Sem controle de concorrência.
 * Demonstra a sobrescrita de buffer e consumo de lixo/espaço vazio,
 * pois não há mutex nem semáforo para gerenciar o in/out e os limites do buffer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define TAM_BUFFER 5
#define NUM_PRODUTORES 2
#define NUM_CONSUMIDORES 2
#define ITENS_POR_PRODUTOR 4

int buffer[TAM_BUFFER];
int in = 0, out = 0;

void imprimir_buffer() {
    printf("BUFFER CAÓTICO: [ ");
    for(int i=0; i<TAM_BUFFER; i++) {
        printf("%d ", buffer[i]);
    }
    printf("]\n");
}

void* produtor(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < ITENS_POR_PRODUTOR; i++) {
        int item = (id * 10) + i; 
        
        printf("[Produtor %d] Produzindo %d (Sem Semáforo/Mutex)\n", id, item);
        
        // Colocando no buffer sem proteção. Ocorre sobrescrita!
        buffer[in] = item;
        usleep(50000); // Pausa no meio da operação para forçar erro de in
        in = (in + 1) % TAM_BUFFER;
        
        imprimir_buffer();
        sleep(1); 
    }
    pthread_exit(NULL);
}

void* consumidor(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < ITENS_POR_PRODUTOR; i++) {
        
        // Consumindo sem verificar se está vazio. Vai ler lixo (zeros antigos ou lixo de memória)!
        int temp_out = out;
        usleep(20000); // Pausa para forçar conflito no ponteiro out
        int item = buffer[temp_out];
        buffer[temp_out] = 0; // Limpa slot
        out = (temp_out + 1) % TAM_BUFFER;
        
        printf("----> [ERRO/Consumo] [Consumidor %d] Consumiu %d na posicao %d\n", id, item, temp_out);
        
        sleep(1); 
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t prods[NUM_PRODUTORES], cons[NUM_CONSUMIDORES];
    int ids[5];

    for(int i=0; i<TAM_BUFFER; i++) buffer[i] = 0;

    printf("Iniciando Produtor/Consumidor SEM CONTROLE.\n");
    printf("Observe mensagens de erro e valores zerados sendo consumidos.\n\n");

    for (int i = 0; i < NUM_PRODUTORES; i++) {
        ids[i] = i + 1;
        pthread_create(&prods[i], NULL, produtor, &ids[i]);
    }
    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        ids[i+3] = i + 1;
        pthread_create(&cons[i], NULL, consumidor, &ids[i+3]);
    }

    for (int i = 0; i < NUM_PRODUTORES; i++) pthread_join(prods[i], NULL);
    for (int i = 0; i < NUM_CONSUMIDORES; i++) pthread_join(cons[i], NULL);

    return 0;
}