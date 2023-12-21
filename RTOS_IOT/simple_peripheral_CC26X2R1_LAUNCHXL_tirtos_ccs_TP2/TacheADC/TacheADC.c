/*
 * TacheADC.c
 *
 *  Created on: 22 nov. 2023
 *      Author: Table2
 */

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
#include <TacheADC/TacheADC.h>
#include <ti/drivers/GPIO.h>
#include <ti_drivers_config.h>
#include <ti/drivers/ADC.h>
#include <TacheLCD/TacheLCD.h>
#include <Profiles/Accelerometre.h>
#include <Application/simple_peripheral.h>

#define ADC_SAMPLE_COUNT (10)

#define TACHEADC_TASK_PRIORITY 1
#define TACHEADC_TASK_STACK_SIZE 1024
Task_Struct TacheADC;
uint8_t TacheADCStack[TACHEADC_TASK_STACK_SIZE];
Semaphore_Struct semTacheADCStruct;
Semaphore_Handle semTacheADCHandle;

void Sampling(uint_least8_t Board_ADC_Number);

void Turn_on_LEDS(void)
{
    GPIO_write(LED1, 1);
    GPIO_write(LED2, 0);
}

void Turn_off_LEDS(void)
{
    GPIO_write(LED1, 0);
    GPIO_write(LED2, 0);
}

extern void TacheADC_init(void);

uint16_t adcValue0;
uint32_t adcValue0MicroVolt;
uint16_t adcValue1[ADC_SAMPLE_COUNT];
uint32_t adcValue1MicroVolt[ADC_SAMPLE_COUNT];
static Clock_Struct myClock;



void myClockSwiFxn(uintptr_t arg0)
{
    Semaphore_post(semTacheADCHandle);
}
uint16_t i;
ADC_Handle adc;
ADC_Params params;
int_fast16_t res;

extern void TacheADC_init(void)
{
    GPIO_init();
    ADC_init();
    ADC_Params_init(&params);
    Clock_Params clockParams;
    Clock_Params_init(&clockParams);
    clockParams.period = 20 * (1000/Clock_tickPeriod),//100ms
    Clock_construct(&myClock, myClockSwiFxn,0, // Initial delay before first timeout
                    &clockParams);
                    Clock_start(Clock_handle(&myClock));//Timer start
}

float accx, accy, accz, joyVer, joyHor;
int count = 0;

static void TacheADC_taskFxn(UArg a0, UArg a1)
{
    TacheADC_init();
        for(;;)
        {
            /*
            Turn_on_LEDS();
            Task_sleep(1000 * (1000 / Clock_tickPeriod));
            Turn_off_LEDS();
            Task_sleep(1000 * (1000 / Clock_tickPeriod));
            */
            //Le semaphore est poste par le timer myClock
            Semaphore_pend(semTacheADCHandle, BIOS_WAIT_FOREVER);

            if (count >= 5)
            {
            Sampling(CONFIG_ADC_0);
            Sampling(CONFIG_ADC_1);
            Sampling(CONFIG_ADC_2);
            count = 0;
            }
            count++;

            Sampling(JOY_VER);
            Sampling(JOY_HOR);



            //SaveDataToSend(accx*100, accy*100, accz*100, joyVer*10, joyHor*10);
            //Carte_enqueueMsg(PZ_MSG_ACCELEROMETRE, NULL);
            //afficherDonnees(accx, accy, accz);
            afficherDonneesJoy(joyVer, joyHor);


        }

    }


void TacheADC_CreateTask(void){
    Semaphore_Params semParams;
    Task_Params taskParams;
    /* Configuration de la tache*/
    Task_Params_init(&taskParams);
    taskParams.stack = TacheADCStack;
    taskParams.stackSize = TACHEADC_TASK_STACK_SIZE;
    taskParams.priority = TACHEADC_TASK_PRIORITY;
    /* Creation de la tache*/
    Task_construct(&TacheADC, TacheADC_taskFxn, &taskParams, NULL);
    /* Construire un objet semaphore pour etre utilise comme outil de verrouillage, comptage initial 0 */
    Semaphore_Params_init(&semParams);
    Semaphore_construct(&semTacheADCStruct, 0, &semParams);
    /* Obtenir la gestion de l'instance */
    semTacheADCHandle = Semaphore_handle(&semTacheADCStruct);
}

float Vaccx, Vaccy, Vaccz, VjoyVer, VjoyHor;
void Sampling(uint_least8_t Board_ADC_Number)
{

    adc = ADC_open(Board_ADC_Number, &params);
    if (adc == NULL){
        while (1);
        }

    if (Board_ADC_Number == JOY_VER || Board_ADC_Number == JOY_HOR)
    {
        for (i = 0; i < ADC_SAMPLE_COUNT; i++)
        {
        res = ADC_convert(adc, &adcValue1[i]);
        if (res == ADC_STATUS_SUCCESS){
            adcValue1MicroVolt[i] = ADC_convertRawToMicroVolts(adc, adcValue1[i]);
            if (Board_ADC_Number == JOY_VER ){
                VjoyVer = adcValue1MicroVolt[i] / 1000000.0;
                joyVer =  (VjoyVer / 3.3) * 100.0;
            }
            if (Board_ADC_Number == JOY_HOR ){
                VjoyHor = adcValue1MicroVolt[i] / 1000000.0;
                joyHor =  (VjoyHor / 3.3) * 100.0;
            }

            }
        }
    }
/*
    else{

    for (i = 0; i < ADC_SAMPLE_COUNT; i++)
    {
    res = ADC_convert(adc, &adcValue1[i]);
    if (res == ADC_STATUS_SUCCESS){
        adcValue1MicroVolt[i] = ADC_convertRawToMicroVolts(adc, adcValue1[i]);
        if (Board_ADC_Number == CONFIG_ADC_0 ){
            Vaccx = adcValue1MicroVolt[i] / 1000000.0;
            accx = (9.81 / 0.66) * (Vaccx - 1.65);
        }
        if (Board_ADC_Number == CONFIG_ADC_1 ){
            Vaccy = adcValue1MicroVolt[i] / 1000000.0;
            accy = (9.81 / 0.66) * (Vaccy - 1.65);
        }
        if (Board_ADC_Number == CONFIG_ADC_2 ){
            Vaccz = adcValue1MicroVolt[i] / 1000000.0;
            accz = (9.81 / 0.66) * (Vaccz - 1.65);
        }

        }
    }
    }
    */

    ADC_close(adc);
}




