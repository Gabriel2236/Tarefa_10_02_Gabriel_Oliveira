#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/i2c.h"

#include "inc/ssd1306.h"
#include "inc/font.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
ssd1306_t ssd;

//Definição de LED RGB
#define led_g 11
#define led_b 12
#define led_r 13

// Define os push bottons
#define BOTTON_A 5
#define BOTTON_J 22


uint32_t last_time = 0;

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (current_time - last_time > 200000) {
        if (gpio == BOTTON_J ) {

            gpio_put(led_g, !gpio_get(led_g));
            if(gpio_get(led_g) == true){
            printf("LED verde ligado\n");
            ssd1306_fill(&ssd, false); // Limpa o display
            ssd1306_draw_string(&ssd, "LED G on", 10, 30); // Escreve no display (posição X=10, Y=30)
            ssd1306_send_data(&ssd);

            }
            else{
            printf("LED verde desligado\n");

            ssd1306_fill(&ssd, false); // Limpa o display
            ssd1306_draw_string(&ssd, "LED G off", 10, 30); // Escreve no display (posição X=10, Y=30)
            ssd1306_send_data(&ssd);

            }


        } else if (gpio == BOTTON_A) {

        }

        last_time = current_time;
    }
}

void setup_gpio_interrupt(uint gpio_pin, gpio_irq_callback_t callback) {
    gpio_init(gpio_pin);
    gpio_set_dir(gpio_pin, GPIO_IN);
    gpio_pull_up(gpio_pin);
    gpio_set_irq_enabled_with_callback(gpio_pin, GPIO_IRQ_EDGE_FALL, true, callback);
}


int main()
{

    stdio_init_all();

    gpio_init(led_g);
    gpio_set_dir(led_g, GPIO_OUT);  
    gpio_put(led_g, 0);

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Define a função GPIO para I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Define a função GPIO para I2C
    gpio_pull_up(I2C_SDA); // Puxa a linha de dados
    gpio_pull_up(I2C_SCL); // Puxa a linha de relógio

    // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

      // Limpa o display. O display inicia com todos os pixels apagados.
      ssd1306_fill(&ssd, false);
      ssd1306_send_data(&ssd);

    setup_gpio_interrupt(BOTTON_A, gpio_irq_handler);
    setup_gpio_interrupt(BOTTON_J, gpio_irq_handler);

    gpio_set_irq_enabled_with_callback(BOTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTTON_J, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);


    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
