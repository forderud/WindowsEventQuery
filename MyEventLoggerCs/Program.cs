using MyEvent;

namespace MyEventLoggerCs
{
    internal class Program
    {
        static void Main(string[] args)
        {
            var log = new MyEventProvider();
            log.EventWriteInteractiveInfoEvent("Hello log!");
        }
    }
}
