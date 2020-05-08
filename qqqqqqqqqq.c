#include <reg52.h>
#include <intrins.h>
#include <stdio.h>
#include <string.h>

#define uchar unsigned char

void delay(unsigned int i)
{
	unsigned int n;
	for (n = 0; n < i; n++)
	{
		_nop_();
	}
}

void sendData(uchar str[], uchar lenth)
{
	unsigned int n;
	for (n = 0; n < lenth; n++)
	{
		SBUF = str[n];
		while (TI == 0)
			;
		TI = 0;
	}
}

void serial_4() interrupt 4{
	uchar tem = 0;
	if(RI){
		tem = SBUF;
		RI = 0;
	}
	SBUF = tem;
	while(!TI);
}

void main(){

	EA = 1;

	TR1 = 1; //开T1定时器，作为串口的波特率发生器
	TMOD |= 0x20;
	TL1 = 0xFD;
	TH1 = 0xFD;

	ES = 1; //串口初始化
	SM0 = 0; 
	SM1 = 1;
	REN = 1;
	
	while(1);
}