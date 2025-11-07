# simavr_board_tutor
Tutorial Board to find my way through the IRQ Jungle. It should start with basic interconnection of pins through the sim_io and sim_irq subsystem and should end in an simulation of 3 "wired or" connected pins (2 outputs -> 1 input). Each tag is described below.

## Tutorial1:
atmega328_tutor.c: Creates a squarewave trough PORTD0. Ends after 20 sleep's. The VCD File ist defined in the AVR Source Code. PIND0 is added as "MYPIN".  
tutor.c: copy of run_avr.c, firmware_filename hardcoded

## Tutorial2:
Now the VCD File is defined in tutor.c, but the Address of PIND Register (Data Space 0x29) is hardcoded.

## Tutorial3:
Same as before, but the irq for PIND0 is now found by avr_io_getirq() instead of avr_iomem_getirq(). (avr_io_getirq is defined in avr_ioport.h)

