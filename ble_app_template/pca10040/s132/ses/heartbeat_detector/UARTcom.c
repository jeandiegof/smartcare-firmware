#include <msp430.h>

void initUart();
void sendData(unsigned char c);
void sendInt(unsigned int n);
void sendString(char str[]);
void sendFloat(float f);

void initUart() // 115200 Baudrate
{
    // Configure UART
    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 |= UCSSEL__SMCLK;

    // Baud Rate calculation
    UCA0BR0 = 8;
    UCA0BR1 = 0x00;
    UCA0MCTLW = 0xF700 | UCOS16 | UCBRF_10;

    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
    //UCA0IE |= UCRXIE;                // Enable USCI_A0 RX interrupt
}

void sendData(unsigned char c)
{
    while(!(UCA0IFG&UCTXIFG));
    UCA0TXBUF = c;
}

void sendInt(unsigned int n)
{
    int num[] = {10000,1000,100,10,1};
    int pos, posI = 0, cnt, i;

    if(n==0)
        sendData('0');
    else
    {
        for (i = 4; i >= 0; i--)
            posI = n >= num[i] ? i : posI;

        for (pos = posI; pos < 5; pos++)
        {
          cnt = 0;
          while (n >= num[pos]) {
            cnt++;
            n -= num[pos];
          }
          sendData(cnt+'0');
        }
    }
}

void sendString(char str[])
{
    unsigned int i;
    for(i=0; str[i]!= '\0'; i++)
        sendData(str[i]);
}

void sendFloat(float f)
{
    int fInteger, fIntegerFract;
    float fFract;

    if(f < 0)
    {
        sendData('-');
        f = -1*f;
    }

    fInteger = (int) f;
    fFract = f -fInteger;
    fFract = 100*fFract;
    fIntegerFract = (int) fFract;
    sendInt((unsigned int) fInteger);
    sendData('.');
    sendInt((unsigned int) fIntegerFract);
}
