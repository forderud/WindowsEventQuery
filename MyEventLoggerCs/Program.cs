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
            log.EventWritePlay("Top Gun");
            log.EventWritePause();
            log.EventWriteRestart("Top Gun");
            log.EventWriteNext("Forrest Gump");
            log.EventWritePrevious("Top Gun");
            log.EventWriteGoto("Pulp Fiction");

            log.EventWriteAuthsuccess("Alice");
            log.EventWriteAuthFailure("Bob");

            log.EventWriteConnected();
            log.EventWriteDisconnected();

            log.EventWriteCacheSizeMB(500);
            log.EventWriteThrottlingMbps(10);
        }
    }
}
