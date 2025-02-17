#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

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

#define JOY_X 26  // Eixo X (Esquerda/Direita)
#define JOY_Y 27  // Eixo Y (Cima/Baixo)

#define PWM_MAX 20000
#define JOY_CENTER 2048
#define DEADZONE 100


uint32_t last_time = 0;



void setup_gpio_interrupt(uint gpio_pin, gpio_irq_callback_t callback) {
    gpio_init(gpio_pin);
    gpio_set_dir(gpio_pin, GPIO_IN);
    gpio_pull_up(gpio_pin);
    gpio_set_irq_enabled_with_callback(gpio_pin, GPIO_IRQ_EDGE_FALL, true, callback);
}

void set_pwm_level(uint pin, uint32_t level) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_gpio_level(pin, level);
    pwm_set_enabled(slice_num, true);
}

bool pwm_enabled = true;

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

            pwm_enabled = !pwm_enabled;

            if (pwm_enabled) {
                // Reativa o PWM
                printf("PWM ativado\n");
                pwm_set_enabled(pwm_gpio_to_slice_num(led_b), true);
                pwm_set_enabled(pwm_gpio_to_slice_num(led_r), true);
            } else {
                // Desativa o PWM
                printf("PWM desativado\n");
                pwm_set_enabled(pwm_gpio_to_slice_num(led_b), false);
                pwm_set_enabled(pwm_gpio_to_slice_num(led_r), false);
            }

        }

        last_time = current_time;
    }
}

void update_pwm_from_joystick() {
    // Leitura do eixo X (LED Vermelho)
    adc_select_input(0);  // Seleciona ADC0 (GPIO26)
    uint16_t adc_x = adc_read();
    int16_t pwm_r = (abs(adc_x - JOY_CENTER) > DEADZONE) ? abs((adc_x - JOY_CENTER) * PWM_MAX) / JOY_CENTER : 0;

    // Leitura do eixo Y (LED Azul)
    adc_select_input(1);  // Seleciona ADC1 (GPIO27)
    uint16_t adc_y = adc_read();
    int16_t pwm_b = (abs(adc_y - JOY_CENTER) > DEADZONE) ? abs((adc_y - JOY_CENTER) * PWM_MAX) / JOY_CENTER : 0;

    if (pwm_enabled) {
        set_pwm_level(led_r, pwm_r);
        set_pwm_level(led_b, pwm_b);
    }
    // Exibe os valores lidos no terminal
    printf("Joystick X: %d, PWM Vermelho: %d | Joystick Y: %d, PWM Azul: %d\n", adc_x, pwm_r, adc_y, pwm_b);
}

int main()
{

    stdio_init_all();
    sleep_ms(2000);
    adc_init();

    gpio_init(led_g);
    gpio_set_dir(led_g, GPIO_OUT);  
    gpio_put(led_g, 0);

    gpio_set_function(led_b, GPIO_FUNC_PWM);
    uint slice_num_b = pwm_gpio_to_slice_num(led_b);


    gpio_set_function(led_r, GPIO_FUNC_PWM);
    uint slice_num_r = pwm_gpio_to_slice_num(led_r);

    // Configura o divisor do relógio de PWM para atingir 50Hz
    pwm_set_clkdiv(slice_num_b, 125.0f);
    pwm_set_clkdiv(slice_num_r, 125.0f);

    // Configura o valor de envolvimento de PWM para 20ms
    pwm_set_wrap(slice_num_b, 20000);
    pwm_set_wrap(slice_num_r, 20000);

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


    adc_gpio_init(26); // Configura GPIO26 como ADC0
    adc_gpio_init(27); // Configura GPIO27 como ADC1

    while (true) {

        update_pwm_from_joystick();
        sleep_ms(50); 

    }
}
