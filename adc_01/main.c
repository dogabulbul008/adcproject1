#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_can.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"

#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/watchdog.h"
#include "driverlib/adc.h"
#include "driverlib/can.h"
#include "driverlib/eeprom.h"
#include "driverlib/timer.h"
#include "driverlib/pwm.h"
#include "inc/hw_hibernate.h"
#include "driverlib/hibernate.h"
#include "driverlib/i2c.h"
#include "inc/hw_i2c.h"
#include "driverlib/ssi.h"

uint32_t ntc_deger;
uint8_t ntcCnt;
uint32_t ntcLastValue;

bool sendData;
char uartBuffer[50];          //UART üzerinden alinan verileri depolamak için kullanilan bir karakter dizisi. 50 byte
uint8_t  uartCnt;             //UART'tan alinan verilerin sayisini tutan bir sayaç. 0-255 arasinda bir tamsayi tutar. 50 bunun icinde

uint32_t adcvalue[1];

void dwinPageControl(uint8_t PageID);

float R1 = 10000;        //kullandigimiz direnc degeri
float logR2, R2;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
uint32_t Temp_C;

void timerInterrupt(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ADCProcessorTrigger(ADC0_BASE, 0);
}

//ADC OMFYMN
void adcInterrupt( void ){

    ADCSequenceDataGet(ADC0_BASE, 0, adcvalue);
    ADCIntClear(ADC0_BASE, 0);

        if(ntcCnt++ <3)
        {
            ntc_deger += adcvalue[0];                           //Ýlk 3 ölçümde, alýnan ADC deðerleri toplanýr.
        }
        else {
            ntcCnt=0;
            ntcLastValue= ntc_deger/3;                          //ortalama deger hesaplanir
            ntc_deger=0;
            R2 = R1 * (4095.0 / (float)ntcLastValue - 1.0);     //NTC'nin direnç deðeri
            logR2 = log(R2);
            Temp_C = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));        // Steinhart-Hart denklemi
            Temp_C = Temp_C - 273.15;                                               //Kelvin cinsinden Celsius'a dönüþüm
            Temp_C = (Temp_C * 9.0) / 5.0 + 32.0;                                   //Celsius'dan Fahrenheit'a dönüþüm.
            Temp_C= (Temp_C - 32) * 5 / 9;
            Temp_C= (Temp_C - 5.0);
            sendData=1;
        }


        //if(Temp_C <=0)Temp_C=0;

}

void uartinterrupt()          // dwin ekrana tikladiginda pc ye veri gönderiyor.
{
    uint32_t ui32Status;
    uint32_t rcvData;

    ui32Status = UARTIntStatus(UART1_BASE, true); //get interrupt status , kesme durumunu kontrol eder
    UARTIntClear(UART1_BASE, ui32Status); //clear the asserted interrupts   //kesme bayragi sifirlanir
    rcvData=UARTCharGetNonBlocking(UART1_BASE);

    uartBuffer[uartCnt++]=rcvData;       //gelen veriyi uartBuffer dizisine ekler. veri geldikce sayac artar
    if( uartBuffer[ uartCnt-2]==0x01 && uartBuffer[ uartCnt-1]==0x00 && uartBuffer[ uartCnt]==0x00)       //son üç bit 100 (dwinde step length, lower limit, upper limit)
    {
        if(uartBuffer[0]==0x5A && uartBuffer[1]==0xA5 && uartBuffer[4]==0x10 && uartBuffer[5]==0x02 )     //dwinde 1. ekraný 1002 olarak ayarladik.
        {

             dwinPageControl(0x01);         //ekran gecisi kontrol
        }
        else if(uartBuffer[0]==0x5A && uartBuffer[1]==0xA5 && uartBuffer[4]==0x10 && uartBuffer[5]==0x03 )      //dwinde 2. ekraný 1003 olarak ayarladik.
        {
             dwinPageControl(0x00);
        }
        uartCnt=0;
        memset(uartBuffer, 0, sizeof(uartBuffer));
        return;
    }
    if( uartBuffer[ uartCnt-2]==79 && uartBuffer[ uartCnt-1]==75){         //son iki bit OK(ASCII) geldiginde diziyi sifirla
        memset(uartBuffer,'\0',sizeof(uartBuffer));
        uartCnt=0;
        return;

    }


}


