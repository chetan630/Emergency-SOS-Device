#include <LPC17xx.h>
#include <stdio.h>

/* ===================== MACROS ===================== */
#define LCD_ADDR    0x27
#define ADXL_WRITE  0xA6     // ADXL345 I2C write address
#define ADXL_READ   0xA7     // ADXL345 I2C read address

/* ===================== DELAY ===================== */
void delay(unsigned long int d);

/* ===================== TIMER ===================== */
void delay_us(unsigned int microseconds);
void initTimer0(void);

/* ===================== I2C FUNCTIONS ===================== */
void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_write(unsigned char data);
unsigned char i2c_read(unsigned char ack);

/* ===================== ADXL345 ===================== */
void adxl345_init(void);                // Configure accelerometer
void ADXL345_SINGLE_TAP_CONF(void);     // Enable single-tap interrupt
void EINT3_IRQHandler(void);            // GPIO interrupt handler
void tri_read(void);                    // Clear ADXL interrupt source

/* ===================== LCD ===================== */
void lcd_write(unsigned char ch, unsigned char rs);
void lcd_init(void);
void lcd_print(char *str);
void lcd_set_cursor(unsigned char row, unsigned char col);

/* ===================== GPS ===================== */
char gprmc_valid(void);                 // Check GPRMC validity
void lat_log_ext(char *val);            // Extract latitude & longitude
void uart0_init(void);
void gps_call(void);

/* ===================== GSM ===================== */
void uart3_init(void);
void uart3_tx(char ch);
void uart3_tx_str(char *s);
void gsm_cmd(void);

/* ===================== GLOBAL VARIABLES ===================== */
unsigned int tag = 0;                   // Set by interrupt on fall detection
unsigned char test[128];                // Stores full GPRMC sentence
char lat[12];                           // Latitude string
char lon[12];                           // Longitude string
int count = 0, i = 0, j = 0;             // Parsing counters
unsigned char c, cha;

int main()
{   
  SystemInit();

  /* -------- UART0 (GPS) Pins -------- */
  LPC_PINCON->PINSEL0 |= 0x0000005A;   // P0.2 TXD0, P0.3 RXD0

  /* -------- Enable UART3 (GSM) -------- */
  LPC_SC->PCONP |= (1 << 25);

  /* -------- GPIO for External Interrupt -------- */
  LPC_GPIO0->FIOMASK0 = 0xFE;          // Enable P0.0
  LPC_GPIO0->FIODIR0  = 0x00;          // Input pin

  /* -------- LED Output -------- */
  LPC_GPIO1->FIOMASK3 = 0xDE;
  LPC_GPIO1->FIODIR3  = 0x20;           // P1.29 output

  /* -------- Disable Pull-up/down -------- */
  LPC_PINCON->PINMODE0 |= (3 << 0);

  /* -------- Enable Rising Edge Interrupt -------- */
  LPC_GPIOINT->IO0IntEnR |= (1 << 0);	
	
	/* -------- Peripheral Initialization -------- */
	initTimer0();
	uart0_init();
  i2c_init();
  delay_us(5); 	
  adxl345_init();
	ADXL345_SINGLE_TAP_CONF();
	lcd_init();
	
	LPC_GPIOINT -> IO0IntClr |= (1<<0); //CLEARING PENDING INTERRUPTS 
	NVIC_EnableIRQ(EINT3_IRQn);  // ENABLES EINT3 INTERRUPT IN NVIC, predefined funstions

	while (1){
		if(tag == 1)// Fall detected
		{ 
			LPC_GPIO1->FIOSET3 = 0x20;   // LED ON
			lcd_set_cursor(0, 0);
      lcd_print("Detected");
			
			delay_us(6000000);
			
			gps_call(); // Read GPS & extract location
			gsm_cmd(); // Send SMS
			tag =0;	// Reset flag
			tri_read();// Clear ADXL interrupt
		}
		else{
			LPC_GPIO1->FIOCLR3 = 0x20; // LED OFF
			lcd_set_cursor(0, 0);
      lcd_print("Normal");
			tri_read();// Clear ADXL interrupt
		}
	}			
}

/* ===================== INTERRUPT HANDLER ===================== */
void EINT3_IRQHandler(void)
{
	LPC_GPIOINT->IO0IntClr = (1<<0);  // Clear interrupt to avoid conflict
	tag = 1;
	NVIC_EnableIRQ(EINT3_IRQn);  // ENABLES EINT3 INTERRUPT IN NVIC, predefined funstions
}

/* ===================== TIMER FUNCTIONS ===================== */
void initTimer0(){
	LPC_TIM0->CTCR =0X00;
	LPC_TIM0->PR = 0x11; // Increment TC on every 18MHZ
	LPC_TIM0->TCR = 0X02; //Reset Timer
}

void delay_us(unsigned int microseconds){
	LPC_TIM0->TCR = 0X02; //Reset Timer
	LPC_TIM0->TCR = 0X01; // Enable Timer
	while(LPC_TIM0->TC < microseconds);
	LPC_TIM0->TCR =0X00;
}

