#include "o3.h"
#include "gpio.h"
#include "systick.h"


/**************************************************************************//**
 * @brief Konverterer nummer til string 
 * Konverterer et nummer mellom 0 og 99 til string
 *****************************************************************************/
void int_to_string(char *timestamp, unsigned int offset, int i) {
    if (i > 99) {
        timestamp[offset]   = '9';
        timestamp[offset+1] = '9';
        return;
    }

    while (i > 0) {
	    if (i >= 10) {
		    i -= 10;
		    timestamp[offset]++;
		
	    } else {
		    timestamp[offset+1] = '0' + i;
		    i=0;
	    }
    }
}

/**************************************************************************//**
 * @brief Konverterer 3 tall til en timestamp-string
 * timestamp-argumentet må være et array med plass til (minst) 7 elementer.
 * Det kan deklareres i funksjonen som kaller som "char timestamp[7];"
 * Kallet blir dermed:
 * char timestamp[7];
 * time_to_string(timestamp, h, m, s);
 *****************************************************************************/
void time_to_string(char *timestamp, int h, int m, int s) {
    timestamp[0] = '0';
    timestamp[1] = '0';
    timestamp[2] = '0';
    timestamp[3] = '0';
    timestamp[4] = '0';
    timestamp[5] = '0';
    timestamp[6] = '\0';

    int_to_string(timestamp, 0, h);
    int_to_string(timestamp, 2, m);
    int_to_string(timestamp, 4, s);
}



typedef struct {
	volatile word CTRL;
	volatile word MODEL;
	volatile word MODEH;
	volatile word DOUT;
	volatile word DOUTSET;
	volatile word DOUTCLR;
	volatile word DOUTTGL;
	volatile word DIN ;
	volatile word PINLOCKN;
} gpio_port_map_t;

typedef struct {
	volatile gpio_port_map_t port[6];
	volatile word unused_space [10];
	volatile word EXTIPSELL;
	volatile word EXTIPSELH;
	volatile word EXTIRISE;
	volatile word EXTIFALL;
	volatile word IEN;
	volatile word IF;
	volatile word IFS;
	volatile word IFC;
	volatile word ROUTE;
	volatile word INSENSE;
	volatile word LOCK;
	volatile word CTRL;
	volatile word CMD;
	volatile word EM4WUEN;
	volatile word EM4WUPOL;
	volatile word EM4WUCAUSE;
} gpio_map_t;

typedef struct {
	volatile word CTRL;
	volatile word LOAD;
	volatile word VAL;
	volatile word CALIB;
} systick_t;


#define BIN_01 0b0001
#define BIN_15 0b1111
#define BIN_04 0b0100
#define BIN_07 0b0111

#define LED_PIN 2
#define PB0_PIN 9
#define PB1_PIN 10

#define SEC 0
#define MIN 1
#define H_HOURS 2
#define COUNTDOWN 3
#define ALARM 4

static int hours = 0;
static int minutes = 0;
static int seconds = 0;
static int state;
static char str[7];

volatile systick_t *SYSTICK = (systick_t *) SYSTICK_BASE;
volatile gpio_map_t *GPIO = (gpio_map_t*) GPIO_BASE;


void set_LED(int on) {
    if(on == 1)
    	GPIO->port[GPIO_PORT_E].DOUTSET = BIN_04;
    else
    	GPIO->port[GPIO_PORT_E].DOUTCLR = BIN_04;
}

void GPIO_EVEN_IRQHandler(void) {
    switch (state) {
    	case H_HOURS:
    		if(hours + minutes + seconds == 0) {
    			state = ALARM;
    			set_LED(1);
    		}
    		else
    			state++;
    		break;
    	case ALARM:
    		state = SEC;
    		set_LED(0);
    		break;
    	case COUNTDOWN:
    		break;
    	default:
    		state++;
    };
    GPIO->IFC = 1<<10;
}

void GPIO_ODD_IRQHandler(void) {
    switch (state) {
		case SEC:
			seconds++;
			break;
		case MIN:
			minutes++;
			break;
		case H_HOURS:
			hours++;
			break;
    };
    GPIO->IFC = 1<<9;
}


void SysTick_Handler() {
	if (state == COUNTDOWN) {
		if (seconds <= 0) {
			if (minutes <= 0) {
				if (hours <= 0) {
					state = ALARM;
					set_LED(1);
				}
				else{
				hours--;
				minutes = 60;
				}
			}
			else{
			minutes--;
			seconds = 60;
			}
		}
		seconds--;
	}
}

void set_4bit_flag(volatile word *w, int i, word flag) {
	*w &= ~(BIN_15 << (i * 4));
	*w |= flag << (i*4);
}

void init_gpio() {

    set_4bit_flag(&GPIO->port[GPIO_PORT_E].MODEL, LED_PIN, GPIO_MODE_OUTPUT);
	set_4bit_flag(&GPIO->port[GPIO_PORT_B].MODEH, PB0_PIN-8, GPIO_MODE_INPUT);
	set_4bit_flag(&GPIO->port[GPIO_PORT_B].MODEH, PB1_PIN-8, GPIO_MODE_INPUT);

    set_4bit_flag(&GPIO->EXTIPSELH, PB0_PIN-8, BIN_01);
	set_4bit_flag(&GPIO->EXTIPSELH, PB1_PIN-8, BIN_01);

	GPIO->EXTIFALL |= 1 << PB0_PIN;
	GPIO->EXTIFALL |= 1 << PB1_PIN;

	GPIO->IEN |= 1 << PB0_PIN;
	GPIO->IEN |= 1 << PB1_PIN;

    SYSTICK->CTRL = BIN_07;
    SYSTICK->LOAD = FREQUENCY;
}

void update_time_display() {
    time_to_string(str, hours, minutes, seconds);
    lcd_write(str);
}

int main(void) {
	init();
	    init_gpio();
		while (1){
		update_time_display();
		}
		return 0;
}

