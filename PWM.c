#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"

#define LED_PWM 12
#define Servo_PWM 22

//Coloca o PWM para 50Hz aproximadamente
const uint16_t WRAP = 39000; // valor maximo do pwm
const float DIVISER = 64; //divisor do clock para PWM

const uint16_t servo0 = 975;
const uint16_t servo90 = 2866;
const uint16_t servo180 = 4680;
const uint16_t add50 = 50;
uint tempo = 10; // Tempo de transição em milissegundos
uint16_t duty = servo180; // duty cicle definido em 0.12% do wrap
const uint16_t limit = 47;//duty limite

volatile bool cond = false;
volatile bool Des_Sob = false;

void pwm_servo(){
    gpio_set_function(Servo_PWM, GPIO_FUNC_PWM); //habilitar o pino GPIO como PWM
    uint slice = pwm_gpio_to_slice_num(Servo_PWM); //obter o canal PWM da GPIO
    pwm_set_clkdiv(slice, DIVISER); //define o divisor de clock do PWM
    pwm_set_wrap(slice, WRAP); //definir o valor de wrap
    pwm_set_gpio_level(Servo_PWM, duty); //definir o ciclo de trabalho (duty cycle) do pwm
    pwm_set_enabled(slice, true); //habilita o pwm no slice correspondente
}

void pwm_led(){
    gpio_set_function(LED_PWM, GPIO_FUNC_PWM); //habilitar o pino GPIO como PWM
    uint slice = pwm_gpio_to_slice_num(LED_PWM); //obter o canal PWM da GPIO
    pwm_set_clkdiv(slice, DIVISER); //define o divisor de clock do PWM
    pwm_set_wrap(slice, WRAP); //definir o valor de wrap
    pwm_set_gpio_level(LED_PWM, duty); //definir o ciclo de trabalho (duty cycle) do pwm
    pwm_set_enabled(slice, true); //habilita o pwm no slice correspondente

}

// Callback para desligar o terceiro LED e liberar a ativação do botão
uint64_t turn_off_Callback3(alarm_id_t id, void *user_data) {
    cond = true;

    return 0;
}

// Callback para desligar o segundo LED e iniciar o temporizador para o próximo
uint64_t turn_off_Callback2(alarm_id_t id, void *user_data) {
    duty = servo0;//0.025% do wrap
    pwm_set_gpio_level(LED_PWM, duty); //definir o ciclo de trabalho (duty cycle) do pwm
    pwm_set_gpio_level(Servo_PWM, duty); //definir o ciclo de trabalho (duty cycle) do pwm
    add_alarm_in_ms(5000, turn_off_Callback3, NULL, false); // Chama o próximo callback após 5 segundos
    return 0;
}

// Callback para desligar o primeiro LED e iniciar o temporizador para o próximo
uint64_t turn_off_Callback1(alarm_id_t id, void *user_data) {
    duty = servo90;//0.0735% do wrap
    pwm_set_gpio_level(LED_PWM, duty); //definir o ciclo de trabalho (duty cycle) do pwm
    pwm_set_gpio_level(Servo_PWM, duty); //definir o ciclo de trabalho (duty cycle) do pwm
    add_alarm_in_ms(5000, turn_off_Callback2, NULL, false); // Chama o próximo callback após 5 segundos
    return 0;
}
// Callback para a repetição do temporizador
bool repeating_timer_callback(struct repeating_timer *t) {
    if(Des_Sob == false){
        duty+=add50;
        if(duty >= servo180){
            Des_Sob = true;
        }
    }else{
        duty-=add50;
        if(duty <=  servo0){
            Des_Sob = false;
        }
    }
    
    pwm_set_gpio_level(LED_PWM, duty); //definir o ciclo de trabalho (duty cycle) do pwm
    pwm_set_gpio_level(Servo_PWM, duty); //definir o ciclo de trabalho (duty cycle) do pwm
    return true;
}


int main()
{
    stdio_init_all();

    pwm_servo();
    pwm_led();

    // Configura o temporizador repetitivo
    struct repeating_timer timer;

    // Definição do intervalo de atualização (1 segundo)
    uint32_t interval = 1000;

    // Tempo da próxima atualização
    absolute_time_t next_wake_time;

    add_alarm_in_ms(5000, turn_off_Callback1, NULL, false);


    while (true) {
        if(cond == true){
            add_repeating_timer_ms(tempo, repeating_timer_callback, NULL, &timer);
            cond = false;
        }
        if (time_reached(next_wake_time)) {
                next_wake_time = delayed_by_us(next_wake_time, interval * 1000);
        }
    }
}
