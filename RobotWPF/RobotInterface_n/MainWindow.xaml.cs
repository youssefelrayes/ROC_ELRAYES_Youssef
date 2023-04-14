using ExtendedSerialPort;
using RobotInterface_n;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace RobotInterface
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    /// 


    public partial class MainWindow : Window
    {
        ReliableSerialPort serialPort1;
        DispatcherTimer timerAffichage;
        Robot robot = new Robot();

        string receivedText;


        public MainWindow()
        {
            InitializeComponent();

            serialPort1 = new ReliableSerialPort("COM8", 115200, System.IO.Ports.Parity.None, 8, System.IO.Ports.StopBits.One);
            serialPort1.OnDataReceivedEvent += SerialPort1_OnDataReceivedEvent;
            serialPort1.Open();

            timerAffichage = new DispatcherTimer();
            timerAffichage.Interval = new TimeSpan(0, 0, 0, 0, 100);
            timerAffichage.Tick += TimerAffichage_Tick;
            timerAffichage.Start();
        }



   
        private void SerialPort1_OnDataReceivedEvent(object sender, DataReceivedArgs e)
        {

            receivedText += Encoding.UTF8.GetString(e.Data, 0, e.Data.Length);
            for(int i = 0 ; i<e.Data.Length ; i++ )
            {
                byte b = e.Data[i];   
                robot.byteListReceived.Enqueue(b);
            }    
        }

        private void TimerAffichage_Tick(object sender, EventArgs e)
        {
            /*
            if(receivedText != "")
            { 
                textBoxReception.Text += receivedText;
                receivedText = "";

             }  
            */

            while(robot.byteListReceived.Count > 0)
            {
                DecodeMessage(robot.byteListReceived.Dequeue());
                //textBoxReception.Text += robot.byteListReceived.Dequeue().ToString();
            }
        }


        void SendMessage()
        {
            serialPort1.WriteLine(textBoxEmission.Text);
            textBoxEmission.Clear();
        }

        void ToggleColor()
        {
            if (buttonEnvoyer.Background == Brushes.RoyalBlue)
            {
                buttonEnvoyer.Background = Brushes.Aqua; 
            }
            else
            { 
                buttonEnvoyer.Background = Brushes.RoyalBlue;
            }
        }

        private void buttonEnvoyer_Click(object sender, RoutedEventArgs e)
        {
            ToggleColor();
            SendMessage();
        }


        byte[] byteList = new byte[20];

        private void Test_Click(object sender, RoutedEventArgs e)
        {
            /*
            for (int i = 0; i < 20; i++)
            {
                byteList[i] = (byte)(2 * i);
            }
            */

            int f = 0x0040;
            //byte[] payload = Encoding.ASCII.GetBytes("Bonjour");

            byte[] payload = { 10, 10 };
            int payload_length = payload.Length;
            UartEncodeAndSendMessage(f, payload_length, payload);     
            
            textBoxEmission.Clear();

            ProcessDecodedMessage(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);
        }
        
        byte CalculateChecksum(int msgFunction,int msgPayloadLength, byte[] msgPayload)
        {
            byte CheckSum = 0;
            CheckSum ^= (byte) (0xFE >> 0);
            CheckSum ^= (byte)(msgFunction >> 8);
            CheckSum ^= (byte)(msgFunction >> 0);
            CheckSum ^= (byte)(msgPayloadLength >> 0);
            for (int i = 0; i < msgPayloadLength; i++)
            {
                CheckSum ^= msgPayload[i];
            }
            //textBoxReception.Text = CheckSum.ToString("X");
            return CheckSum;
        }
        
        void UartEncodeAndSendMessage(int msgFunction, int msgPayloadLength, byte[] msgPayload)
        {
            var message = new byte[msgPayloadLength + 6];
            int pos = 0;
            message[pos++] = 0xFE;
            message[pos++] = (byte)(msgFunction >> 8);
            message[pos++] = (byte)(msgFunction >> 0);
            message[pos++] = (byte)(msgPayloadLength >> 8);
            message[pos++] = (byte)(msgPayloadLength >> 0);
            for (int i = 0 ; i<msgPayloadLength ; i++)
            {
                message[pos++] = msgPayload[i];
            }
            message[pos++] = CalculateChecksum(msgFunction, msgPayloadLength, msgPayload);

            serialPort1.Write(message, 0, message.Length);

        }
        
        public enum StateReception
        {
            Waiting,
            FunctionMSB,
            FunctionLSB,
            PayloadLengthMSB,
            PayloadLengthLSB,
            Payload,
            CheckSum
        }

        StateReception rcvState = StateReception.Waiting;

        int msgDecodedFunction = 0;
        int msgDecodedPayloadLength = 0;
        byte[] msgDecodedPayload;
        int msgDecodedPayloadIndex = 0;

        private void DecodeMessage(byte c)
        {
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
                        msgDecodedPayload = new byte[msgDecodedPayloadLength];
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
                    byte receivedChecksum = c;
                    byte calculatedChecksum = CalculateChecksum(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);
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

        public enum Command
        {
            text = 0x0080,
            LED = 0x0020,
            IR = 0x0030,
            Vitesse = 0x0040
        } 


        void ProcessDecodedMessage(int msgFunction, int msgPayloadLength, byte[] msgPayload)
        {
            switch (msgFunction)
            {
                case (int)Command.text:

                    for(int i = 0; i<msgPayloadLength; i++)
                    {
                        textBoxReception.Text += "0x" + msgPayload[i].ToString("X") + " ";
                    }
                    textBoxReception.Text += Environment.NewLine;
                    textBoxReception.Text += "Texte reçu : " + Encoding.ASCII.GetString(msgPayload);
                    textBoxReception.Text += Environment.NewLine;
                    break;

                case (int)Command.LED:

                    for (int i = 0; i < msgPayloadLength; i++)
                    {
                        textBoxReception.Text += "0x" + msgPayload[i].ToString("X") + " ";
                    }
                    textBoxReception.Text += Environment.NewLine;
                    textBoxReception.Text += "LED Number : " + msgPayload[0].ToString() + " ";
                    if (msgPayload[1] == 1)
                    {
                        textBoxReception.Text += "( Allumée )";
                        checkBoxLed1.IsChecked = true;
                    }
                    else
                        textBoxReception.Text += "( éteinte )";
                    textBoxReception.Text += Environment.NewLine;
                    break;

                case (int)Command.IR:

                    for (int i = 0; i < msgPayloadLength; i++)
                    {
                        textBoxReception.Text += "0x" + msgPayload[i].ToString("X") + " ";
                    }
                    textBoxReception.Text += Environment.NewLine;
                    textBoxReception.Text += "Télémètre Gauche : " + msgPayload[0].ToString() + Environment.NewLine;
                    textBoxReception.Text += "Télémètre Centre : " + msgPayload[1].ToString() +  Environment.NewLine;
                    textBoxReception.Text += "Télémètre Droit : " + msgPayload[2].ToString() + Environment.NewLine;
                    textBoxReception.Text += Environment.NewLine;

                    IR_Gauche.Content = msgPayload[0];
                    IR_Centre.Content = msgPayload[1];
                    IR_Droit.Content = msgPayload[2];
                    break;

                case (int)Command.Vitesse:

                    for (int i = 0; i < msgPayloadLength; i++)
                    {
                        textBoxReception.Text += "0x" + msgPayload[i].ToString("X") + " ";
                    }
                    textBoxReception.Text += Environment.NewLine;
                    textBoxReception.Text += "Moteur Gauche : " + msgPayload[0].ToString() + Environment.NewLine;
                    textBoxReception.Text += "Moteur Droit : " + msgPayload[1].ToString() + Environment.NewLine;
                    textBoxReception.Text += Environment.NewLine;


                    Moteur_Gauche.Content = msgPayload[0];
                    Moteur_Droit.Content = msgPayload[1];

                    break;

            }




        }





    }

   
}

