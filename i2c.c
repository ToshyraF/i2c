

/*
 * I2C.c
 *
 * Created: 23/2/2560 10:15:20
 * Author : Bank
 */ 
#define LCD_RS	0
#define LCD_RW	1
#define LCD_LED	3
#define LCD_EN	2
#define LCD_Clear  0b00000001
#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#define LCD_SetCursor	0b10000000	// set cursor position

void I2C_Start();
void I2C_Write(uint8_t dat);
void I2C_Stop(void);
void I2C_AddressR_W(uint8_t addr, uint8_t r_w);
void LCD_CMNDWRT41(uint8_t addr, uint8_t cmnd);
void LCD_CMNDWRT42(uint8_t addr, uint8_t cmnd);
void LCD_DATWRT42(uint8_t addr, uint8_t dat);
void LCD_CLS(uint8_t addr);
uint8_t swap(uint8_t cmnd);
void LCD_gotoxy(uint8_t addr,uint8_t col, uint8_t row);
void LCD_Print(uint8_t addr, char * str);

//////////////////////// I2C//////////////////////////
void I2C_Init(void) {
	TWSR = 0x00;  //set prescaler to zero
	TWBR = 12;    //set clock 16 MHz
	TWCR = 0x04;  //enable TWI module
}

void I2C_Start(void) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while ((TWCR & (1 << TWINT)) == 0);
}
void I2C_Stop(void)
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	while ((TWCR & (1 << TWINT)) == 0x80);
}

void I2C_Write(uint8_t dat)
{
	TWDR = dat ;
	TWCR = ((1<< TWINT) | (1<<TWEN));
	while (!(TWCR & (1 <<TWINT)));
}

void I2C_AddressR_W(uint8_t addr, uint8_t r_w){
	addr = (addr << 1)| r_w;
	TWDR = addr;
	TWCR = ((1<< TWINT) | (1<<TWEN));
	while (!(TWCR & (1 <<TWINT)));
}
///////////////////////////////////////////////////////////////////////


////////////////////////////////LCD////////////////////////////////////
void LCD_gotoxy(uint8_t addr, uint8_t col, uint8_t row)
{
	if(row ==2){
		LCD_CMNDWRT42(addr,0xC0);
	}
	for(int i=1;i<col;i++){
		LCD_DATWRT42(addr,LCD_SetCursor);
	}
}

void Init_LCD4D(uint8_t addr){
	_delay_ms(50);
	LCD_CMNDWRT41(addr, 0x30);
	_delay_ms(4.1);
	
	LCD_CMNDWRT41(addr, 0x30);
	_delay_us(100);
	
	LCD_CMNDWRT41(addr, 0x30);
	_delay_us(100);
	
	LCD_CMNDWRT41(addr, 0x20);
	_delay_us(100);
	
	LCD_CMNDWRT42(addr, 0x28);
	_delay_us(100);
	
	LCD_CMNDWRT42(addr, 0x0C);
	_delay_us(100);
	
	LCD_CLS(addr);
	_delay_us(100);
	
	LCD_CMNDWRT42(addr, 0x06);
	_delay_us(100);
	
}
void LCD_DATWRT42(uint8_t addr, uint8_t dat){
	uint8_t data = dat;
	data = data & 0xF0;
	data |= (1<<LCD_RS);	// led,en,rw,rs = 1101
	data |= (1<<LCD_EN);	// led,en,rw,rs = 1101
	data |= (1<<LCD_LED);   // led,en,rw,rs = 1101
	I2C_Start();            // Start I2C communication
	I2C_AddressR_W(addr, 0);
	I2C_Write(data);
	_delay_us(1);
	data &= 0xF0;
	data &= ~(1<<LCD_EN);//set EN = 0// led,en,rw,rs = 1001
	I2C_Write(data);
	I2C_Stop();
	
	data = swap(dat);
	data = data & 0xF0;
	data |= (1<<LCD_RS);	// led,en,rw,rs = 1101
	data |= (1<<LCD_EN);	// led,en,rw,rs = 1101
	data |= (1<<LCD_LED);   // led,en,rw,rs = 1101
	I2C_Start();            // Start I2C communication
	I2C_AddressR_W(addr, 0);
	I2C_Write(data);
	_delay_us(1);
	data &= ~(1<<LCD_EN);//set EN = 0 // led,en,rw,rs = 1001
	I2C_Write(data);
	I2C_Stop();
	_delay_us(100);
}

void LCD_CLS(uint8_t addr){
	LCD_CMNDWRT42(addr,LCD_Clear);
	_delay_ms(2);
}

void LCD_CMNDWRT41(uint8_t addr, uint8_t cmnd){
	uint8_t command = cmnd;
	command = command & 0xF0;
	command |= (1<<LCD_EN);	// led,en,rw,rs = 1100
	command |= (1<<LCD_LED); // led,en,rw,rs = 1100
	I2C_Start();            // Start I2C communication
	I2C_AddressR_W(addr,0);  // write address
	I2C_Write(command);
	command &= ~(1<<LCD_EN); // led,en,rw,rs = 1000
	I2C_Write(command);
	I2C_Stop();             // Stop I2C communication
}

uint8_t swap(uint8_t cmnd){
	unsigned command = cmnd;
	unsigned temp = command & 0xF0;
	temp = temp >> 4;
	command = command << 4;
	command |= temp;
	return command;
}

void LCD_CMNDWRT42(uint8_t addr, uint8_t cmnd){
	uint8_t command = cmnd;
	LCD_CMNDWRT41(addr, command);
	
	command = swap(cmnd);
	command = command & 0xF0;
	command |= (1<<LCD_EN);	// led,en,rw,rs = 1100
	command |= (1<<LCD_LED); // led,en,rw,rs = 1100
	I2C_Start();            // Start I2C communication
	I2C_AddressR_W(addr, 0);
	I2C_Write(command);
	_delay_us(1);
	command &= ~(1<<LCD_EN); // led,en,rw,rs = 1000
	I2C_Write(command);
	I2C_Stop();
	_delay_us(100);
}
void LCD_Print(uint8_t addr, char * str)
{
	while(*str)
	{
		LCD_DATWRT42(addr, *str);
		str++;
	}
}
////////////////////////////////////////////////////////////////////////

int main(void)
{
	// lcd 1 display
	uint8_t lcd1_addr = 0x27;
	I2C_Init();             // Initialize the I2c module.
	Init_LCD4D(lcd1_addr);
	LCD_CLS(lcd1_addr);
	LCD_gotoxy(lcd1_addr, 1, 1);
	LCD_Print(lcd1_addr, "LCD1 Address0x27");
	LCD_gotoxy(lcd1_addr, 4, 2);
	LCD_Print(lcd1_addr, "K.Thanawat");
	
	//lcd 2 display
	uint8_t lcd2_addr = 0x26;
	Init_LCD4D(lcd2_addr);
	LCD_CLS(lcd2_addr);
	LCD_gotoxy(lcd2_addr, 1, 1);
	LCD_Print(lcd2_addr, "LCD2 Address0x26");
	LCD_gotoxy(lcd2_addr, 4, 2);   
	LCD_Print(lcd2_addr,"W.Jirayud");
	
	while (1){}
}
