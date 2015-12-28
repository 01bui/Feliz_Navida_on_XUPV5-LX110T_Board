#include <stdio.h>
#include <xio.h>
#include <xgpio.h>
#include <stdlib.h>
#include <xuartlite_l.h>
#include <xparameters.h>
#include <xtmrctr_l.h>

#define ESC             0x1b
#define PIEZO_ON        0x1
#define PIEZO_OFF       0x0
#define PLAY_SOUND      1

void tone (int freq, int duration);
static void myprintf(char *c);
void echo();
char InputGetChar();

int main(void)
{
  char ch;
  int i;
  for (i = 0; i < 50; i++)
  {
		xil_printf("**********   Do something   ********** \r\n\r\n");
		xil_printf("1. Write something\r\n");
		xil_printf("2. Play ring tone\r\n");
		/* set gpio bits as all outputs to piezo speaker */
		XGpio_WriteReg(XPAR_PIEZO_GPIO_0_BASEADDR, 0x4, 0x00000000);
		ch =InputGetChar();
		if (ch == '1')
      {
			do
			{
				myprintf("Typing...\r\n");
				echo();
				myprintf("\r\n Done...\r\n");
			} while (XUartLite_RecvByte(XPAR_RS232_UART_1_BASEADDR) != ESC);
		}
		else
		{
			xil_printf("\r\n Playing...\r\n");
      //a,d,c#,d,b,p,b,e,d,b,a,p,a,d,c#,d,b,g,b,b,a,a,b,a,g,g,f#			
	      tone(2700,500000); //A
    		tone(1800,800000);
			tone(1700,500000); //C#
			tone(1800,600000); //D
			tone(3000,600000); //B
		   tone(0,500000); //p
    		tone(3000,500000); //B
			tone(2000,800000); //E
			tone(1800,500000); //D
			tone(3000,600000); //B
			tone(2700,600000); //A
		   tone(0,500000); //p
			tone(2700,500000); //A
			tone(1800,800000); //D
			tone(1700,500000); //C#
			tone(1800,600000); //D
			tone(3000,500000); //B
			tone(2400,400000); //G
			tone(3000,400000); //B
			tone(3000,400000); //B
			tone(2700,500000); //A
			tone(2700,500000); //A
			tone(3000,500000); //B
			tone(2700,500000); //A
			tone(2400,500000); //G
			tone(2400,500000); //G
			tone(2300,500000); //F#
			xil_printf("\r\n Done...\r\n");
		}
	}
}

void tone (int freq, int duration)
{
   float period;	
   Xuint32 clockRate;
   Xuint32 halfPeriod;
   Xuint32 timeDuration;
   Xuint32 End_Time;
   Xuint32 Current_Time;
   Xuint32 cycle_start_time;
   Xuint32 transition_time;
   clockRate = 100;
   Xuint32 ControlStatus;	
   timeDuration=duration;
	
   if( freq!=0)
   {
     period=1e6/((Xuint32) freq);
     halfPeriod=(Xuint32)period/2;
   }
   else
   {
     //set any value for freq=0
     period=1e6/400;
     halfPeriod=period/2;
   }
    
   // Clear the Control Status Register
   XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 0,0x0);

   // Reset timer
   XTmrCtr_SetLoadReg(XPAR_TMRCTR_0_BASEADDR, 0, 0x0);
   XTmrCtr_LoadTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 0);

   // Clear the Load Timer bit in the Control Status Register
   ControlStatus = XTmrCtr_GetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 0);
   XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 0, ControlStatus & (~XTC_CSR_LOAD_MASK));
   XTmrCtr_Enable(XPAR_TMRCTR_0_BASEADDR, 0);
	
   //calculate how long to play note
   Current_Time = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 0);
	//xil_printf("\r\n Current Time: %x\r\n",Current_Time);
   End_Time=Current_Time + timeDuration * clockRate;
   //xil_printf("\r\n End Time: %x\r\n",End_Time);	

   do{
     //get start time
     cycle_start_time = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 0);
	  //xil_printf("\r\n cycle_start_time: %x\r\n",cycle_start_time);
	  
     //get time where sound goes low
     transition_time = cycle_start_time + halfPeriod*clockRate; //convert to ticks
	  //xil_printf("\r\n transition_time: %x\r\n",transition_time);
     Current_Time = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 0);
	  //xil_printf("\r\n Current_Time: %x\r\n",Current_Time);
		
#if PLAY_SOUND
     //set high to speaker
     if(freq > 20 )
	  XIo_Out32 (XPAR_PIEZO_GPIO_0_BASEADDR, PIEZO_ON);
#endif		
     //while(current_time is less than half_period_time) set high to speaker
     while(Current_Time < transition_time )
     Current_Time = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 0);
#if PLAY_SOUND
     //set high to speaker
     if(freq > 20 )
	  XIo_Out32 (XPAR_PIEZO_GPIO_0_BASEADDR, PIEZO_ON);
#endif
     //set low to speaker
     XIo_Out32 (XPAR_PIEZO_GPIO_0_BASEADDR, PIEZO_OFF);	

     //get time where period ends
     transition_time = cycle_start_time + (Xuint32)2*halfPeriod*clockRate;
     Current_Time = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 0);

     //while(current_time is less than half_period_time) do nothing;
     while(Current_Time < transition_time)
     Current_Time = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 0);

   }while(Current_Time < End_Time ); 

   XTmrCtr_Disable(XPAR_TMRCTR_0_BASEADDR, 0);
}

void myprintf(char *c)
{
  while (*c) 
  {
    XUartLite_SendByte(XPAR_RS232_UART_1_BASEADDR, *c);
    c++;
  }
}

void echo()
{
  char c;
  do {
    c = XUartLite_RecvByte(XPAR_RS232_UART_1_BASEADDR);
    if(c == '\r')
    XUartLite_SendByte(XPAR_RS232_UART_1_BASEADDR, '\n');
    XUartLite_SendByte(XPAR_RS232_UART_1_BASEADDR, c);
  } while (c != ESC);
}

char InputGetChar()
{
  char c;
  c = XUartLite_RecvByte(XPAR_RS232_UART_1_BASEADDR);
  return c;
}