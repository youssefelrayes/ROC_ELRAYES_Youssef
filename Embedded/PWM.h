/* 
 * File:   PWM.h
 * Author: Table2
 *
 * Created on 7 décembre 2022, 13:54
 */

#ifndef PWM_H
#define	PWM_H

#define MOTEUR_GAUCHE 1
#define MOTEUR_DROIT  0

void InitPWM(void);
void PWMSetSpeed(int m,float vitesseEnPourcents);

#endif	/* PWM_H */

