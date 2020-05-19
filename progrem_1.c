#include <reg52.h>
#include <intrins.h>
#include <string.h>
#include <stdio.h>

#define IAP_ADDR 0x4000 //内部REPPROM存储语音模块音量值的地址
#define uchar unsigned char
#define uint unsigned int

sfr WDT_CONTR = 0xE1; //定义看门狗控制寄存器
sfr IAP_DATA = 0xE2;  //定义操作EPPROM相关的特殊功能寄存器
sfr IAP_ADDRH = 0xE3;
sfr IAP_ADDRL = 0xE4;
sfr IAP_CMD = 0xE5;
sfr IAP_TRIG = 0xE6;
sfr IAP_CONTR = 0xE7;
sbit Echo = P1 ^ 1; //定义各类引脚
sbit Trig = P1 ^ 2;
sbit P2_0 = P2 ^ 0;
sbit P2_1 = P2 ^ 1;
bit lightStatus = 0; //定义各类标志位和辅助变量
bit distanceNew = 0;
bit test = 1;
uchar distanceNum = 0;
uchar lightNum = 0;
uchar lightStatusNum = 0;
uchar distanceNewNum = 0;
uchar distanceLastRange = 0x00;
uint distanceLast = 0;
uchar mediaCommand[] = {0x7E, 0xFF, 0x06, 0x00, 0x00, 0x00, 0x00, 0xEF}; //语音播放模块的指令
uint volume = 0x0f;//语音模块的当前声音大小
uchar *gsmCmd[10] = {"AT+CIPSHUT\n", "AT+CGATT?\n", "AT+CSTT=\"CMNET\"\n",
					 "AT+CIICR\n", "AT+CIFSR\n", "AT+CIPSTART=\"TCP\",\"120.78.203.170\",\"12777\"\n",
					 "AT+CIPSEND\n", "gps", "\x1A", "AT+CIPCLOSE\n"}; //循环发送的AT指令
uchar gsmNum = 0, gsmIndex = 0;	//控制AT指令的发送顺序
uint receiveCount = 0;
uchar receive[100], gpsSave[100]; //接收和保存GPS定位数据

void delay(uint i) //延时函数
{
	uint n;
	for (n = 0; n < i; n++)
	{
		_nop_();
	}
}

void sendData(uchar str[], uchar lenth) //向串行口发送数据
{
	uint n;
	for (n = 0; n < lenth; n++)
	{
		SBUF = str[n];
		while (TI == 0)
			;
		TI = 0;
	}
}

void sendAt() //发送AT指令
{
	sendData(gsmCmd[gsmIndex], strlen(gsmCmd[gsmIndex]));
	gsmNum = 0;
	gsmIndex++;
	gsmIndex %= 10;
	delay(1000);
}

void IapIdle() //进入待机模式，无ISP/IAP操作
{
	IAP_CONTR = 0;
	IAP_CMD = 0;
	IAP_TRIG = 0;
	IAP_ADDRH = 0x80;
	IAP_ADDRL = 0;
}

uchar IapReadByte(uint addr) //对内部RPPROM进行读取数据操作
{
	uchar dat;
	IAP_CONTR = 0x81;
	IAP_CMD = 0x01;
	IAP_ADDRL = addr;
	IAP_ADDRH = addr >> 8;
	IAP_TRIG = 0x46;
	IAP_TRIG = 0xb9;
	_nop_();
	dat = IAP_DATA;
	IapIdle();
	return dat;
}

void IapProgramByte(uint addr, uchar dat) //对内部REPPROM进行写入数据操作
{
	IAP_CONTR = 0x81;
	IAP_CMD = 0x02;
	IAP_ADDRL = addr;
	IAP_ADDRH = addr >> 8;
	IAP_DATA = dat;
	IAP_TRIG = 0x46;
	IAP_TRIG = 0xb9;
	_nop_();
	IapIdle();
}

void IapEraseSector(uint addr) //擦除内部EPPROM的指定扇区
{
	IAP_CONTR = 0x81;
	IAP_CMD = 0x03;
	IAP_ADDRL = addr;
	IAP_ADDRH = addr >> 8;
	IAP_TRIG = 0x46;
	IAP_TRIG = 0xb9;
	_nop_();
	IapIdle();
}

void Time0() interrupt 1 using 0 //定时器T0中断函数
{

	if (++lightNum >= 5) //检查外部光线强度的变化
	{
		if (P2_0 != lightStatus)
		{
			lightStatusNum++;
		}
		else
		{
			lightStatusNum = 0;
		}
		if (lightStatus)
		{
			P2_1 = !P2_1; //指示灯闪烁
		}
		lightNum = 0;
	}

	gsmNum++;
	distanceNum++;
	TH0 = 0x4C; //重新装载初值
	TL0 = 0x00;
}

