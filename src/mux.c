#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include <wiringPi.h>
#include <pthread.h>

#define SW 22
#define RED 23
#define YEL 24
#define WHI 25
#define MOUT0 4
#define MOUT1 5

struct signal_args{
    int pin;
    int del;
};

struct mux_args{
    int input;
    int select;
};

struct mux_args m = {.input = 0, .select = 0};


static void* newSig(void* sigarg){
    struct signal_args *args = sigarg;
    int pin = args->pin;
    int del = args->del;
    pinMode(pin, OUTPUT);
    
    while(1){
        digitalWrite(pin, 1);
        delay(del);
        digitalWrite(pin, 0);
        delay(del);
    }
}



static void* mux(void* void_muxargs){
    int out;
    while(1){
        out = m.select ? MOUT0 : MOUT1;
        pinMode(out, OUTPUT);
        digitalWrite(out, m.input);
    } 
    
    
}

static void* inputSig(void* void_del){
    int del =  *((int*) void_del);
    while(1){
        m.input = 0;
        delay(del);
        m.input = 1;
        delay(del);
    }
}

static void* swStatus(void* void_sw){
    int sw = *((int*) void_sw);
    pinMode(sw, INPUT);
    
    while(1){
        m.select = digitalRead(sw);
        //printf("m.select is: %d\r", m.select);
    }
}



int main(){
    
    int sw = SW;
    int clk_delay = 100;
    
    if (wiringPiSetup() == -1) exit(1);
    
    pthread_t inputSig_thread;
    pthread_t switch_thread;
    pthread_t mux_thread;
    
    pthread_create(&switch_thread, NULL, swStatus, (void*) &sw);
    pthread_create(&inputSig_thread, NULL, inputSig, (void*) &clk_delay);
    pthread_create(&mux_thread, NULL, mux, NULL);
    
    pthread_join(inputSig_thread, NULL);
    pthread_join(switch_thread, NULL);    
    pthread_join(mux_thread, NULL);
}
/*
pinMode(SIG, GPIO_CLOCK);
gpioClockSet(SIG, 1);

struct signal_args sa_red = {.pin = RED, .del = 100};
pthread_t signal1;
pthread_create(&signal1, NULL, newSig, (void*) &sa_red);
pthread_join(signal1, NULL);

*/
