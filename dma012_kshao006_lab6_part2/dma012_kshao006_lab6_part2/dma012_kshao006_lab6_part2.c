/*
 * Daniel Ma <dma012@ucr.edu>
 * Ken Shao <kshao006@ucr.edu>
 *
 * Lab section: 023
 * Assignment: Lab 6 Exercise 2
 * Exercise description: 
 *
 * I acknowledge all content created herein, excluding template or example code, 
 *  is my own original work.
 * Created: 4/23/2019 09:09:01
 *
 * dma012_kshao006_lab6_part2
 * dma012_kshao006_lab6_part2
 * Author : Daniel Ma <dma012@ucr.edu>
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

volatile uint8_t TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
    TCCR1B = 0x0B; // Scale /64
    OCR1A = 125;
    TIMSK1 = 0x02; // cmp match interrupt
    
    TCNT1 = 0;
    
    _avr_timer_cntcurr = _avr_timer_M;
    
    sei();
}

void TimerOff() {
    TCCR1B = 0x00;
}

void TimerISR() {
    TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
    _avr_timer_cntcurr--;
    if (_avr_timer_cntcurr == 0) {
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}

void TimerSet(uint32_t M) {
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

const uint32_t TICKTIME = 30;

enum state_t {S_RESET, S_LIT, S_WAIT_DOWN, S_WAIT, S_RESTART_DOWN} state;

void tick(uint8_t button_down, uint8_t *output) {
    static uint32_t cnt;
    static uint8_t ledstate;
    
    // Transitions
    
    switch(state) {
        case S_RESET:
        cnt = 0;
        state = S_LIT;
        break;
        
        case S_LIT:
        if (GET_BIT(button_down, 0) == 1) {
            cnt = 0;
            state = S_WAIT_DOWN;
        }
        break;
        
        case S_WAIT_DOWN:
        if (GET_BIT(button_down, 0) == 0) {
            state = S_WAIT;
        } else {
            state = S_WAIT_DOWN;
        }
        break;
        
        case S_WAIT:
        if (GET_BIT(button_down, 0) == 1) {
            cnt = 0;
            state = S_RESTART_DOWN;
        }
        break;
        
        case S_RESTART_DOWN:
        if (GET_BIT(button_down, 0) == 0) {
            state = S_RESET;
        }
        break;
        
        default:
        state = S_RESET;
        break;
    }
    
    switch(state) {
        case S_RESET:
        ledstate = 0x00;
        cnt = 0;
        break;
        case S_LIT:
        if (cnt <= 300 / TICKTIME) {
            ledstate = 0x01;
            cnt++;
        } else if (cnt <= 600 / TICKTIME) {
            ledstate = 0x02;
            cnt++;
        } else if (cnt <= 900 / TICKTIME) {
            ledstate = 0x04;
            cnt++;
        } else {
            ledstate = 0x01;
            cnt = 1;
        }
        
        break;
        
        case S_WAIT_DOWN:
        break;
        
        case S_WAIT:
        break;
        
        case S_RESTART_DOWN:
        break;
        
        
        
        default:
        break;
        
    }
    
    *output = ledstate;
}


int main(void) {
    INITIALISE_PORT(A, 0x00);
    INITIALISE_PORT(B, 0xFF);
    // tick evey 30ms for higher button pressing resolution
    TimerSet(TICKTIME); 
    TimerOn();
    
    uint8_t buf_A = 0x00;
    uint8_t buf_B = 0x00;
    
    while (1) {
        buf_A = (~PINA & 0x01);
        
        tick(buf_A, &buf_B);
        PORTB = buf_B;
        
        while(!TimerFlag);
        TimerFlag = 0;
    }
}