void delay(unsigned long int di)
{
	unsigned long int i;
	for (i=0; i<di; i++);
}


/* ---------------- I2C FUNCTIONS ---------------- */
void i2c_init(void)
{
    LPC_SC->PCONP |= 0x00000080; 
    LPC_PINCON->PINSEL1 = 0x01400000;  // P0.27 SDA0, P0.28 SCL0
    LPC_I2C0->I2SCLH = 0x0000005A; // 90
    LPC_I2C0->I2SCLL = 0x0000005A; //90
    LPC_I2C0->I2CONSET = 0x00000040;   // Enable I2C 0100 0000
}

void i2c_start(void)
{
    LPC_I2C0->I2CONSET = 0x00000020; //0010 0000 START FLAG
    while (LPC_I2C0->I2STAT != 0x08); //0000 1000 IF SI BIT SET HIGH THEN I2STAT BIT IS 0X08
    LPC_I2C0->I2CONCLR = 0x00000028; //0010 1000 START BIT, SI 
}

void i2c_stop(void)
{
    LPC_I2C0->I2CONSET = 0x00000010; //0001 0000 STOP FLAG
    LPC_I2C0->I2CONCLR = 0x00000008; //0010 1000 START BIT, SI 
    delay(0x2000);
}

void i2c_write(unsigned char data)
{
		LPC_I2C0->I2DAT = data; //7:0 DATA
    LPC_I2C0->I2CONCLR = 0x00000008; //0010 1000 START BIT, SI 
    while ((LPC_I2C0->I2CONSET & 0x08) == 0);// CHECKING SI BIT CLEARED OR NOT
}

unsigned char i2c_read(unsigned char ack)
{
    if (ack) // from slave if gets same adress it sends ack 
        LPC_I2C0->I2CONSET = 0x00000004; //0100 AA  set 
    else
        LPC_I2C0->I2CONCLR = 0x00000004; // AA clear 

    LPC_I2C0->I2CONCLR = 0x00000008; // 1000 SI clear 
    while ((LPC_I2C0->I2CONSET & 0x08) == 0x00); //SI bit cleared or not
    return LPC_I2C0->I2DAT;
}

/* ---------------- ADXL345 FUNCTIONS ---------------- */
void adxl345_init(void)
{
    // DATA_FORMAT (0x31): a2g, full resolution
    i2c_start();
    i2c_write(ADXL_WRITE);
    i2c_write(0x31);//DATA FORMAT REG ADDRESS 
    i2c_write(0x08); // 0000, 1,0,00  ,FULL RESULUTION,JUSTIFY(MSB LSB),RANGE 
    i2c_stop();
    delay(0x5000);

    // BW_RATE (0x2C): 100Hz
    i2c_start();
    i2c_write(ADXL_WRITE);
    i2c_write(0x2C);//BW RATE REG ADRESS 
    i2c_write(0x0A); // 000,0 ,1010 LOW POWER MODE,BW RATE 100Hz 
    i2c_stop();
    delay(0x5000);

    // POWER_CTL (0x2D): Measure mode
    i2c_start();
    i2c_write(ADXL_WRITE);
    i2c_write(0x2D); // POWER CONTROL REG ADRESS 
    i2c_write(0x08); // 0000 ,1,000 MESURE MODE 
    i2c_stop();
    delay(0x10000);
}

void tri_read(void){
	i2c_start();
	i2c_write(ADXL_WRITE);
	i2c_write(0x30);   // INT_SOURCE
	i2c_stop();

	i2c_start();
	i2c_write(ADXL_READ);
	i2c_read(0);       // read clears interrupt
	i2c_stop();
}

void ADXL345_SINGLE_TAP_CONF(void){
	
	i2c_start();
  i2c_write(ADXL_WRITE);
  i2c_write(0x1D); // THRESH_TAP
  i2c_write(0x30); // 0000 ,1,000 MESURE MODE 
  i2c_stop();
  delay(0x10000);
	
	i2c_start();
  i2c_write(ADXL_WRITE);
  i2c_write(0x21); // DUR
  i2c_write(0x20); // 0000 ,1,000 MESURE MODE 
  i2c_stop();
  delay(0x10000);
	
	i2c_start();
  i2c_write(ADXL_WRITE);
  i2c_write(0x2A); // TAP_AXIS
  i2c_write(0x07); // en single tap 
  i2c_stop();
  delay(0x10000);
	
	i2c_start();
  i2c_write(ADXL_WRITE);
  i2c_write(0x2F); // INT_MAP
  i2c_write(0x00); // INT1
  i2c_stop();
  delay(0x10000);
	
	i2c_start();
  i2c_write(ADXL_WRITE);
  i2c_write(0x2E); // 
  i2c_write(0x40); // INT1
  i2c_stop();
  delay(0x10000);
	
}

