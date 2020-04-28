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
bit ledStatus = 0;
bit newDistance = 0;
uchar numWave = 0;
uchar numLed = 0;
uchar ledTip = 0;
uint lastDistance = 0;

uchar gpsCount = 0;
uchar serialCount = 0;
uchar receive[100],save[100] = "is null";



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

void Time0() interrupt 1 using 0{

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
	
	numWave++;
	gpsCount++;
	TH0 =0x4C;
	TL0= 0x00;
}

void Time2() interrupt 5 using 1{

	if(EXF2){
		
		uint distance = ((RCAP2H<<8|RCAP2L)*1.085)/58;
		EXF2 = 0;
		if(distance < (lastDistance-50) || distance > (lastDistance+50)){
			lastDistance = distance;
			newDistance = 1;
		}
		//TI = 1;
		//printf("%d\n",distance);
  }
}

void serial_4() interrupt 4 using 3{
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
	     
	ES = 1;
	SM0 = 0; //串口初始化
	SM1 = 1;
	REN = 1;
	
	while(1){
		
		if(numWave>=20){
			P1_2 = 1;
			delay(10);
			P1_2 = 0;
			while(P1_1 == 0);
			TH2 = 0x00;
			TL2 = 0x00;
			numWave = 0;
		}
		
		if(newDistance){

		uchar n, play[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x01, 0x00,0xEF};
		uchar curRang = 0x00;
		newDistance = 0;
		
		if(lastDistance>=25 && lastDistance<75){
			curRang = 0x01;
		}
		if(lastDistance>=75 && lastDistance<125){
			curRang = 0x02;
		}
		if(lastDistance>=125 && lastDistance<175){
			curRang = 0x03;
		}
		if(lastDistance>=175 && lastDistance<225){
			curRang = 0x04;
		}
		if(lastDistance>=225 && lastDistance<275){
			curRang = 0x05;
		}
		if(lastDistance>=275 && lastDistance<325){
			curRang = 0x06;
		}
		if(lastDistance>=325 && lastDistance<375){
			curRang = 0x07;
		}
		if(lastDistance>=375 && lastDistance<425){
			curRang = 0x08;
		}

		play[6] = curRang;
		for(n = 0;n<8;n++){
			//sendData(play[n]);
		}
		delay(6800);
	}
		if(gpsCount >500){
			//TI = 1;
			//printf("%s",save);
			//TI = 0;
			uchar i,setApn[]="AT+CSTT=\"CMNET\"\n\r",ciicr[]="AT+CIICR\n\r", connect[] = "AT+CIPSTART=\"TCP\",\"120.78.203.170\",12777\n\r",send[] = "AT+CIPSEND\n\r",end = 0x1A,close[] = "AT+CIPCLOSE\n\r",shut[]="AT+CIPSHUT\n\r";

			for(i=0;i<strlen(setApn);i++){
				sendData(setApn[i]);
			}
			delay(1000);
			
			for(i=0;i<strlen(ciicr);i++){
				sendData(ciicr[i]);
			}
			delay(1000);
			
			for(i=0;i<strlen(connect);i++){
				sendData(connect[i]);
			}
			delay(5000);
			
			for(i=0;i<strlen(send);i++){
				sendData(send[i]);
			}
			delay(1000);
			
			for(i=0;i<strlen(save);i++){
				sendData(save[i]);
			}
			delay(1000);
			
			sendData(end);
			delay(1000);
			
			for(i=0;i<strlen(close);i++){
				sendData(close[i]);
			}
			delay(1000);
			
			for(i=0;i<strlen(shut);i++){
				sendData(shut[i]);
			}
			gpsCount = 0;
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
	