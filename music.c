#include<reg52.h>
#include<intrins.h>
#include<stdio.h>

sbit P2_3 = P2^3;

void delay(unsigned char i){
 unsigned char n;
	for(n=0;n<i;n++){
	_nop_();
	}
}

void sendData(unsigned char str){

	SBUF = str;
	while(TI == 0);
	TI = 0;
	
}

void main(){

	TR1 = 1;//开T1定时器，作为串口的波特率发生器
	TMOD |= 0x20;
	TL1 = 0xFD;
	TH1 = 0xFD;
	
	SM0 = 0;
	SM1 = 1;
	REN = 1;
	
	
while(1){	
		if(P2_3==0){
		 delay(100000000);
			if(P2_3 == 0){
			unsigned char n;
			//unsigned char comand[8] = {0x7E, 0xFF, 0x06, 0x01, 0x00, 0x00, 0x00, 0xEF};
			unsigned char comand[8] =	{0x7E, 0xFF, 0x06,0x18, 0x00, 0x00, 0x00, 0xEF};
			for(n =0;n<8;n++){
						sendData(comand[n]);
					}
			}

		}
	}
}