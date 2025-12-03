#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define MAX_PROCESOS 100        // procesos maximos corriendo
#define MAX_PAGINAS_PROCESOS 50 // cantidad de paginas que puede tener un proceso
#define MAX_TAM 100             // Tamaño maximo procesos
#define MIN_TAM 1               // Tamaño minimo procesos
#define SWAP 100                // Tamaño memoria swap

typedef struct
{
    int pid;             // id del dueño
    int numero_pagina;   // numero pag en el proceso
    time_t tiempo_carga; // FIFO
} pagina;

typedef struct
{
    int pid;
    double tamano_mb;
    int num_paginas;
    int paginas_ram;
    int paginas_swap;
    int indices_ram[MAX_PAGINAS_PROCESOS];
    int indices_swap[MAX_PAGINAS_PROCESOS];
} PCB; // pcb lol

// variables globales
double memoria_fisica;
double memoria_virtual;
double tam_pagina;
double memoria_swap;

int total_paginas_ram;
int total_paginas_swap;

pagina *tabla_ram;
pagina *tabla_swap;

PCB procesos[MAX_PROCESOS];
int num_procesos_activos = 0;
int contador_pid = 0;
int total_procesos_creados = 0;

// funciones necesarias para el funcionamiento de la función principal
void iniciar_simulacion()
{
    // system.println("hola mundo");
    printf("===INICIANDO SIMULACIÓN v3===\n");
    printf("Ingrese tamaño de la ram (MB):");
    scanf("%lf", &memoria_fisica);
    printf("Ingrese tamaño pagina (marco):");
    scanf("%lf", &tam_pagina);

    // generar memoria virtual de forma "aleatoria"
    double factor = 1.5 + (rand() / (double)RAND_MAX) * (4.5 - 1.5);
    memoria_virtual = memoria_fisica * factor;
    memoria_swap = memoria_virtual - memoria_fisica;
    total_paginas_ram = (int)(memoria_fisica / tam_pagina);
    total_paginas_swap = (int)(memoria_swap / tam_pagina);

    // asignar memoira para las tablas de páginas
    tabla_ram = (pagina *)malloc(total_paginas_ram * sizeof(pagina));
    tabla_swap = (pagina *)malloc(total_paginas_swap * sizeof(pagina));

    // inicializar todas las pags. libres
    for (int i = 0; i < total_paginas_ram; i++)
    {
        tabla_ram[i].pid = -1;
        tabla_ram[i].numero_pagina = -1;
    }
    for (int i = 0; i < total_paginas_swap; i++)
    {
        tabla_swap[i].pid = -1;
        tabla_swap[i].numero_pagina = -1;
    }

    // inicializar array de procesos
    for (int i = 0; i < MAX_PROCESOS; i++)
    {
        procesos[i].pid = -1;
    }

    printf("\n=== SISTEMA INICIALIZADO ===\n");
    printf("Memoria fisica: %.2f MB\n", memoria_fisica);
    printf("Memoria virtual: %.2f MB (%.2fx)\n", memoria_virtual, factor);
    printf("Memoria SWAP: %.2f MB\n", memoria_swap);
    printf("Tamaño de pagina: %.4f MB\n", tam_pagina);
    printf("Paginas en RAM: %d\n", total_paginas_ram);
    printf("Paginas en SWAP: %d\n", total_paginas_swap);
    printf("Rango de tamaño de procesos: %.2f - %.2f MB\n\n", MAX_TAM, MIN_TAM);
}

int paginas_libres_ram()
{
    int libres = 0;
    for (int i = 0; i < total_paginas_ram; i++)
    {
        if (tabla_ram[i].pid == -1)
            libres++;
    }
    return libres;
}
int paginas_libres_swap()
{
    int libres = 0;
    for (int i = 0; i < total_paginas_swap; i++)
    {
        if (tabla_swap[i].pid == -1)
            libres++;
    }
    return libres;
}

