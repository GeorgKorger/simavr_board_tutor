/*
	tutor.c is derived from run_avr.c	(Copyright 2008, 2010 Michel Pollet <buserror@gmail.com>)

 */

#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <signal.h>
#include "sim_avr.h"
#include "sim_elf.h"
#include "sim_core.h"
#include "sim_gdb.h"
#include "sim_hex.h"
#include "sim_vcd_file.h"

#include "sim_core_decl.h"

#include "avr_ioport.h"

static avr_t * avr = NULL;

/************ PULLUP CODE START ******************/

enum {
	IRQ_PULLUP_DDR = 0,
	IRQ_PULLUP_PORT,
	IRQ_PULLUP_PIN,
	IRQ_PULLUP_COUNT
};

typedef struct pullup_t {
	avr_irq_t * irq;
	struct avr_t * avr;
} pullup_t;


static void pullup_cb(avr_irq_t *irq, uint32_t value, void *param) {
  
  uint32_t type = irq->irq;
  uint32_t old_value = irq->value;
  
  switch(type) {
    case IRQ_PULLUP_DDR:
      printf("Type: DDR, Old Value: %X, Value: %X\n", old_value, value);
      break;
    case IRQ_PULLUP_PORT:
      printf("Type: PORT, Old Value: %X, Value: %X\n", old_value, value);
      break;
    default:
      return;
/*    uint8_t oddr  = value;
    uint8_t oport = state.port;
    if ((oddr ^ state.ddr) & (1<<2)) { //betrifft PIND2 */
  }
}

void pullup_init( avr_t *avr, pullup_t * pu ) {
  avr_irq_t  *base_irq;
  static const char *irq_names[IRQ_PULLUP_COUNT] = {
    [IRQ_PULLUP_DDR]  = "PullUp_DDR",
    [IRQ_PULLUP_PORT] = "PullUp_PORT",
    [IRQ_PULLUP_PIN]  = "PullUp_PIN",
  };
  
  pu->avr = avr;
  pu->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_PULLUP_COUNT, irq_names);
  avr_irq_register_notify(pu->irq+IRQ_PULLUP_DDR, pullup_cb, NULL);
  avr_irq_register_notify(pu->irq+IRQ_PULLUP_PORT, pullup_cb, NULL);

  uint32_t   ioctl = (uint32_t)AVR_IOCTL_IOPORT_GETIRQ('D');
  base_irq = avr_io_getirq(avr, ioctl, 0);

  avr_connect_irq(base_irq + IOPORT_IRQ_DIRECTION_ALL ,pu->irq+IRQ_PULLUP_DDR) ;
  avr_connect_irq(base_irq + IOPORT_IRQ_REG_PORT ,pu->irq+IRQ_PULLUP_PORT) ;
}

/************ PULLUP CODE END ******************/



static void
sig_int(
		int sign)
{
	printf("signal caught, simavr terminating\n");
	if (avr)
		avr_terminate(avr);
	exit(0);
}



int
main(
		int argc,
		char *argv[])
{
#ifdef CONFIG_SIMAVR_TRACE
	int trace = 0;
#endif //CONFIG_SIMAVR_TRACE
	elf_firmware_t f = {{0}};
	uint32_t f_cpu = 0;
	int gdb = 0;
	int log = LOG_ERROR;
	int port = 1234;
	char name[24] = "";
	uint32_t loadBase = AVR_SEGMENT_OFFSET_FLASH;
	int trace_vectors[8] = {0};
	int trace_vectors_count = 0;
	const char *vcd_input = NULL;

  char firmware_file[] = "atmega328_tutor.axf";
  
	sim_setup_firmware(firmware_file, loadBase, &f, argv[0]);

	// Frequency and MCU type were set early so they can be checked when
	// loading a hex file. Set them again because they can also be set
 	// in an ELF firmware file.

	if (strlen(name))
		strcpy(f.mmcu, name);
	if (f_cpu)
		f.frequency = f_cpu;

	avr = avr_make_mcu_by_name(f.mmcu);
	if (!avr) {
		fprintf(stderr, "%s: AVR '%s' not known\n", argv[0], f.mmcu);
		exit(1);
	}
	avr_init(avr);
	avr->log = (log > LOG_TRACE ? LOG_TRACE : log);
#ifdef CONFIG_SIMAVR_TRACE
	avr->trace = trace;
#endif //CONFIG_SIMAVR_TRACE

	avr_load_firmware(avr, &f);

  /* Connect External PullUp to PinD0 (from sim_elf.c)
	avr_ioport_external_t e = {
			.name = 'D',
			.mask = 1,  //Pin 0
			.value = 1, //PullUP
		};
		avr_ioctl(avr, AVR_IOCTL_IOPORT_SET_EXTERNAL(e.name), &e);
	*/

  // Create VCD File
	avr->vcd = malloc(sizeof(*avr->vcd));
	memset(avr->vcd, 0, sizeof(*avr->vcd));
	avr_vcd_init(avr, "gtkwave_trace.vcd", avr->vcd, 1000);
	
	// Add Signal for PIND0, Name PIND0
  avr_irq_t * bit = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), IOPORT_IRQ_PIN0);
  avr_vcd_add_signal(avr->vcd, bit, 1, "PIN0");
  avr_vcd_start(avr->vcd);

	if (f.flashbase) {
		printf("Attempted to load a bootloader at %04x\n", f.flashbase);
		avr->pc = f.flashbase;
	}
	for (int ti = 0; ti < trace_vectors_count; ti++) {
		for (int vi = 0; vi < avr->interrupts.vector_count; vi++)
			if (avr->interrupts.vector[vi]->vector == trace_vectors[ti])
				avr->interrupts.vector[vi]->trace = 1;
	}
	if (vcd_input) {
		static avr_vcd_t input;
		if (avr_vcd_init_input(avr, vcd_input, &input)) {
			fprintf(stderr, "%s: Warning: VCD input file %s failed\n", argv[0], vcd_input);
		}
	}

	// even if not setup at startup, activate gdb if crashing
	avr->gdb_port = port;
	if (gdb) {
		avr->state = cpu_Stopped;
		avr_gdb_init(avr);
	}

	signal(SIGINT, sig_int);
	signal(SIGTERM, sig_int);

  pullup_t pullup;
  pullup_init(avr, &pullup);
  
	for (;;) {
		int state = avr_run(avr);
		if (state == cpu_Done || state == cpu_Crashed)
			break;
	}

	avr_terminate(avr);
}
