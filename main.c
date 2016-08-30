#include <plib.h>
#include <stdlib.h>

#include "ledmatrix_ws2812.h"
#include "pattern.h"
#include "animationA.h"
#include "animationB.h" //BROKEN
#include "animationD.h" //NOAMRL BLINK
#include "animationF.h"
//#include "animationJ.h" //WAVE
#include "animationL.h" //HANABI
//#include "suika.h"
#include "sleep_heart.h" //SLEEP HEART
//#include "star.h" //STAR
//#include "garapiko17.h" //KAERU,ONPU,KARASU
//#include "patolamp_hasami.h" //KAMIFUBUKI,JAJAN
//#include "cho2.h" //
//#include "chulip.h" //
//#include "heart_20160810.h" //
#include "pencil.h" //
#include "leaf_20160830.h" //



// SYSCLK = 40 MHz (8MHz Crystal/ FPLLIDIV * FPLLMUL / FPLLODIV)
// PBCLK  = 40 MHz
#define SYSCLK  40000000L                      //
#define FCY          SYSCLK
#define SYSCLKdiv10MHz    (SYSCLK/10000000)    //
#define GetSystemClock()       (SYSCLK)


// Configuration Bit settings
// DEVCFG3:
#pragma config IOL1WAY  = OFF           // Peripheral Pin Select Configuration
// DEVCFG2:
#pragma config FPLLODIV = DIV_2         // PLL Output Divider
#pragma config FPLLMUL  = MUL_20        // PLL Multiplier
#pragma config FPLLIDIV = DIV_2         // PLL Input Divider
// DEVCFG1:
#pragma config FWDTEN   = OFF           // Watchdog Timer
#pragma config WDTPS    = PS1           // Watchdog Timer Postscale
#pragma config FCKSM    = CSDCMD        // Clock Switching & Fail Safe Clock Monitor
#pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor
#pragma config OSCIOFNC = OFF           // CLKO Enable
#pragma config POSCMOD  = OFF           // Primary Oscillator
#pragma config IESO     = OFF           // Internal/External Switch-over
#pragma config FSOSCEN  = OFF           // Secondary Oscillator Enable (KLO was off)
#pragma config FNOSC    = FRCPLL        // Oscillator Selection
// DEVCFG0:
#pragma config CP       = OFF           // Code Protect
#pragma config BWP      = ON            // Boot Flash Write Protect
#pragma config PWP      = OFF           // Program Flash Write Protect
#pragma config ICESEL   = ICS_PGx1      // ICE/ICD Comm Channel Select
#pragma config JTAGEN   = OFF           // JTAG Enable
#pragma config DEBUG    = OFF           // Background Debugger Enable

#define neopixel_pin      LATBbits.LATB9


unsigned char red[1024];
unsigned char blu[1024];
unsigned char grn[1024];

unsigned char myRed;
unsigned char myGrn;
unsigned char myBlu;

//PROTOTYPE
extern void setPixelColor(unsigned int pixel, unsigned char r, unsigned char g, unsigned char b);
extern void show();
extern void resetAnimation();
extern void InitUart1();
extern void setPattern(const unsigned char* ptn, unsigned char div);
extern void deletePattern();

#define TRIS_TX1                TRISBbits.TRISB3
#define TRIS_RX1                TRISBbits.TRISB2

unsigned char aCnt;
unsigned char frameCount;
unsigned char firstReset;

void InitUart1() {

    TRIS_TX1 = 0;
    TRIS_RX1 = 1;

    // Create a UART TX/RX Pin
    SYSKEY = 0xAA996655; // Write Key1 to SYSKEY
    SYSKEY = 0x556699AA; // Write Key2 to SYSKEY

    U1RXRbits.U1RXR = 0b0100; //U1RX -- RB2
    RPB3Rbits.RPB3R = 0b0001; // U1TX -- RB3

    SYSKEY = 0; // Locks the pin Configurations

    U1MODE = 0;
    U1STA = 0;

    U1MODEbits.BRGH = 0; //16xbaud
    U1BRG = (FCY / (16 * 9600)) - 1;

    U1MODEbits.RTSMD = 1; //No flow control

    U1MODEbits.PDSEL = 0; //8bit noParuty
    U1MODEbits.STSEL = 0; //Stpbit 1

    U1STAbits.UTXEN = 0; //Tx disnable
    U1STAbits.URXEN = 1; //Rx enable

    U1MODEbits.UEN = 0; //NO CTS  & NO RTS
    U1MODEbits.ON = 1;

    IFS1bits.U1RXIF = 0;
    IEC1bits.U1RXIE = 1;

    IPC8bits.U1IP = 4; //Interrupts Priority

}

unsigned char dataPos;
unsigned char myData[2];
unsigned char lastData;

