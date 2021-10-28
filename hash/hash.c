#include "hash.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lista.h"
#define CAPACIDAD_INICIAL 11

size_t hash_f(const char* str, size_t n) {
  size_t hash = 5381;
  size_t c;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash % n;
}

typedef struct item {
  void* dato;
  const char* clave;
} item_t;

struct hash {
  lista_t** lista;
  hash_destruir_dato_t destruir_dato;
  size_t capacidad;
  size_t cantidad;
};

struct hash_iter {
  const hash_t* hash;
  lista_iter_t* lista_iter;
  size_t pos;
};

hash_t* _hash_malloc(hash_destruir_dato_t destruir_dato, size_t capacidad) {
  hash_t* hash = calloc(1, sizeof(hash_t));
  hash->capacidad = capacidad;
  lista_t** lista = calloc(capacidad, sizeof(lista_t*));
  for (size_t i = 0; i < capacidad; i++) {
    lista[i] = lista_crear();
  }
  if (!hash || !lista) return NULL;
  hash->lista = lista;
  hash->destruir_dato = destruir_dato;
  return hash;
}

hash_t* hash_crear(hash_destruir_dato_t destruir_dato) {
  return _hash_malloc(destruir_dato, CAPACIDAD_INICIAL);
}

bool hash_redimensionar(hash_t* hash, size_t n) {
  // Creo el nuevo hash con la nueva capacidad
  hash_t* nuevo_hash = _hash_malloc(hash->destruir_dato, n);
  if (!nuevo_hash) return false;
  // Itero sobre el hash actual
  for (size_t i = 0; i < hash->capacidad; i++) {
    lista_iter_t* iter = lista_iter_crear(hash->lista[i]);
    if (!iter) return false;
    while (!lista_iter_al_final(iter)) {
      item_t* item = lista_iter_ver_actual(iter);
      // Agrego el item al nuevo hash
      if (!hash_guardar(nuevo_hash, item->clave, item->dato)) return false;
      lista_iter_avanzar(iter);
    }
    lista_iter_destruir(iter);
  }
  // Libero la memoria del hash actual
  for (size_t i = 0; i < hash->capacidad; i++) {
    lista_destruir(hash->lista[i], NULL);
  }
  free(hash->lista);
  free(hash);
  // Actualizo el puntero al hash
  hash = nuevo_hash;
  return true;
}

bool hash_guardar(hash_t* hash, const char* clave, void* dato) {
  if (hash->cantidad / hash->capacidad > 2) {
    if (!hash_redimensionar(hash, hash->capacidad * 2)) return false;
  }
  size_t clave_hash = hash_f(clave, hash->capacidad);
  item_t* item = calloc(1, sizeof(item_t));
  if (!item) return false;
  item->dato = dato;
  item->clave = clave;
  // Preguntar si ya existe
  if (hash_pertenece(hash, clave)) {
    // Si existe, reemplazar el dato
    lista_iter_t* iter = lista_iter_crear(hash->lista[clave_hash]);
    while (!lista_iter_al_final(iter)) {
      item_t* item = lista_iter_ver_actual(iter);
      if (item->clave == clave) {
        item->dato = dato;
        break;
      }
      lista_iter_avanzar(iter);
    }

    lista_iter_destruir(iter);
    hash->cantidad++;
    return true;
  }

  // Inserto y pregunto si hay que redimensionar el hash
  if (!lista_insertar_ultimo(hash->lista[clave_hash], item)) return false;
  hash->cantidad++;
  return true;
}

void* hash_borrar(hash_t* hash, const char* clave) {
  size_t clave_hash = hash_f(clave, hash->capacidad);
  if (!hash->lista[clave_hash]) return NULL;

  lista_iter_t* iter = lista_iter_crear(hash->lista[clave_hash]);
  if (!iter) return NULL;

  while (!lista_iter_al_final(iter)) {
    item_t* actual = (item_t*)lista_iter_ver_actual(iter);
    if (actual->clave == clave) {
      item_t* dato = lista_iter_borrar(iter);
      hash->cantidad--;
      lista_iter_destruir(iter);
      return dato->dato;
    }
    lista_iter_avanzar(iter);
  }

  lista_iter_destruir(iter);
  return NULL;
}

