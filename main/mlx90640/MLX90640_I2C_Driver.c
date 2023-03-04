/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
 /**
 * As the timings depend heavily on the MCU in use, it is recommended
 * to make sure that the proper timings are achieved. For that purpose
 * an oscilloscope might be needed to strobe the SCL and SDA signals.
 * The Wait(int) function could be modified in order to better
 * trim the frequency. For coarse setting of the frequency or
 * dynamic frequency change using the default function implementation.
 */

#include "MLX90640_I2C_Driver.h"
#include <stdio.h>
#include "driver/i2c.h"



#define PIN_SCL          	26//19//26//19 // v.1.1 большой разъём	// 22 // Старый вариант (разъём I2C)             	/*!< gpio number for I2C master clock */
#define PIN_SDA          	33//22//26//21//25 // v.1.1 большой разъём  // 21 // Старый вариант (разъём I2C)              	/*!< gpio number for I2C master data  */
#define I2C_NUM             I2C_NUM_1        	/*!< I2C port number for master dev */
#define I2C_TX_BUF_DISABLE  0                	/*!< I2C master do not need buffer */
#define I2C_RX_BUF_DISABLE  0                	/*!< I2C master do not need buffer */
#define I2C_FREQ_HZ         1000000	// 400000   /*!< I2C master clock frequency */

#define WRITE_BIT           I2C_MASTER_WRITE 	/*!< I2C master write */
#define READ_BIT            I2C_MASTER_READ  	/*!< I2C master read */
#define ACK_CHECK_EN        1              		/*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS       0              		/*!< I2C master will not check ack from slave */
#define ACK_VAL             (i2c_ack_type_t) 0  /*!< I2C ack value */
#define NACK_VAL            (i2c_ack_type_t) 1  /*!< I2C nack value */


//==============================================================================
// Процедура инициализации i2c ESP32
//==============================================================================
void MLX90640_I2CInit()
{
    int i2c_master_port = I2C_NUM_1;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = PIN_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = PIN_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, I2C_RX_BUF_DISABLE, I2C_TX_BUF_DISABLE, 0);
}
//==============================================================================


//==============================================================================
// Функция чтения массива байт по i2c ESP32
//==============================================================================
int MLX90640_I2CReadBytes(uint8_t slaveAddr, uint16_t startAddress, uint16_t nBytesRead, uint8_t *data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Сначала выбираем внутренний адрес сенсора
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, slaveAddr << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, startAddress >> 8, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, startAddress & 0xFF, ACK_CHECK_EN);

    // Теперь читаем из сенсора
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, slaveAddr << 1 | READ_BIT, ACK_CHECK_EN);
    if (nBytesRead > 1)
        i2c_master_read(cmd, data, nBytesRead - 1, ACK_VAL);
    i2c_master_read_byte(cmd, data + nBytesRead - 1, NACK_VAL);
    i2c_master_stop(cmd);

    int ret = i2c_master_cmd_begin(I2C_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}
//==============================================================================

//uint8_t mlxBuff[2];

//==============================================================================
// Функция чтения массива 2-байтных слов по i2c ESP32
//==============================================================================
int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    uint8_t *mlxBuff = (uint8_t *) heap_caps_malloc(nMemAddressRead << 1, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);
    int ret = MLX90640_I2CReadBytes(slaveAddr, startAddress, nMemAddressRead << 1, mlxBuff);
    if (ret < 0)
    {
        heap_caps_free(mlxBuff);
      	return ret;
    }

    for (int cnt = 0, i = 0; cnt < nMemAddressRead; cnt++, i += 2)
        *(data++) = ((uint16_t) mlxBuff[i] << 8) + mlxBuff[i + 1];

    heap_caps_free(mlxBuff);

    return 0;
}
//==============================================================================


//==============================================================================
// Функция записи 2-байтного слова по i2c ESP32
//==============================================================================
int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Сначала выбираем внутренний адрес сенсора
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slaveAddr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, writeAddress >> 8, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, writeAddress & 0xFF, ACK_CHECK_EN);

    // Пишем данные
    i2c_master_write_byte(cmd, data >> 8, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data & 0xFF, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    // Проверяем запись чтением
    static uint16_t dataCheck;
    MLX90640_I2CRead(slaveAddr, writeAddress, 1, &dataCheck);

    if ( dataCheck != data)
        return -2;
    
    return ret;
}
//==============================================================================
