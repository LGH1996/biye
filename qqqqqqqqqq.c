#include <reg52.h>
#include <intrins.h>
#include <stdio.h>
#include <string.h>

#define uchar unsigned char
sbit P2_1 = P2^1;
bit a =0;
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

void external_0() interrupt 0 using 2 //�ⲿ�ж�INT0���жϺ���������ģ��������1
{
	a =1;
}

void main(){

	EA = 1;

	TR1 = 1; //��T1��ʱ������Ϊ���ڵĲ����ʷ�����
	TMOD |= 0x20;
	TL1 = 0xFD;
	TH1 = 0xFD;

	SM0 = 0; //���ڳ�ʼ��
	SM1 = 1;
	REN = 1;
	
	EX0 = 1; //���ⲿ�ж�INT0��INT1
	IT0 = 1;
	
	while(1){
		if(a){
			uchar  shut[]="AT+CIPSHUT\n", cgatt[]="AT+CGATT?\n", cstt[]="AT+CSTT=\"CMNET\"\n",ciicr[]="AT+CIICR\n",cifsr[]="AT+CIFSR\n",start[]="AT+CIPSTART=\"TCP\",\"120.78.203.170\",\"12777\"\n",send[]="AT+CIPSEND\n",end[1] = {0x1A},close[]="AT+CIPCLOSE\n";
			P2_1 =0;
			sendData(shut,sizeof(shut)/sizeof(shut[0]));
			delay(5000);
			sendData(cgatt,sizeof(cgatt)/sizeof(cgatt[0]));
			delay(5000);
			sendData(cstt,sizeof(cstt)/sizeof(cstt[0]));
			delay(10000);;
			sendData(ciicr,sizeof(ciicr)/sizeof(ciicr[0]));
			delay(10000);
			sendData(cifsr,sizeof(cifsr)/sizeof(cifsr[0]));
			delay(5000);
			sendData(start,sizeof(start)/sizeof(start[0]));
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			delay(50000);
			sendData(send,sizeof(send)/sizeof(send[0]));
			delay(5000);
			sendData("AAABB",5);
			delay(5000);
			sendData(end,1);
			delay(5000);
			sendData(close,sizeof(close)/sizeof(close[0]));
			P2_1 = 1;
			a =0;
		}
	}
}