#include "reg51.h"
#include "intrins.h"
#include "absacc.h"
#include "stdio.h"
#include "stdlib.h"

sbit IIC_SDA=P3^2;        //声明IIC总线的数据线接在单片机的P3.2端口。
sbit IIC_SCL=P3^3;        //声明IIC总线的时钟线接在单片机的P3.3端口。

sbit RS=P2^7;
sbit RW=P2^6;
sbit EN=P2^5;

#define DATA_OUT P0
#define FOSC 11059200L
#define BAUD 9600

//IIC----------------------------------------------------------------
void delay_IIC(void);
void IIC_Init(void);
void IIC_start(void);
void IIC_stop(void); 
bit IIC_Tack(void);
void IIC_write_byte(unsigned char Data);
unsigned char IIC_read_byte();
void IIC_single_byte_write(unsigned char Daddr,unsigned char Waddr,unsigned char Data);
unsigned char IIC_single_byte_read(unsigned char Daddr,unsigned char Waddr);
//-------------------------------------------------------------------
//LCD1602------------------------------------------------------------
void delay_ms(unsigned int ms);
void LCD_CheckBusy(void);
void LCD_Write(unsigned char ucData, bit bComOrData);
void LCD_Display(unsigned char *s);
void LCD_Init(void);
void LCD_Move(unsigned char x,unsigned char y);
//-------------------------------------------------------------------
//MAIN---------------------------------------------------------------
void InitUart();
void Delay100ms();
void IntoS(unsigned int a,unsigned char *str);
//-------------------------------------------------------------------

void main()
{	
	unsigned char s;
	unsigned char s1;
	unsigned int a;	
	unsigned int b;
	unsigned int sum;	
	unsigned char ubuffer[4];
	
	InitUart();
	IIC_Init();
	LCD_Init();
	delay_ms(5);
	
	LCD_Move(0,0);
	LCD_Display("juli:");
	LCD_Move(1,4);
	LCD_Display("mm");
	
	IIC_single_byte_write(0xa4,0x09,0);		   //选择数据发送方式：被动接受
	Delay100ms();
	
	while(1)
	{
		if(RI)
		{
			RI = 0;             //清除接受标志位
			
			s=IIC_single_byte_read(0xa4,0x04);//接受高八位数据类型为unsigned char
			a=s;//高八位的ASCII码
			s1=IIC_single_byte_read(0xa4,0x05);//接受低八位数据类型为unsigned char
			b=s1;//低八位的ASCII码
			sum=256*s+s1;//实际的数据

			IntoS(sum,ubuffer);

			LCD_Move(1,0);
			LCD_Display(ubuffer);
		}
	}
}

//MAIN---------------------------------------------------------------
void IntoS(unsigned int a,unsigned char *str)
{//将数字转换为字符串
	unsigned int i=0,j=0;
	unsigned char temp[4];
	while(a)
	{
		temp[i++]=a%10+'0';  //将数字加字符0就变成相应字符
		a/=10;               //此时的字符串为逆序
	}
	temp[i]='\0';
	i=i-1;
	while(i!=0xFFFF)
	{
		str[j++]=temp[i--];   //将逆序的字符串转为正序
	}
	str[j]='\0';              //字符串结束标志
}
void Delay100ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 5;
	j = 52;
	k = 195;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
