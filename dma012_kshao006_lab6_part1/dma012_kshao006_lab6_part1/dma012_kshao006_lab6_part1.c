/*
 * Daniel Ma <dma012@ucr.edu>
 * Ken Shao <kshao006@ucr.edu>
 *
 * Lab section: 023
 * Assignment: dma012_kshao006_lab6_part1
 * Exercise description: 
 *
 * I acknowledge all content created herein, excluding template or example code, 
 *  is my own original work.
 * Created: 4/22/2019 20:57:31
 *
 * Authors : Daniel Ma <dma012@ucr.edu>
 *           Ken Shao <kshao006@ucr.edu>
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

// 0 for input, 1 for output
#define INITIALISE_PORT(port, mode) { \
    DDR##port = (uint8_t)mode; PORT##port = (uint8_t)(~mode); \
}

// bit 0 for the lsb, bit 7 for the msb
#define GET_BIT(val, bit) ((val >> bit) & 0x01)
#define SET_BIT(var, bit, val) { if (val) var |= 0x01 << bit; else var &= ~(0x01 << bit); }

volatile unsigned char TimerFlag = 0;

// These variables are user vars that count the number of 
//    ISR interrupts
uint32_t _avr_timer_M = 1; // Start count from here, down to 0. (ms)
uint32_t _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
    // AVR timer/counter controller register TCCR1
    TCCR1B = 0x0B;  // TCCR1B[3] = 0: CTC mode (clear timer on cmp)
                    // TCCR1B[2:0] = 011: pre-scaler /64
                    // 8MHz clock / 64 = 125 ktps
                    // TCNT1 will count at 125 ktps
                    
    // AVR output cmp register OCR1A
    OCR1A = 125;    // Timer interrupt when TCNT1==OCR1A
                    // 1ms tick. .001s*125ktps=125t
                    // When TCNT1==125, 1ms has passed.
    
    // AVR timer interrupt mask register
    TIMSK1 = 0x02; // TIMSK1[1] = 1: OCIE1A: enables cmp match interrupt
    
    // Initialise avr counter
    TCNT1 = 0;
    
    _avr_timer_cntcurr = _avr_timer_M;
    // _avr_timer_cntcurr decrement on every tick; TimerISR called every time it hits zero
    
    // Enable global interrupts
    sei();
    
}

void TimerOff() {
    TCCR1B = 0x00;
}

void TimerISR() {
    TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
    // CPU auto-calls when TCNT1==OCR1
    _avr_timer_cntcurr--; // Count down to 0
    if (_avr_timer_cntcurr == 0) {
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
    
}

// Change the TimerISR to tick every M interrupts
void TimerSet(uint32_t M) {
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}
// Interrupts are generated every 1ms

enum state_t {S_RESET, S_0, S_1, S_2} state;

// State masheen
void tick(uint8_t button_down, uint8_t *output) {
    // Case transitions
    switch(state) {
        case S_0:
        state = S_1;
        break;
        
        case S_1:
        state = S_2;
        break;
        
        case S_2:
        state = S_0;
        break;
        
        case S_RESET:
        default:
        state = S_0;
        break;
    }
    switch(state) {
        case S_0:
        *output = 0x01;
        break;
        
        case S_1:
        *output = 0x02;
        break;
        
        case S_2:
        *output = 0x04;
        break;
        
        case S_RESET:
        *output = 0xFF;
        break;
        
    }
}


int main(void) {
    INITIALISE_PORT(B, 0xFF);
    TimerSet(1000); // Tick TimerISR every 1000ms
    TimerOn();
    
    uint8_t buf_B = 0x00;
    while (1) {
        
        tick(0x00, &buf_B);
        PORTB = buf_B;
        while (!TimerFlag); // Wait 1s
        TimerFlag = 0;
    }
}

