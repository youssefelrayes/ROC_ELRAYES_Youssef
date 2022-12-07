/* 
 * File:   main.c
 * Author: Table2
 *
 * Created on 16 novembre 2022, 10:00
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "ChipConfig.h"
#include "IO.h"
#include "timer.h"
#include "Robot.h"
#include "ToolBox.h"
#include "PWM.h"

int main(void) {
    /***************************************************************************************************/
    //Initialisation de l?oscillateur
    /****************************************************************************************************/
    InitOscillator();

    /****************************************************************************************************/
    // Configuration des entrées sorties
    /****************************************************************************************************/
    InitIO();
    
    InitTimer23();
    InitTimer1();
    
    InitPWM();
    PWMSetSpeed(MOTEUR_DROIT,60);
    PWMSetSpeed(MOTEUR_GAUCHE,60);

    LED_BLANCHE = 1;
    LED_BLEUE = 1;
    LED_ORANGE = 1;

    /****************************************************************************************************/
    // Boucle Principale
    /****************************************************************************************************/
    while (1) {
     //LED_BLANCHE = !LED_BLANCHE;
  
 

   
        
    } // fin main
}