void __ISR(_UART_1_VECTOR, IPL4) U1RXHandler(void) {
    unsigned char RcvData;
    IFS1bits.U1RXIF = 0;
    RcvData = getcUART1();
    if (RcvData == 255) {
        dataPos = 0;
    } else {
        if (
                RcvData == 'X' || RcvData == 'G'
                || RcvData == 'O' || RcvData == 'P'

                || RcvData == 'U' || RcvData == 'D'
                || RcvData == 'L' || RcvData == 'R'

                || RcvData == 'g' //STK-L LEFT
                || RcvData == 'h' //STK-L RIGHT
                || RcvData == 'i' //STK-L UP
                || RcvData == 'j'

                || RcvData == 'k' //STK-R LEFT
                || RcvData == 'l' //STK-R RIGHT
                || RcvData == 'm' //STK-R UP
                || RcvData == 'n' //STK-R DOWN


                || RcvData == 'a' //L1
                || RcvData == 'b' //R1
                || RcvData == 'c' //L2
                || RcvData == 'd' //R2
                ) {

            if (lastData != RcvData) {
                myData[dataPos] = RcvData;

                if (RcvData == 'k' || RcvData == 'l' || RcvData == 'h' || RcvData == 'i' || RcvData == 'g'
                        || RcvData == 'U' || RcvData == 'D' || RcvData == 'L' || RcvData == 'R'
                        || RcvData == 'X'

                        || RcvData == 'm' //STK-R UP
                        || RcvData == 'n' //STK-R DOWN
                        ) {
                    frameCount = 0;
                    aCnt = 0;
                } else {
                    deletePattern();
                }
            }
            dataPos++;

            if (dataPos >= 2)dataPos = 0;
        }
    }
    lastData = myData[0];
}

int main(void) {
    unsigned int i;

    SYSTEMConfigPerformance(SYSCLK);
    INTEnableSystemMultiVectoredInt();
    INTEnableInterrupts();

    ANSELA = 0x0000; // all digital pins
    ANSELB = 0x0000;

    TRISB = 0;
    TRISA = 0;

    InitUart1();

    resetAnimation();

    while (1) {
        frameCount++;
        switch (myData[0]) {

            default:
                myData[0] = 0;
                deletePattern();
                break;

                //UP
            case 'U':

                myData[0] = 0;
                deletePattern();
                break;

                //DOWN
                //HANABI
            case 'D':
                if (frameCount % 3 == 0) {
                    frameCount = 0;
                    aCnt++;
                    if (aCnt >= sizeof (hanabi_frame) / sizeof (unsigned char)) {
                        aCnt = sizeof (hanabi_frame) / sizeof (unsigned char) - 1;
                        myData[0] = 0;
                        deletePattern();
                    }
                }
                setPattern(hanabi[hanabi_frame[aCnt]], 2);
                break;


                //LEFT
                //NORMAL BLINK
            case 'L':
                if (frameCount % 16 == 0) {
                    frameCount = 0;
                    aCnt++;
                    if (aCnt >= sizeof (normal_frame) / sizeof (unsigned char)) {
                        aCnt = 0;
                    }
                }
                setPattern(normal[normal_frame[aCnt]], 1);
                break;

                //RIGHT 
                //BROKEN BLINK
            case 'R':
                if (frameCount % 3 == 0) {
                    frameCount = 0;

                    aCnt++;
                    if (aCnt >= sizeof (broken_frame) / sizeof (unsigned char)) {
                        aCnt = 0;
                    }
                }
                setPattern(broken[broken_frame[aCnt]], 1);
                break;


                //BATSU
                //DELETE
            case 'X':
                myData[0] = 0;
                deletePattern();
                break;

                //SANKAKU
                //BATSU
            case 'G':
                setPattern(batsu, 1);
                break;

                //MARU
            case 'O':
                //HATENA?
                setPattern(hatena, 1);
                break;

                //SIKAKU
            case 'P':
                //BREAK HEART
                setPattern(break_heart, 1);
                break;

                //STK-L LEFT
            case 'g':
                myData[0] = 0;
                deletePattern();

                break;
                //STK-L RIGHT
                //ENERGY
            case 'h':
                if (frameCount % 4 == 0) {
                    frameCount = 0;
                    aCnt++;
                    if (aCnt >= sizeof (frameA_1) / sizeof (unsigned char)) {
                        aCnt = sizeof (frameA_1) / sizeof (unsigned char) - 1;
                    }
                }
                setPattern(animationA[frameA_1[aCnt]], 2);
                break;

                //STK-L UP
                //START UP
            case 'i':
                if (frameCount % 4 == 0) {
                    frameCount = 0;
                    aCnt++;
                    if (aCnt >= sizeof (startup_frame) / sizeof (unsigned char)) {
                        aCnt = sizeof (startup_frame) / sizeof (unsigned char) - 1;
                        myData[0] = 0;
                        deletePattern();
                    }
                }
                setPattern(startup[startup_frame[aCnt]], 2);
                break;

                //STK-L DOWN
            case 'j':
                myData[0] = 0;
                deletePattern();

                break;

                //STK-R LEFT
            case 'k':
                //SLEEPHEART
                if (frameCount % 5 == 0) {
                    aCnt++;
                    if (aCnt >= sizeof (frameSleepHeart_2) / sizeof (unsigned char)) {
                        aCnt = sizeof (frameSleepHeart_2) / sizeof (unsigned char) - 1;
                    }
                }
                setPattern(sleep_heart[frameSleepHeart_2[aCnt]], 1);
                break;

                //STK-R RIGHT
            case 'l':
                //SLEEPHEART
                if (frameCount % 5 == 0) {
                    aCnt++;
                    if (aCnt >= sizeof (frameSleepHeart_1) / sizeof (unsigned char)) {
                        aCnt = sizeof (frameSleepHeart_1) / sizeof (unsigned char) - 1;
                    }
                }
                setPattern(sleep_heart[frameSleepHeart_1[aCnt]], 1);
                break;

                //STK-R UP
            case 'm':
                //Heart_1
                if (frameCount % 10 == 0) {
                    frameCount = 0;
                    aCnt++;
                    if (aCnt >= sizeof (frameHEART_20160810_1) / sizeof (unsigned char)) {
                        aCnt = sizeof (frameHEART_20160810_1) / sizeof (unsigned char) - 1;
                    }
                }
                setPattern(heart_20160810[frameHEART_20160810_1[aCnt]], 1);
                break;

                //STK-R DOWN
            case 'n':
                //Heart_2
                if (frameCount % 10 == 0) {
                    frameCount = 0;
                    aCnt++;
                    if (aCnt >= sizeof (frameHEART_20160810_2) / sizeof (unsigned char)) {
                        aCnt = sizeof (frameHEART_20160810_2) / sizeof (unsigned char) - 1;
                    }
                }
                setPattern(heart_20160810[frameHEART_20160810_2[aCnt]], 1);

                break;
        }

        show();


    }
}

