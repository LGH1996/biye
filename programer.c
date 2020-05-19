#include <reg52.h>
#include <intrins.h>
#include <string.h>

#define ID "#0000000001"

#define IAP_ADDR 0x4000 //�ڲ�REPPROM�洢����ģ������ֵ�ĵ�ַ
#define uchar unsigned char
#define uint unsigned int

sfr WDT_CONTR = 0xE1; //���忴�Ź����ƼĴ���
sfr IAP_DATA = 0xE2;  //�������EPPROM��ص����⹦�ܼĴ���
sfr IAP_ADDRH = 0xE3;
sfr IAP_ADDRL = 0xE4;
sfr IAP_CMD = 0xE5;
sfr IAP_TRIG = 0xE6;
sfr IAP_CONTR = 0xE7;
sbit Echo = P1 ^ 1; //�����������
sbit Trig = P1 ^ 2;
sbit P2_0 = P2 ^ 0;
sbit P2_1 = P2 ^ 1;
bit lightStatus = 0; //��������־λ�͸�������
bit distanceNew = 0;
uchar distanceNum = 0;
uchar lightNum = 0;
uchar lightStatusNum = 0;
uchar distanceNewNum = 0;
uchar distanceLastRange = 0x00;
uint distanceLast = 0;
uchar mediaCommand[] = {0x7E, 0xFF, 0x06, 0x00, 0x00, 0x00, 0x00, 0xEF}; //��������ģ���ָ��
uchar volume = 0x0f;//����ģ��ĵ�ǰ������С
uchar *gsmCmd[10] = {"AT+CIPSHUT\n", "AT+CGATT?\n", "AT+CSTT=\"CMNET\"\n",
                     "AT+CIICR\n", "AT+CIFSR\n", "AT+CIPSTART=\"TCP\",\"120.78.203.170\",\"12777\"\n",
                     "AT+CIPSEND\n", "gps", "\x1A", "AT+CIPCLOSE\n"}; //ѭ�����͵�ATָ��
uchar gsmNum = 0, gsmIndex = 0; //����ATָ��ķ���˳��
uint receiveCount = 0;
uchar receive[100], gpsSave[150]; //���պͱ���GPS��λ����

void delay(uint i) //��ʱ����
{
    uint n;
    for (n = 0; n < i; n++)
    {
        _nop_();
    }
}

void sendData(uchar str[], uchar lenth) //���пڷ�������
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

void sendAt() //����ATָ��
{
    sendData(gsmCmd[gsmIndex], strlen(gsmCmd[gsmIndex]));
    gsmNum = 0;
    gsmIndex++;
    gsmIndex %= 10;
    delay(1000);
}

void IapIdle() //�������ģʽ����ISP/IAP����
{
    IAP_CONTR = 0;
    IAP_CMD = 0;
    IAP_TRIG = 0;
    IAP_ADDRH = 0x80;
    IAP_ADDRL = 0;
}

uchar IapReadByte(uint addr) //���ڲ�RPPROM���ж�ȡ���ݲ���
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

void IapProgramByte(uint addr, uchar dat) //���ڲ�REPPROM����д�����ݲ���
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

void IapEraseSector(uint addr) //�����ڲ�EPPROM��ָ������
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

void Time0() interrupt 1 using 0 //��ʱ��T0�жϺ���
{

    if (++lightNum >= 5) //����ⲿ����ǿ�ȵı仯
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
            P2_1 = !P2_1; //ָʾ����˸
        }
        lightNum = 0;
    }

    gsmNum++;
    distanceNum++;
    TH0 = 0x4C; //����װ�س�ֵ
    TL0 = 0x00;
}

