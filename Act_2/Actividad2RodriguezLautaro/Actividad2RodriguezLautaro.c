#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"

/**
 * @brief Programa principal
*/
int main(void) {
  // Inicializo el USB
  stdio_init_all();
  
  sleep_ms(500);
  gpio_init(4);
  gpio_set_dir(4,GPIO_IN);
  gpio_init(25);
  gpio_set_dir(25, GPIO_OUT); 
 bool x;

  while (true) {

    x=gpio_get(4);
    if(x==1){
        gpio_put(25, 1);
    }
    else if (x==0) {
        gpio_put(25, 0); 
    }

    /* Prendo LED
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    */

    // Resolver logica para GPIO20 -> GPIO6

    // Resolver logica para GPIO21 -> GPIO7

    // Resolver logica para GPIO22 -> GPIO8

  }
  return 0;
}