void dwinPageControl(uint8_t PageID)        //sayfa degisim kontrolü
{

    uint8_t sendBuf[10];

    sendBuf[0]=0x5A;       //baslangic bayti
    sendBuf[1]=0xA5;       //baslangic baytý
    sendBuf[2]=0X07;       //veri uzunlugu
    sendBuf[3]=0X82;       //komut kodu
    sendBuf[4]=0X00;       //yer tutucu
    sendBuf[5]=0X84;       //ek bilgi
    sendBuf[6]=0X5A;       //kontrol bayti
    sendBuf[7]=0X01;       //kontrol bayti
    sendBuf[8]=0X00;       //yer bayti
    sendBuf[9]=PageID;

    for( int i=0; i<10 ; i++){
        if (UARTSpaceAvail(UART1_BASE)){
             while(!UARTCharPutNonBlocking( UART1_BASE, sendBuf[i] )){}
        }
    }
}


void dwinPtext(uint16_t add , uint16_t value)     //akrana data yazdirmak icin
{
    uint8_t lowadd =add ;
    uint8_t upadd  =add>>8;

    uint8_t lowbyte=value;
    uint8_t upbyte=value>>8;

    uint8_t sendBuf[8];

    sendBuf[0]=0x5A;
    sendBuf[1]=0xA5;
    sendBuf[2]=0X05;
    sendBuf[3]=0X82;
    sendBuf[4]=upadd;
    sendBuf[5]=lowadd;
    sendBuf[6]=upbyte;
    sendBuf[7]=lowbyte;

    for( int i=0; i<8 ; i++){
      while(!UARTCharPutNonBlocking( UART1_BASE, sendBuf[i] )){}
    }
}


int main(void)
{
        SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);   //portf üzerindeki pinleri kullanabiliriz
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

        SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
        TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);          //timer0 ý alir ve periyodik sekilde yapilandirir.
        TimerLoadSet(TIMER0_BASE, TIMER_A,SysCtlClockGet()-1 );   //-1 olmasi 1 sn gecmesi demek
        TimerIntRegister(TIMER0_BASE, TIMER_A,timerInterrupt );
        IntEnable(INT_TIMER0A);
        TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
        TimerEnable(TIMER0_BASE, TIMER_A);


        //UART uygulamasi

        SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);       //UART1 aktif
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);       //GPIOB aktif

        GPIOPinConfigure(GPIO_PB0_U1RX);                   //uart icin gerekli pini yapilandirir. PB0 alici RX(receive)
        GPIOPinConfigure(GPIO_PB1_U1TX);                   //uart icin gerekli pini yapilandirir. PB1 verici TX(transmit)
        GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

        UARTDisable( UART1_BASE );
        UARTClockSourceSet( UART1_BASE, UART_CLOCK_PIOSC );
        UARTConfigSetExpClk( UART1_BASE, 16000000, 115200, UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE );
        UARTIntRegister( UART1_BASE, uartinterrupt );
        UARTIntEnable( UART1_BASE, UART_INT_RX | UART_INT_RT );
        UARTEnable( UART1_BASE);


//        //ADC OMYFMN
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
        ADCSequenceDisable(ADC0_BASE, 0);
        ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
        ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
        ADCSequenceEnable(ADC0_BASE, 0);

        ADCIntClear(ADC0_BASE, 0);
        ADCIntEnable(ADC0_BASE, 0);
        ADCIntRegister(ADC0_BASE, 0, adcInterrupt);
        IntDisable(INT_ADC0SS0);
        IntEnable(INT_ADC0SS0);


        while(1)
        {
            if(sendData)
            {
                dwinPtext(0x1050,Temp_C);     //1050 dwindeki adresimiz VP(0x), kendimiz belirledik adresi,gönderdigimiz data: 4095
                sendData=0;
            }
        }

}






