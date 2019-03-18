#include <avr/io.h>
#include <util/delay.h>

#define NPN_R1     PD2
#define NPN_R2     PD3
#define NPN_T1     PD4
#define NPN_T2     PD5
#define NPN_DDR    DDRD
#define NPN_PORT   PORTD

#define T1         PC0
#define T2         PC1

#define PNP_1      PB0
#define PNP_2      PB1
#define PNP_DDR    DDRB
#define PNP_PORT   PORTB

#define N_ROWS     2
#define N_COLS     2
#define DELAY_TIME 100     /* time in ms to heat up each element */

#define ADC_ERROR  3       // change later
#define ADC_R1_C1  245     // ideal ADC values at each point row and col
#define ADC_R1_C2  378     // 10 bit ADC, so max is 1023
#define ADC_R2_C1  24
#define ADC_R2_C2  863




/* ==== GLOBAL VARIABLES ====  */

uint16_t adc_set[N_ROWS][N_COLS] = { {ADC_R1_C1, ADC_R1_C2}, {ADC_R2_C1, ADC_R2_C2} }; /* ideal adc values */
uint8_t channel;       /* the current ADC channel */



/* ======= FUNCTIONS ======= */

void setup(void){
  /* define pin modes and ADC parameters */
  NPN_DDR |= (1<<NPN_R1) | (1<<NPN_R2) | (1<<NPN_T1) | (1<<NPN_T2); /* set NPN transistor pins to output mode */
  PNP_DDR |= (1<<PNP_1) | (1<<PNP_2);      /* set PNP transistor pins to output mode */
  ADMUX |= (1 << REFS0);                   /* use reference voltage on AVCC */
  ADCSRA |= (1 << ADPS1) | (1 << ADPS0);   /* ADC clock prescaler /8 */
  ADCSRA |= (1 << ADEN);                   /* enable ADC */
}


void turn_on_column(int col){
  /* turn on PNP for column, turning all others off */
  switch (col){ 
  case 0:
    PNP_PORT = (1 << PNP_1);
    break;
  case 1:
    PNP_PORT = (1 << PNP_2);
    break;
  }
}
  

void turn_on_sensor(int row){
  /* turn on sensor NPN, turning all others off, and define current ADC channel */
  switch (row){ 
    case 0:
      NPN_PORT = (1 << NPN_T1);
      channel = T1;
      break;
    case 1:
      NPN_PORT = (1 << NPN_T2);
      channel = T2;
      break;
    }    
}

void turn_on_heating_element(int row){
  /* turn on heating element NPN, turning all others off */
  switch (row){
    case 0:
      NPN_PORT = (1 << NPN_R1);
      break;
    case 1:
      NPN_PORT = (1 << NPN_R2);
      break;
    }
}

uint16_t read_ADC(void) {
  /* get ADC value from sensor */
  ADMUX = (0xf0 & ADMUX) | channel;       /* set ADC channel */
  ADCSRA |= (1 << ADSC);                  /* begin conversion */
  loop_until_bit_is_clear(ADCSRA, ADSC);  /* wait for conversion */
  return (ADC);                           /* return ADC value */
}


void check_temp(int row, int col){
  /* compare temp reading with set value and heat if it's too low */
  uint16_t adc_ideal = adc_set[row][col]; /* ADC set value at point */
  uint16_t adc_val = read_ADC();          /* get actual ADC reading at point */
  if( (adc_val +  ADC_ERROR) <  adc_ideal){
    turn_on_heating_element(row);         /* heat element if temp is too low */
  }     
}



void mux_loop(void){
  /* loop through each element checking its temperature and heating if it's too low */
  for (int col=0; col < N_COLS; col++){
    turn_on_column(col);                  /* turn on PNP for column */   
    for (int row=0; row < N_ROWS; row++){
      turn_on_sensor(row);                /* turn on sensor NPN */
      check_temp(row, col);               /* check temp and turn on heating element */
      _delay_ms(DELAY_TIME);              /* give time to heat up ??  */
    }
  }
}
    

/* ===== MAIN FUNCTION ===== */

int main(void){
  setup();
  while (1) {
    mux_loop();
  }
  return 0; /* This line is never reached */
}