void InitUart()
{
    SCON=0x50;
    TMOD=0x20;
    TH1=TL1=-(FOSC/12/32/BAUD);
    TR1=1;
    TI=1;
}
//-------------------------------------------------------------------
//IIC----------------------------------------------------------------
void delay_IIC(void)
{//通讯延时大于30us
    unsigned char i;
	_nop_();
	_nop_();
	i = 80;
	while (--i);
}
void IIC_Init(void)
{//IIC总线初始化函数
    IIC_SDA=1;//释放IIC总线的数据线。
    IIC_SCL=1;//释放IIC总线的时钟线。
}
void IIC_start(void)
{//IIC总线产生起始信号函数  
     IIC_SDA=1;//拉高数据线
	 IIC_SCL=1;//拉高时钟线
	 delay_IIC();
	 IIC_SDA=0;//在时钟线为高电平时，拉低数据线，产生起始信号。
	 delay_IIC();
     IIC_SCL=0;//拉低时钟线
}
void IIC_stop(void) 
{//IIC总线产生停止信号函数
    IIC_SDA=0;//拉低数据线
	IIC_SCL=0;//拉低时钟线
    delay_IIC();
    IIC_SCL=1;//拉高时钟线。
    delay_IIC();
    IIC_SDA=1;//时钟时线为高电平时，拉高数据线，产生停止信号。
    delay_IIC();
}
bit IIC_Tack(void)
{//接收应答信号函数
    bit ack;//定义一个位变量，来暂存应答状态。
    IIC_SDA=1;//释放数据总线，准备接收应答信号。
    delay_IIC();
    IIC_SCL=1;//拉高时钟线。
    delay_IIC();
    ack=IIC_SDA;//读取应答信号的状态。
    delay_IIC();
    IIC_SCL=0;//拉低时钟线。
    delay_IIC();
    return ack;//返回应答信号的状态，0表示应答，1表示非应答。
}
void IIC_write_byte(unsigned char Data)
{//向IIC总线写入一个字节的数据函数 
	unsigned char i;
	 for(i=0;i<8;i++)//有8位数据
	{
		IIC_SDA=Data&0x80;//写最高位的数据
		delay_IIC();
		IIC_SCL=1; //拉高时钟线，将数写入到设备中。
		delay_IIC();
		IIC_SCL=0;//拉低时钟线，允许改变数据线的状态
		delay_IIC();
		Data=Data<<1;//数据左移一位，把次高位放在最高位,为写入次高位做准备
	}
}
unsigned char IIC_read_byte()
{//从IIC总线读取一个字节的数据函数
    unsigned char i;
    unsigned char Data;       //定义一个缓冲寄存器。
    for(i=0;i<8;i++)//有8位数据
    {
        IIC_SCL=1;//拉高时钟线，为读取下一位数据做准备。
        delay_IIC();
        Data=Data<<1;//将缓冲字节的数据左移一位，准备读取数据。
        delay_IIC();
        
        if(IIC_SDA)//如果数据线为高平电平。
            Data=Data|0x1;//则给缓冲字节的最低位写1。
        IIC_SCL=0;//拉低时钟线，为读取下一位数据做准备。
        delay_IIC();
    }
    return Data;//返回读取的一个字节数据。
}
void IIC_single_byte_write(unsigned char Daddr,unsigned char Waddr,unsigned char Data)
{//向任意地址写入一个字节数据函数
    IIC_start();//产生起始信号
    IIC_write_byte(Daddr);//写入设备地址（写）
    IIC_Tack();//等待设备的应答
    IIC_write_byte(Waddr);//写入要操作的单元地址。
    IIC_Tack();//等待设备的应答。

    IIC_write_byte(Data);//写入数据。
    IIC_Tack();//等待设备的应答。
    IIC_stop();//产生停止符号。
}
unsigned char IIC_single_byte_read(unsigned char Daddr,unsigned char Waddr)
{//从任意地址读取一个字节数据函数
    unsigned char Data;//定义一个缓冲寄存器。

    IIC_start();//产生起始信号
    IIC_write_byte(Daddr);//写入设备地址（写）
    IIC_Tack();//等待设备的应答
    IIC_write_byte(Waddr);//写入要操作的单元地址。
    IIC_Tack();//等待设备的应答。
    IIC_stop();//产生停止符号。

    IIC_start();//产生起始信号
    IIC_write_byte(Daddr+1);//写入设备地址（读）。
    IIC_Tack();//等待设备的应答。
    Data=IIC_read_byte();//写入数据。
    IIC_stop();//产生停止符号。
    //-------------------返回读取的数据--------------------
    return Data;//返回读取的一个字节数据。
}
//-------------------------------------------------------------------
//LCD1602------------------------------------------------------------
void delay_ms(unsigned int ms)
{
	unsigned int i,j;
	for(j=ms;j>0;j--)
		for(i=220;i>0;i--);
}
void LCD_CheckBusy(void)
{
	unsigned char i=255;
	DATA_OUT=0xFF;//读之前先置位，准备读取IO口数据
	RS=0;
	RW=1;//使液晶处于读数据状态
	EN=1;//使能液晶，高电平有效
	while((i--)&(DATA_OUT&0x80));//忙检测
	EN=0;
}
void LCD_Write(unsigned char ucData, bit bComOrData)
{
	LCD_CheckBusy();
	RS=bComOrData;
	DATA_OUT=ucData;
	RW=0;
	delay_ms(5);
	EN=1;
	delay_ms(5);
	EN=0;                
}
void LCD_Display(unsigned char *s)
{
	while(*s>0)
	{
		LCD_Write(*s,1);
		s++;
	}
}
void LCD_Init(void)
{
	LCD_Write(0x38,0);
	delay_ms(10);
	LCD_Write(0x0c,0); 
	delay_ms(10);
	LCD_Write(0x06,0);
	delay_ms(10);
	LCD_Write(0x01,0);
	delay_ms(10);
}
void LCD_Move(unsigned char x,unsigned char y) //光标位置
{
	if(x==0)
		LCD_Write((0x80 | y),0);
	if(x==1)
		LCD_Write((0xC0 | y),0);
}
//-------------------------------------------------------------------