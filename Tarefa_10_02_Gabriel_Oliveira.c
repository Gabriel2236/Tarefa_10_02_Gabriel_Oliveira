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
#define DEADZONE 500

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

int point_x = DISPLAY_WIDTH / 2;  // Posição inicial X do ponto (centro)
int point_y = DISPLAY_HEIGHT / 2; // Posição inicial Y do ponto (centro)

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

bool border_enabled = false; // Variável global para alternar a borda

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (current_time - last_time > 200000) {
        if (gpio == BOTTON_J ) {
            gpio_put(led_g, !gpio_get(led_g));

            border_enabled = !border_enabled;


        } else if (gpio == BOTTON_A) {
            pwm_enabled = !pwm_enabled;
            
            if (pwm_enabled) {
                printf("PWM ativado\n");
                pwm_set_enabled(pwm_gpio_to_slice_num(led_b), true);
                pwm_set_enabled(pwm_gpio_to_slice_num(led_r), true);
            } else {
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
    adc_select_input(1);  // Seleciona ADC0 (GPIO26)
    uint16_t adc_x = adc_read();
    int16_t pwm_r = (abs(adc_x - JOY_CENTER) > DEADZONE) ? abs((adc_x - JOY_CENTER) * PWM_MAX) / JOY_CENTER : 0;

    // Leitura do eixo Y (LED Azul)
    adc_select_input(0);  // Seleciona ADC1 (GPIO27)
    uint16_t adc_y = adc_read();
    int16_t pwm_b = (abs(adc_y - JOY_CENTER) > DEADZONE) ? abs((adc_y - JOY_CENTER) * PWM_MAX) / JOY_CENTER : 0;

    if (pwm_enabled) {
        set_pwm_level(led_r, pwm_r);
        set_pwm_level(led_b, pwm_b);
    }
    // Exibe os valores lidos no terminal
    printf("Joystick X: %d, PWM Vermelho: %d | Joystick Y: %d, PWM Azul: %d\n", adc_x, pwm_r, adc_y, pwm_b);
}

void update_point_position() {
    // Leitura do eixo X (Controle do ponto na tela)
    adc_select_input(1);  // Seleciona ADC0 (GPIO26)
    uint16_t adc_x = adc_read();
    int delta_x = (adc_x - JOY_CENTER);

    // Leitura do eixo Y (Controle do ponto na tela)
    adc_select_input(0);  // Seleciona ADC1 (GPIO27)
    uint16_t adc_y = adc_read();
    int delta_y = (adc_y - JOY_CENTER);

    // Se o joystick estiver na posição central (dentro da DEADZONE), reseta o ponto para o meio
    if (abs(delta_x) < DEADZONE && abs(delta_y) < DEADZONE) {
        point_x = DISPLAY_WIDTH / 2;
        point_y = DISPLAY_HEIGHT / 2;
    } else {
        // Atualiza a posição do ponto com base no joystick (invertendo os sinais)
        point_x += delta_x / 200;  // Ajuste a velocidade de movimento
        point_y -= delta_y / 200;  // Inverte o movimento do eixo Y
    }

    // Limita a posição do ponto dentro dos limites do display
    if (point_x < 0) point_x = 0;
    if (point_x >= DISPLAY_WIDTH - 8) point_x = DISPLAY_WIDTH - 8;
    if (point_y < 0) point_y = 0;
    if (point_y >= DISPLAY_HEIGHT - 8) point_y = DISPLAY_HEIGHT - 8;

    // Desenha o ponto no display
    ssd1306_fill(&ssd, false); // Limpa o display

    if (border_enabled) {
        ssd1306_rect(&ssd, 0, 0, 128, 64, true, false);
    }

    ssd1306_draw_block(&ssd, point_x, point_y, true); // Desenha o ponto
    ssd1306_send_data(&ssd); // Atualiza o display
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
        update_point_position();
        update_pwm_from_joystick();
        sleep_ms(50); 

    }
}
