#include <iostream>
#include <vector>
#include "mapa.h"
using namespace std;

int main() {
    int tamanio, max_obstaculos, obstaculos;
    bool continuar;
    string respuesta;
    auto comparador_global = [](Nodo* a, Nodo* b) {return a->global > b->global;};
    auto distancia_fisica = [](Nodo* a, Nodo* b) {
        float xsquared = (a->x - b->x)*(a->x - b->x);
        float ysquared = (a->y - b->y)*(a->y - b->y);
        float result = sqrt(xsquared + ysquared);
        return result;
    };

    cout << "BUSCADOR DE CAMINOS A*" << endl;
    cout << "----------------------" << endl;
    cout << "Ingrese el ancho del mapa: " << endl;
    cin >> tamanio;
    max_obstaculos = (tamanio*tamanio)/5;
    cout << "Ingrese la cantidad de obstaculos (La cantidad máxima es de " << max_obstaculos << " obstaculos): " << endl;
    cin >> obstaculos;
    while (obstaculos > max_obstaculos) {
        cout << "Cantidad de obstaculos inválida" << endl;
        cout << "Ingrese la cantidad de obstaculos (La cantidad máxima es de " << max_obstaculos << " obstaculos): " << endl;
        cin >> obstaculos;
    }
    Mapa mapa(tamanio);
    mapa.crear_obstaculos(obstaculos);
    int inicio_x = rand() % mapa.get_dimension();
    int inicio_y = rand() & mapa.get_dimension();
    int fin_x, fin_y;
    do {
        fin_x = rand() % mapa.get_dimension();
        fin_y = rand() % mapa.get_dimension();
    }
    while (inicio_x == fin_x && inicio_y == fin_y);
    mapa.set_inicio(inicio_x, inicio_y);
    mapa.set_fin(fin_x, fin_y);
    cout << endl;
    mapa.a_star(comparador_global, distancia_fisica);
    for (int i = 0; i < mapa.get_dimension(); i++) {
        for (int j = 0; j < mapa.get_dimension(); j++) {
            cout << static_cast<char>(mapa.get_mapa()[j*mapa.get_dimension() + i].estado) << "  ";
        }
        cout << endl;
    }
    cout << endl;
    cout << "El camino mínimo es de : " << mapa.get_longitud() << " pasos de longitud" << endl;
}