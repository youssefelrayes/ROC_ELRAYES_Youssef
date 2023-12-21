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
#include "math.h"


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
} sensors = {OPT3001_LAUNCHPAD_ADDR,  OPT_RESULT_REG, "3001"};

static uint8_t slaveAddress;
char dataSensor;


uint8_t         txBuffer[10];
uint8_t         rxBuffer[10];
float value;
float lux;
static void TacheI2C_taskFxn(UArg a0, UArg a1)
{
    uint16_t        sample;
    int16_t         light;

    I2C_Handle      i2c;
    I2C_Params      i2cParams;
    I2C_Transaction i2cTransaction;

    /* Call driver init functions */
    GPIO_init();
    I2C_init();

    /* Create I2C for usage */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(CONFIG_I2C_LIGHT, &i2cParams);
    if (i2c == NULL) {
        while (1);
    }

    /* Common I2C transaction setup */

        txBuffer[0] = 0x00;

        i2cTransaction.writeBuf   = txBuffer;
        i2cTransaction.writeCount = 1;
        i2cTransaction.readBuf    = rxBuffer;
        i2cTransaction.readCount  = 2;

        i2cTransaction.slaveAddress = 0x44;

        //1) Read Device ID
        /*
        if (I2C_transfer(i2c, &i2cTransaction)) {
            //check id
            dataSensor = rxBuffer[0];
            dataSensor = dataSensor << 8 ;
            dataSensor = dataSensor | rxBuffer[1];
            slaveAddress = sensors.address;
        }
        */
        /*
        //If It is ok lets continue
        //2) Config sensor
        txBuffer[0] = 0x01;
        txBuffer[1] = 0b11000110;
        txBuffer[2] = 0b00010000;

        i2cTransaction.writeBuf   = txBuffer;
        i2cTransaction.writeCount = 3;
        i2cTransaction.readBuf    = rxBuffer;
        i2cTransaction.readCount  = 0;
        */





        for(;;){
            if (I2C_transfer(i2c, &i2cTransaction)) {
                //check id
                dataSensor = (rxBuffer[0]>>4) & 0x0F ;

                switch(dataSensor){
                case 0: value = 0.01; break;
                case 1: value = 0.02; break;
                case 2: value = 0.04; break;
                case 3: value = 0.08; break;
                case 4: value = 0.16; break;
                case 5: value = 0.32; break;
                case 6: value = 0.64; break;
                case 7: value = 1.28; break;
                case 8: value = 2.56; break;
                case 9: value = 5.12; break;
                case 10: value = 10.24; break;
                case 11: value = 20.48; break;
                }

                lux = 0.01*pow(2,value)*1000;
            }
            afficherDonneesLux(lux);
                }

        /*


        if (slaveAddress == 0) {
            I2C_close(i2c);
            while (1);
        }

        i2cTransaction.slaveAddress = slaveAddress;

        i2cTransaction.readCount  = 2;
        for (sample = 0; sample < 20; sample++) {
             if (I2C_transfer(i2c, &i2cTransaction)) {

                        light = (rxBuffer[0] << 8) | (rxBuffer[1]);
                        light *= 0.0078125;

                        if (rxBuffer[0] & 0x80) {
                            light |= 0xF000;
                        }

                    }
                    sleep(1);
                    }
        */



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













