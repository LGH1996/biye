#include <reg52.h>
#include <intrins.h>
#include <stdio.h>
#include <string.h>

#define uchar unsigned char

sfr WDT_CONTR = 0xE1;
sbit P1_1 = P1 ^ 1;
sbit P1_2 = P1 ^ 2;
sbit P2_0 = P2 ^ 0;
sbit P2_1 = P2 ^ 1;
bit ledStatus = 0;
bit newDistance = 0;
uchar numWave = 0;
uchar numLed = 0;
uchar ledTip = 0;
uchar distanceCount = 0;
uchar lastRang = 0x00;
int lastDistance = 0;

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

void Time0() interrupt 1 using 0
{

	if (++numLed >= 5)
	{
		if (P2_0 != ledStatus)
		{
			ledTip++;
		}
		else
		{
			ledTip = 0;
		}
		if (ledStatus)
		{
			P2_1 = !P2_1;
		}
		numLed = 0;
		WDT_CONTR = 0x37;
	}
	numWave++;
	TH0 = 0x4C;
	TL0 = 0x00;
}

void Time2() interrupt 5 using 1
{

	if (EXF2)
	{
		int distance = ((RCAP2H << 8 | RCAP2L) * 1.085) / 58;
		EXF2 = 0;
		if (distance < (lastDistance - 50) || distance > (lastDistance + 50))
		{
			lastDistance = distance;
			distanceCount = 0;
			newDistance = 1;
		}
		else
		{
			distanceCount++;
		}
	}
}

void exter_0() interrupt 0 using 2
{
	uchar volumnUp[] = {0x7E, 0xFF, 0x06, 0x04, 0x00, 0x00, 0x00, 0xEF};
	sendData(volumnUp, 8);
}

void exter_1() interrupt 2 using 2
{
	uchar volumnDown[] = {0x7E, 0xFF, 0x06, 0x05, 0x00, 0x00, 0x00, 0xEF};
	sendData(volumnDown, 8);
}

void main()
{

	EA = 1;

	ET0 = 1; //开T0定时器
	TR0 = 1;
	TMOD |= 0x01;
	TH0 = 0x4C;
	TL0 = 0x00;

	TR1 = 1; //开T1定时器，作为串口的波特率发生器
	TMOD |= 0x20;
	TL1 = 0xFD;
	TH1 = 0xFD;

	ET2 = 1; //开T2捕获模式，测量超声波测距模块反馈的电压持续时间
	TR2 = 1;
	EXEN2 = 1;
	CP_RL2 = 1;

	SM0 = 0; //串口初始化
	SM1 = 1;
	REN = 1;

	EX0 = 1; //开外部中断0和1
	EX1 = 1;
	IT0 = 1;
	IT1 = 1;

	WDT_CONTR = 0x37; //开启看门狗

	while (1)
	{

		if (numWave >= 10)
		{
			P1_2 = 0;
			delay(10);
			P1_2 = 1;
			while (P1_1 == 0)
				;
			TH2 = 0x00;
			TL2 = 0x00;
			numWave = 0;
			WDT_CONTR = 0x37;
		}

		if (newDistance && distanceCount >= 2)
		{

			uchar play[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x01, 0x00, 0xEF};
			uchar curRang = 0x00;

			if (lastDistance >= 25 && lastDistance < 75)
			{
				curRang = 0x01;
			}
			if (lastDistance >= 75 && lastDistance < 125)
			{
				curRang = 0x02;
			}
			if (lastDistance >= 125 && lastDistance < 175)
			{
				curRang = 0x03;
			}
			if (lastDistance >= 175 && lastDistance < 225)
			{
				curRang = 0x04;
			}
			if (lastDistance >= 225 && lastDistance < 275)
			{
				curRang = 0x05;
			}
			if (lastDistance >= 275 && lastDistance < 325)
			{
				curRang = 0x06;
			}
			if (lastDistance >= 325 && lastDistance < 375)
			{
				curRang = 0x07;
			}
			if (lastDistance >= 375 && lastDistance < 425)
			{
				curRang = 0x08;
			}
			if (lastDistance >= 425 && lastDistance < 475)
			{
				curRang = 0x09;
			}
			if (lastDistance >= 475 && lastDistance < 525)
			{
				curRang = 0x0A;
			}
			if (lastDistance >= 525 && lastDistance < 575)
			{
				curRang = 0x0B;
			}

			if (curRang != 0x00 && curRang != lastRang)
			{
				lastRang = curRang;
				play[6] = curRang;
				sendData(play, 8);
				delay(13500);
				WDT_CONTR = 0x37;
			}
			newDistance = 0;
			distanceCount = 0;
		}

		if (ledTip >= 10)
		{
			uchar play[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x02, 0x00, 0xEF};
			ledStatus = P2_0;
			P2_1 = !P2_0;
			play[6] = ledStatus ? 0x01 : 0x02;
			sendData(play, 8);
			delay(13500);
			WDT_CONTR = 0x37;
			ledTip = 0;
		}
	}
}