int crear_procesos()
{
    if (num_procesos_activos >= MAX_PROCESOS)
    {
        printf("Error limite de procesos alcanzados\n");
        return 0;
    }

    // generar tamaño "aleatorio"
    double tam_proceso = MIN_TAM + ((double)rand() / RAND_MAX) * (MAX_TAM - MIN_TAM);

    // calcular paginas necesarias
    int paginas_necesarias = (int)((tam_proceso / tam_pagina) + 0.999);

    int pid = contador_pid++;
    printf("\nCreando proceso P%d: %.2f MB, %d paginas necesarias\n", pid, tam_proceso, paginas_necesarias);

    // verificar si hay suficiente memorira para el proceso
    int libres_ram = paginas_libres_ram();
    int libres_swap = paginas_libres_swap();
    int total = libres_ram + libres_swap;

    if (total < paginas_necesarias)
    {
        printf("No hay suficiente memoria para almacenar el proceso:%d \n", pid);
        return -1;
    }

    // Crear el PCB
    PCB *proceso = &procesos[num_procesos_activos];
    proceso->pid = pid;
    proceso->tamano_mb = tam_proceso;
    proceso->num_paginas = paginas_necesarias;
    proceso->paginas_ram = 0;
    proceso->paginas_swap = 0;

    // Inicializar arrays de índices
    for (int i = 0; i < MAX_PAGINAS_PROCESOS; i++)
    {
        proceso->indices_ram[i] = -1;
        proceso->indices_swap[i] = -1;
    }

    // Asignar páginas primero en RAM
    int paginas_asignadas = 0;
    for (int i = 0; i < total_paginas_ram && paginas_asignadas < paginas_necesarias; i++)
    {
        if (tabla_ram[i].pid == -1)
        {
            tabla_ram[i].pid = pid;
            tabla_ram[i].numero_pagina = paginas_asignadas;
            proceso->indices_ram[proceso->paginas_ram] = i;
            proceso->paginas_ram++;
            paginas_asignadas++;
        }
    }

    // Si faltan páginas, asignar en SWAP
    for (int i = 0; i < total_paginas_swap && paginas_asignadas < paginas_necesarias; i++)
    {
        if (tabla_swap[i].pid == -1)
        {
            tabla_swap[i].pid = pid;
            tabla_swap[i].numero_pagina = paginas_asignadas;
            proceso->indices_swap[proceso->paginas_swap] = i;
            proceso->paginas_swap++;
            paginas_asignadas++;
        }
    }

    printf("Proceso P%d creado exitosamente\n", pid);
    printf("     Paginas en RAM: %d, Paginas en SWAP: %d\n",
           proceso->paginas_ram, proceso->paginas_swap);

    num_procesos_activos++;
    total_procesos_creados++;

    return 1;
}

void terminar_proceso_aleatorio()
{
    if (num_procesos_activos == 0)
    {
        printf("\nNo hay procesos activos\n");
        return;
    }

    int idx = rand() % num_procesos_activos;
    PCB *proceso = &procesos[idx];
    int pid = proceso->pid;

    printf("\nP%d (%.2f MB, %d paginas)\n",
           pid, proceso->tamano_mb, proceso->num_paginas);

    // Liberar páginas en RAM
    for (int i = 0; i < proceso->paginas_ram; i++)
    {
        int idx_ram = proceso->indices_ram[i];
        if (idx_ram != -1)
        {
            tabla_ram[idx_ram].pid = -1;
            tabla_ram[idx_ram].numero_pagina = -1;
            tabla_ram[idx_ram].tiempo_carga = 0;
        }
    }

    // Liberar páginas en SWAP
    for (int i = 0; i < proceso->paginas_swap; i++)
    {
        int idx_swap = proceso->indices_swap[i];
        if (idx_swap != -1)
        {
            tabla_swap[idx_swap].pid = -1;
            tabla_swap[idx_swap].numero_pagina = -1;
            tabla_swap[idx_swap].tiempo_carga = 0;
        }
    }

    printf("P%d terminado - Memoria liberada\n", pid);
}

int buscar_proceso(int pid)
{
    for (int i = 0; i < num_procesos_activos; i++)
    {
        if (procesos[i].pid == pid)
        {
            return i;
        }
    }
    return -1;
}