void Time2() interrupt 5  using 1//定时器T2的中断函数
{

	if (EXF2) //使用定时器T2的捕获模式
	{
		double rcap = (RCAP2H << 8 | RCAP2L);				//获取发生中断时的TH2和TL2的值
		uint distance = (rcap * (12 / 11.0592) * 0.034) / 2; //计算出与障碍物的距离
		EXF2 = 0;
		if (distance < (distanceLast - 50) || distance > (distanceLast + 50)) //判断与障碍的距离是否发生了较大的变化
		{
			distanceLast = distance;
			distanceNewNum = 0;
			distanceNew = 1;
		}
		else
		{
			distanceNewNum++;
		}
	}
}

void serial_4() interrupt 4 using 2 //UART中断，主要用来接收GPS模块的定位数据
{
	if (RI)
	{
		uchar temp = SBUF;
		RI = 0;
		receive[receiveCount++] = temp;
		if (temp == '\n')
		{
			receive[receiveCount] = 0;
			if (receive[0] == '$' && receive[3] == 'G' && receive[4] == 'G' && receive[5] == 'A' && receiveCount >= 50 && receiveCount < 100)
			{
				strcpy(gpsSave, receive);
			}
			receiveCount = 0;
			receive[0] = 0;
		}
	}
}

void external_0() interrupt 0 using 3 //外部中断INT0的中断函数，语音模块音量加1
{
	if (volume < 30)
	{
		mediaCommand[3] = 0x06;
		mediaCommand[6] = ++volume;
		if(test){
			TI = 1;
			printf("音量加键被按下，语音播放模块音量加1%d ->%d\n",volume-1,volume);
			printf("向语音模块发送指令：7E FF 06 06 00 00 %X EF\n",volume);
			TI =0;
		} else {
			sendData(mediaCommand, 8);
		}
		IapEraseSector(IAP_ADDR);
		IapProgramByte(IAP_ADDR, volume);
	}
}

void external_1() interrupt 2 using 3 //外部中断INT1的中断函数，语音模块音量减1
{
	if (volume > 5)
	{
		mediaCommand[3] = 0x06;
		mediaCommand[6] = --volume;
		if(test){
			T1 =1;
			printf("音量减键被按下，语音播放模块音量加1：%d->%d\n",volume+1,volume);
			printf("向语音模块发送指令：7E FF 06 06 00 00 %X EF\n",volume);
			TI = 0;
		} else {
			sendData(mediaCommand, 8);
		}
		
		IapEraseSector(IAP_ADDR);
		IapProgramByte(IAP_ADDR, volume);
	}
}

