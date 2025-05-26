using MyEvent;

namespace MyEventLoggerCs
{
    internal class Program
    {
        static void Main(string[] args)
        {
            // create logging object
            var log = new MyMoviePlayer();

            // generate log entries
            log.EventWriteInteractiveInfoEvent("Hello log!");
        }
    }
}
