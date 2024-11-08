#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "mpu6050.h"

#define SDA_GPIO  4
#define SCL_GPIO  5 
#define LED_PIN_X 14 // Led rojo - Eje X
#define LED_PIN_Y 15 // Led verde - Eje Y
#define LED_PIN_VALVE 13 // Valvula de agua
#define X_LIMIT 40.20 // Límite de inclinación para el eje X
#define Y_LIMIT 40.20 // Límite de inclinación para el eje Y


int main(void)
{  
    stdio_init_all();
    sleep_ms(1000);

    printf("Inicializando I2C...\n");

    // Inicializacion de I2C
    i2c_init(i2c0, 400000);
    gpio_set_function(SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(SCL_GPIO, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_GPIO);
    gpio_pull_up(SCL_GPIO);

    printf("Inicializando MPU6050...\n");

    // Inicializacion de MPU6050
    mpu6050_init(i2c0, 0x68);
    sleep_ms(2000);

    printf("Who am I = 0x%2x\n", mpu6050_who_am_i());

    // Configuración de los LEDs como salidas
    gpio_init(LED_PIN_X);
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_X, GPIO_OUT);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);
    gpio_init(LED_PIN_VALVE);
    gpio_set_dir(LED_PIN_VALVE, GPIO_OUT);


    while (true) {

        // Estructura para datos de aceleracion
        mpu_accel_t accel = {0};
        mpu_gyro_t gyro = {0};
        int16_t temp = 0;

        // Leo MPU
        mpu6050_read_accel(&accel);
        mpu6050_read_gyro(&gyro);
        mpu6050_read_temp(&temp);

        // Convertir aceleracion a G(gravedad)
        float ax = (9.81 * (accel.accel_x / 33678.0));
        float ay = (9.81 * (accel.accel_y / 32768.0));
        float az = (9.81 * (accel.accel_z / 32768.0));

        //Calcular los angulos de inclinacion
        float accel_ang_x = atan(ax / sqrt(pow(ay, 2) + pow(az, 2))) * (180.0 / 3.14);
        float accel_ang_y = atan(ay / sqrt(pow(ax, 2) + pow(az, 2))) * (180.0 / 3.14);
        // Muestro valores de aceleracion y angulos de inclinacion
        printf("Ax = %.2f, Ay = %.2f, Az = %.2f\n", ax, ay, az);
        printf("Inclinacion en X: %.2f\tInclinacion en Y: %.2f\n", accel_ang_x, accel_ang_y);
        printf("Temp = %d\n", temp);
        
        // Verificar si ambos ejes están dentro del rango de ±40 grados
        bool leds_on = (fabs(accel_ang_x) < 40.0) && (fabs(accel_ang_y) < 40.0);
        
        // Controlar los LEDs
        gpio_put(LED_PIN_X, leds_on);  // Enciende el LED rojo si ambos ejes cumplen la condición
        gpio_put(LED_PIN_Y, leds_on);  // Enciende el LED verde si ambos ejes cumplen la condición

        // Controlar la válvula que se abre si ambos LEDs están encendidos
        if (leds_on) {
            gpio_put(LED_PIN_VALVE, 0);  // Abrir la válvula
        } else {
            gpio_put(LED_PIN_VALVE, 1);  // Cerrar la válvula
        }

        // Pausa de 100 ms entre lecturas
        sleep_ms(1000);
    }
}