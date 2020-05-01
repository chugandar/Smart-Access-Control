#include<lpc214x.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "LCD.H"
#define IR1 (IO1PIN & 1<<24)
#define IR2 (IO1PIN & 1<<27)
#define PLOCK 0x00000400
#define LED_ON (IO0CLR=1U<<31)
#define LED_OFF (IO0SET=1U<<31)
#define COL0 (IO1PIN & 1<<19)
#define COL1 (IO1PIN & 1<<18)
#define COL2 (IO1PIN & 1<<17)
#define COL3 (IO1PIN & 1<<16)
#define uint16_t unsigned int
	
typedef struct
{
	unsigned char sec;
  unsigned char min;
  unsigned char hour;
  unsigned char weekDay;
  unsigned char date;
  unsigned char month;
  unsigned int year;  
}rtc_t;

void SystemInit(void);
void delay_ms(unsigned int j);
void uart_init(void);
char* board();
void buzzer(unsigned int val);
void RTC_Init(void);
void stepper_motor(int ch);
void timer1_Init(void);// generates interrupt every 1sec
void delay(int cnt);
void RTC_SetDateTime(rtc_t *rtc);
void RTC_GetDateTime(rtc_t *rtc);
unsigned int adc(int no,int ch);// to read LDR(AD1.3),LM35(AD1.5)
void serialPrint(unsigned val);//print int on serialport
void serialPrintStr(char * buf);//print string on serialport
void seven_seg(char*);
unsigned char getAlphaCode(unsigned char alphachar);
void runDCMotor(unsigned int direction,unsigned int speed);
//global variables
rtc_t rtc; // declare a variable to store date,time
#define RELAY_ON (IO0SET = 1 << 7)
#define RELAY_OFF (IO0CLR = 1 << 7)
unsigned  int x=0,count=0;
// ISR Routine to blink LED D7 to indicate project working
__irq   void  Timer1_ISR(void)
 {
	x = ~x;//x ^ 1;
  if (x)   
    IO0SET  =  1u << 31;   //P0.31  =  1
  else   
    IO0CLR =   1u <<31;   // P0.31  = 0	 
	T1IR  =  0x01; // clear match0 interrupt, and get ready for the next interrupt
  VICVectAddr = 0x00000000 ; //End of interrupt 
 }
