#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"


#include "pico/stdlib.h"

// ==========================================================
// RASPBERRY PI PICO W + MODULO L298N
//
// GP2 -> IN1       Motor 1 -> OUT1 y OUT2
// GP3 -> IN2
// GP4 -> IN3       Motor 2 -> OUT3 y OUT4
// GP5 -> IN4
// GP6 -> ENA
// GP7 -> ENB
// GND -> GND del modulo y de la fuente
//
// Retirar los jumpers ENA y ENB antes de conectar la Pico.
// Los GPIO trabajan a 3,3 V: no conectarles 5 V.
// Los motores necesitan alimentacion externa mediante el modulo.
// ==========================================================

// Numeros GPIO, no posiciones fisicas del conector.
#define IN1 2
#define IN2 3
#define IN3 4
#define IN4 5
#define ENA 6
#define ENB 7

// Ordenes disponibles.
#define ATRAS    (-1)
#define DETENIDO  0
#define ADELANTE  1

// Pausa minima entre movimientos de un mismo motor.
#define PAUSA_US 2000000ULL

// Variables que modificara el futuro control.
int orden_motor_1 = DETENIDO;
int orden_motor_2 = DETENIDO;

// Indice 0: motor 1. Indice 1: motor 2.
static const uint pin_a[2]  = {IN1, IN3};
static const uint pin_b[2]  = {IN2, IN4};
static const uint pin_en[2] = {ENA, ENB};

// Movimiento aplicado y momento de parada de cada motor.
static int estado[2] = {DETENIDO, DETENIDO};
static uint64_t parada[2] = {0, 0};

// Configurar una salida inicialmente en cero.
static void configurar_salida(uint pin)
{
    gpio_init(pin);
    gpio_put(pin, 0);
    gpio_set_dir(pin, GPIO_OUT);
}

// Aplicar una orden sin bloquear al otro motor.
// motor = 0: motor 1.
// motor = 1: motor 2.
static void actualizar_motor(uint motor, int orden, uint64_t ahora)
{
    // Evitar acceder fuera de los arreglos.
    if (motor > 1) {
        return;
    }

    // Una orden invalida significa detener.
    if (orden != ADELANTE && orden != ATRAS) {
        orden = DETENIDO;
    }

    // No repetir escrituras si ya cumple la orden.
    if (orden == estado[motor]) {
        return;
    }

    // Antes de cambiar un movimiento, deshabilitar el canal.
    if (estado[motor] != DETENIDO) {
        gpio_put(pin_en[motor], 0);
        gpio_put(pin_a[motor], 0);
        gpio_put(pin_b[motor], 0);

        estado[motor] = DETENIDO;
        parada[motor] = ahora;
    }

    // Deshabilitar es inmediato.
    // El motor se detiene por inercia, sin frenado activo.
    if (orden == DETENIDO) {
        return;
    }

    // Esperar dos segundos antes de volver a arrancar.
    // Esta pausa no bloquea el programa ni mide el giro real.
    if (ahora - parada[motor] < PAUSA_US) {
        return;
    }

    // Definir el sentido antes de habilitar el canal.
    gpio_put(pin_a[motor], orden == ADELANTE);
    gpio_put(pin_b[motor], orden == ATRAS);
    gpio_put(pin_en[motor], 1);

    estado[motor] = orden;
}

// Prueba automatica de una sola ejecucion:
//
//  0- 3 segundos: ambos detenidos.
//  3- 5 segundos: motor 1 adelante.
//  5- 7 segundos: ambos detenidos.
//  7- 9 segundos: motor 1 atras.
//  9-11 segundos: ambos detenidos.
// 11-13 segundos: motor 2 adelante.
// 13-15 segundos: ambos detenidos.
// 15-17 segundos: motor 2 atras.
// Desde los 17 segundos: ambos detenidos.
static void prueba_automatica(uint64_t tiempo)
{
    orden_motor_1 = DETENIDO;
    orden_motor_2 = DETENIDO;

    if (tiempo >= 3000000ULL && tiempo < 5000000ULL) {
        orden_motor_1 = ADELANTE;
    }
    else if (tiempo >= 7000000ULL && tiempo < 9000000ULL) {
        orden_motor_1 = ATRAS;
    }
    else if (tiempo >= 11000000ULL && tiempo < 13000000ULL) {
        orden_motor_2 = ADELANTE;
    }
    else if (tiempo >= 15000000ULL && tiempo < 17000000ULL) {
        orden_motor_2 = ATRAS;
    }
}

int main(void)
{
    // Deshabilitar primero ambos canales del puente.
    configurar_salida(ENA);
    configurar_salida(ENB);

    // Configurar las cuatro señales de direccion.
    configurar_salida(IN1);
    configurar_salida(IN2);
    configurar_salida(IN3);
    configurar_salida(IN4);

    const uint64_t inicio = time_us_64();

    parada[0] = inicio;
    parada[1] = inicio;

    while (true) {
        // Una sola lectura del reloj por ciclo.
        const uint64_t ahora = time_us_64();

        // Reemplazar esta llamada cuando agreguemos el control.
        // La prueba sobrescribe las variables de orden.
        prueba_automatica(ahora - inicio);

        // Aplicar la orden independiente de cada motor.
        actualizar_motor(0, orden_motor_1, ahora);
        actualizar_motor(1, orden_motor_2, ahora);

        sleep_ms(1);
    }

    return 0;
}