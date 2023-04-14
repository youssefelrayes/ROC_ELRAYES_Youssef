#include <xc.h>
#include "UART_Protocol.h"
unsigned char UartCalculateChecksum(int msgFunction,
int msgPayloadLength, unsigned char* msgPayload)
{
    unsigned char CheckSum = 0;
    CheckSum ^= (0xFE >> 0);
    CheckSum ^= (msgFunction >> 8);
    CheckSum ^= (msgFunction >> 0);
    CheckSum ^= (msgPayloadLength >> 0);
     for (int i = 0; i < msgPayloadLength; i++)
            {
                CheckSum ^= msgPayload[i];
            }

            return CheckSum;
//Fonction prenant entree la trame et sa longueur pour calculer le checksum
}
void UartEncodeAndSendMessage(int msgFunction,
int msgPayloadLength, unsigned char* msgPayload)
{
//Fonction d?encodage et d?envoi d?un message
    unsigned char message[msgPayloadLength + 6];
    int pos = 0;
    message[pos++] = 0xFE;
    message[pos++] = (msgFunction >> 8);
    message[pos++] = (msgFunction >> 0);
    message[pos++] = (msgPayloadLength >> 8);
    message[pos++] = (msgPayloadLength >> 0);
    for (int i = 0 ; i<msgPayloadLength ; i++)
    {
         message[pos++] = msgPayload[i];
    }
    message[pos++] = UartCalculateChecksum(msgFunction, msgPayloadLength, msgPayload);

}

enum StateReception
        {
            Waiting,
            FunctionMSB,
            FunctionLSB,
            PayloadLengthMSB,
            PayloadLengthLSB,
            Payload,
            CheckSum
        };
StateReception rcvState = StateReception.Waiting;

int msgDecodedFunction = 0;
int msgDecodedPayloadLength = 0;
unsigned char msgDecodedPayload[128];
int msgDecodedPayloadIndex = 0;
void UartDecodeMessage(unsigned char c)
{
//Fonction prenant en entree un octet et servant a reconstituer les trames
 switch (rcvState)
            {
                case StateReception.Waiting:
                    if (c == 0xFE)
                        rcvState = StateReception.FunctionMSB;
                    break;
                case StateReception.FunctionMSB:
                    msgDecodedFunction = c << 8;
                    rcvState = StateReception.FunctionLSB;
                    break;
                case StateReception.FunctionLSB:
                    msgDecodedFunction += c << 0;
                    rcvState = StateReception.PayloadLengthMSB;
                    break;
                case StateReception.PayloadLengthMSB:
                    msgDecodedPayloadLength = c << 8;
                    rcvState = StateReception.PayloadLengthLSB;
                    break;
                case StateReception.PayloadLengthLSB:
                    msgDecodedPayloadLength += c << 0;
                    if (msgDecodedPayloadLength == 0)
                        rcvState = StateReception.CheckSum;
                    else
                    {
                        rcvState = StateReception.Payload;
                        msgDecodedPayloadIndex = 0;
                        msgDecodedPayload[msgDecodedPayloadLength];
                    }
                    break;
                case StateReception.Payload:
                    msgDecodedPayload[msgDecodedPayloadIndex++] = c ;
                    if (msgDecodedPayloadIndex >= msgDecodedPayloadLength)
                    {
                        rcvState = StateReception.CheckSum;
                    }
                    break;
                case StateReception.CheckSum:                    
                    unsigned char receivedChecksum = c;
                    unsigned char calculatedChecksum = UartCalculateChecksum(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);
                if (calculatedChecksum == receivedChecksum)
                    {
                      //Success, on a un message valide
                    }
                    rcvState = StateReception.Waiting;
                    break;
                default:
                    rcvState = StateReception.Waiting;
                    break;
            }
        }

enum Command
        {
            text = 0x0080,
            LED = 0x0020,
            IR = 0x0030,
            Vitesse = 0x0040
        }; 

void UartProcessDecodedMessage(int function,
int payloadLength, unsigned char* payload)
{
//Fonction appelee apres le decodage pour executer l?action
//correspondant au message recu
 switch (function)
            {
                case (int)Command.text:
                    
                    break;

                case (int)Command.LED:

                    break;

                case (int)Command.IR:

                    break;

                case (int)Command.Vitesse:

                    break;

            }
}
//*************************************************************************/
//Fonctions correspondant aux messages
//*************************************************************************/

