#include<reg52.h>
#include<intrins.h>
#include<stdio.h>
//»úÆ÷ÖÜÆÚ £¨1s/11.0592M£©*12=1.08507us

sbit P1_0 = P1^2;
sbit P1_1 = P1^1;
sbit P1_3 = P1^3;
unsigned int duration =0;
double distance;
bit flag = 0;
unsigned int num =0;
unsigned int count =0;
void delay(unsigned int i){
	int a;
	for(a =0;a<i;a++){
	_nop_();
	}
}

void time2() interrupt 5{
	
if(EXF2 ==1){
 EXF2 =0;
 duration = (RCAP2H<<8)|RCAP2L;
	distance = (duration*1.08507)/58;
 printf("%f",distance);
	}

	if(TF2 == 1){
	TF2 =0;
	}
}

void time0() interrupt 1{
num++;
	if(num > 20){
	P1_0 = 1;
  delay(10);
	P1_0 =0;
	while(P1_1==0);
	TL2 = 0x00;
	TH2 = 0x00;
	num = 0;
   TH0 = (65536-45872)/256;
   TL0 = (65536-45872)%256;
		
		
	}
}

void main(){
	
	P1_0 = 0;
	P1_1 =0;
	
	EA = 1;
	ET2 = 1;
	EXEN2 =1;
	CP_RL2 =1;
	
	TMOD = 0x21;
	TI =1;
	
	SM0=0;
	SM1= 1;
	TR1 =1;
	TR2=1;
	
	TH1 = 0xfd;
	TL1 = 0xfd;
	
 ET0 =1;
 TR0 =1;
 TH0 = (65536-45872)/256;
 TL0 = (65536-45872)%256;
 
	
	flag =1;
	
	
while(1);
}