void realizar_swap_fifo(int pid_solicitante, int num_pagina)
{
    // Buscar la página más antigua en RAM (FIFO)
    int idx_victima = -1;
    time_t tiempo_mas_antiguo = time(NULL) + 1000;

    for (int i = 0; i < total_paginas_ram; i++)
    {
        if (tabla_ram[i].pid != -1 && tabla_ram[i].tiempo_carga < tiempo_mas_antiguo)
        {
            tiempo_mas_antiguo = tabla_ram[i].tiempo_carga;
            idx_victima = i;
        }
    }

    if (idx_victima == -1)
    {
        printf("No se pudo encontrar página víctima\n");
        return;
    }

    pagina victima = tabla_ram[idx_victima];
    printf("Victima: P%d:Pag%d (en RAM hace %ld seg)\n",
           victima.pid, victima.numero_pagina, time(NULL) - victima.tiempo_carga);

    // Buscar espacio libre en SWAP
    int idx_swap_libre = -1;
    for (int i = 0; i < total_paginas_swap; i++)
    {
        if (tabla_swap[i].pid == -1)
        {
            idx_swap_libre = i;
            break;
        }
    }

    if (idx_swap_libre == -1)
    {
        printf("No hay espacio en SWAP\n");
        return;
    }

    // Encontrar el proceso solicitante
    int idx_proceso_solicitante = buscar_proceso(pid_solicitante);
    if (idx_proceso_solicitante == -1)
        return;

    PCB *proc_solicitante = &procesos[idx_proceso_solicitante];
    int idx_swap_solicitante = -1;

    // Buscar la página en SWAP
    for (int i = 0; i < proc_solicitante->paginas_swap; i++)
    {
        int idx = proc_solicitante->indices_swap[i];
        if (idx != -1 && tabla_swap[idx].numero_pagina == num_pagina)
        {
            idx_swap_solicitante = idx;
            break;
        }
    }

    if (idx_swap_solicitante == -1)
        return;

    // Actualizar proceso víctima
    int idx_proceso_victima = buscar_proceso(victima.pid);
    if (idx_proceso_victima != -1)
    {
        PCB *proc_victima = &procesos[idx_proceso_victima];

        // Remover de RAM
        for (int i = 0; i < proc_victima->paginas_ram; i++)
        {
            if (proc_victima->indices_ram[i] == idx_victima)
            {
                proc_victima->indices_ram[i] = -1;
                // Compactar array
                for (int j = i; j < proc_victima->paginas_ram - 1; j++)
                {
                    proc_victima->indices_ram[j] = proc_victima->indices_ram[j + 1];
                }
                proc_victima->paginas_ram--;
                break;
            }
        }

        // Agregar a SWAP
        proc_victima->indices_swap[proc_victima->paginas_swap] = idx_swap_libre;
        proc_victima->paginas_swap++;
    }

    // Mover víctima a SWAP
    tabla_swap[idx_swap_libre] = victima;
    tabla_swap[idx_swap_libre].tiempo_carga = time(NULL);

    // Mover página solicitante a RAM
    tabla_ram[idx_victima] = tabla_swap[idx_swap_solicitante];
    tabla_ram[idx_victima].tiempo_carga = time(NULL);

    // Liberar posición en SWAP
    tabla_swap[idx_swap_solicitante].pid = -1;
    tabla_swap[idx_swap_solicitante].numero_pagina = -1;

    // Actualizar proceso solicitante
    for (int i = 0; i < proc_solicitante->paginas_swap; i++)
    {
        if (proc_solicitante->indices_swap[i] == idx_swap_solicitante)
        {
            proc_solicitante->indices_swap[i] = -1;
            for (int j = i; j < proc_solicitante->paginas_swap - 1; j++)
            {
                proc_solicitante->indices_swap[j] = proc_solicitante->indices_swap[j + 1];
            }
            proc_solicitante->paginas_swap--;
            break;
        }
    }

    proc_solicitante->indices_ram[proc_solicitante->paginas_ram] = idx_victima;
    proc_solicitante->paginas_ram++;

    printf("P%d:Pag%d -> RAM[%d], P%d:Pag%d -> SWAP[%d]\n",
           pid_solicitante, num_pagina, idx_victima,
           victima.pid, victima.numero_pagina, idx_swap_libre);
}

