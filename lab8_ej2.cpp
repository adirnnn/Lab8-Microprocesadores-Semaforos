#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

double saldo = 100000.00;  
sem_t acceso;  
pthread_mutex_t mutex;  

void* cajero(void* arg) {
    double monto;
    

    sem_wait(&acceso);

    pthread_mutex_lock(&mutex);

    printf("Ingrese el monto que desea retirar: ");
    scanf("%lf", &monto);  

    printf("Retirando Q%.2f\n", monto);
    

    if (monto <= saldo) {
        saldo -= monto;
        printf("Cliente retiro Q%.2f.\n", monto);
        printf("Saldo restante: Q%.2f\n", saldo);
        
    } 
    else {
        printf("Saldo insuficiente: Q%.2f\n", saldo);

    }

    pthread_mutex_unlock(&mutex);

    sem_post(&acceso);

    return NULL;
}

int main() {
    int num_clientes;

    // Solicitar la cantidad de clientes
    printf("Ingrese la cantidad de clientes: ");
    scanf("%d", &num_clientes);

    pthread_t clientes[num_clientes];  

    // Inicializar semáforo y mutex
    sem_init(&acceso, 0, 1);  
    pthread_mutex_init(&mutex, NULL);  

    // Crear los hilos para cada cliente
    for (int i = 0; i < num_clientes; i++) {
        pthread_create(&clientes[i], NULL, cajero, NULL);
    }

    for (int i = 0; i < num_clientes; i++) {
        pthread_join(clientes[i], NULL);
    }

    // Destruir el semáforo y el mutex
    sem_destroy(&acceso);
    pthread_mutex_destroy(&mutex);

    return 0;
}
