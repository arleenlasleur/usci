// Forked from these repos:
// shajeebtm/Arduino-audio-spectrum-visualizer-analyzer
// victornpb/manch_decode

#include <arduinoFFT.h>
#define SAMPLES     (64)  // FFT window size, 2^n
#define HFVOL_THR   (60)  // HIGH level threshold, 0..127

double vReal[SAMPLES];
double vImag[SAMPLES];
char data_avgs; // data_avgs[32];
byte data_hi,data_lo,cmd_new,cmd_old;
bool b_mark;
arduinoFFT FFT = arduinoFFT();

namespace Manch{      // Required: send 11110100 preamble before data
    char status;
    byte bpos; // debug
    enum { PREAMBLE,  // waiting for checksum
           SYNC,      // synced signal
           CHECK,     // passed checksum
           RESYNC };  // lost sync
    void decode(bool s, uint16_t t){       // called every falling or rising edge
                                           // s - current logic state, t - time in ms since last transition
        static uint16_t T; // timing of a half bit
        static uint8_t i;  // count edges on the preamble stage
        static bool w;     // keep track of the 2T timing
        static struct{     // used to store bits until a full byte is received
            byte buffer;
            uint8_t pos;
        } bits;
        ++i;
        if(status==RESYNC){   // reset everything and wait for a preamble and resync
            i=0;
            T=0;
            w=0;
            bits.buffer=0x0;  // clear bits buffer
            bits.pos=0;
            status=PREAMBLE; 
        }
        if(status==PREAMBLE){
            if(i>3){
                if(!T){ T=t; }                   // default if T is undefined
                else if(t<T*0.75){ T=t;  }       // found smaller, use that instead
                else if(t>T*0.75 && t<=T*1.5){ } // within 1.0*T, do nothing
                else if(t>T*1.5 && t<T*2.5 ){    // first 2T is a sync signal
                    status = SYNC;
                    i=0;
                    bits.pos=4; //checksum start at the 4th bit
                    bits.buffer = bits.buffer | s<<7-bits.pos++;
                }
                else status = RESYNC;                                     // found much longer T
            }
        }else{                                                            // SYNCED, start decoding
            if(t>T*2.5 || t<T*0.75){ status=RESYNC; }                     // huge timing error
            if(t>T*1.5){ bits.buffer = bits.buffer | s<<7-bits.pos++; }   // 2T, decode
            else{                                                         // T
                if(w<1){ w=1; }                                           // first t
                else{                                                     // second t
                    bits.buffer = bits.buffer | s<<7-bits.pos++;
                    w=0;
                }        
            }
            if(status==SYNC){                    // do a CHECKSUM after sync signal
                if(bits.pos>7){                  // wait for a complete byte  
                    if(bits.buffer==0b0100){     // ok
                        status = CHECK;
                        bits.buffer=0x0;         // clear
                        bits.pos=0;
                    }
                    else status=RESYNC;          // invalid
                } 
            }
            else if(status==CHECK){              // start pushing bytes to out buffer
                if(bits.pos>7){                  // received a full byte
                    data_hi = bits.buffer >>4;
                    data_lo = bits.buffer & 0B00001111;
//                    data_ready = true;
                    bits.buffer = 0x0;           // clear
                    bits.pos = 0;
                }
            }
        }                           // decoding on SYNCED state ends
        bpos=bits.pos;
    }
}

void edgeDetect(){
    bool pinState = 0;              // current state
    static bool lastPinState = 0;   // previous state
    pinState = b_mark;
    if (pinState!=lastPinState) pinChanged(pinState);
    lastPinState = pinState;
}

void pinChanged(bool state){
    static long previousMillis = 0; 
    unsigned long currentMillis = millis();
    int duration = currentMillis - previousMillis;
    previousMillis = currentMillis;
    Manch::decode(state, duration);
}

void setup(){
    ADCSRA = 0b11100101;            // set ADC to free running mode and set pre-scalar to 32 (0xe5)
    ADMUX = 0b00000000;             // use pin A0 and external voltage reference
    delay(50);                      // wait to get reference voltage stabilized
    pinMode(5, OUTPUT);
    // Serial.begin(9600);           // debug
    digitalWrite(5,0);
}
 
void loop(){
   byte i,j,k;
   for(i=0; i<SAMPLES; i++){        // sampling
      while(!(ADCSRA & 0x10));      // wait for ADC to complete current conversion ie ADIF bit set
      ADCSRA = 0b11110101 ;         // clear ADIF bit so that ADC can do next operation (0xf5)
      int value = ADC - 512 ;       // subtract DC offset
      vReal[i] = value/8;           // copy to bins after compressing
      vImag[i] = 0;                         
    }
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    data_avgs = vReal[31];                                // last band only
    b_mark = (vReal[31]>HFVOL_THR) ? 1 : 0;
    //Serial.println(vReal[31]);          // debug
    edgeDetect();                    // runs on loop

    if(Manch::status==2 && Manch::bpos==0){
      cmd_new = (data_hi<<4) + data_lo;
      if(cmd_old==cmd_new) return;
      cmd_old = cmd_new;
      if(cmd_new==0x31) digitalWrite(5,1);
      if(cmd_new==0x32) digitalWrite(5,0);
      // Serial.println(cmd_new,HEX);      //debug
    }
} 