void acceder_memoria_virtual()
{
    if (num_procesos_activos == 0)
    {
        printf("\nNo hay procesos activos\n");
        return;
    }

    // Seleccionar proceso aleatorio
    int idx_proceso = rand() % num_procesos_activos;
    PCB *proceso = &procesos[idx_proceso];

    // Generar número de página aleatorio
    int num_pagina = rand() % proceso->num_paginas;

    // Calcular offset aleatorio
    int offset = rand() % (int)(tam_pagina * 1024); // offset en KB

    printf("\nP%d - Pagina Virtual: %d, Offset: %d KB\n",
           proceso->pid, num_pagina, offset);

    // Buscar si la página está en RAM
    int encontrada_ram = 0;
    int idx_en_ram = -1;

    for (int i = 0; i < proceso->paginas_ram; i++)
    {
        int idx = proceso->indices_ram[i];
        if (idx != -1 && tabla_ram[idx].numero_pagina == num_pagina)
        {
            encontrada_ram = 1;
            idx_en_ram = idx;
            break;
        }
    }

    if (encontrada_ram)
    {
        printf("Pagina encontrada en RAM[%d]\n", idx_en_ram);
    }
    else
    {
        printf("Pagina NO en RAM\n");

        // Verificar si está en SWAP
        int encontrada_swap = 0;
        for (int i = 0; i < proceso->paginas_swap; i++)
        {
            int idx = proceso->indices_swap[i];
            if (idx != -1 && tabla_swap[idx].numero_pagina == num_pagina)
            {
                encontrada_swap = 1;
                printf("Pagina encontrada en SWAP[%d]\n", idx);
                realizar_swap_fifo(proceso->pid, num_pagina);
                break;
            }
        }

        if (!encontrada_swap)
        {
            printf("Pagina no encontrada en el sistema\n");
        }
    }
}

void mostrar_estado()
{
    int ram_ocupadas = total_paginas_ram - paginas_libres_ram();
    int swap_ocupadas = total_paginas_swap - paginas_libres_swap();

    printf("\n=== ESTADO DEL SISTEMA ===\n");
    printf("Procesos: %d activos | Creados: %d",
           num_procesos_activos, total_procesos_creados);
    printf("RAM: %d/%d (%.1f%%) | SWAP: %d/%d (%.1f%%)\n",
           ram_ocupadas, total_paginas_ram, (ram_ocupadas * 100.0) / total_paginas_ram,
           swap_ocupadas, total_paginas_swap, (swap_ocupadas * 100.0) / total_paginas_swap);

    printf("==========================\n");
}

void ejecutar_simulacion()
{
    printf("\n=== INICIANDO SIMULACION ===\n");
    printf("Fase 1 (0-30s): Creacion de procesos cada 2 segundos\n");
    printf("Fase 2 (30s+): + Terminacion cada 5s + Acceso memoria cada 5s\n\n");

    int tiempo = 0;
    int ultimo_terminar = 30;
    int ultimo_acceso = 30;

    while (1)
    {
        // Crear proceso cada 2 segundos
        if (tiempo % 2 == 0)
        {
            int resultado = crear_procesos();
            if (resultado == -1)
                break;
        }

        // Después de 30 segundos
        if (tiempo >= 30)
        {
            // Terminar proceso cada 5 segundos
            if (tiempo - ultimo_terminar >= 5)
            {
                terminar_proceso_aleatorio();
                ultimo_terminar = tiempo;
            }

            // Acceder a memoria cada 5 segundos
            if (tiempo - ultimo_acceso >= 5)
            {
                acceder_memoria_virtual();
                ultimo_acceso = tiempo;
            }
        }

        // Mostrar estado cada 10 segundos
        if (tiempo % 10 == 0 && tiempo > 0)
        {
            mostrar_estado();
        }

        sleep(1);
        tiempo++;

        if (tiempo >= 60)
        {
            printf("\nSimulacion completada (60 segundos)\n");
            break;
        }
    }

    printf("\n=== FIN DE SIMULACION ===\n");
    mostrar_estado();
}

int main()
{
    iniciar_simulacion();
    ejecutar_simulacion();

    return 0;
}
