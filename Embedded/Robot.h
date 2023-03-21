/* 
 * File:   Robot.h
 * Author: Table2
 *
 * Created on 7 décembre 2022, 13:43
 */

#ifndef ROBOT_H
#define ROBOT_H
typedef struct robotStateBITS {
union {
struct {
unsigned char taskEnCours;
float vitesseGaucheConsigne;
float vitesseGaucheCommandeCourante;
float vitesseDroiteConsigne;
float vitesseDroiteCommandeCourante;

float distanceTelemetreDroit;
float distanceTelemetreCentre;
float distanceTelemetreGauche;

float distanceTelemetreGaucheGauche;
float distanceTelemetreDroitDroit;
}

;}
;} ROBOT_STATE_BITS;


extern volatile ROBOT_STATE_BITS robotState;
#endif /* ROBOT_H */
