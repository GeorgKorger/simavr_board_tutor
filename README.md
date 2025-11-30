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

### Tutorial6b:
We do not need the type_ Variables! We get the same information from irq->irq (type uint32_t) and irq is the first parameter of the callback function pullup_cb.  
At this step i also removed a lot of code from the original run_avr.c not needed for that example. Also the AVR_IOCTL_IOPORT_SET_EXTERNAL call is remarked, since our component should replace that function.

## Tutorial7:
To get information about the registers that did NOT raise the interrupt we use `avr_ioctl(avr, AVR_IOCTL_IOPORT_GETSTATE('D'), &state)`. We now have the informations about the PORT and DDR Register. This leads to a pin value if the pin is pulled up. First check, if "our" pin (PIND0 hardcoded in the moment) has to be checked (The PORT and DDR irq will be raised on every change of port or ddr). We check this by (value XOR old_value) AND 2⁰ `if ((value ^ old_value) & (1<<0))`
Now the Logic:
| DDR |PORT | PIN |
|:---:|:---:|:---:|
|  0  |  0  |  1  |
|  0  |  1  |  1  |
|  1  |  0  |  0  |
|  1  |  1  |  1  |
This looks like new_pin = (NOT ddr) OR port `~ddr|port` masked with a bitmask to mask out our pin: `new_pin_state = ( (1<<0) & ( ~ddr|port) ) ? 1 : 0 ;`
Next rise our PULLUP_PIN irq with the new_pin_state value.  
Since we need the base address of our PULLUP irqs, in the pullup_init we send the pu pointer as parameter to the pullup_cb. So we can get the right PULLUP_PIN irq address inside the pullup_cb. 
At least, in our pullup_init code, connect our PULLUP_PIN irq to the right pin irq of the ioport.  
The atmega328_tutor.c is also changed, so that it toggles the DDR bit to switch the pin between LOW ACTIVE and TRISTATE. So we can see the difference if we rise the PULLUP_PIN irq in the pullup_cb or remark it out.