void main()
{

	EA = 1; //开总中断

	ET0 = 1; //开T0中断
	TR0 = 1;
	TMOD |= 0x01;
	TH0 = 0x4C;
	TL0 = 0x00;

	TR1 = 1; //设置定时器T1为波特率发生器
	TMOD |= 0x20;
	TL1 = 0xFD;
	TH1 = 0xFD;

	ET2 = 1; //开定时器/计数器T2的捕获模式
	TR2 = 1;
	EXEN2 = 1;
	CP_RL2 = 1;

	ES = 1; //开UART中断，设置串行口通讯的工作模式
	SM0 = 0;
	SM1 = 1;
	REN = 1;

	EX0 = 1; //开外部中断INT0和INT1
	EX1 = 1;
	IT0 = 1;
	IT1 = 1;

	WDT_CONTR = 0x37; //开看门狗，在受到干扰使程序跑飞时自动复位

	memset(receive, 0, 100);
	memset(gpsSave, 0, 100);
	gsmCmd[7] = gpsSave;

	delay(20000); //等待语音模块启动完毕后
	volume = IapReadByte(IAP_ADDR);
	mediaCommand[3] = 0x06;
	mediaCommand[6] = volume;
	if(test){
		TI = 1;
		printf("启动时初始化语音模块音量：7E FF 06 06 00 00 %X EF\n",volume);
		TI =0;
	}
	sendData(mediaCommand, 8); //初始化语音模块的音量大小
	while (1)
	{

		if (distanceNum >= 10) //在间隔一定的时间后向测距模块发送高电平脉冲
		{
			Trig = 0;
			delay(10);
			Trig = 1;
			while (!Echo)
				; //等待测距模块返回高电平信号
			TH2 = 0x00;
			TL2 = 0x00;
			distanceNum = 0; //喂看门狗
			WDT_CONTR = 0x37;
		}

		if (distanceNew && distanceNewNum >= 2) //如果测距的数据发生新的变化
		{
			uchar distanceCurRange = 0x00;

			if (distanceLast >= 25 && distanceLast < 75) //根据新的测距数据确定要播放的语音文件
			{
				distanceCurRange = 0x01;
			}
			if (distanceLast >= 75 && distanceLast < 125)
			{
				distanceCurRange = 0x02;
			}
			if (distanceLast >= 125 && distanceLast < 175)
			{
				distanceCurRange = 0x03;
			}
			if (distanceLast >= 175 && distanceLast < 225)
			{
				distanceCurRange = 0x04;
			}
			if (distanceLast >= 225 && distanceLast < 275)
			{
				distanceCurRange = 0x05;
			}
			if (distanceLast >= 275 && distanceLast < 325)
			{
				distanceCurRange = 0x06;
			}
			if (distanceLast >= 325 && distanceLast < 375)
			{
				distanceCurRange = 0x07;
			}
			if (distanceLast >= 375 && distanceLast < 425)
			{
				distanceCurRange = 0x08;
			}
			if (distanceLast >= 425 && distanceLast < 475)
			{
				distanceCurRange = 0x09;
			}
			if (distanceLast >= 475 && distanceLast < 525)
			{
				distanceCurRange = 0x0A;
			}
			if (distanceLast >= 525 && distanceLast < 575)
			{
				distanceCurRange = 0x0B;
			}

			if (distanceCurRange != 0x00 && distanceCurRange != distanceLastRange) //如果前方有障碍物或已经还没提示过了，则发送相关指令
			{
				distanceLastRange = distanceCurRange;
				mediaCommand[3] = 0x0F;
				mediaCommand[5] = 0x01;
				mediaCommand[6] = distanceCurRange;
				if(test){
					TI =1;
					printf("与障碍物的距离：%d cm\n",distanceLast);
					printf("向语音模块发送指令：7E FF 06 0F 00 01 %X EF\n",(int)distanceCurRange);
					TI =0;
				}
				sendData(mediaCommand, 8); //向语音模块发送指令
				delay(13500);
				WDT_CONTR = 0x37; //喂看门狗
			}
			distanceNew = 0;
			distanceNewNum = 0;
		}

		if (lightStatusNum >= 10) //如果外部环境亮度发生了变化，且持续了一段时间
		{
			lightStatus = P2_0;
			P2_1 = !P2_0;
			mediaCommand[3] = 0x0F;
			mediaCommand[5] = 0x02;
			mediaCommand[6] = lightStatus ? 0x01 : 0x02;
			if(test){
				TI = 1;
				printf("环境光线较%s，指示灯已%s\n",lightStatus?"暗":"亮",lightStatus?"打开":"关闭");
				printf("向语音模块发送指令：7E FF 06 0F 00 02 %X EF\n",(int)(lightStatus ? 0x01 : 0x02));
				TI =0;
			}
			sendData(mediaCommand, 8); //向语音模块发送指令
			delay(13500);
			WDT_CONTR = 0x37; //喂看门狗
			lightStatusNum = 0;
		}
		switch (gsmIndex) //分别控制每条AT指令的发送间隔时间
		{
		case 0: //AT+CIPSHUT
			if (gsmNum > 10)
			{
				sendAt();
			}
			break;
		case 1: //AT+CGATT?
			if (gsmNum > 10)
			{
				sendAt();
			}
			break;
		case 2: //AT+CSTT="CMNET"
			if (gsmNum > 10)
			{
				sendAt();
			}
			break;
		case 3: //AT+CIICR
			if (gsmNum > 50)
			{
				sendAt();
			}
			break;
		case 4: //AT+CIFSR
			if (gsmNum > 50)
			{
				sendAt();
			}
			break;
		case 5: //AT+CIPSTART="TCP","120.78.203.170","12777"
			if (gsmNum > 10)
			{
				sendAt();
			}
			break;
		case 6: //AT+CIPSEND
			if (gsmNum > 150)
			{
				sendAt();
			}
			break;
		case 7: //gps定位数据
			if (gsmNum > 50)
			{
				sendAt();
			}
			break;
		case 8: //0x1A
			if (gsmNum > 5)
			{
				sendAt();
			}
			break;
		case 9: //AT+CIPCLOSE
			if (gsmNum > 10)
			{
				sendAt();
			}
			break;
		}
	}
}