unsigned char lookup_table[4][4]={{'0','1','2','3'},{'4','5','6','7'},{'8','9','a','b'},{'c','d','e','f'}};
unsigned char rowsel=0,colsel=0;
int main(){
    unsigned char msg[100];
	  unsigned int light_i,temp;
	char *a,*b="abcd",*ms;
	int num;
  
 // initialize the peripherals & the board GPIO
	
    //Board_Init();
    SystemInit();
    uart_init();
	  RTC_Init();
	  timer1_Init(); 
	  LCD_Reset();
		LCD_Init();

	// set date & time to 7thApril 2020,10:00:00am 
	
		rtc.hour = 17;rtc.min =  00;rtc.sec =  00;//10:00:00am
    rtc.date = 14;rtc.month = 04;rtc.year = 2020;//07th April 2020
    RTC_SetDateTime(&rtc);  // comment this line after first use
	 while(1){
	       RTC_GetDateTime(&rtc);//get current date & time stamp
			sprintf((char *)msg,"time:%2d:%2d:%2d  Date:%2d/%2d/%2d \x0d\xa",(uint16_t)rtc.hour,(uint16_t)rtc.min,(uint16_t)rtc.sec,(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year);
			// use the time stored in the variable rtc for date & time stamping
			serialPrintStr((char*)msg);
			LCD_CmdWrite(0x80); LCD_DisplayString((char*)msg);
			delay(2000);
			while(1){
			a=board();
			num=strcmp(a,b);
			if(num==0){
			stepper_motor(1);
				delay(5);
				buzzer(100);
				sprintf((char*)msg,"Welcome");
				serialPrintStr((char*)msg);
				LCD_CmdWrite(0x80); LCD_DisplayString((char*)msg);
				RELAY_ON;
				if(IR1==0){
					count++;
					sprintf((char*)msg,"Count = %d \n",count);
					serialPrintStr((char*)msg);
			RTC_GetDateTime(&rtc);//get current date & time stamp
			sprintf((char *)msg,"time:%2d:%2d:%2d  Date:%2d/%2d/%2d \x0d\xa",(uint16_t)rtc.hour,(uint16_t)rtc.min,(uint16_t)rtc.sec,(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year);
			// use the time stored in the variable rtc for date & time stamping
			serialPrintStr((char*)msg);
				}
				break;
			}
			else{
			sprintf((char*)msg,"Error code");
				serialPrintStr((char*)msg);
			}}
			
		  //Light Measurement
			/*light_i = adc(1,3);//readLDR();
			//FORMULA OR LOOK UP TABLE
			sprintf((char *)msg,"Light:%5d \x0d\xa",light_i);
			serialPrintStr((char*)msg);
			LCD_CmdWrite(0xC0); LCD_DisplayString((char*)msg);
			delay(2000);*/
			
			//Temperature Measurement
			temp    = adc(1,4);//readTemp();
			sprintf((char *)msg,"Temperature:%5d \x0d\xa",temp);
			serialPrintStr((char*)msg);
			LCD_CmdWrite(0x94); LCD_DisplayString((char*)msg);
			sprintf((char*)ms,"%d",temp);
				seven_seg(ms);
			delay(2000);
							
			// to control Relay on/off based on Light intensity
			/*if(light_i > 300) 
				RELAY_ON;
			else
				RELAY_OFF;
			*/
			// to control DC Motor Speed based on Temperature
			if(temp > 500){
				sprintf((char*)msg,"Temperature %d",temp);
				serialPrintStr((char*)msg);
				sprintf((char*)ms,"%d",temp);
				seven_seg(ms);
				runDCMotor(1,100);
				
			}
			else if((temp > 300) && (temp <=500)){
				runDCMotor(1,70);
				sprintf((char*)msg,"Medium");
				serialPrintStr((char*)msg);
				sprintf((char*)ms,"%d",temp);
				seven_seg(ms);
			}
			else if((temp>200) && (temp<=300)){
				runDCMotor(1,60);
				sprintf((char*)msg,"Low");
				serialPrintStr((char*)msg);
				sprintf((char*)ms,"%d",temp);
				seven_seg(ms);
			}
			else
				runDCMotor(1,0);
			if(IR2==0){
			count--;
				stepper_motor(2);
			sprintf((char*)msg,"Count = %d \n",count);
					serialPrintStr((char*)msg);
			RTC_GetDateTime(&rtc);//get current date & time stamp
			sprintf((char *)msg,"time:%2d:%2d:%2d  Date:%2d/%2d/%2d \x0d\xa",(uint16_t)rtc.hour,(uint16_t)rtc.min,(uint16_t)rtc.sec,(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year);
			// use the time stored in the variable rtc for date & time stamping
			serialPrintStr((char*)msg);
if(count==0){
		RELAY_OFF;
runDCMotor(1,0);
	sprintf((char*)msg,"Empty");
	serialPrintStr((char*)msg);
}
			}
	 }

}
char* board(void){
	char *a;
	int i=0;
	IO0DIR|= 1U<<31|0X00FF0000;
	SystemInit();
	uart_init();
	
	LED_ON;delay_ms(500);
	do{
	while(1){
	rowsel=0;IO0SET=0X000F0000;IO0CLR=1<<16;
		if(COL0==0){colsel=0;break;};if(COL1==0){colsel=1;break;};
		if(COL2==0){colsel=2;break;};if(COL3==0){colsel=3;break;};
	rowsel=1;IO0SET=0x000F0000;IO0CLR=1<<17;
		if(COL0==0){colsel=0;break;};if(COL1==0){colsel=1;break;};
		if(COL2==0){colsel=2;break;};if(COL3==0){colsel=3;break;};
	rowsel=2;IO0SET=0x000F0000;IO0CLR=1<<18;
		if(COL0==0){colsel=0;break;};if(COL1==0){colsel=1;break;};
		if(COL2==0){colsel=2;break;};if(COL3==0){colsel=3;break;};
	rowsel=3;IO0SET=0x000F0000;IO0CLR=1<<19;
		if(COL0==0){colsel=0;break;};if(COL1==0){colsel=1;break;};
		if(COL2==0){colsel=2;break;};if(COL3==0){colsel=3;break;};
	};
	delay_ms(50);
	while(COL0==0||COL1==0||COL2==0||COL3==0);
	delay_ms(50);
	IO0SET=0x000F0000;
	U0THR=lookup_table[rowsel][colsel];
	a[i++]=lookup_table[rowsel][colsel];
	}
	while(i<4);
	return a;
}
void uart_init(void){
 PINSEL0 |= 0x00000005;  // P0.0 & P0.1 ARE CONFIGURED AS TXD0 & RXD0
 U0LCR = 0x83;   /* 8 bits, no Parity, 1 Stop bit    */
 U0DLM = 0; U0DLL = 8; // 115200 baud rate,PCLK = 15MHz
 U0LCR = 0x03;
 U0FCR=0X07;
}
void SystemInit(void){
   PLL0CON = 0x01; 
   PLL0CFG = 0x24; 
   PLL0FEED = 0xAA; 
   PLL0FEED = 0x55; 
   while( !( PLL0STAT & 0x00000400 ))
   { ; }
   PLL0CON = 0x03;
   PLL0FEED = 0xAA;  // lock the PLL registers after setting the required PLL
   PLL0FEED = 0x55;
   VPBDIV = 0x01;      
} 

void buzzer(unsigned int val){
PINSEL1|=0x00080000;
DACR=((1<<16)|(val<<6));
delay_ms(500);
DACR=((1<<16)|(1023<<6));
delay_ms(1);
}
unsigned int adc(int no,int ch)
{
  // adc(1,4) for temp sensor LM35, digital value will increase as temp increases
	// adc(1,3) for LDR - digival value will reduce as the light increases
	// adc(1,2) for trimpot - digital value changes as the pot rotation
	unsigned int val;
	PINSEL0 |=  0x0F300000;   
	/* Select the P0_13 AD1.4 for ADC function */
  /* Select the P0_12 AD1.3 for ADC function */
	/* Select the P0_10 AD1.2 for ADC function */
  switch (no)        //select adc
    {
        case 0: AD0CR   = 0x00200600 | (1<<ch); //select channel
                AD0CR  |= (1<<24) ;            //start conversion
                while ( ( AD0GDR &  ( 1U << 31 ) ) == 0);
                val = AD0GDR;
                break;
 
        case 1: AD1CR = 0x00200600  | ( 1 << ch );       //select channel
                AD1CR |=  ( 1 << 24 ) ;                              //start conversion
                while ( ( AD1GDR & (1U << 31) ) == 0);
                val = AD1GDR;
                break;
    }
    val = (val  >>  6) & 0x03FF;         // bit 6:15 is 10 bit AD value
    return  val;
}

void RTC_Init(void)
{
   //enable clock and select external 32.768KHz
	   CCR = ((1<< 0 ) | (1<<4));//D0 - 1 enable, 0 disable
} 														// D4 - 1 external clock,0 from PCLK

// SEC,MIN,HOUR,DOW,DOM,MONTH,YEAR are defined in LPC214x.h
void RTC_SetDateTime(rtc_t *rtc)//to set date & time
{
     SEC   =  rtc->sec;       // Update sec value
     MIN   =  rtc->min;       // Update min value
     HOUR  =  rtc->hour;      // Update hour value 
     DOW   =  rtc->weekDay;   // Update day value 
     DOM   =  rtc->date;      // Update date value 
     MONTH =  rtc->month;     // Update month value
     YEAR  =  rtc->year;      // Update year value
}

void RTC_GetDateTime(rtc_t *rtc)
{
     rtc->sec     = SEC ;       // Read sec value
     rtc->min     = MIN ;       // Read min value
     rtc->hour    = HOUR;       // Read hour value 
     rtc->weekDay = DOW;      // Read day value 
     rtc->date    = DOM;       // Read date value 
     rtc->month   = MONTH;       // Read month value
     rtc->year    = YEAR;       // Read year value

}
void timer1_Init()
{
	T1TCR = 0X00;
	T1MCR = 0X03;  //011 
	T1MR0 = 150000;
	T1TC  = 0X00;
	VICIntEnable = 0x0000020;  //00100000 Interrupt Souce No:5, D5=1
	VICVectAddr5 = (unsigned long)Timer1_ISR;  // set the timer ISR vector address
	VICVectCntl5 = 0x0000025;  // set the channel,D5=1,D4-D0->channelNo-5
	T1TCR = 0X01;
}
int readLDR(void)
{
	int adc_output;
	PINSEL1 |=  1 << 24;  // P0.28 AS AD0.1 ( "01" -> bit24,bit25 of PINSEL1)
	AD0CR= (1 << 1 | 1 << 21 | 1 << 24) ;
	/* set the Bit 21 - Make ADC operational  
     set the Bit 1  - Select the channel AD0.1, Bit0 to Bit5 – for AD0.0toAD0.5
     set 001 on bits 26 25 24 - Issue SOC signal */
	while ( (AD0GDR & (unsigned long) 1 << 31) == 0);
  // Check for the conversion to complete, by reading Bit31 of  GDR
	adc_output = (AD0GDR >> 6 ) & 0x3FF ;
	// read the Digital output from GDR, after aligning the result to LSB  (Bit0) and masking other bits to Zero
	return adc_output;
}
void serialPrint(unsigned val)
{
	int i=0;
	unsigned char buf[50],ch;
	sprintf((char *)buf,"%d\x0d\xa",val);
	while((ch = buf[i++])!= '\0')
	  {
		while((U0LSR & (0x01<<5))== 0x00){}; 
    U0THR= ch;                         
	  }
}
void serialPrintStr(char * buf)
{
	int i=0;
	char ch;
	while((ch = buf[i++])!= '\0')	
	  {
		  while((U0LSR & (1u<<5))== 0x00){}; 
      U0THR= ch;   
	  }
	//send new line
	//while(U0LSR & (0x01<<5)){};U0THR = 13;
	//while(U0LSR & (0x01<<5)){};U0THR = 10;	
	
}
void delay(int cnt)
{
	T0MR0 = 1000;//14926; // some arbitrary count for delay
	T0MCR = 0x0004; // set Tiimer 0 Stop after Match
	while(cnt--)
	{
		T0TC = 0X00;
	  T0TCR = 1; // start the timer (enbale)
		while(!(T0TC == T0MR0)){};// wait for the match
	  T0TCR = 2;// stop the timer		
	}
}

int readTemp(void)
{	
	int adc_output;
	PINSEL1 |=  1 << 24;  //  AS AD0.2 ( "01" -> bit24,bit25 of PINSEL1)
	AD0CR= (1 << 2 | 1 << 21 | 1 << 24) ;
	/* set the Bit 21 - Make ADC operational  
     set the Bit 1  - Select the channel AD0.1, Bit0 to Bit5 – for AD0.0toAD0.5
     set 001 on bits 26 25 24 - Issue SOC signal */
	while ( (AD0GDR & (unsigned long) 1 << 31) == 0);
  // Check for the conversion to complete, by reading Bit31 of  GDR
	adc_output = (AD0GDR >> 6 ) & 0x3FF ;
	// read the Digital output from GDR, after aligning the result to LSB  (Bit0) and masking other bits to Zero
	return adc_output;
}
unsigned char getAlphaCode(unsigned char alphachar){
	switch(alphachar){
		case '1':return 0xf9;
		case '2':return 0xa4;
		case '3':return 0xb0;
		case '4':return 0x9b;
		case '0':return 0xc0;
		case '5':return 0x92;
		case '6':return 0x82;
		case '7':return 0xf8;
		case '8':return 0x80;
		case '9':return 0x90;
		case ' ':return 0xff;
		default:break;		
	}
	return 0xff;
}
void seven_seg(char *buf){
	unsigned char i,j;
	unsigned char seg7data,temp=0;
IO0DIR|=1U<<31|1U<<14|1U<<11|1U<<15;
IO0CLR|=1U<<31;
SystemInit();

	for(i=0;i<5;i++){
	seg7data=getAlphaCode(*(buf+i));
		for(j=0;j<8;j++){
		temp=seg7data&0x80000000;
			if(temp==0x80)
				IO0SET|=1<<14;
			else
				IO0CLR=1<<14;
			IO0SET|=1<<11;
			delay_ms(1);
			IO0CLR|=1<<11;
			seg7data=seg7data<<1;
		}
	}
	IO0SET|=1<<15;
	delay_ms(1);
	IO0CLR|=1<<15;
	return;
}
void stepper_motor(int ch){
unsigned int no_of_steps_clk = 100, no_of_steps_aclk = 100;
IO1DIR |= 0x00F00000; // to set P1.20 to P1.23 as o/ps
	switch(ch){
		case 1:
do{
IO1CLR = 0X00F00000;IO1SET = 0X00100000;delay_ms(10);if(--no_of_steps_clk == 0) break;
IO1CLR = 0X00F00000;IO1SET = 0X00200000;delay_ms(10);if(--no_of_steps_clk == 0) break;
IO1CLR = 0X00F00000;IO1SET = 0X00400000;delay_ms(10);if(--no_of_steps_clk == 0) break;
IO1CLR = 0X00F00000;IO1SET = 0X00800000;delay_ms(10);if(--no_of_steps_clk == 0) break;
}while(1);
break;
		case 2:
do{
IO1CLR = 0X00F00000;IO1SET = 0X00800000;delay_ms(10);if(--no_of_steps_aclk == 0) break;
IO1CLR = 0X00F00000;IO1SET = 0X00400000;delay_ms(10);if(--no_of_steps_aclk == 0) break;
IO1CLR = 0X00F00000;IO1SET = 0X00200000;delay_ms(10);if(--no_of_steps_aclk == 0) break;
IO1CLR = 0X00F00000;IO1SET = 0X00100000;delay_ms(10);if(--no_of_steps_aclk == 0) break;
}while(1);break;
default:break;
}
IO1CLR = 0X00F00000;
while(1);
}
void runDCMotor(unsigned int direction,unsigned int speed)
{
	//P0.28 pin is used to control direction of motor
	if(direction==1)
		IO0CLR = 1U << 28;
	else
		IO0SET = 1U << 28;
	
	// assume it is connected to PWM6 (P0.9)
	PINSEL0 |= 2U << 18; //0X00000008;   // 10
	PWMMR0 = 10000;
	PWMMR1 = 10000;
	PWMMR6 = 10000 * speed / 100;
	PWMMCR = 0X02;
	PWMPCR = 1u << 14 | 1<<9;//0X00000800;//
	PWMTCR = 0X09; //1001;
	PWMLER = 0X43; //01000001	
}