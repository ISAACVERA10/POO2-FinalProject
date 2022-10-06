//
// Created by Francisco Magot on 14/06/21.
//
#ifndef PROYECTO_MAPA_H
#define PROYECTO_MAPA_H
#include <queue>
#include <vector>
#include <mutex>
#include <cmath>
#include <thread>
using namespace std;


enum Estado : char {
    Libre = '0',
    Inicio = 'I',
    Fin = 'F',
    Obstaculo = '*',
    Camino = '|',
    Visitado = '_'
};

struct Nodo {
    int x;
    int y;
    float local;
    float global;
    bool es_obstaculo;
    bool visitado;
    Estado estado;
    vector<Nodo*> nodos_adyacentes;
    Nodo* nodo_padre;
};

mutex mtx;

class Mapa {
    Nodo* mapa;
    int dimension;
    int longitud = 0;
    Nodo* nodo_inicial;
    Nodo* nodo_final;
public:
    Mapa() = default;
    Mapa(int tamanio) {
        this->dimension = tamanio;
        mapa = new Nodo[tamanio*tamanio];
        for (int x = 0; x < tamanio; x++) {
            for (int y = 0; y < tamanio; y++) {
                mapa[y*dimension + x].x = x;
                mapa[y*dimension + x].y = y;
                mapa[y*dimension + x].es_obstaculo = false;
                mapa[y*dimension + x].estado = Libre;
                mapa[y*dimension + x].visitado = false;
                mapa[y*dimension + x].nodo_padre = nullptr;
            }
        }
        nodo_inicial = &mapa[0];
        nodo_inicial->estado = Inicio;
        nodo_final = &mapa[tamanio*(tamanio)-1];
        nodo_final->estado = Fin;
    }

    void set_inicio(int x, int y) {
        if (x < dimension && y < dimension) {
            nodo_inicial->estado = Libre;
            nodo_inicial = &mapa[y*dimension + x];
            nodo_inicial->estado = Inicio;
        }
    }

    void set_fin(int x, int y) {
        if (x < dimension && y < dimension) {
            nodo_final->estado = Libre;
            nodo_final = &mapa[y * dimension + x];
            nodo_final->estado = Fin;
        }
    }

    Nodo* get_mapa() {
        return this->mapa;
    }

    int get_dimension() {
        return this->dimension;
    }

    int get_longitud() {
        return this->longitud;
    }

    void crear_obstaculos(int num_obstaculos) {
        srand(time(NULL));
        if (num_obstaculos <= ((dimension * dimension) / 5)) {
            // Cantidad de obstáculos restringida a 20% de los nodos totales para evitar problemas seguidos de encierro de nodo inicial o final
            for (int i = 0; i < num_obstaculos; i++) {
                int x = rand() % dimension;
                int y = rand() % dimension;
                // Generar coordenadas aleatorias dentro de las dimensiones del mapa para incluir obstáculos
                if (!mapa[y*dimension + x].es_obstaculo && !(&mapa[y * dimension + x] == nodo_inicial) && !(&mapa[y * dimension + x] == nodo_final)) {
                    // En caso la coordenada aleatoria seleccionada es el nodo inicial o el nodo final, entonces no se volverá a generar un nuevo par de coordenadas
                    mapa[y*dimension + x].es_obstaculo = true;
                    mapa[y*dimension + x].estado = Obstaculo;
                }
                else {
                    i--;
                }
            }
        }
    }

    void actualizar_nodos_adyacentes() {
        for (int x = 0; x < dimension; x++) {
            for (int y = 0; y < dimension; y++) {
                if (y > 0 && !(mapa[(y - 1) * dimension + (x + 0)].es_obstaculo)) {
                    // Restringir la coordenada y para poder revisar el nodo inferior sin tener problemas de acceso al arreglo dinámico
                    // Si el nodo inferior es un obstáculo, entonces no se agregará al vector de nodos adyacentes
                    mapa[y * dimension + x].nodos_adyacentes.push_back(&mapa[(y - 1) * dimension + (x + 0)]);
                }
                if (y < dimension - 1 && !mapa[(y + 1) * dimension + (x + 0)].es_obstaculo) {
                    // Restringir la coordenada y para poder revisar el nodo superior sin tener problemas de acceso al arreglo dinámico
                    // Si el nodo inferior es un obstáculo, entonces no se agregará al vector de nodos adyacentes
                    mapa[y * dimension + x].nodos_adyacentes.push_back(&mapa[(y + 1) * dimension + (x + 0)]);
                }
                if (x > 0 && !mapa[(y + 0) * dimension + (x - 1)].es_obstaculo) {
                    // Restringir la coordenada y para poder revisar el nodo izquierdo sin tener problemas de acceso al arreglo dinámico
                    // Si el nodo inferior es un obstáculo, entonces no se agregará al vector de nodos adyacentes
                    mapa[y * dimension + x].nodos_adyacentes.push_back(&mapa[(y + 0) * dimension + (x - 1)]);
                }
                if (x < dimension - 1 && !mapa[(y + 0) * dimension + (x + 1)].es_obstaculo) {
                    // Restringir la coordenada y para poder revisar el nodo derecho sin tener problemas de acceso al arreglo dinámico
                    // Si el nodo inferior es un obstáculo, entonces no se agregará al vector de nodos adyacentes
                    mapa[y * dimension + x].nodos_adyacentes.push_back(&mapa[(y + 0) * dimension + (x + 1)]);
                }
            }
        }
    }

