#include "msp430.h"
int i=0;
unsigned long int results[3]={0,0,0};

void senduart(void)
{
  while (!(IFG1 & UTXIFG0));                // USART0 TX buffer ready?
  TXBUF0 = 0xA5;
  while (!(IFG1 & UTXIFG0));                // USART0 TX buffer ready?
  TXBUF0 = results[1]&0xff;
  while (!(IFG1 & UTXIFG0));                // USART0 TX buffer ready?
  TXBUF0 = (results[1]>>8)&0xff;
  while (!(IFG1 & UTXIFG0));                // USART0 TX buffer ready?
  TXBUF0 = (results[1]>>16)&0xff;
  while (!(IFG1 & UTXIFG0));                // USART0 TX buffer ready?
  TXBUF0 = (results[1]>>24)&0xff;
  
  while (!(IFG1 & UTXIFG0));                // USART0 TX buffer ready?
  TXBUF0 = results[2]&0xff;
  while (!(IFG1 & UTXIFG0));                // USART0 TX buffer ready?
  TXBUF0 = (results[2]>>8)&0xff;
  while (!(IFG1 & UTXIFG0));                // USART0 TX buffer ready?
  TXBUF0 = (results[2]>>16)&0xff;
  while (!(IFG1 & UTXIFG0));                // USART0 TX buffer ready?
  TXBUF0 = (results[2]>>24)&0xff;
  while (!(IFG1 & UTXIFG0)); 
  TXBUF0 = 0xA5;
}

void main(void)
{
  volatile unsigned int i;
  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P1SEL |= BIT3;                       // P1.3, = USART0 TXD
  //DCO ile 12 mhz
  //  do
  //  {
  //      IFG1 &= ~OFIFG;                       // Clear OSCFault flag
  //      for (i = 0x47FF; i > 0; i--);         // Time for flag to set
  //  }
  //  while ((IFG1 & OFIFG));                   // OSCFault flag still set?
  //      /* Check if 12MHz Calibration is present */
  //        if (CALBC1_12MHZ != 0xFF)
  //        {
  //            DCOCTL = 0; // Select lowest DCOx and MODx
  //            BCSCTL1 = CALBC1_12MHZ; // Set range
  //            DCOCTL = CALDCO_12MHZ; // Set DCO step + modulation
  //        }
  
  /* 12mHz kristal */
  P2SEL |= (BIT6 | BIT7); // XIN, XOUT
  P2SEL2 &= ~(BIT6|BIT7); // XIN, XOUT
  
  BCSCTL1 ^= (XT2OFF); // xt2on
  BCSCTL3 |= (XT2S_2); // 2-16mhz kristal
  do
  {
    IFG1 &= ~OFIFG;                       // Clear OSCFault flag
    for (i = 0x47FF; i > 0; i--);         // Time for flag to set
  }
  while ((IFG1 & OFIFG));                   // OSCFault flag still set?
  {
    BCSCTL2 |= (SELS+SELM1); // MCLK=SMCLk=xt2=12MHZ
  }
  
  
  ME1 |= UTXE0;                     // Enable USART0 TXD/RXD
  U0CTL |= CHAR;                            // 8-bit character
  U0TCTL |= SSEL1;                          // UCLK= SMCLK
  U0BR0 = 8;                               // 1MHz 115200
  U0BR1 = 0x00;                             // 1MHz 115200
  U0MCTL = 0x00;                            // 1MHz 115200 modulation
  U0CTL &= ~SWRST;                          // Initialize USART state machine
  // IE1 |= URXIE0;                            // Enable USART0 RX interrupt
  //P1SEL2 |= BIT0;                           // Set SMCLK at P1.0
  
  SD24CTL   = SD24REFON+SD24VMIDON+SD24SSEL0+SD24DIV0;          // 1.2V ref, SMCLK 12mhz/12=1mhz
  SD24CCTL0 = SD24SNGL+SD24OSR_128+SD24IE;
  SD24CCTL1 = SD24OSR_1024+SD24IE;
  
  //SD24INCTL0 |= SD24INTDLY0;  SD24OSR_512+ +SD24DIV1+SD24XDIV0+
  SD24INCTL1 |= SD24GAIN_32;
  SD24INCTL0 |= SD24GAIN_2;
  for (i = 0; i < 0x3600; i++);             // Delay for 1.2V ref startup
  
    
  P1IE |= BIT4;                             // P1.4 interrupt enabled
  P1IES |= BIT4;                            // P1.4 Hi/lo edge
  P1IFG &= ~BIT4;                           // P1.4 IFG cleared
  SD24CCTL1 |= SD24SC;

    __bis_SR_register(GIE);       // Enter w/ interrupt
    SD24PRE0 = 0;
 
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=SD24_VECTOR
__interrupt void SD24AISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(SD24_VECTOR))) SD24AISR (void)
#else
#error Compiler not supported!
#endif
{
  switch (__even_in_range(SD24IV,SD24IV_SD24MEM2)) {
  case SD24IV_NONE: break;
  case SD24IV_SD24OVIFG: break;
  case SD24IV_SD24MEM0:
    senduart();
    results[1] = SD24MEM0;                   // Save CH0 results (clears IFG)
    SD24CCTL0 |= SD24LSBACC;
    results[1] = ((SD24MEM0)+ ((results[1] >>11) <<16));
    SD24CCTL0 ^= SD24LSBACC;  
  case SD24IV_SD24MEM1:
    results[2] = SD24MEM1;                   // Save CH2 results (clears IFG)
    SD24CCTL1 |= SD24LSBACC;
    results[2] = ((SD24MEM1)+ ((results[2] >>2) <<16));
    SD24CCTL1 ^= SD24LSBACC;
    
  case SD24IV_SD24MEM2: break;
  default: break;
  }
  
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  SD24CCTL0 |= SD24SC;
  P1IFG &= ~BIT4;                           // P1.4 IFG cleared
  }
