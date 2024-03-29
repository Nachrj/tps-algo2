#include "heap.h"
#include <string.h> //strcmp
#include <stdio.h>
#include <stdlib.h>

#define CAPACIDAD 10

struct heap{
    void** lista;
    size_t cantidad;
    size_t capacidad;
    cmp_func_t cmp;
};

void swap(void** lista, int a, int b){
    void* aux = lista[a];
    lista[a] = lista[b];
    lista[b] = aux;
}

bool heap_redimensionar(heap_t* heap, size_t tam_nuevo) {
    void** datos_nuevo = realloc(heap->lista, tam_nuevo * sizeof(void*));

    // Cuando tam_nuevo es 0, es correcto si se devuelve NULL.
    // En toda otra situación significa que falló el realloc.
    if (tam_nuevo > 0 && datos_nuevo == NULL) {
        return false;
    }
    heap->lista = datos_nuevo;
    heap->capacidad = tam_nuevo;
    return true;
}


void upheap(heap_t* heap, int actual){
    if(actual-1 < 0) return;
    int padre = (actual-1)/2;
    if(heap->cmp(heap->lista[padre],heap->lista[actual]) > 0){
        return;
    }
    if(heap->cmp(heap->lista[padre],heap->lista[actual]) <= 0){
        swap(heap->lista,actual,padre);
        upheap(heap,padre);
    }
    return;
}

void downheap(void* arreglo[], cmp_func_t cmp, size_t cant, int actual){
    int hijo_izq = (actual*2)+1;
    int hijo_der = (actual*2)+2;
    int hijo_max;
    if(hijo_izq > cant-1 && hijo_der > cant-1){
        return;
    } else if(hijo_izq > cant-1 && hijo_der <= cant-1){
        hijo_max = hijo_der;
    } else if(hijo_izq <= cant-1 && hijo_der > cant-1){
        hijo_max = hijo_izq;
    } else{
        hijo_max = cmp(arreglo[hijo_izq],arreglo[hijo_der]) > 0 ? hijo_izq : hijo_der;
    }
    
    if(cmp(arreglo[hijo_max],arreglo[actual]) <= 0){
        return;
    }
    if(cmp(arreglo[hijo_max],arreglo[actual]) > 0){
        swap(arreglo,actual,hijo_max);
        downheap(arreglo, cmp, cant, hijo_max);
    }
    return;
}

void heapify(void* elementos[], size_t cant, cmp_func_t cmp){
    for(int i = (int)cant-1; i >= 0; i--){
        downheap(elementos, cmp, cant, i);
    }
}

void heap_sort(void *elementos[], size_t cant, cmp_func_t cmp){
    // Le damos forma de heap al arreglo (heapify)
    if(cant <= 0) return;
    heapify(elementos, cant, cmp);
    for (int i = (int)cant-1; i > 0; i--) {
        // Swapeamos primero con el ultimo
        swap(elementos, 0, i);
        downheap(elementos, cmp, i, 0);
    }
}

heap_t *heap_crear(cmp_func_t cmp){
    heap_t* heap = calloc(1,sizeof(heap_t));
    if (!heap) return NULL;
    void** lista = calloc(CAPACIDAD,sizeof(void*));
    if (!lista) {
        free(heap);
        return NULL;
    }
    heap->capacidad = CAPACIDAD;
    heap->lista = lista;
    heap->cmp = cmp;
    return heap;
}

void** copia_arreglo(void* arreglo[] , size_t cant) {
    void** lista = calloc(cant,sizeof(void*));
    if(!lista) return NULL;
    for(int i = 0; i < cant; i++){
        lista[i] = arreglo[i];
    }
    return lista;
}

heap_t *heap_crear_arr(void *arreglo[], size_t n, cmp_func_t cmp){
    heap_t* heap = calloc(1,sizeof(heap_t));
    if (!heap) return NULL;
    if(n > CAPACIDAD){
        heap->capacidad = n*2;
    } else {
        heap->capacidad = CAPACIDAD;
    }
    heap->cantidad = n;
    
    void** lista = copia_arreglo(arreglo, n);
    if(!lista){
        lista = calloc(CAPACIDAD,sizeof(void*));
    }
    heapify(lista,n,cmp);
    heap->lista = lista;
    heap->cmp = cmp;
    return heap;
}

void heap_destruir(heap_t *heap, void (*destruir_elemento)(void *e)){
    if(destruir_elemento) {
        for (int i = 0; i < heap->cantidad; i++) {
            destruir_elemento(heap->lista[i]);
        }
    }
    free(heap->lista);
    free(heap);
}

size_t heap_cantidad(const heap_t *heap){
    return heap->cantidad;
}

bool heap_esta_vacio(const heap_t *heap){
    return !heap->cantidad;
}

void *heap_ver_max(const heap_t *heap){
    return heap->lista[0];
}

bool heap_encolar(heap_t *heap, void *elem){
    if(heap->cantidad == heap->capacidad){
        if(!heap_redimensionar(heap, heap->capacidad*2)){
            return false;
        }
    }
    int i = (int)heap_cantidad(heap);
    heap->lista[i] = elem;
    heap->cantidad++;
    upheap(heap,i);
    
    return true;
}

void *heap_desencolar(heap_t *heap){
    if (heap_esta_vacio(heap)) return NULL;
    if(heap->cantidad <= heap->capacidad/4){
        if(!heap_redimensionar(heap, heap->capacidad/2)){
            return false;
        }
    }
    void* dato = heap->lista[0];
    heap->lista[0] = heap->lista[heap_cantidad(heap)-1];
    heap->lista[heap_cantidad(heap)-1] = NULL;
    heap->cantidad--;
    if(heap_esta_vacio(heap)) return dato;
    downheap(heap->lista, heap->cmp, heap->cantidad,0);
    
    return dato;
}
