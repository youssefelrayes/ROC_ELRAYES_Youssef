using ExtendedSerialPort;
using RobotInterface_n;
using System;
using System.Collections.Generic;
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
                textBoxReception.Text += robot.byteListReceived.Dequeue().ToString();
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

            int f = 0x0080;
            byte[] payload = new byte[20];
            int payload_length = payload.Length;
            UartEncodeAndSendMessage(f, payload_length, payload);

            textBoxEmission.Clear();
        }
        
        byte CalculateChecksum(int msgFunction,int msgPayloadLength, byte[] msgPayload)
        {

            string Sum = "FE" + msgFunction.ToString("X") + msgPayloadLength.ToString("X") + BitConverter.ToString(msgPayload);
            string Sub = Sum.Substring("FF".Length);
            byte Checksum = Convert.ToByte(Sub);
            return Checksum;
        }

        void UartEncodeAndSendMessage(int msgFunction, int msgPayloadLength, byte[] msgPayload)
        {
            byte CheckSum = CalculateChecksum(msgFunction, msgPayloadLength, msgPayload);
            serialPort1.WriteLine("FE" + msgFunction.ToString() + msgPayloadLength.ToString() + BitConverter.ToString(msgPayload) + Convert.ToString(CheckSum));
        }
        





    }

   
}

