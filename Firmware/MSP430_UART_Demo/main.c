#include "msp430afe253.h"
int i=0;
unsigned long int results[3]={0,0,0};

void senduart(void){
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

   SD24CCTL0 |= SD24SC;
   SD24CCTL1 |= SD24SC;
    
}

void main(void)
{
  volatile unsigned int i;

  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P1SEL |= BIT3+BIT4;                       // P1.3,1.4 = USART0 TXD/RXD
  do
  {
      IFG1 &= ~OFIFG;                       // Clear OSCFault flag
      for (i = 0x47FF; i > 0; i--);         // Time for flag to set
  }
  while ((IFG1 & OFIFG));                   // OSCFault flag still set?
      /* Check if 12MHz Calibration is present */
        if (CALBC1_12MHZ != 0xFF)
        {
            DCOCTL = 0; // Select lowest DCOx and MODx
            BCSCTL1 = CALBC1_12MHZ; // Set range
            DCOCTL = CALDCO_12MHZ; // Set DCO step + modulation
        }
            
  ME1 |= UTXE0 + URXE0;                     // Enable USART0 TXD/RXD
  U0CTL |= CHAR;                            // 8-bit character
  U0TCTL |= SSEL1;                          // UCLK= SMCLK
  U0BR0 = 8;                               
  U0BR1 = 0x00;                           
  U0MCTL = 0x00;                            
  U0CTL &= ~SWRST;                          // Initialize USART state machine
 // IE1 |= URXIE0;                            // Enable USART0 RX interrupt
  P1SEL2 |= BIT0;                           // Set SMCLK at P1.0
  
  SD24CTL   = SD24REFON+SD24SSEL0;          // 1.2V ref, SMCLK
  SD24CTL ^= SD24VMIDON;
  SD24CCTL0 = SD24SNGL+SD24OSR_64+SD24GRP;             // Single conv, group with CH1
  SD24CCTL1 = SD24SNGL+SD24OSR_64+SD24IE;             // Single conv, group with CH2
  //SD24INCTL0 |= SD24GAIN_2;                     //gain 4 olmuyor
  //SD24INCTL1 |= SD24GAIN_16;
 
  for (i = 0; i < 0x3600; i++);             // Delay for 1.2V ref startup
  
    SD24CCTL0 |= SD24SC ;
    SD24CCTL1 |= SD24SC;
    __bis_SR_register(GIE);       // Enter LPM0 w/ interrupt
   
 
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
    case SD24IV_SD24MEM0: break;
    case SD24IV_SD24MEM1:
    results[1] = SD24MEM0;                   // Save CH1 results (clears IFG)
    SD24CCTL0 |= SD24LSBACC;
    results[1] = ((SD24MEM0 <<8)+ (results[1] << 16))>>8;
    SD24CCTL0 ^= SD24LSBACC;
    
    results[2] = SD24MEM1;                   // Save CH2 results (clears IFG)
    SD24CCTL1 |= SD24LSBACC;
    results[2] = ((SD24MEM0 <<8)+ (results[2] << 16))>>8;
    SD24CCTL1 ^= SD24LSBACC;
    senduart();
    
    case SD24IV_SD24MEM2: break;
    default: break;
    }
  
}
