# Pagination
## **`void iniciar_simulacion()`**
**Qué hace:**
- Pide al usuario el tamaño de RAM y tamaño de página.
- Genera memoria virtual aleatoria entre 1.5× y 4.5× la RAM.
- Calcula:
    - memoria swap
    - número total de páginas en RAM
    - número total de páginas en swap
- Reserva memoria (`malloc`) para las tablas de RAM y SWAP.
- Inicializa todas las páginas como libres.
- Inicializa el arreglo de procesos.
---
## **`int paginas_libres_ram()`**
**Qué hace:**
- Recorre la tabla de RAM contando cuántos frames tienen `pid = -1`.
**Retorna:**  
Número de páginas libres en RAM.

---
## **`int paginas_libres_swap()`**
**Qué hace:**
- Igual que la anterior, pero en la tabla de SWAP.
**Retorna:**  
Número de páginas libres en SWAP.

---
## **`int crear_procesos()`**
**Qué hace:**
1. Verifica si se alcanzó el máximo de procesos.
2. Genera un tamaño aleatorio para el proceso.
3. Calcula cuántas páginas necesita.
4. Comprueba si hay suficiente memoria (RAM + SWAP).
5. Crea un PCB y le asigna:
    - pid
    - tamaño del proceso
    - número total de páginas
6. Asigna primero páginas en RAM.    
7. Si no alcanza, asigna el resto en SWAP.
8. Guarda los índices ocupados en `indices_ram[]` y `indices_swap[]`.

**Retorna:**
- `1` si el proceso fue creado
- `0` si hubo error de límite
- `-1` si no hay memoria suficiente
---
## **`void terminar_proceso_aleatorio()`**
**Qué hace:**
1. Si no hay procesos activos, no hace nada.
2. Selecciona un proceso al azar del arreglo.
3. Libera todas sus páginas en RAM.
4. Libera todas sus páginas en SWAP.
5. Quita el PCB del arreglo (compactación simple).
6. Decrementa `num_procesos_activos`.

**Propósito:**  
Simular que un proceso termina y su memoria se libera.

---
## **`int buscar_proceso(int pid)`**
**Qué hace:**
- Recorre el arreglo de procesos activos.
- Busca un PCB cuyo `pid` coincida con el argumento.
**Retorna:**
- El índice del proceso
- `-1` si no existe
---
## **`void realizar_swap_fifo(int pid_solicitante, int num_pagina)`**
**Qué hace:**
1. Buscar la página más antigua en RAM (por `tiempo_carga`) → **FIFO**.
2. Sacar esa página de RAM (víctima).
3. Moverla a un espacio libre en SWAP.
4. Traer desde SWAP la página pedida por el proceso que generó el page fault.
5. Actualizar índices del PCB de ambos procesos.
6. Actualizar `tiempo_carga` del frame recién cargado.
**Propósito:**  
Implementar la política de reemplazo FIFO cuando ocurre un page fault.

---

[video](https://youtu.be/l7dUF3s0xVI)
