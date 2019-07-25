using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Management;
using System.Linq;
using System.IO.Ports;

namespace USB
{
    [Serializable]
    public struct FeedbackStruct
    {
        // member variables for the struct
    }

    public struct MotionData
    {
        public ushort xAcc;
        public ushort yAcc;
        public ushort zAcc;

        public ushort xGyro;
        public ushort yGyro;
        public ushort zGyro;

        public ushort xMag;
        public ushort yMag;
        public ushort zMag;
    };

    class USB
    {
        public static SerialPort _serialPort;
        public SerialPort Open(int baudRate, int readTimeout, int writeTimeout)
        {
            _serialPort = new SerialPort();
            List<string> serialPortList = QuerySerialPorts();
            _serialPort.PortName = serialPortList[0];
            _serialPort.BaudRate = baudRate;
            _serialPort.ReadTimeout = readTimeout;
            _serialPort.WriteTimeout = writeTimeout;

            _serialPort.Open();

            return _serialPort;
        }

        public void Close()
        {
            if(_serialPort.IsOpen)
            {
                _serialPort.Close();
            }
        }

        public List<string> QuerySerialPorts()
        {
            // initialize a return list
            List<string> serialPortList = new List<string>();

            // query WMI (Window Management Instrumentation) for Serial Port
            using (var searcher = new ManagementObjectSearcher(@"SELECT * FROM WIN32_SerialPort"))
            {
                // Obtain a list of serial port from serial port class
                string[] portnames = SerialPort.GetPortNames();

                // Obtain a list of serial port from WMI
                var ports = searcher.Get().Cast<ManagementBaseObject>().ToList();

                // Compare the two list
                serialPortList = (from n in portnames
                            join p in ports on n equals p["DeviceID"].ToString()
                            select n).ToList();
            }

            return serialPortList;
        }

        public MotionData Read()
        {
            byte[] bytes = {};
            MotionData data;
            try
            {
                _serialPort.Read(bytes, 0, System.Runtime.InteropServices.Marshal.SizeOf(typeof(MotionData)));
            }
            catch (TimeoutException)
            {
                Console.WriteLine("Read timeout exception");
            }

            GCHandle handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            try
            {
                data = (MotionData)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(MotionData));
            }
            finally
            {
                handle.Free();
            }
            return data;
        }

        public void Write(string data)
        {
            try
            {
                _serialPort.Write(data);
            }
            catch (TimeoutException)
            {
                Console.WriteLine("Write timeout exception");
            }
        }
    }
}
