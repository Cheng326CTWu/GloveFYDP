using System;
using System.Collections.Generic;
using System.Management;
using System.Linq;
using System.IO.Ports;

using USB;

namespace USBTest
{
    class Program
    {
        static void Main(string[] args)
        {
            USB.USB glove = new USB.USB();

            glove.Open(115200, 1000, 1000);

            glove.Write("data");

            for (int i = 0; i < 10; i++)
            {
                MotionData data = glove.Read();
                Console.WriteLine(data.xAcc);
            }

            glove.Write("stop");
        }
    }
}
