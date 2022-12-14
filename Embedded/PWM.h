/* 
 * File:   PWM.h
 * Author: Table2
 *
 * Created on 7 d�cembre 2022, 13:54
 */

#ifndef PWM_H
#define	PWM_H

#define MOTEUR_GAUCHE 1
#define MOTEUR_DROIT  0

void InitPWM(void);
//void PWMSetSpeed(float vitesseEnPourcents,int m);
void PWMUpdateSpeed(void);
void PWMSetSpeedConsigne(float vitesseEnPourcents, char moteur);
#endif	/* PWM_H */