void Time2() interrupt 5 using 1 //��ʱ��T2���жϺ���
{

    if (EXF2) //ʹ�ö�ʱ��T2�Ĳ���ģʽ
    {
        double rcap = (RCAP2H << 8 | RCAP2L); //��ȡ�����ж�ʱ��TH2��TL2��ֵ
        uint distance = (rcap * (12 / 11.0592) * 0.034) / 2; //��������ϰ���ľ���
        EXF2 = 0;
        if (distance < (distanceLast - 50) || distance > (distanceLast + 50)) //�ж����ϰ��ľ����Ƿ����˽ϴ�ı仯
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

void serial_4() interrupt 4 using 3 //UART�жϣ���Ҫ��������GPSģ��Ķ�λ����
{
    if (RI)
    {
        uchar temp = SBUF;
        RI = 0;
        receive[receiveCount++] = temp;
        if (temp == '\n')
        {
            receive[receiveCount] = 0;
            if (receive[0] == '$' && receive[3] == 'G' && receive[4] == 'G' && receive[5] == 'A' && receiveCount >= 60 && receiveCount < 100)
            {
                strcpy(gpsSave, receive);
								strcat(gpsSave,ID);
            }
            receiveCount = 0;
            receive[0] = 0;
        }
    }
}

void external_0() interrupt 0 using 2 //�ⲿ�ж�INT0���жϺ���������ģ��������1
{
    if (volume < 30)
    {
        mediaCommand[3] = 0x06;
        mediaCommand[6] = ++volume;
        sendData(mediaCommand, 8);
        IapEraseSector(IAP_ADDR);
        IapProgramByte(IAP_ADDR, volume);
    }
}

void external_1() interrupt 2 using 2 //�ⲿ�ж�INT1���жϺ���������ģ��������1
{
    if (volume > 5)
    {
        mediaCommand[3] = 0x06;
        mediaCommand[6] = --volume;
        sendData(mediaCommand, 8);
        IapEraseSector(IAP_ADDR);
        IapProgramByte(IAP_ADDR, volume);
    }
}

void main()
{

    EA = 1; //�����ж�

    ET0 = 1; //��T0�ж�
    TR0 = 1;
    TMOD |= 0x01;
    TH0 = 0x4C;
    TL0 = 0x00;

    TR1 = 1; //���ö�ʱ��T1Ϊ�����ʷ�����
    TMOD |= 0x20;
    TL1 = 0xFD;
    TH1 = 0xFD;

    ET2 = 1; //����ʱ��/������T2�Ĳ���ģʽ
    TR2 = 1;
    EXEN2 = 1;
    CP_RL2 = 1;

    ES = 1; //��UART�жϣ����ô��п�ͨѶ�Ĺ���ģʽ
    SM0 = 0;
    SM1 = 1;
    REN = 1;

    EX0 = 1; //���ⲿ�ж�INT0��INT1
    EX1 = 1;
    IT0 = 1;
    IT1 = 1;

    WDT_CONTR = 0x37; //�����Ź������ܵ�����ʹ�����ܷ�ʱ�Զ���λ

    memset(receive, 0, 100);
    memset(gpsSave, 0, 100);
		gsmCmd[7] = gpsSave;

    delay(20000); //�ȴ�����ģ��������Ϻ�
    volume = IapReadByte(IAP_ADDR);
    mediaCommand[3] = 0x06;
    mediaCommand[6] = volume;
    sendData(mediaCommand, 8); //��ʼ������ģ���������С

    while (1)
    {

        if (distanceNum >= 10) //�ڼ��һ����ʱ�������ģ�鷢�͸ߵ�ƽ����
        {
            Trig = 0;
            delay(10);
            Trig = 1;
            while (!Echo)
                ; //�ȴ����ģ�鷵�ظߵ�ƽ�ź�
            TH2 = 0x00;
            TL2 = 0x00;
            distanceNum = 0; //ι���Ź�
            WDT_CONTR = 0x37;
        }

        if (distanceNew && distanceNewNum >= 2) //����������ݷ����µı仯
        {
            uchar distanceCurRange = 0x00;

            if (distanceLast >= 25 && distanceLast < 75) //�����µĲ������ȷ��Ҫ���ŵ������ļ�
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

            if (distanceCurRange != 0x00 && distanceCurRange != distanceLastRange) //���ǰ�����ϰ�����Ѿ���û��ʾ���ˣ��������ָ��
            {
                distanceLastRange = distanceCurRange;
                mediaCommand[3] = 0x0F;
                mediaCommand[5] = 0x01;
                mediaCommand[6] = distanceCurRange;
                sendData(mediaCommand, 8); //������ģ�鷢��ָ��
                delay(13500);
                WDT_CONTR = 0x37; //ι���Ź�
            }
            distanceNew = 0;
            distanceNewNum = 0;
        }

        if (lightStatusNum >= 10) //����ⲿ�������ȷ����˱仯���ҳ�����һ��ʱ��
        {
            lightStatus = P2_0;
            P2_1 = !P2_0;
            mediaCommand[3] = 0x0F;
            mediaCommand[5] = 0x02;
            mediaCommand[6] = lightStatus ? 0x01 : 0x02;
            sendData(mediaCommand, 8); //������ģ�鷢��ָ��
            delay(13500);
            WDT_CONTR = 0x37; //ι���Ź�
            lightStatusNum = 0;
        }
        switch (gsmIndex) //�ֱ����ÿ��ATָ��ķ��ͼ��ʱ��
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
        case 7: //gps��λ����
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
