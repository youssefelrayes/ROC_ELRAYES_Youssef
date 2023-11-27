/*
 * Copyright (c) 2018-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== i2ctmp.c ========
 */
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/display/Display.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include <ti_drivers_config.h>
#include <TacheLCD/TacheLCD.h>
#include <driverlib/ssi.h>


/* Driver configuration */
#include "ti_drivers_config.h"

#define TASKSTACKSIZE       640

/* Light result registers */
#define OPT_RESULT_REG          0x0001


/* I2C slave addresses */
#define OPT3001_LAUNCHPAD_ADDR      0x44



#define TACHEI2C_TASK_PRIORITY 1
#define TACHEI2C_TASK_STACK_SIZE 1024
Task_Struct TacheI2C;
uint8_t TacheI2CStack[TACHEI2C_TASK_STACK_SIZE];
Semaphore_Struct semTacheI2CStruct;
Semaphore_Handle semTacheI2CHandle;
extern void TacheI2C_init(void);




static const struct {
    uint8_t address;
    uint8_t resultReg;
    char *id;
} sensors = {
    { OPT3001_LAUNCHPAD_ADDR,  OPT_RESULT_REG, "3001" }
};

static uint8_t slaveAddress;
static Display_Handle display;

static void i2cErrorHandler(I2C_Transaction *transaction,
    Display_Handle display);

/*
 *  ======== mainThread ========
 */
uint16_t dataSensor;


static void TacheI2C_taskFxn(UArg a0, UArg a1)
{
    for(;;)
    {
    }
}


void TacheI2C_CreateTask(void){
    Semaphore_Params semParams;
    Task_Params taskParams;
    /* Configuration de la tache*/
    Task_Params_init(&taskParams);
    taskParams.stack = TacheI2CStack;
    taskParams.stackSize = TACHEI2C_TASK_STACK_SIZE;
    taskParams.priority = TACHEI2C_TASK_PRIORITY;
    /* Creation de la tache*/
    Task_construct(&TacheI2C, TacheI2C_taskFxn, &taskParams, NULL);
    /* Construire un objet semaphore
    pour etre utilise comme outil de
    verrouillage, comptage initial 0 */
    Semaphore_Params_init(&semParams);
    Semaphore_construct(&semTacheI2CStruct, 0, &semParams);
    /* Obtenir la gestion de l'instance */
    semTacheI2CHandle = Semaphore_handle(&semTacheI2CStruct);
    }

//==============================================================
void *mainThread(void *arg0)
{
    uint16_t        sample;
    int16_t         temperature;
    uint8_t         txBuffer[1];
    uint8_t         rxBuffer[2];
    I2C_Handle      i2c;
    I2C_Params      i2cParams;
    I2C_Transaction i2cTransaction;

    /* Call driver init functions */
    Display_init();
    GPIO_init();
    I2C_init();



    /* Open the UART display for output */
    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
        while (1);
    }


    Display_printf(display, 0, 0, "Starting the i2ctmp example\n");

    /* Create I2C for usage */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(CONFIG_I2C_LIGHT, &i2cParams);
    if (i2c == NULL) {
        Display_printf(display, 0, 0, "Error Initializing I2C\n");
        while (1);
    }
    else {
        Display_printf(display, 0, 0, "I2C Initialized!\n");
    }

    /* Common I2C transaction setup */
    i2cTransaction.writeBuf   = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf    = rxBuffer;
    i2cTransaction.readCount  = 2;


    i2cTransaction.slaveAddress = 0x44;
    txBuffer[0] = 0x7F;

    if (I2C_transfer(i2c, &i2cTransaction)) {
        dataSensor = rxBuffer[0];
        dataSensor = dataSensor << 8 ;
        dataSensor = dataSensor | rxBuffer[1];
        slaveAddress = sensors.address;
        Display_printf(display, 0, 0, "Detected TMP%s sensor with slave"
        " address 0x%x", sensors.id, sensors.address);
    }
    else {
        i2cErrorHandler(&i2cTransaction, display);
    }



    /*
     * Determine which I2C sensors are present by querying known I2C
     * slave addresses.
     */


    /* If we never assigned a slave address */
    if (slaveAddress == 0) {
        Display_printf(display, 0, 0, "Failed to detect a sensor!");
        I2C_close(i2c);
        while (1);
    }

    Display_printf(display, 0, 0, "\nUsing last known sensor for samples.");
    i2cTransaction.slaveAddress = slaveAddress;

    /* Take 20 samples and print them out onto the console */
    i2cTransaction.readCount  = 2;
    for (sample = 0; sample < 20; sample++) {
        if (I2C_transfer(i2c, &i2cTransaction)) {
            /*
             * Extract degrees C from the received data;
             * see TMP sensor datasheet
             */
            temperature = (rxBuffer[0] << 8) | (rxBuffer[1]);
            temperature *= 0.0078125;

            /*
             * If the MSB is set '1', then we have a 2's complement
             * negative value which needs to be sign extended
             */
            if (rxBuffer[0] & 0x80) {
                temperature |= 0xF000;
            }

            Display_printf(display, 0, 0, "Sample %u: %d (C)",
                sample, temperature);
        }
        else {
            i2cErrorHandler(&i2cTransaction, display);
        }

        /* Sleep for 1 second */
        sleep(1);
    }

    I2C_close(i2c);
    Display_printf(display, 0, 0, "I2C closed!");

    return (NULL);
}

/*
 *  ======== i2cErrorHandler ========
 */
static void i2cErrorHandler(I2C_Transaction *transaction,
    Display_Handle display)
{
    switch (transaction->status) {
    case I2C_STATUS_TIMEOUT:
        Display_printf(display, 0, 0, "I2C transaction timed out!");
        break;
    case I2C_STATUS_CLOCK_TIMEOUT:
        Display_printf(display, 0, 0, "I2C serial clock line timed out!");
        break;
    case I2C_STATUS_ADDR_NACK:
        Display_printf(display, 0, 0, "I2C slave address 0x%x not"
            " acknowledged!", transaction->slaveAddress);
        break;
    case I2C_STATUS_DATA_NACK:
        Display_printf(display, 0, 0, "I2C data byte not acknowledged!");
        break;
    case I2C_STATUS_ARB_LOST:
        Display_printf(display, 0, 0, "I2C arbitration to another master!");
        break;
    case I2C_STATUS_INCOMPLETE:
        Display_printf(display, 0, 0, "I2C transaction returned before completion!");
        break;
    case I2C_STATUS_BUS_BUSY:
        Display_printf(display, 0, 0, "I2C bus is already in use!");
        break;
    case I2C_STATUS_CANCEL:
        Display_printf(display, 0, 0, "I2C transaction cancelled!");
        break;
    case I2C_STATUS_INVALID_TRANS:
        Display_printf(display, 0, 0, "I2C transaction invalid!");
        break;
    case I2C_STATUS_ERROR:
        Display_printf(display, 0, 0, "I2C generic error!");
        break;
    default:
        Display_printf(display, 0, 0, "I2C undefined error case!");
        break;
    }
}

//=============================================










