// Autores: Adrian Lopez y Sofia Lopez para Microprocesadores

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

const char* productos[] = {"Pata", "Respaldo", "Asiento", "Pata", "Pata"};
const int numProductos = 5;
const int MAX_BUFFER = 5;
const int MAX_SILLAS = 3;

int buffer[MAX_BUFFER];
int in = 0;
int out = 0;
int sillasProducidas = 0;
bool produccionTerminada = false;  //Variable para indicar que se ha alcanzado el limite de sillas
int piezasSobrantes[numProductos] = {0};  //Arreglo para almacenar cuantas piezas de cada tipo sobran

//Semaforos y mutex
sem_t vacios;
sem_t llenos;
pthread_mutex_t mutex;

//Función simulacion de un productor (para fabricar una pieza de silla)
void* productor(void* arg) {
    int id = *(int*)arg;
    int piezaId;

    while (true) {
        //Verificar si ya se alcanzo el limite de sillas
        pthread_mutex_lock(&mutex);
        if (produccionTerminada) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        piezaId = rand() % numProductos;  //Seleccionar una pieza al azar

        sem_wait(&vacios);  //Espera hasta que haya espacio en el buffer
        pthread_mutex_lock(&mutex);  //Protege el acceso al buffer

        //verificar nuevamente si la produccion ha terminado mientras esta en la region critica
        if (produccionTerminada) {
            pthread_mutex_unlock(&mutex);
            sem_post(&vacios);  //Libera el semaforo por si otros productores estan bloqueados
            break;
        }

        buffer[in] = piezaId;  // Añade la pieza al buffer
        cout << "Productor " << id << " ha fabricado la pieza " << productos[piezaId]
             << " y la coloco en la posicion " << in << endl;
        in = (in + 1) % MAX_BUFFER;  // Avanza el indice del buffer

        pthread_mutex_unlock(&mutex);
        sem_post(&llenos);  //Incrementa el numero de productos disponibles

        sleep(1);  //Simula el tiempo de fabricacion
    }

    return NULL;
}

//Funcion simulación de un consumidor (ensambla una silla)
void* consumidor(void* arg) {
    int id = *(int*)arg;
    int piezaId;
    int piezasConsumidas = 0;

    while (true) {
        //Verificar si ya se ensamblaron todas las sillas
        pthread_mutex_lock(&mutex);
        if (sillasProducidas >= MAX_SILLAS) {
            produccionTerminada = true;  //Señala que la producción ha terminado
            pthread_mutex_unlock(&mutex);

            //Despertar a los hilos productores y consumidores restantes
            for (int i = 0; i < MAX_BUFFER; ++i) {
                sem_post(&vacios);  //Desbloquear a los productores
                sem_post(&llenos);  //Desbloquear a los consumidores
            }
            break;  //Salir del ciclo si se ensamblaron suficientes sillas
        }
        pthread_mutex_unlock(&mutex);

        sem_wait(&llenos);  //Espera hasta que existan productos disponibles
        pthread_mutex_lock(&mutex);  //Protege el acceso al buffer

        //Retirar una pieza del buffer
        piezaId = buffer[out];
        cout << "Consumidor " << id << " ha retirado la pieza " << productos[piezaId]
             << " de la posicion " << out << endl;
        out = (out + 1) % MAX_BUFFER;  //Avanza en el indice del buffer

        //Contar la pieza consumida y ensamblar una silla cuando se hayan consumido todas las necesarias
        piezasConsumidas++;
        if (piezasConsumidas == numProductos) {
            sillasProducidas++;
            cout << "Consumidor " << id << " ha ensamblado una silla completa. Sillas ensambladas: "
                 << sillasProducidas << "/" << MAX_SILLAS << endl;
            piezasConsumidas = 0;  //Reiniciar el contador de piezas consumidas
        }

        pthread_mutex_unlock(&mutex);
        sem_post(&vacios);  //Incrementa el numero de espacios vacios

        sleep(2);  //Simula el tiempo de ensamblaje
    }

    return NULL;
}

//Función para generar el reporte de las piezas sobrantes
void generarReporte() {
    //Contar las piezas sobrantes en el buffer
    for (int i = 0; i < MAX_BUFFER; ++i) {
        piezasSobrantes[buffer[i]]++;  //Incrementa el contador de piezas sobrantes por tipo
    }

    //Imprimir el reporte
    cout << "\n===== REPORTE FINAL =====" << endl;
    cout << "Total de sillas ensambladas: " << sillasProducidas << endl;
    cout << "Piezas sobrantes en el almacen: " << endl;
    for (int i = 0; i < numProductos; ++i) {
        cout << productos[i] << ": " << piezasSobrantes[i] << endl;
    }
}

int main() {
    int numProductores, numConsumidores;

    //Solicitar la cantidad de productores y consumidores
    cout << "Ingrese el numero de productores: ";
    cin >> numProductores;
    cout << "Ingrese el numero de consumidores: ";
    cin >> numConsumidores;

    pthread_t productores[100], consumidores[100];
    int idProductores[100], idConsumidores[100];

    //Inicializa semaforos y mutex
    sem_init(&vacios, 0, MAX_BUFFER);
    sem_init(&llenos, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    //Crea hilos productores
    for (int i = 0; i < numProductores; ++i) {
        idProductores[i] = i + 1;
        pthread_create(&productores[i], NULL, productor, &idProductores[i]);
    }

    //Crea hilos consumidores
    for (int i = 0; i < numConsumidores; ++i) {
        idConsumidores[i] = i + 1;
        pthread_create(&consumidores[i], NULL, consumidor, &idConsumidores[i]);
    }

    //Espera a que los hilos terminen
    for (int i = 0; i < numProductores; ++i) {
        pthread_join(productores[i], NULL);
    }

    for (int i = 0; i < numConsumidores; ++i) {
        pthread_join(consumidores[i], NULL);
    }

    // Generar el reporte antes de terminar
    generarReporte();

    //Destruye semáforos y mutex
    sem_destroy(&vacios);
    sem_destroy(&llenos);
    pthread_mutex_destroy(&mutex);

    return 0;
}
