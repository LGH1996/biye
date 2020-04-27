#include<reg52.h>
#include<intrins.h>
#include<stdio.h>
sbit P1_3 = P1^3;
sbit P1_4 = P1^4;

void delay(unsigned int i){
	int a;
	for(a =0;a<i;a++){
	_nop_();
	}
}
void main(){

while(1){

	if(P1_3 == 1){
	delay(20);
		if(P1_3 ==1){
		P1_4 =0;
		}
	}
	
	if(P1_3 == 0){
	delay(20);
		if(P1_3 ==0){
		P1_4 =1;
		}
	}

}


}