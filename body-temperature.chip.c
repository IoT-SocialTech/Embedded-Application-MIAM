#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    pin_t output_pin;
    int temperature;
} chip_data_t;

void chip_timer_callback(void *data) {
    chip_data_t *chip_data = (chip_data_t*)data;


    int temperature = attr_read(chip_data->temperature);


    float volts = temperature * 3.3 / 100.0;

    printf("Temperatura: %d °C, Voltaje: %f V\n", temperature, volts);
    pin_dac_write(chip_data->output_pin, volts);
}

void chip_init() {
    printf("Inicializando sensor de temperatura corporal\n");
    chip_data_t *chip_data = (chip_data_t*)malloc(sizeof(chip_data_t));

    chip_data->temperature = attr_init("temperature", 0);

    chip_data->output_pin = pin_init("OUT", ANALOG);

    const timer_config_t config = {
        .callback = chip_timer_callback,
        .user_data = chip_data,
      };

    timer_t timer_id = timer_init(&config);
    timer_start(timer_id, 1000, true);  
}
