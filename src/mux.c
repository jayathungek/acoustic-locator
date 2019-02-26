#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include <wiringPi.h>
#include <pthread.h>

#define SIG 7
#define MUXIN 24
#define MUXSE 25
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

struct mux{
    int input_pin;
    int sel_pin;
    int out0_pin;
    int out1_pin;
    struct mux_args internal;
};

struct mux m;

static void setupMux(int in_pin, int sel_pin, int out0_pin, int out1_pin){
    struct mux_args in = {.input = 0, .select = 0};
    m = (struct mux){
        .input_pin  = in_pin,
        .sel_pin = sel_pin,
        .out0_pin = out0_pin,
        .out1_pin = out1_pin,
        .internal = in  
    };
}

static void* readMuxIn(void* void_args){
    int input_pin = m.input_pin;
    int sel_pin = m.sel_pin;
    pinMode(input_pin, INPUT);
    pinMode(sel_pin, INPUT);
    
    while(1){
        m.internal.input = digitalRead(input_pin);
        m.internal.select = digitalRead(sel_pin);
        //printf("%d\r", m.internal.select);
    }
}

static void* writeMuxOut(void* void_muxargs){
    int out;
    while(1){
        out = m.internal.select ? m.out0_pin : m.out1_pin;
        pinMode(out, OUTPUT);
        digitalWrite(out, m.internal.input);
    }
}

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





static void* inputSig(void* void_del){
    int del =  *((int*) void_del);
    while(1){
        m.internal.input = 0;
        delay(del);
        m.internal.input = 1;
        delay(del);
    }
}

static void* swStatus(void* void_sw){
    int sw = *((int*) void_sw);
    pinMode(sw, INPUT);
    printf("Switch pin: %d\n", sw);
    while(1){
        //m.internal.select = digitalRead(sw);
        printf("Switch is: %d\r", digitalRead(sw));
    }
}



int main(){
    struct signal_args sig_args ={.pin = SIG, .del = 50};
    setupMux(MUXIN, MUXSE, MOUT0, MOUT1);
    
    if (wiringPiSetup() == -1) exit(1);
    
    pthread_t inputSig_thread; 
    pthread_t mux_write_thread;
    pthread_t mux_read_thread; 
     
    pthread_create(&inputSig_thread, NULL, newSig, (void*) &sig_args);
    pthread_create(&mux_write_thread, NULL, writeMuxOut, NULL);
    pthread_create(&mux_read_thread, NULL, readMuxIn, NULL);
    
    pthread_join(inputSig_thread, NULL); 
    pthread_join(mux_write_thread, NULL);
    pthread_join(mux_read_thread, NULL); 
}
/*
pinMode(SIG, GPIO_CLOCK);
gpioClockSet(SIG, 1);

struct signal_args sa_red = {.pin = RED, .del = 100};
pthread_t signal1;
pthread_create(&signal1, NULL, newSig, (void*) &sa_red);
pthread_join(signal1, NULL);

*/