void setPattern(const unsigned char* ptn, unsigned char div) {
    unsigned char x, y;

    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {

            myRed = ptn[x + y * 16] >> div;
            myGrn = ptn[256 + x + y * 16] >> div;
            myBlu = ptn[512 + x + y * 16] >> div;

            setPixelColor((x * 2) + (y * 64), myRed, myGrn, myBlu);
            setPixelColor((x * 2) + (y * 64) + 1, myRed, myGrn, myBlu);
            setPixelColor(63 + (y * 64) - (x * 2), myRed, myGrn, myBlu);
            setPixelColor(63 + (y * 64) - (x * 2 + 1), myRed, myGrn, myBlu);
        }
    }


}

void setPixelColor(unsigned int pixel, unsigned char r, unsigned char g, unsigned char b) {

    red[pixel] = r;
    grn[pixel] = g;
    blu[pixel] = b;
}

void show() {
    unsigned int i;

    INTDisableInterrupts();

    neopixel_pin = 0;
    for (i = 0; i < 5000; i++) {
        Nop();
    }

    for (i = 0; i < 1024; i++) {
        //Bit7
        if (grn[i] & 0b10000000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit6
        if (grn[i] & 0b01000000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit5
        if (grn[i] & 0b00100000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit4
        if (grn[i] & 0b00010000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit3
        if (grn[i] & 0b00001000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit2
        if (grn[i] & 0b00000100) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit1
        if (grn[i] & 0b00000010) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }
        //Bit0
        if (grn[i] & 0b00000001) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }


        //Bit7
        if (red[i] & 0b10000000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit6
        if (red[i] & 0b01000000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit5
        if (red[i] & 0b00100000) {
            neopixel_pin = 1;
            Neopixel700us();
            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit4
        if (red[i] & 0b00010000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit3
        if (red[i] & 0b00001000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit2
        if (red[i] & 0b00000100) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit1
        if (red[i] & 0b00000010) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }
        //Bit0
        if (red[i] & 0b00000001) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }






        //Bit7
        if (blu[i] & 0b10000000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit6
        if (blu[i] & 0b01000000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit5
        if (blu[i] & 0b00100000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit4
        if (blu[i] & 0b00010000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit3
        if (blu[i] & 0b00001000) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit2
        if (blu[i] & 0b00000100) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }

        //Bit1
        if (blu[i] & 0b00000010) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {
            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }
        //Bit0
        if (blu[i] & 0b00000001) {
            neopixel_pin = 1;
            Neopixel700us();

            neopixel_pin = 0;
            Neopixel600us();

        } else {

            neopixel_pin = 1;
            Neopixel350us();
            neopixel_pin = 0;
            Neopixel700us();

        }
    }

    INTEnableInterrupts();

}

void deletePattern() {
    int i;
    for (i = 0; i < 1024; i++) {

        red[i] = 0;
        grn[i] = 0;
        blu[i] = 0;
    }
}

void resetAnimation() {
    int i = 0;

    for (i = 0; i < 1024; i++) {
        red[i] = 0;
        grn[i] = 0;
        blu[i] = 0;
    }
    frameCount = 0;
    aCnt = 0;

}
