#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdlib.h>
#include <math.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

#define I2C_PORT i2c1
#define I2C_SDA 14  // Pino GPIO para SDA (dados do I2C)
#define I2C_SCL 15  // Pino GPIO para SCL (clock do I2C)
#define ENDERECO 0x3C  // Endereço I2C do display OLED

// Arquivo do programa PIO (responsável pela manipulação dos LEDs da matriz)
#include "DisplayC.pio.h"

// Definições de constantes para a matriz de LEDs
#define LINHAS 5    // Número total de linhas na matriz de LEDs
#define COLUNAS 5   // Número total de colunas na matriz de LEDs
#define OUT_PIN 7   // Pino de saída usado para controle dos LEDs
#define TEMPO 100   // Tempo de espera para piscar LEDs (em milissegundos)

// Configuração dos pinos GPIO
const uint LED_R_PIN = 13; // LED Vermelho => GPIO13
const uint LED_B_PIN = 12; // LED Azul => GPIO12
const uint LED_G_PIN = 11; // LED Verde => GPIO11
const uint BUTTON_A = 5; // Botão para alterar estado LED verde (A) => GPIO5
const uint BUTTON_B = 6; // Botão para alterar estado LED azul  (B) => GPIO6

// Variáveis globais
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (para debouncing)
static int current_number = 0;          // Número atual exibido na matriz de LEDs

// Estrutura para configuração do periférico PIO
typedef struct {
    PIO pio;  // Periférico PIO utilizado
    uint sm;  // State Machine (SM) utilizada dentro do PIO
} PioConfig;

PioConfig config;  // Instância global da configuração PIO
ssd1306_t ssd; // Instância global da configuração SSD

// Protótipo da função que desenha um número na matriz de LEDs
void desenho_pio(int numero);
void gpio_irq_handler(uint gpio, uint32_t events);

// Definição das matrizes que representam os números de 0 a 9
int numeros[10][5][5] = {
    // Número 0
    {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    },
    // Número 1
    {
        {0, 0, 1, 0, 0},
        {0, 1, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {1, 1, 1, 1, 1}
    },
    // Número 2
    {
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 1}
    },
    // Número 3
    {
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {0, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    },
    // Número 4
    {
        {0, 1, 0, 0, 1},
        {1, 0, 0, 1, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 1, 0},
        {0, 1, 0, 0, 0}
    },
    // Número 5
    {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    },
    // Número 6
    {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    },
     // Número 7
    {
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0}
    },
    // Número 8
    {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    },
    // Número 9
    {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    }
};

int main() {
    // Configuração inicial do periférico PIO
    config.pio = pio0; // Utiliza o PIO0
    config.sm = pio_claim_unused_sm(config.pio, true); // Reivindica um State Machine livre

    // Configuração do clock do sistema
    set_sys_clock_khz(128000, false); // Define a frequência do clock do processador para 128 MHz

    // Carrega o programa PIO na memória e inicializa a State Machine
    uint offset = pio_add_program(config.pio, &pio_matrix_program); // Adiciona o programa PIO
    pio_matrix_program_init(config.pio, config.sm, offset, OUT_PIN); // Inicializa o programa PIO com o pino de saída

    // Configuração da comunicação I2C para o display OLED
    i2c_init(I2C_PORT, 400 * 1000); // Inicializa I2C a 400kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Define o pino SDA como função I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Define o pino SCL como função I2C
    gpio_pull_up(I2C_SDA); // Habilita pull-up no pino SDA
    gpio_pull_up(I2C_SCL); // Habilita pull-up no pino SCL

    // Inicialização do display OLED
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT); // Configura o display
    ssd1306_config(&ssd); // Aplica configurações adicionais
    ssd1306_fill(&ssd, false); // Limpa o display (tela preta)
    ssd1306_send_data(&ssd); // Atualiza a tela

    // Configuração dos LEDs como saída
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);

    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);

    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

     // Configuração dos botões como entrada com pull-up interno
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Configuração das interrupções para os botões
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    stdio_init_all(); // Inicializa a comunicação serial

    char recebido;
    
    while (true) {
        if (stdio_usb_connected()) { // Verifica se o USB está conectado
            if (scanf("%c", &recebido) == 1) { // Lê um caractere via USB
                printf("Recebido: '%c'\n", recebido);

                // Atualiza o display OLED com o caractere recebido
                ssd1306_fill(&ssd, false); // Limpa o display
                ssd1306_draw_char(&ssd, recebido, 50, 30); // Exibe o caractere no display
                ssd1306_send_data(&ssd); // Atualiza a tela do display

                // Verifica se o caractere recebido é um número entre '0' e '9'
                if (recebido >= '0' && recebido <= '9') {
                    current_number = recebido - '0'; // Converte char para número inteiro
                    
                    // Atualiza a matriz de LEDs com o número recebido
                    desenho_pio(current_number);
                }
            }
        }
        sleep_ms(100); // Pequeno delay para evitar leituras repetidas
    }
}

// Função para desenhar um número na matriz de LEDs utilizando PIO
void desenho_pio(int numero) {
    int R = 0;   // Intensidade da cor vermelha (desligado)
    int G = 25; // Intensidade da cor verde (máxima)
    int B = 0;   // Intensidade da cor azul (desligado)

    for (int16_t i = LINHAS - 1; i >= 0; i--) { // Varre as linhas de baixo para cima
        for (int16_t j = 0; j < COLUNAS; j++) { // Varre as colunas da esquerda para a direita
            int tempG = G * numeros[numero][i][j]; // Calcula a intensidade do verde com base no número
            uint32_t valor_led = (tempG << 24) | (R << 16) | (B << 8); // Combina os valores RGB em um único número
            pio_sm_put_blocking(config.pio, config.sm, valor_led); // Envia o valor RGB para o periférico PIO
        }
    }
}
  // Função de interrupção com debouncing para tratar os botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    static bool led_green_state = false;
    static bool led_blue_state = false;
    uint32_t current_time = to_us_since_boot(get_absolute_time()); // Obtém o tempo atual em microssegundos

    if (current_time - last_time > 200000) { // Verifica se passaram mais de 200 ms desde o último evento (debouncing)
        last_time = current_time;           // Atualiza o tempo do último evento

        // Verifica se o botão A foi pressionado (muda LED verde)
        if (gpio == BUTTON_A) {
            led_green_state = !led_green_state;
            gpio_put(LED_G_PIN, led_green_state); // Liga/Desliga LED Verde

            // Atualiza a tela OLED
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, led_green_state ? "VERDE ON" : "VERDE OFF", 20, 30);
            ssd1306_send_data(&ssd);
            printf("Muda estado LED verde");
        }

        // Verifica se o botão B foi pressionado (muda LED azul)
        if (gpio == BUTTON_B) {
            led_blue_state = !led_blue_state;
            gpio_put(LED_B_PIN, led_blue_state); // Liga/Desliga LED Azul

            // Atualiza a tela OLED
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, led_blue_state ? "AZUL ON" : "AZUL OFF", 20, 30);
            ssd1306_send_data(&ssd);
            printf("Muda estado LED azul");
        }
    }
}



