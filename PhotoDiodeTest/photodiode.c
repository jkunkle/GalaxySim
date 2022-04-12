#define F_CPU 1000000
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/io.h>

#define SHIFT_RESET PB0
#define SHIFT_COPY PD7
#define SHIFT_ENABLE PD6
#define SHIFT_DATA PD5


const uint8_t number_map[10] PROGMEM = {
    0x3f,
    0x06,
    0x5b,
    0x4f,
    0x66,
    0x6d,
    0x7d,
    0x07,
    0x7f,
    0x6f
};
  
    
uint16_t ReadADC(uint8_t ADCchannel)
{
    //select ADC channel with safety mask
    ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
    //single conversion mode
    ADCSRA |= (1<<ADSC);
    // wait until ADC conversion is complete
    while( ADCSRA & (1<<ADSC) );
    return ADC;
}

void InitADC(void)
{
    // Select Vref=internal
    ADMUX |= (1<<REFS0) | (1 << REFS1);
    //set prescaller to 8 and enable ADC
    ADCSRA |= (1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);
}



void write_to_shift( uint8_t cathode, uint8_t annode )
{
    // disable output
    PORTD = PORTD | (0x1 << SHIFT_ENABLE);
    for(int dd = 3; dd >= 0; --dd)
    {
        if( cathode & ( 0x1 << dd ) )
        {
            PORTD = PORTD | ( 0x1 << SHIFT_DATA );
        }

        PORTD |= (0x1 << SHIFT_COPY );
        PORTD &= ~(0x1 << SHIFT_COPY );
        PORTD = PORTD & ~(0x1 << SHIFT_DATA );
    }


    for(int dd = 7; dd >= 0; --dd)
    {
        if( annode & ( 0x1 << dd ) )
        {
            PORTD = PORTD | ( 0x1 << SHIFT_DATA );
        }

        PORTD |= (0x1 << SHIFT_COPY );
        PORTD &= ~(0x1 << SHIFT_COPY );
        PORTD = PORTD & ~(0x1 << SHIFT_DATA );
    }

    // one more copy is needed
    PORTD |= (0x1 << SHIFT_COPY );
    PORTD &= ~(0x1 << SHIFT_COPY );
    //// enable
    PORTD &= ~(0x1 << SHIFT_ENABLE);
}

uint8_t get_segment_pattern(int value)
{
    if( value < 0 )
    {
        return 0;
    }
    else
    {
        return pgm_read_word(&(number_map[value]));
    }

}
   
void display_digits(int num0, int num1, int num2, int num3, int time)
{

    uint8_t seg0 = get_segment_pattern(num0);
    uint8_t seg1 = get_segment_pattern(num1);
    uint8_t seg2 = get_segment_pattern(num2);
    uint8_t seg3 = get_segment_pattern(num3);
    for( int i = 0 ; i < time; i++ ) {
        write_to_shift( 0x01, seg3);
        _delay_us( 100 );
        write_to_shift( 0x02, seg1);
        _delay_us( 100 );
        write_to_shift( 0x04, seg2);
        _delay_us( 100 );
        write_to_shift( 0x08, seg0);
        _delay_us( 100 );
    }


}

void display_number(uint16_t number, int time)
{

    int thousands = number/1000;
    int hundreds = (number-thousands*1000)/100;
    int tens = (number-thousands*1000-hundreds*100)/10;
    int ones = number-thousands*1000-hundreds*100-tens*10;

    if( thousands == 0 ) {
        thousands = -1;
        if( hundreds == 0) { 
            hundreds = -1;
            if( tens == 0 ) { 
                tens = -1;
            }
        }
    }

    display_digits(thousands, hundreds, tens, ones, time);

}

void reset_shift(void)
{
    PORTB &= ~(0x01 << SHIFT_RESET);
    _delay_us(1);
    PORTB |= (0x01 << SHIFT_RESET);
}

int main(void)
{
  DDRD |= (1 << PD5 ) | (1 << PD6 ) | (1 << PD7);   // enable PORTD.2, PORTD.3, PORTD.4 pin pull up resistor
  DDRB |= ( 1 << PB0 ); 

  reset_shift();

  InitADC();

  _delay_us(100);
  while(1) {

      uint16_t value = ReadADC(0);
      display_number(value, 100);
  }
}

