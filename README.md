## **Descrição do Projeto**
Este projeto utiliza a placa **BitDogLab** para implementar funcionalidades que combinam hardware e software, empregando os seguintes componentes:
- **Matriz 5x5 de LEDs WS2812** conectada à GPIO 7.
- **LED RGB comum**, com os pinos conectados às GPIOs (11, 12 e 13).
- **Botão A** conectado à GPIO 5.
- **Botão B** conectado à GPIO 6.
- **Display SSD1306** conectado via I2C (GPIO 14 e GPIO 15).

Os caracteres suportados pelo sistema são:
- Letras maiúsculas de **A a Z**.
- Letras minúsculas específicas: **c, a, m, i, l**.
- Números de **0 a 9**.

---

## **Funcionalidades do Projeto**

### **1. Entrada de Caracteres**
- Utilizar o Serial Monitor do VS Code para enviar caracteres ao microcontrolador.
- Cada caractere digitado será exibido no display SSD1306.
  - *Nota*: Apenas um caractere será enviado por vez.
- Quando um número entre 0 e 9 for enviado, o símbolo correspondente será exibido, também, na matriz 5x5 WS2812.

### **2. Interação com o Botão A**
- Pressionar o botão A alterna o estado do LED RGB Verde (ligado/desligado).
- O estado do LED será registrado:
  - "LED VERDE ON" OU "LED VERDE OFF" será exibido no display SSD1306.
  - "Muda estado LED verde" será enviado ao Serial Monitor.

### **3. Interação com o Botão B**
- Pressionar o botão B alterna o estado do LED RGB Azul (ligado/desligado).
- O estado do LED será registrado:
  - "LED AZUL ON" OU "LED AZUL OFF" será exibido no display SSD1306.
  - "Muda estado LED azul" será enviado ao Serial Monitor.

---

## **Estrutura Geral do Código**

O código principal é dividido nas seguintes seções:

1. Configuração inicial dos periféricos:
   - Configuração dos pinos GPIO para LEDs e botões.
   - Inicialização da comunicação I2C para controle do display SSD1306.
   - Configuração do display SSD1306.
   - Configuração da matriz WS2812 utilizando PIO (Programável Input/Output).
  
2. Manipulação dos LEDs:
   - Controle dos LEDs RGB comum (GPIOs 11, 12, 13).
   - Exibição de números na matriz WS2812 utilizando padrões predefinidos para cada número (0 a 9).

3. Interrupções e Debouncing:
   - Implementação das rotinas IRQ para os botões A e B, garantindo o tratamento correto de bouncing via software.

4. Comunicação Serial:
   - Leitura de caracteres enviados pelo PC via UART.
   - Exibição dos caracteres no display OLED ou atualização da matriz WS2812 caso seja um número.

---

## **Como Executar**

1. Conecte a placa BitDogLab ao computador.
2. Compile e carregue o código no microcontrolador utilizando a IDE VS Code ou outra ferramenta compatível.
3. Abra o Serial Monitor na IDE para enviar caracteres ao dispositivo.
4. Teste as funcionalidades:
   - Digite caracteres no Serial Monitor para exibi-los no display OLED ou na matriz WS2812 (se for um número).
   - Pressione os botões A e B para alternar os estados dos LEDs RGB Verde e Azul, respectivamente.

---
## **Vídeo demostrativo:**
