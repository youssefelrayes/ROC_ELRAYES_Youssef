/*
 * TacheLCD.h
 *
 *  Created on: 22 nov. 2023
 *      Author: Table2
 */

#ifndef TACHELCD_TACHELCD_H_
#define TACHELCD_TACHELCD_H_


void afficherDonnees(float accx, float accy, float accz);
void afficherDonneesJoy(float VjoyVer, float VjoyHor);
void afficherDonneesLux(float sensorData);
void TacheLCD_CreateTask(void);;


#endif /* TACHELCD_TACHELCD_H_ */
