#pragma once

#ifdef BENCHMARK
    typedef struct {
        time_t s;
        uint16_t ms;
    } TimeMS;
    
    static TimeMS timer;
    
    static void startTimer() {
        time_ms( &timer.s, &timer.ms );
    }
    
    static uint32_t endTimer() {
        TimeMS timer2;
        
        time_ms( &timer2.s, &timer2.ms );
        
        return (uint32_t) timer2.s - timer.s + timer2.ms - timer.ms;
    }
#endif