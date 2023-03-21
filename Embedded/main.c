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
#include "ADC.h"
#include "main.h"

unsigned char stateRobot;
unsigned char nextStateRobot=0;

int main(void) {
    /***************************************************************************************************/
    //Initialisation de l?oscillateur
    /****************************************************************************************************/
    InitOscillator();

    /****************************************************************************************************/
    // Configuration des entr�es sorties
    /****************************************************************************************************/
    InitIO();

    InitTimer23();
    InitTimer1();
    InitPWM();
    InitADC1();
    InitTimer4();

    
    /*
    LED_BLANCHE = 1;
    LED_BLEUE = 1;
    LED_ORANGE = 1;
    */
    


    /****************************************************************************************************/
    // Boucle Principale
    /****************************************************************************************************/
    while (1) {


    
    if (ADCIsConversionFinished() == 1)
    {
        ADCClearConversionFinishedFlag();
        unsigned int * result = ADCGetResult();
        
        float volts = ((float) result [0])* 3.3 / 4096 * 3.2;
        robotState.distanceTelemetreDroitDroit = 34 / volts - 5;
        
        volts = ((float) result [1])* 3.3 / 4096 * 3.2;
        robotState.distanceTelemetreDroit = 34 / volts - 5;
        
        volts = ((float) result [2])* 3.3 / 4096 * 3.2;
        robotState.distanceTelemetreCentre = 34 / volts - 5; 
        
        volts = ((float) result [3])* 3.3 / 4096 * 3.2;
        robotState.distanceTelemetreGaucheGauche = 34 / volts - 5;
        
        volts = ((float) result [4])* 3.3 / 4096 * 3.2;
        robotState.distanceTelemetreGauche = 34 / volts - 5;
       
         
    }
    OperatingSystemLoop();

    } // fin main
}


void OperatingSystemLoop(void)
{
switch (stateRobot)
{
case STATE_ATTENTE:
timestamp = 0;
PWMSetSpeedConsigne(0, MOTEUR_DROIT);
PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
stateRobot = STATE_ATTENTE_EN_COURS;
case STATE_ATTENTE_EN_COURS:
if (timestamp > 1000)
stateRobot = STATE_AVANCE;
break;
case STATE_AVANCE:
PWMSetSpeedConsigne(24, MOTEUR_DROIT);
PWMSetSpeedConsigne(24, MOTEUR_GAUCHE);
stateRobot = STATE_AVANCE_EN_COURS;
break;
case STATE_AVANCE_EN_COURS:
SetNextRobotStateInAutomaticMode();
break;
case STATE_TOURNE_GAUCHE:
PWMSetSpeedConsigne(24, MOTEUR_DROIT);
PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
stateRobot = STATE_TOURNE_GAUCHE_EN_COURS;
break;
case STATE_TOURNE_GAUCHE_EN_COURS:
SetNextRobotStateInAutomaticMode();
break;
case STATE_TOURNE_DROITE:
PWMSetSpeedConsigne(0, MOTEUR_DROIT);
PWMSetSpeedConsigne(24, MOTEUR_GAUCHE);
stateRobot = STATE_TOURNE_DROITE_EN_COURS;
break;
case STATE_TOURNE_DROITE_EN_COURS:
SetNextRobotStateInAutomaticMode();
break;
case STATE_TOURNE_SUR_PLACE_GAUCHE:
PWMSetSpeedConsigne(15, MOTEUR_DROIT);
PWMSetSpeedConsigne(-15, MOTEUR_GAUCHE);
stateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS;
break;
case STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS:
SetNextRobotStateInAutomaticMode();
break;
case STATE_TOURNE_SUR_PLACE_DROITE:
PWMSetSpeedConsigne(-15, MOTEUR_DROIT);
PWMSetSpeedConsigne(15, MOTEUR_GAUCHE);
stateRobot = STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS;
break;
case STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS:
SetNextRobotStateInAutomaticMode();
break;
default :
stateRobot = STATE_ATTENTE;
break;
}
}

void SetNextRobotStateInAutomaticMode()
{
unsigned char positionObstacle = PAS_D_OBSTACLE;
//�Dtermination de la position des obstacles en fonction des ���tlmtres
if ( robotState.distanceTelemetreDroit < 25 || robotState.distanceTelemetreDroitDroit < 30 &&
robotState.distanceTelemetreCentre > 20 &&
robotState.distanceTelemetreGauche > 30 ) //Obstacle �droite
positionObstacle = OBSTACLE_A_DROITE;
else if(robotState.distanceTelemetreDroit > 30 &&
robotState.distanceTelemetreCentre > 20 &&
robotState.distanceTelemetreGauche < 25 || robotState.distanceTelemetreGaucheGauche < 30) //Obstacle �gauche
positionObstacle = OBSTACLE_A_GAUCHE;
else if(robotState.distanceTelemetreCentre < 20) //Obstacle en face
positionObstacle = OBSTACLE_EN_FACE;
else if(robotState.distanceTelemetreDroit > 25 && robotState.distanceTelemetreDroitDroit > 30 &&
robotState.distanceTelemetreCentre > 20 &&
robotState.distanceTelemetreGauche > 25 && robotState.distanceTelemetreGaucheGauche > 30) //pas d?obstacle
positionObstacle = PAS_D_OBSTACLE;
//�Dtermination de l�?tat �venir du robot
if (positionObstacle == PAS_D_OBSTACLE)
nextStateRobot = STATE_AVANCE;
else if (positionObstacle == OBSTACLE_A_DROITE)
nextStateRobot = STATE_TOURNE_GAUCHE;
else if (positionObstacle == OBSTACLE_A_GAUCHE)
nextStateRobot = STATE_TOURNE_DROITE;
else if (positionObstacle == OBSTACLE_EN_FACE)
nextStateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE;
//Si l?on n?est pas dans la transition de l�?tape en cours
if (nextStateRobot != stateRobot - 1)
stateRobot = nextStateRobot;
}

