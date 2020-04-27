#include<reg52.h>
#include<intrins.h>
sbit rw = P2^2;
sbit rs = P2^0;
sbit e = P2^1;
unsigned char table0[] = "I LOVE YOU GOOD FOR YOU 2281442260@qq.com HELLOW YOUT ARWE STUPID";
unsigned char table1[] = "WLECOM";
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

void main(){
	int a =0;
	unsigned char cursor = 0x00;
	write_com(0x38);
	write_com(0x0c);
	write_com(0x06);
	for(a=0;a<50;a++){
		write_data(table0[a]);
		delay(10);
	}
	write_com(0x80+0x40);
	for(a= 0;a<6;a++){
	write_data(table1[a]);
	}
	while(1){
	write_com(0x1c);
		delay(10000);
	}

}