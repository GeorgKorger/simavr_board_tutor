# simavr_board_tutor
Tutorial Board to find my way through the IRQ Jungle. It should start with basic interconnection of pins through the sim_io and sim_irq subsystem and should end in an simulation of 3 "wired or" connected pins (2 outputs -> 1 input). Each tag is described below.

## Tutorial1:
atmega328_tutor.c: Creates a squarewave trough PORTD0. Ends after 20 sleep's. The VCD File ist defined in the AVR Source Code. PIND0 is added as "MYPIN".  
tutor.c: copy of run_avr.c, firmware_filename hardcoded

## Tutorial2:
Now the VCD File is defined in tutor.c, but the Address of PIND Register (Data Space 0x29) is hardcoded.

## Tutorial3:
Same as before, but the irq for PIND0 is now found by avr_io_getirq() instead of avr_iomem_getirq(). (avr_io_getirq is defined in avr_ioport.h)

## Tutorial4:
Instead of PORTD0 i will alter DDRD0. If the PinD0 is pulled high, it should go high and low again.  
But - without any simulation of an external PullUp nothing happens on PIND0

## Tutorial5:
Connecting an external PullUp (as seen in sim_elf.h) ... now PIND is alternating again!

## Tutorial6:
Try to create an external Component ("Wired OR with pullup") which can be connected to the STATE Interrupts of IO Ports

### Tutorial6a:
Create base of pullup Component. Create Names for the Interrupts to avoid Runtime Warnings. The type_ Variables are used to distinguish which interupt calls the callback. They are used as *param in the avr_irq_register_notify() call. We allocate IRQ_PULLUP_COUNT irq's. The first and the second will notify pullup_cb with different type_ params. Then we get the ioctl for IO-Port D and the base irq for this port. At least our two interrupts are connected to IOPORT_IRQ_DIRECTION_ALL and IOPORT_IRQ_REG_PORT irq of the IO-Port.
The pullup_cb callback routine just prints the type and the value. type is 0 for DDR callbacks and 1 for PORT callbacks.



