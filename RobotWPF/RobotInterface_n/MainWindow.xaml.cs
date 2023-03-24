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

namespace RobotInterface
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    /// 


    public partial class MainWindow : Window
    {

        ReliableSerialPort serialPort1;
        {
            InitializeComponent();
            
            serialPort1 = new ReliableSerialPort("COM3", 115200, Parity.None, 8, StopBits.One);
            serialPort1.Open();

        }



        int n = 0;
        private void buttonEnvoyer_Click(object sender, RoutedEventArgs e)
        {
            ToggleColor();
            SendMessage();
            textBoxEmission.Clear();


            void SendMessage()
            {
                string texvalue = textBoxEmission.Text;

                RichTextBox.Text += "Reçu : Message " + n + " : " + texvalue + "\n";
                n = n + 1;
            }

            void ToggleColor()
            {
                if (buttonEnvoyer.Background == Brushes.RoyalBlue)
                { buttonEnvoyer.Background = Brushes.Aqua; }
                else
                { buttonEnvoyer.Background = Brushes.RoyalBlue; }
            }
        }
    }
}
