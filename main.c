#include <msp430.h>
int t=10;
void init_timer()
{
	UCSCTL4	=0x0045;

	TA1CCTL0 = CCIE;                          // CCR0 interrupt enabled
    TA1CCR0 = 14;					// Counter value for 38khz
    TA1CTL = TASSEL_2 + MC_1 + TACLR;         // SMCLK, upmode, clear TAR

//  __bis_SR_register(GIE); 			 //  enable interrupts

}
void init_adc()
{
	  P6SEL = 0x0F;        // Enable A/D channel inputs
	  ADC12CTL0 = ADC12ON+ADC12MSC+ADC12SHT0_8; // Turn on ADC12, extend sampling time
	                                            // to avoid overflow of results
	  ADC12CTL1 = ADC12SHP+ADC12CONSEQ_3;       // Use sampling timer, repeated sequence
	  ADC12MCTL0 = ADC12INCH_0;                 // ref+=AVcc, channel = A0
	  ADC12MCTL1 = ADC12INCH_1;                 // ref+=AVcc, channel = A1
	  ADC12MCTL2 = ADC12INCH_2;	                // ref+=AVcc, channel = A2
	  ADC12MCTL3 = ADC12INCH_3+ADC12EOS;
	  ADC12CTL0 |= ADC12ENC;


}
void button_init()
{
	P1DIR |= 0x00; // Set P1.1 to input direction
	P1IE |= 0x02; // P1.1 interrupt enabled
    P1IES |= 0x02; // P1.1 Hi/lo edge
    P1IFG &= ~0x02; // P1.1 IFG cleared
    _BIS_SR(GIE);
}

void init_gpio()
{
	P2DIR |= 0x15;	//P2.0,P2.2 Motor output pins,P2.4 Buzzer
	P3DIR |= 0x10;   //P3.3 IR in P3.4 IR out
}
void turn_right()
{
	//Left Motor ON right Motor OFF
	P2OUT |= BIT0;
	P2OUT &= ~BIT2;
	__delay_cycles(450000); //Approx delay for 90Degree turn
	P2OUT &= ~BIT0;
}
/*
 * main.c
 */

int main(void)
{

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	init_gpio();
    //init_timer();
	init_adc();
	button_init();
	while(1)
	{
		//P2OUT |= BIT4 ;
		if((P2IN&BIT1)!=BIT1 && t==2)
	 {
		do{
	   	  ADC12CTL0 |= ADC12SC;                     // Start convn - software trigger
     	  while (!(ADC12IFG & BIT3));
		  if( ADC12MEM0 >= 0xBB8 )				//Left motor line detection
			  P2OUT &= ~BIT0 ;
		  else{
		     	P2OUT |= BIT0 ;
		      }
		if( ADC12MEM2 >= 0xBB8 )				//Right motor line detect
		P2OUT &= ~BIT2 ;
		else{
		      	 P2OUT |= BIT2 ;
			}
		 if ( ADC12MEM0  >= 0xBB8 && ADC12MEM2 >= 0xBB8 ) //Grid Detection
			{
				__delay_cycles(500);
			    P2OUT &= ~BIT0 ;
				P2OUT &= ~BIT2 ;
			    __delay_cycles(500000);
				t--;
			}
		  }while(t>0);

	 }
	else if(t==0)
	{
		turn_right();
		while(t==0)
		{
			while (!(ADC12IFG & BIT2));
					  if( ADC12MEM0 >= 0xBB8 )				//Left motor line detection
						  P2OUT &= ~BIT0 ;
					  else{
					     	P2OUT |= BIT0 ;
					      }
					if( ADC12MEM2 >= 0xBB8 )				//Right motor line detect
					P2OUT &= ~BIT2 ;
					else{
					      	 P2OUT |= BIT2 ;
						}
					 if ( ADC12MEM0  >= 0xBB8 && ADC12MEM2 >= 0xBB8 ) //Grid Detection
						{

						    P2OUT &= ~BIT0 ;
							P2OUT &= ~BIT2 ;
						    __delay_cycles(500000);
							t--;
						}


	}
	}
	else
	{
		P2OUT &= ~BIT0;
		P2OUT &= ~BIT2;
	}
	}

}


// Timer1 A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) TIMER1_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
	P3OUT ^= BIT4;                            // Toggle P3.4
}

//Button interrupt for turning on port p1.1
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	t=2;
	P1IFG &= ~0x02; // P1.1 IFG cleared

}
