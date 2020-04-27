#include<reg52.h>
#include<intrins.h>
#include<stdio.h>
sbit P1_3 = P1^3;
sbit P1_4 = P1^4;
sbit P1_5 = P1^5;
char str = 'A';
bit end;
void delay(unsigned int i){
	int a;
	for(a =0;a<i;a++){
	_nop_();
	}
}

void I20() interrupt 0{
	if(end == 1){
		end =0;
P1_3 = !P1_3;
printf("%c",str++);
		delay(10000);
		end =1;
	}
}

void main(){
	EA = 1;
	EX0 =1;
	IT0 = 1;
	TMOD = 0x20;
	TI =1;
	end =1;
	
	SM0=0;
	SM1= 1;
	TR1 =1;
	
	TH1 = 0xfd;
	TL1 = 0xfd;
while(1);
}