#include<reg52.h>
#include<intrins.h>
#include<stdio.h>
#include<string.h>

#define uchar unsigned char
#define uint unsigned int

sbit P1_1 = P1^1;
sbit P1_2 = P1^2;
sbit P2_0 = P2^0;
sbit P2_1 = P2^1;
sbit PSH = 0xB4;
sbit PT0H = 0xB1;
sbit PT2H = 0xB5;
bit launchWave = 0;
bit ledStatus = 0;
uchar numWave = 0;
uchar numLed = 0;
uchar lastRang = 0;
uchar lastTip = 0;
uint ledTip = 0;
uint distanceTip = 0;
uint serialCount = 0;
uchar receive[100],save[100];
uchar updateGps = 0;



void delay(uint i){
  uint n;
	for(n=0;n<i;n++){
	_nop_();
	}
}
void sendData(uchar str){

	SBUF = str;
	while(TI == 0);
	TI = 0;
	
}

void Serial_4() interrupt 4{
	if(RI){
	  uchar temp = SBUF;
		RI = 0;
		receive[serialCount++] = temp;
		if(temp == '\n'){
			receive[serialCount] = 0;
			if(receive[0]=='$' && receive[3] == 'G' && receive[4]=='G' && receive[5]=='A' && serialCount >=60 && serialCount <80){
				strcpy(save,receive);
			}
			serialCount = 0;
			receive[0] = 0;
		}
	}
}
void Time0() interrupt 1{
	
	if(++numWave >= 10){
		launchWave = 1;
		numWave = 0;
	}
	
	if(++numLed >= 5){
		if(ledStatus != P2_0){
			if(++ledTip >= 4){
				ledStatus = P2_0;
				P2_1 = !P2_0;
			}
		} else if(ledTip < 4) {
			ledTip = 0;
		}
		if(ledStatus){
			P2_1 = !P2_1;
		}
		numLed = 0;
	}
	
	if(++updateGps>=105){
		TI = 1;
		printf("%s",save);
		updateGps = 0;
	}
	
	TH0 =0x4C;
	TL0= 0x00;
}

void Time2() interrupt 5{

	if(EXF2){
		
		uchar curRang = 0x00;
	  uint distance = ((RCAP2H<<8|RCAP2L)*1.085)/58;
		EXF2 = 0;
		//TI = 1;
		//printf("%d\n",distance);
		
		
		if(distance>=50 && distance<100){
			curRang = 0x01;
		}
		if(distance>=100 && distance<150){
			curRang = 0x02;
		}
		if(distance>=150 && distance<200){
			curRang = 0x03;
		}
		if(distance>=200 && distance<250){
			curRang = 0x04;
		}
		if(distance>=250 && distance<300){
			curRang = 0x05;
		}
		if(distance>=300 && distance<350){
			curRang = 0x06;
		}
		if(distance>=350 && distance<400){
			curRang = 0x07;
		}
		if(distance>=400 && distance<450){
			curRang = 0x08;
		}
		
		if(curRang != 0x00){
			distanceTip = curRang == lastRang ? ++distanceTip : 0;
			lastRang = curRang;
		}
  }
}

void main(){

	EA = 1; 
	
	ET0 = 1; //开T0定时器
	TR0 = 1;
  TMOD |= 0x01;
	TH0 =0x4C;
	TL0= 0x00;
	
	TR1 = 1;//开T1定时器，作为串口的波特率发生器
	TMOD |= 0x20;
	TL1 = 0xFD;
	TH1 = 0xFD;
	
  ET2 = 1;       //开T2捕获模式，测量超声波测距模块反馈的电压持续时间
	TR2 = 1;
	EXEN2 = 1;
	CP_RL2 = 1;
	
	ES = 1; //串口初始化
	SM0 = 0; 
	SM1 = 1;
	REN = 1;
	
	PSH = 1;
	PS = 0;
	PT0H = 1;
	PT0 = 1;
	PT2H = 1;
	PT2 =1;

	
	while(1){
		
		if(launchWave){
			P1_2 = 1;
			delay(10);
			P1_2 = 0;
			while(P1_1 == 0);
			TH2 = 0x00;
			TL2 = 0x00;
			launchWave = 0;
		}
		
		if(distanceTip >= 2 && lastTip != lastRang){
	
			uchar n, play[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x01, 0x00,0xEF};
			play[6] = lastRang;
			for(n = 0;n<8;n++){
				//sendData(play[n]);
			}
			lastTip = lastRang;
			distanceTip = 0;
			delay(6800);
		}
		
		if(ledTip >= 4){
			uchar n, play[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x02, 0x00,0xEF};
			play[6] = ledStatus ? 0x01 : 0x02;
			for(n = 0;n<8;n++){
				//sendData(play[n]);
			}
			ledTip = 0;
			delay(7000);
		}
		
		}
	}
	