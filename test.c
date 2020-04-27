#include<reg52.h>
#include<intrins.h>
#include<stdio.h>

sbit rw = P2^2;
sbit rs = P2^0;
sbit e = P2^1;
sbit P1_7 =P1^7;
char a='A';
void delay(unsigned int i){
	int a;
	for(a =0;a<i;a++){
	_nop_();
	}
}
void write_com(unsigned char com){
rs =0;
	rw =0;
P3 = com;
delay(10);
e =  1;
delay(10);
e = 0;	
}

void write_data(unsigned char str){
rs = 1;
	rw =0;
	P3 = str;
	delay(10);
	e = 1;
	delay(10);
	e =0;
}

void time2() interrupt 5{
	
	if(EXF2 == 1){
	unsigned int s;
	write_com(0x80+0x00);
	write_data(++a);
	write_com(0x80+0x40);
	s = (TH2<<8)|TL2;
	write_data(s/10000+48);
	write_data((s%10000)/1000+48);
	write_data((s%1000)/100+48);
	write_data((s%100)/10+48);
	write_data(s%10+48);
	EXF2 =0;
	}
	}

void main(){
	EA = 1;
	ET2 = 1;
	EXEN2 =1;
  TR2=1;
	CP_RL2 =1;
	
	write_com(0x38);
	write_com(0x0c);
	write_com(0x06);
	write_data(a);
	while(1){
		delay(10000);
	P1_7 = !P1_7;
	
	}
}