    void actualizar_camino_final() {
        Nodo* current = nodo_final->nodo_padre;
        while (current != nullptr) {
            if (current == nodo_inicial) { // Si se llega al nodo inicial, este se mantendrá marcado con una 'I' para representar que es el nodo inicial
                current->estado = Inicio;
            }
            else { // Caso contrario, el nodo será marcado como parte del camino utilizando un '|'
                current->estado = Camino;
            }
            longitud++;
            current = current->nodo_padre; // Seleccionar el nodo padre
        }
    }

    template <typename Cont, typename Lambda>
    void actualizar_adyacentes_thread(Nodo* actual, Nodo* adyacente, Cont& nodos_por_seleccionar, Lambda heuristica) {
        lock_guard<mutex> lg(mtx); // Evitar race condition
        if (!adyacente->visitado && !adyacente->es_obstaculo) {
            nodos_por_seleccionar.push(adyacente); // En caso no haya sido visitado el nodo previamente y este nodo es un obstaculo, se agregará a la lista de nodos por seleccionar
        }
        float nuevo_local = actual->local + heuristica(actual, adyacente); // Cálculo de nuevo valor local para comparar
        if (nuevo_local < adyacente->local) { // Si el nuevo valor local es menor al valor local del nodo adyacente se actualizará el nodo
            adyacente->nodo_padre = actual; // Set nuevo padre
            adyacente->local = nuevo_local; // Set nuevo valor local
            adyacente->global = adyacente->local + heuristica(adyacente, nodo_final); // Set nuevo valor global
        }
    }

    template <typename Lambda1, typename Lambda2>
    void a_star(Lambda1 comparador, Lambda2 heuristica) {
        actualizar_nodos_adyacentes(); // Actualizar vector de nodos adyacentes en el mapa para poder iterar por ellos
        for (int x = 0; x < dimension; x++) { // Inicializar todos los nodos con valores locales y globales de infinito, así como marcarlos como no visitados y sin padre
            for (int y = 0; y < dimension; y++) {
                mapa[y * dimension + x].visitado = false;
                mapa[y * dimension + x].global = INFINITY;
                mapa[y * dimension + x].local = INFINITY;
                mapa[y * dimension + x].nodo_padre = nullptr;
            }
        }
        Nodo *actual = nodo_inicial; // Se selecciona el nodo inicial
        actual->local = 0.0f; // Se inicializa el nodo inicial con valor local de 0
        actual->global = heuristica(nodo_inicial, nodo_final); // se incializa el nodo inicial con la funcion heuristica evaluada en el nodo inicial y el nodo final
        priority_queue<Nodo*, vector<Nodo*>, decltype(comparador)> nodos_por_seleccionar(comparador); // Se declara el minheap de nodos por seleccionar haciendo uso de la funcion
        // lambda proporcionada en los parametros.
        nodos_por_seleccionar.push(nodo_inicial); // Se agrega el nodo inicial al minheap de nodos por selccionar
        while (!nodos_por_seleccionar.empty() && actual != nodo_final) {
            while (!nodos_por_seleccionar.empty() && nodos_por_seleccionar.top()->visitado) { // Si el nodo con menor valor global ya ha sido visitado, se elimina
                nodos_por_seleccionar.pop();
            }

            if (nodos_por_seleccionar.empty()) {
                break; // Si ya no existen nodos por seleccionar, se rompe el loop. Esto se ejecuta cuando el nodo inicial o final se encuentra acorralado por obstáculos y
                // no permite que ningún camino ingrese
            }
            actual = nodos_por_seleccionar.top(); // Se selecciona el nodo con menor valor global
            actual->visitado = true; // Se marca como visitado
            if (actual != nodo_final && actual != nodo_inicial) {
                actual->estado = Visitado; // Para no perder la marca de nodo inicial o final, todos los nodos seleccionados menos el inicio y final serán marcados con un '_'
            }
            vector<thread> threads(actual->nodos_adyacentes.size()); // Crear el vector de threads para iterar por los nodos adyacentes de manera paralela
            for (int i = 0; i < threads.size(); i++) {
                Nodo* adyacente = actual->nodos_adyacentes.at(i);
                threads.at(i) = thread(&Mapa::actualizar_adyacentes_thread<decltype(nodos_por_seleccionar), decltype(heuristica)>, this, actual, adyacente, ref(nodos_por_seleccionar), heuristica);
                // Inicializar el thread con la función y parámetros que deberá ejectuar
            }
            for (auto& t: threads) {
                t.join();
            }
        }
        actualizar_camino_final(); // En base a los nodos padres, se trazará el camino mínimo que ha sido calculado.
    }
};
#endif //PROYECTO_MAPA_H
