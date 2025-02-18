# Projeto: Controle de LED RGB e Display OLED com Joystick no Raspberry Pi Pico

## Descrição

Este projeto utiliza um **Raspberry Pi Pico** para controlar um **LED RGB** e um **display OLED SSD1306** através de um **joystick analógico**. O código permite:

- Controlar a intensidade das cores do LED RGB com base nos movimentos do joystick.
- Mover um ponto na tela do display OLED com o joystick.
- Alternar a exibição de uma borda no display apertando um botão junto com o estado de um LED verde.
- Ativar e desativar o PWM para os LEDs através de um botão.

## Componentes Utilizados

- **Raspberry Pi Pico**
- **Joystick analógico** (com saídas X, Y e botão de pressão)
- **Display OLED SSD1306** (128x64, interface I2C)
- **LED RGB** (cátodo comum)
- **Resistores** para proteção dos LEDs (se necessário)
- **Botões push-button** para controle


## Ligações

### Display OLED (I2C)

| Pino do Pico  | Componente  |
| ------------- | ----------- |
| GPIO 14 (SDA) | SDA do OLED |
| GPIO 15 (SCL) | SCL do OLED |

### LED RGB

| Pino do Pico | Cor do LED     |
| ------------ | -------------- |
| GPIO 11      | Verde          |
| GPIO 12      | Azul (PWM)     |
| GPIO 13      | Vermelho (PWM) |

### Joystick

| Pino do Pico   | Função            |
| -------------- | ----------------- |
| GPIO 26 (ADC0) | Eixo Y            |
| GPIO 27 (ADC1) | Eixo X            |
| GPIO 22        | Botão do joystick |

### Botões

| Pino do Pico | Função      |
| ------------ | ----------- |
| GPIO 5       | Alterna PWM |

## Como Funciona

1. O **joystick** controla a intensidade das cores dos LEDs vermelho e azul usando **PWM**.
2. A posição do joystick também controla um ponto na tela do **display OLED**.
3. Pressionar o **botão do joystick** alterna a exibição de uma borda na tela.
4. O **botão externo** ativa ou desativa o controle **PWM** do LED RGB.

## Como Compilar e Executar

1. Instale o **SDK do Raspberry Pi Pico** e configure o ambiente de desenvolvimento.
2. Compile o código usando o CMake.
3. Carregue o firmware no **Raspberry Pi Pico**.
4. Conecte os componentes corretamente e reinicie o Pico.

## Vídeo

link: