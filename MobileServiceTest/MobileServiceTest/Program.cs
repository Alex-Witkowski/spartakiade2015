using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.WindowsAzure.MobileServices;

namespace MobileServiceTest
{
    class Program
    {
        static void Main(string[] args)
        {
            var app = new App();
            app.DoWork().Wait();
        }
    }

    public class App
    {
        const string applicationUrl = "http://spartakiade2015.azure-mobile.net";
        const string applicationKey = "eCwuHEJspgPeuOSiEDFapozuMfKMTc29";
        static IMobileServiceClient client = new MobileServiceClient(applicationUrl, applicationKey);
        private static IMobileServiceTable<Telemetry> telemetryTable;
        private static IMobileServiceTable<Messages> messageTable;

        public async Task DoWork()
        {
            telemetryTable = client.GetTable<Telemetry>();
            var entries = await telemetryTable.Select(e => e.value).ToListAsync();
            foreach (var entry in entries)
            {
                Console.WriteLine(entry);
            }

            var messageTable = client.GetTable<Messages>();
            while (true)
            {
                Console.WriteLine("Type Message");
                var input = Console.ReadLine();
                var msg = input;
                Console.WriteLine("Type R value");
                input = Console.ReadLine();
                var r = Int32.Parse(input ?? "0");
                Console.WriteLine("Type G value");
                input = Console.ReadLine();
                var g = Int32.Parse(input ?? "0");
                Console.WriteLine("Type B value");
                input = Console.ReadLine();
                var b = Int32.Parse(input ?? "0");

                var message = new Messages { R = r, G = g, B = b, Message = msg };
                await messageTable.InsertAsync(message);
            }
        }
    }

    public class Telemetry
    {
        public string id { get; set; }
        public float value { get; set; }
    }

    public class Messages
    {
        public string Id { get; set; }
        public string Message { get; set; }
        public int R { get; set; }
        public int G { get; set; }
        public int B { get; set; }
    }
}
