# Shell implementation - nicpoyia-shell

This project includes a custom shell implementation from scratch, covering basic bash functionality.

The purpose is to demonstrate how a shell can be natively developed using the C language.

## How to build and run
* cd build && make clean && make all
* ./nicpoyia-shell

## Natively implemented features:
* Analytic parsing of each input script.
* Many built-in bash commands.
* Foreground / Background process and job handling.
* Serial / Concurrent sequences of commands can be handled (using ; or &).
* File redirection [>, >>, <], also using [0, 1, 2] file descriptor numbers.
* Pipelined sequences of commands implemented using FIFO interconnected processes.
* Limit of the concurrent processes and jobs running (upto 10).
* Signals are properly handled (may be forwarded to a foreground process).
* Full environmental support (environmental variables handled properly).
