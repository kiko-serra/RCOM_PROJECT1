Protocolo de Ligação de Dados (1st Lab Work)

Nota: 18.0
---------------------------------------------------

Instructions to Run the Project


1. Compile the application and the virtual cable program using the provided Makefile.
2. Run the virtual cable program (either by running the executable manually or using the Makefile target):
	$ ./bin/cable_app
	$ sudo make run_cable

3. Test the protocol without cable disconnections and noise
	3.1 Run the receiver (either by running the executable manually or using the Makefile target):
		$ ./bin/main /dev/ttyS11 rx received-penguin.gif
		$ make run_tx

	3.2 Run the transmitter (either by running the executable manually or using the Makefile target):
		$ ./bin/main /dev/ttyS10 tx penguin.gif
		$ make run_rx

	3.3 Check if the file received matches the file sent, using the diff Linux command or using the Makefile target:
		$ diff -s penguin.gif received-penguin.gif
		$ make check_files


---------------------------------------------------

Trabalho realizado por:

Francisco Pimentel Serra - up202007723
João Paulo Moreira Araújo - up202004293
