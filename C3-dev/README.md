XMMC-cogs-demo
==============
This an example of an propgcc application architecture that has 1 XMMC process that handles human interaction and 3 independent processes running on 3 separate cogs. The code was tested on a Parallax C3.  

The xmmc process (main) is a simple command processor.  It can start, stop or display status of code running on any of three cogs (cogA, cogB, cogC).  It supports the following commands:
* startA - load cogA code into a cog and start it
* startB - load cogB code into a cog and start it
* startC - load cogC code into a cog and start it
* stopA  - stop the cog running cogA code
* stopB  - stop the cog running cogB code
* stopC  - stop the cog running cogC code
* queryA - set flag telling the cogA code to increment query counter for the cog
* queryB - set flag telling the cogB code to increment query counter for the cog
* queryC - set flag telling the cogC code to increment query counter for the cog
* status - display the status of all cogs and query counters
* exit   - terminate application

The code running on each cog is identical, however each cog has its own control block.  All communication with a cog is accomplished through its control block.  When a cog starts up it zeros the cog query counter and query flag both of which are located in shared memory (the cog's control block).  It then loops, checking its control block for a request to increment the query counter.  If it get one, it resets the flag then increments the query counter. Locks are not used to keep it simple.