/* ---------------- LCD FUNCTIONS ---------------- */
void lcd_init(void)
{
    unsigned char cmd[] = {0x33, 0x32, 0x28, 0x0E, 0x01, 0x06, 0x80};
    unsigned char i;
    delay(0x10000);
    for (i = 0; i < 7; i++)
    {
        lcd_write(cmd[i], 0);
        delay(0x5000);
    }
}

void lcd_print(char *str)
{
    while (*str)
    {
        lcd_write(*str++, 1);
        delay(0x1000);
    }
}

void lcd_set_cursor(unsigned char row, unsigned char col)
{
    unsigned char pos;
    pos = (row == 0) ? (0x80 + col) : (0xC0 + col);
    lcd_write(pos, 0);
}

void lcd_write(unsigned char ch, unsigned char rs)
{
    unsigned char high_nib, low_nib;
    unsigned char data;

    high_nib = ch & 0xF0;
    low_nib = (ch << 4) & 0xF0;

    data = high_nib | (rs ? 0x01 : 0x00) | 0x08;
    i2c_start();
    i2c_write(LCD_ADDR << 1);
    i2c_write(data | 0x04);
    delay(0x1000);
    i2c_write(data & ~0x04);
    delay(0x10000);
    i2c_stop();

    data = low_nib | (rs ? 0x01 : 0x00) | 0x08;
    i2c_start();
    i2c_write(LCD_ADDR << 1);
    i2c_write(data | 0x04);
    delay(0x1000);
    i2c_write(data & ~0x04);
    delay(0x10000);
    i2c_stop();

    delay(0x3000);
}

/* ===================== GPS FUNCTIONS ===================== */
char gprmc_valid(void){
	/* -------- Wait for '$' -------- */
	do {
		while ((LPC_UART0->LSR & 0x01) == 0);
		cha = LPC_UART0->RBR; 
		i++;
	}
	while(cha != '$');
							
	/* -------- Store Sentence -------- */	i=0;
	do {
		while ((LPC_UART0->LSR & 0x01) == 0);
		cha = LPC_UART0->RBR;
		test[i++] = cha;
	} while (cha != '\r' && i < 127);
							
	test[i] = '\0';
	/* -------- Check Status Field -------- */
	i=0;
	while(test[i] != '\0'){
		if (test[i] == ',') {
			count++;
			if (count == 2) {
				c = test[i+1];
				return c;
			}
		}		
		i++;
	}
	return 'V';
}

void lat_log_ext(char *val)
{
    i = count = j = 0;

    while (val[i] != '\0') {
        if (val[i] == ',') {
            count++;

            // -------- Latitude (3rd field) --------
            if (count == 3) {
                j = 0;
                i++;
                while (val[i] != ',' && val[i] != '\0') {
                    lat[j++] = val[i++];
                }
                lat[j] = '\0';
								continue;
            }

            // -------- Longitude (5th field) --------
            if (count == 5) {
                j = 0;
                i++;
                while (val[i] != ',' && val[i] != '\0') {
                    lon[j++] = val[i++];
                }
                lon[j] = '\0';
								continue;
            }
        }
        i++;
    }

    // Fail-safe
    if (count < 5) {
        lat[0] = '\0';
        lon[0] = '\0';
    }
}

void uart0_init(void)
{
	LPC_UART0->LCR = 0x83;     // 8-bit, DLAB=1
	LPC_UART0->DLM = 0x00;
	LPC_UART0->DLL = 0x75;     // Baud rate 9600 @18MHz PCLK
	LPC_UART0->FDR = 0x10;
	LPC_UART0->LCR = 0x03;     // DLAB=0
}

void gps_call(void){
	// reset all control variables for next frame
	i = j = count = c = 0;
	memset(test, 0, sizeof(test));
	memset(lat, 0, sizeof(lat));
	memset(lon, 0, sizeof(lon));
	
	gprmc_valid();
	
	if (c=='A' && test[2] == 'R'){
		lat_log_ext(test);
	}
}

/* ===================== GSM FUNNCTIONS ===================== */
void uart3_init(void)
{
    LPC_UART3->LCR = 0x83;
    LPC_UART3->DLM = 0x00;
    LPC_UART3->DLL = 0x75;   // 9600 baud @ 18MHz
    LPC_UART3->FDR = 0x10;
    LPC_UART3->LCR = 0x03;
}

void uart3_tx(char ch)
{
    while (!(LPC_UART3->LSR & 0x20));
    LPC_UART3->THR = ch;
}

void uart3_tx_str(char *s)
{
    while (*s)
        uart3_tx(*s++);
}

void gsm_cmd(){

  uart3_tx_str("AT\r\n");
  delay_us(800000);

  uart3_tx_str("AT+CPIN?\r\n");
  delay_us(800000);
  
	uart3_tx_str("AT+CREG?\r\n");
  delay_us(800000);

  uart3_tx_str("AT+CMGF=1\r\n");
  delay_us(800000);

  uart3_tx_str("AT+CMGS=\"YOUR_NUMBER\"\r\n");
  delay_us(1500000);   // WAIT for '>'

  uart3_tx_str(lat);
	uart3_tx_str(lon);
  uart3_tx(0x1A);
}