void* hash_obtener(const hash_t* hash, const char* clave) {
  size_t clave_hash = hash_f(clave, hash->capacidad);
  if (!hash->lista[clave_hash]) return NULL;

  lista_iter_t* iter = lista_iter_crear(hash->lista[clave_hash]);
  if (!iter) return NULL;

  while (!lista_iter_al_final(iter)) {
    item_t* actual = lista_iter_ver_actual(iter);
    if (actual->clave == clave) {
      lista_iter_destruir(iter);
      return actual->dato;
    }
    lista_iter_avanzar(iter);
  }

  lista_iter_destruir(iter);
  return NULL;
}

bool hash_pertenece(const hash_t* hash, const char* clave) {
  size_t clave_hash = hash_f(clave, hash->capacidad);
  if (!hash->lista[clave_hash]) return false;

  lista_iter_t* iter = lista_iter_crear(hash->lista[clave_hash]);
  if (!iter) return false;

  while (!lista_iter_al_final(iter)) {
    item_t* actual = (item_t*)lista_iter_ver_actual(iter);
    if (actual->clave == clave) {
      lista_iter_destruir(iter);
      return true;
    }
    lista_iter_avanzar(iter);
  }

  lista_iter_destruir(iter);
  return false;
}

size_t hash_cantidad(const hash_t* hash) {
  return hash->cantidad;  // Devuelve la cantidad de elementos del hash
}

size_t hash_capacidad(const hash_t* hash) {
  return hash->capacidad;  // Devuelve la cantidad de elementos del hash
}

void hash_destruir(hash_t* hash) {
  for (size_t i = 0; i < hash->capacidad; i++) {
    if (hash->lista[i]) {
      lista_destruir(hash->lista[i], hash->destruir_dato);
    }
  }
  free(hash->lista);
  free(hash);
}

/* Iterador del hash */

hash_iter_t* hash_iter_crear(const hash_t* hash) {
  hash_iter_t* iter = calloc(1, sizeof(hash_iter_t));
  if (!iter) return NULL;
  iter->hash = hash;
  iter->pos = 0;
  iter->lista_iter = lista_iter_crear(hash->lista[iter->pos]);
  if (!iter->lista_iter) return NULL;
  return iter;
}

bool hash_iter_avanzar(hash_iter_t* iter) {
  if (lista_iter_avanzar(iter->lista_iter)) return true;
  if (hash_iter_al_final(iter)) return false;

  while (iter->pos < iter->hash->capacidad) {
    iter->pos++;
    lista_iter_t* iter_antiguo = iter->lista_iter;
    lista_iter_destruir(iter_antiguo);
    iter->lista_iter = lista_iter_crear(iter->hash->lista[iter->pos]);
  }
  return true;
}

// Devuelve clave actual, esa clave no se puede modificar ni liberar.
const char* hash_iter_ver_actual(const hash_iter_t* iter) {
  item_t* actual = (item_t*)lista_iter_ver_actual(iter->lista_iter);
  if (!actual) return NULL;
  return actual->clave;
}

// Comprueba si terminó la iteración
bool hash_iter_al_final(const hash_iter_t* iter) {
  if (lista_iter_al_final(iter->lista_iter)) {
    if (iter->pos == iter->hash->cantidad) return true;
    return false;
  }
  return false;
}

// Destruye iterador
void hash_iter_destruir(hash_iter_t* iter) {
  lista_iter_destruir(iter->lista_iter);
  free(iter);
}
/*
int main() {
    char* dato = "MUNDO";
    //void *dato2 = "Mundo";
    //void *dato3 = "Planeta";
    hash_t* hash = hash_crear(NULL);
    printf("%ld",hash_f("hola",hash->capacidad));
    printf("Creo el hash");
    printf("%d", hash_guardar(hash,"hola", dato));
    item_t* ankara = (item_t*)lista_ver_primero(hash->lista[9]);
    printf("%p", &dato);
    printf("\n");
    printf("%d", *(int*)ankara->dato);
    printf("\n");
    printf("%d",*(int*)hash_obtener(hash, "hola"));

    //printf("%s",*(char**)ankara->dato);
    //printf("%d", hash_guardar(hash,"chau", dato2));
    //printf("%d", hash_guardar(hash,"adios", dato3));
    //printf("%d",hash_pertenece(hash,"hola"));
    //printf("\n");
    hash_destruir(hash);
    return 0;
}*/
