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
using System.IO.Ports;
using System.Threading;
using System.IO;

namespace ESP32CAM_SerialComunicator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        static bool _continue;
        static SerialPort _serialPort;
        private delegate void SetTextDeleg(string text);
        private delegate void SetImageDeleg(byte[] bytes);
        private void si_DataReceived(string data) { textBox1.Text = data.Trim(); }
        private void img_DataReceived(byte[] decodedBytes) { image1.Source = LoadImage(decodedBytes);  }

        public MainWindow()
        {
            _serialPort = new SerialPort("COM8", 9600, Parity.None, 8, StopBits.One);
            _serialPort.Handshake = Handshake.None;
            _serialPort.DataReceived += new SerialDataReceivedEventHandler(sp_DataReceived);
            _serialPort.Open();
            InitializeComponent();
        }

        void sp_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            //Thread.Sleep(500);
            string data = _serialPort.ReadLine();
            // Invokes the delegate on the UI thread, and sends the data that was received to the invoked method.
            // ---- The "si_DataReceived" method will be executed on the UI thread, which allows populating the textbox.
            this.Dispatcher.BeginInvoke(new SetTextDeleg(si_DataReceived), new object[] { data });
            //byte[] decodedBytes = Convert.FromBase64String(data);
            //this.Dispatcher.BeginInvoke(new SetImageDeleg(img_DataReceived), new object[] { decodedBytes });
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                if (!(_serialPort.IsOpen))
                    _serialPort.Open();
                _serialPort.Write("1\n");
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error opening/writing to serial port :: " + ex.Message, "Error!");
            }
        }

        private static BitmapImage LoadImage(byte[] imageData)
        {
            if (imageData == null || imageData.Length == 0) return null;
            var image = new BitmapImage();
            using (var mem = new MemoryStream(imageData))
            {
                mem.Position = 0;
                image.BeginInit();
                image.CreateOptions = BitmapCreateOptions.PreservePixelFormat;
                image.CacheOption = BitmapCacheOption.OnLoad;
                image.UriSource = null;
                image.StreamSource = mem;
                image.EndInit();
            }
            image.Freeze();
            return image;
        }

    }
}
