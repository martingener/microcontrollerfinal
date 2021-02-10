#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include <p18f4620.h>
#include <usart.h>
#pragma config OSC      =   INTIO67
#pragma config BOREN    =   OFF
#pragma config WDT      =   OFF
#pragma config LVP      =   OFF
#pragma config CCP2MX   =   PORTBE
#define _XTAL_FREQ          8000000
#define DS3231_ID       0x68
#define DS3231_Add_00   0x00
#define DS3231_Add_07   0x07
#define DS3231_Add_0E   0x0E
#define DS3231_Add_0F   0x0F
#define DS3231_Add_11   0x11
#define ACCESS_CFG      0xAC
#define START_CONV      0xEE
#define READ_TEMP       0xAA
#define CONT_CONV       0x02
#define ACK             1
#define NAK             0
#define ON              1
#define OFF             0
#define Key_Up          0x00ffe21d
#define Key_Dn          0x00ffa25d
#define Key_Done        0x00ffc23d
#define Key_Abort       0x00ff22dd
#define Key_Next        0x00ff02fd
#define Key_EQ          0x00ff906f
#define Key_Plus        0x00ffa857
#define Key_Minus       0x00ffe01f

//  MAIN SCREEN & TIME SETUP SCREEN
#define start_x             1
#define start_y             2
#define temp_x              28
#define temp_y              13
#define circ_x              40
#define circ_y              26
#define data_tempc_x        15
#define data_tempc_y        26
#define tempc_x             45
#define tempc_y             26
#define cirf_x              95
#define cirf_y              26
#define data_tempf_x        70
#define data_tempf_y        26
#define tempf_x             100
#define tempf_y             26
#define time_x              50
#define time_y              46
#define data_time_x         15
#define data_time_y         56
#define date_x              50
#define date_y              76
#define data_date_x         15
#define data_date_y         86
#define alarm_x             10
#define alarm_y             107
#define data_alarm_x        14
#define data_alarm_y        118
#define LED_Int_x           50
#define LED_Int_y           107
#define data_LED_Int_x      50
#define data_LED_Int_y      118     
#define HTR_x               85
#define HTR_y               107
#define data_HTR_x          95
#define data_HTR_y          118
#define HTR_S_x             15
#define HTR_S_y             130    
#define data_HTR_S_x        46
#define data_HTR_S_y     144
#define cirf_HTR_S_x     73
#define cirf_HTR_S_y     144      
#define tempf_HTR_S_x     78
#define tempf_HTR_S_y     144    
  
//  Setup Heater SCREEN    
#define S_HTR_S_x           15
#define S_HTR_S_y           80    
#define S_data_HTR_S_x     46
#define S_data_HTR_S_y     94
#define S_cirf_HTR_S_x     73
#define S_cirf_HTR_S_y     94      
#define S_tempf_HTR_S_x     78
#define S_tempf_HTR_S_y     94 
    
#define TS_1            1               // Size of Normal Text
#define TS_2            2               // Size of Number Text
void interrupt high_priority chkisr() ;
void INT0_isr(void);
void INT1_isr(void);
void INT2_isr(void);
void TIMER1_isr(void);
void Do_Setup_Time();
void Do_Setup_Alarm_Time(void);
void Do_Setup_Heater_Temp(void);
void Initialize_Main_Screen(void);
void Do_Update_Main_Screen(void);
void Initialize_Setup_Time_Screen(void);
void Update_Setup_Time_Screen(void);
void Initialize_Setup_Alarm_Screen(void);
void Update_Setup_Alarm_Screen(void);
void Update_Setup_Screen_Cursor(char);
void Initialize_Setup_Heater_Screen(void);
void Update_Setup_Heater_Screen(void);
void DS3231_Init(char);
void DS3231_Read_Time(char);
void DS3231_Write_Time(char);
int  DS3231_Read_Temp(char);
void DS3231_Read_Alarm_1_Time(char);
void DS3231_Write_Alarm_1_Time(char);
void DS3231_Turn_Off_Alarm(char device);
void DS3231_Turn_On_Alarm(char device);
void Do_LED_Up(void);
void Do_LED_Down(void);
void delay_ms(int ms);
void gen_2khz_sound();
void turn_off_sound();
int  bcd_2_dec(char);
int  dec_2_bcd(char);
unsigned int Get_Full_ADC(void);
void Select_ADC(char);
void  SPI_out(unsigned char);

unsigned int get_full_ADC(void);
float Read_Volt_In(void);
void Do_LED_Out(char volt);
void LED_Alarm(void);

#define D1RED PORTDbits.RD0
#define D1GREEN PORTDbits.RD1
#define D1BLUE PORTDbits.RD2

#define TFT_DC          PORTEbits.RE0
#define TFT_CS          PORTEbits.RE1
#define TFT_RST         PORTEbits.RE2
#define PULSE           PORTAbits.RA5
                                
#define SCL_PIN         PORTBbits.RB5
#define SCL_DIR         TRISBbits.RB5
#define SDA_PIN         PORTBbits.RB4
#define SDA_DIR         TRISBbits.RB4
#define alarm_sw        PORTAbits.RA4                                           // put your port assigment here from the schematics
#define heater_sw       PORTAbits.RA5                                           // put your port assigment here from the schematics
#define Chip_Select     PORTCbits.RC1                                           // put your port assigment here from the schematics
#define heater_on_led   PORTCbits.RC0                                           // put your port assigment here from the schematics
#define BEEP            PORTBbits.RB3
char buffer[31] = " ECE3301L Spring 2019\0";
char *nbr;
char *txt;
char tempC[]            = "25";
char tempF[]            = "77";
char time[]             = "00:00:00";
char date[]             = "01/01/00";
char alarm_text[]       = "OFF";
char LED_Int_text[]     = "00";
char heater_text[]      = "OFF";   
char heater_S_text[] = "60";
char setup_time[]       = "00:00:00";
char setup_date[]       = "01/01/00";
char setup_alarm_time[] = "00:00:00"; 
    
char setup_heater_S_text[]  = "60";
    
char nec_state, nec_ok;
unsigned long long nec_code;
unsigned char cnt, second, minute, hour, dow, day, month, year, old_second;
unsigned char alarm_second, alarm_minute, alarm_hour, alarm_date;
unsigned char setup_alarm_second, setup_alarm_minute, setup_alarm_hour;
unsigned char setup_second, setup_minute, setup_hour, setup_day, setup_month, setup_year;
unsigned char alarm_mode, alarm_match_flag, alarm_clear_flag, alarm_on_flag;
unsigned char heater_mode;
char contr;
int DS3231_tempC, DS3231_tempF;
unsigned char set_heater_temp, setup_heater_temp;
float Volt;
unsigned char LED_Intensity;
int bit_count;
int count = 0;
int alarm_count = 0;
void interrupt high_priority chkisr()    //hardware flag
{
    if (INTCONbits.INT0IF == 1) INT0_isr();
    if (INTCON3bits.INT1IF == 1) INT1_isr();
    if (INTCON3bits.INT2IF == 1) INT2_isr();
    if (PIR1bits.TMR1IF == 1) TIMER1_isr();
}
void INT0_isr(void)
{
    INTCONbits.INT0IF = 0;
    alarm_on_flag = 0;
    turn_off_sound();
    PORTD = 0;
    DS3231_Turn_Off_Alarm(DS3231_ID);
    INTCON3bits.INT1IE = 0;
    
}
void INT1_isr(void)
{
    INTCON3bits.INT1IF = 0;
    if (alarm_mode == 1) alarm_on_flag = 1;
    else alarm_on_flag = 0;
}
void INT2_isr(void)
{
unsigned int timer;    
 INTCON3bits.INT2IF = 0;                                                        // Clear external interrupt 
 
 if (nec_state != 0)   
 {      
  timer = (TMR1H << 8) | TMR1L;                                                 // Store Timer1 value      
  TMR1H = 0;                                                                    // Reset Timer1     
  TMR1L = 0;      
 }   
 
 switch(nec_state)    
 {      
  case 0 : 
    TMR1H = 0;
    TMR1L = 0;
    T1CON = 0x90;                                                               // Timer mode with count
    T1CONbits.TMR1ON = 1;            
    bit_count = 0;                                                              // Force the bit count to be 0
    nec_code = 0;
    INTCON2bits.INTEDG2 = 1;                                                    // 1 is rising edge 
    nec_state = 1;
    return;
        
  case 1 :
    if ((timer >= 8500) && (timer <= 9500))
    {
     INTCON2bits.INTEDG2 = 0;                                                   // 0 is falling edge  (high to low)
     nec_state = 2;
    }
            
    else  nec_state = 0;
    return;
            
  case 2 :
    if (( timer >= 4000) && (timer <= 5000))
    {
     INTCON2bits.INTEDG2 = 1;                                                   // 1 is rising edge (low to high)
     nec_state = 3;
    }
            
    else nec_state = 0;
    return;
        
  case 3: 
    if ((timer >= 400) && (timer <= 700))
    {
     INTCON2bits.INTEDG2 = 0;                                                   // 1 is rising edge (high to low))
     nec_state = 4;
    }
     
    else nec_state = 0;
    return;
            
  case 4:
    if((timer >= 400) && (timer <= 1800))
    {
     nec_code = nec_code << 1;
     if( timer > 1000)
     {
      nec_code = nec_code + 1;
     }
     bit_count = bit_count + 1;
                
     if(bit_count > 31)
     {
      nec_ok = 1;
      INTCON3bits.INT2IE = 0;              
     }
            
     else
     {
      INTCON2bits.INTEDG2 = 1;                                                  // high to low rising edge
      nec_state = 3;
     }
    }
    
    else nec_state = 0;
    return;          
   } 
}
void TIMER1_isr(void)
{
    nec_state = 0;                                                                 // Resets the decoding process   
    INTCON2bits.INTEDG2 = 0;                                                       // INT2 falling edge  
    T1CONbits.TMR1ON=0;                                                            // Disables the T1 Timer  
    PIR1bits.TMR1IF=0;                                                             // Clears the interrupt flag 
}
#include "ST7735_TFT.inc"
#include "softi2c.inc"

void putch (char c)
{   
    while (!TRMT);       
    TXREG = c;
}
void init_UART()
{
    OpenUSART (USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 25);
    OSCCON = 0x70;
}
void main()
{
    init_UART();
    OSCCON = 0x70;
    ADCON1 = 0x0b;                       //
    nRBPU = 0;                           // Enable PORTB internal pull up resistor
    TRISA = 0x3f;
    TRISB = 0x07;
    TRISC = 0x00;
    TRISD = 0x00;
    TRISE = 0x00;  
    CCP2CON = 0x0;
    
    
    INTCONbits.INT0IF  = 0;             // INT0 IF is in INTCON
    INTCON3bits.INT1IF = 0;             // INT1 IF is in INTCON3
    INTCON3bits.INT2IF = 0;             // Clear external interrupt
    
    INTCONbits.INT0IE  = 1;             // INT0 IE is in INTCON
    INTCON3bits.INT1IE = 1;             // INT1 IE is in INTCON3
    INTCON3bits.INT2IE = 1;             // INT2 IE is in INTCON3
   
    
    INTCON2bits.INTEDG0 = 0;            // Edge programming for INT0, INT1 and 
    INTCON2bits.INTEDG1 = 0;            // INT2 are in INTCON2
    INTCON2bits.INTEDG2 = 0;            // Edge programming for INT2 falling edge   
    
    INTCONbits.PEIE = 1;                // Enable Peripheral interrupt
    INTCONbits.GIE = 1;
// enable global interrupt
     
    I2C_Init(100000);                   // Initialize I2C Master with 100KHz clock 
    txt = buffer;    
    LCD_Reset();                        // Screen reset
    TFT_GreenTab_Initialize();     
    DS3231_Init(DS3231_ID);
    set_heater_temp = 60;
    
    Initialize_Main_Screen();
    
    TMR1L = 0;
    TMR1H = 0;
    T1CON = 0x83;
    PIR1bits.TMR1IF=0;                   // Clear interrupt flag
    PIE1bits.TMR1IE = 1;                 // Enable Timer 1 interrupt    
    nec_ok = 0;  
    nec_code = 0x0;
    alarm_mode = 0;
    heater_mode = 0;    
    alarm_match_flag = 0;
    LED_Intensity = 0;
    SPI_out(LED_Intensity + 100);
    PORTD = 0;
    
    BEEP = 0;
    
    while(TRUE)
    {     
        DS3231_Read_Time(DS3231_ID);
        if (second != old_second)
        {
            alarm_mode = alarm_sw;
            if (alarm_mode == 1)
                DS3231_Turn_On_Alarm(DS3231_ID);
            if (alarm_mode == 0)
                DS3231_Turn_Off_Alarm(DS3231_ID);
            heater_mode = heater_sw;

            DS3231_tempC = DS3231_Read_Temp(DS3231_ID);                
            Do_Update_Main_Screen();
            
            if((DS3231_tempF < set_heater_temp) && (heater_mode == 1))
                heater_on_led = ON;
            else
                heater_on_led = OFF;
            
          
            printf ("Time: %02x:%02x:%02x  Date: %02x:%02x:%02x  ",hour,minute,second,month,day,year);
            printf ("Temp: %2d C %2d F\r", DS3231_tempC,DS3231_tempF);
            old_second = second;                  
            /*if (alarm_on_flag == 1 && alarm_mode == 1)
            {
                gen_2khz_sound();
                LED_Alarm();
            }*/
            if (alarm_on_flag == 1 && alarm_mode == 1)
            {
                gen_2khz_sound();
                LED_Alarm();
            }
         
        }
        
        Read_Volt_In();
        Do_LED_Out(Volt);
        printf("\rVolt = %f", Volt);
            
        if (nec_ok == 1)
        {       
            INTCON3bits.INT2IE = 1;                 // Enable external interrupt
            INTCON2bits.INTEDG2 = 0;                // Edge programming for INT0 falling edge
            nec_ok = 0;
            if (nec_code == Key_EQ) Do_Setup_Time();
            if (nec_code == Key_Plus) Do_Setup_Alarm_Time();
            if (nec_code == Key_Minus) Do_Setup_Heater_Temp();
            if (nec_code == Key_Up) Do_LED_Up();
            if (nec_code == Key_Dn) Do_LED_Down();
        } 
        
    }
}
void Initialize_Main_Screen(void) 
{ 
    fillScreen(ST7735_BLACK);                                    // Fills background of screen with color passed to it
 
    strcpy(txt, " ECE3301L Spring 2019\0");                                 // Text displayed 
    drawtext(start_x , start_y, txt, ST7735_WHITE  , ST7735_BLACK, TS_1);   // X and Y coordinates of where the text is to be displayed
    strcpy(txt, "Temperature:");
    drawtext(temp_x  , temp_y , txt, ST7735_MAGENTA, ST7735_BLACK, TS_1);                                                                                               // including text color and the background of it
    drawCircle(circ_x, circ_y , 2  , ST7735_YELLOW);
    strcpy(txt, "C/");
    drawtext(tempc_x , tempc_y, txt, ST7735_YELLOW , ST7735_BLACK, TS_2); 
    drawCircle(cirf_x, cirf_y , 2  , ST7735_YELLOW);
    strcpy(txt, "F");         
    drawtext(tempf_x , tempf_y, txt, ST7735_YELLOW , ST7735_BLACK, TS_2);
    strcpy(txt, "Time");
    drawtext(time_x  , time_y , txt, ST7735_BLUE   , ST7735_BLACK, TS_1);
    strcpy(txt, "Date");
    drawtext(date_x  , date_y , txt, ST7735_RED    , ST7735_BLACK, TS_1);
    strcpy(txt, "ALARM");
    drawtext(alarm_x , alarm_y, txt, ST7735_WHITE   , ST7735_BLACK, TS_1);
    strcpy(txt, "LED");
    drawtext(LED_Int_x, LED_Int_y, txt, ST7735_WHITE   , ST7735_BLACK, TS_1);
    strcpy(txt, "Heater");
    drawtext(HTR_x   , HTR_y  , txt, ST7735_WHITE   , ST7735_BLACK, TS_1);
    strcpy(txt, "Heater Set Temp");
    drawtext(HTR_S_x , HTR_S_y, txt, ST7735_YELLOW  , ST7735_BLACK, TS_1);
    drawCircle(cirf_HTR_S_x, cirf_HTR_S_y , 2  , ST7735_BLUE);
    strcpy(txt, "F");         
    drawtext(tempf_HTR_S_x , tempf_HTR_S_y, txt, ST7735_BLUE , ST7735_BLACK, TS_2);
}
void Do_Update_Main_Screen(void)
{
    DS3231_tempC = DS3231_Read_Temp(DS3231_ID);
    DS3231_tempF = DS3231_tempC*9/5 + 32;
    tempC[0]  = DS3231_tempC/10  + '0';
    tempC[1]  = DS3231_tempC%10  + '0';
    tempF[0]  = DS3231_tempF/10  + '0';
    tempF[1]  = DS3231_tempF%10  + '0';
    time[0]  = (hour>>4)  + '0';
    time[1]  = (hour & 0x0f)+ '0';  
    time[3]  = (minute>>4)  + '0';
    time[4]  = (minute & 0x0f)+ '0';
    time[6]  = (second>>4)  + '0';
    time[7]  = (second & 0x0f)+ '0'; 
    date[0]  = (month>>4)  + '0';
    date[1]  = (month & 0x0f)+ '0';  
    date[3]  = (day>>4)  + '0';
    date[4]  = (day & 0x0f)+ '0';
    date[6]  = (year>>4)  + '0';
    date[7]  = (year & 0x0f)+ '0';
    
    if (alarm_mode == 1) 
    {   
     
        strcpy(alarm_text, "ON ");
        
    }
    else 
    {
        strcpy(alarm_text, "OFF");
    }
    
    LED_Int_text[0]  = LED_Intensity/10  + '0';
    LED_Int_text[1]  = LED_Intensity%10  + '0';
    
    if (heater_mode == 1)
    {
        strcpy(heater_text, "ON ");
        heater_S_text[0]  = set_heater_temp/10  + '0';
        heater_S_text[1]  = set_heater_temp%10  + '0';
    
    }
    else 
    {
        strcpy(heater_text, "OFF");
        heater_S_text[0]  = '-';
        heater_S_text[1]  = '-';
    }
    
    drawtext(data_tempc_x, data_tempc_y , tempC , ST7735_YELLOW , ST7735_BLACK , TS_2);       
    drawtext(data_tempf_x, data_tempf_y , tempF , ST7735_YELLOW , ST7735_BLACK , TS_2);
    drawtext(data_time_x , data_time_y  , time  , ST7735_CYAN   , ST7735_BLACK , TS_2);
    drawtext(data_date_x , data_date_y  , date  , ST7735_GREEN  , ST7735_BLACK , TS_2);
    drawtext(data_alarm_x, data_alarm_y , alarm_text, ST7735_MAGENTA, ST7735_BLACK, TS_1);
    drawtext(data_LED_Int_x, data_LED_Int_y , LED_Int_text, ST7735_MAGENTA, ST7735_BLACK, TS_1);    
    drawtext(data_HTR_x  , data_HTR_y   , heater_text, ST7735_MAGENTA, ST7735_BLACK, TS_1);
    drawtext(data_HTR_S_x, data_HTR_S_y , heater_S_text, ST7735_BLUE, ST7735_BLACK, TS_2);
}
void Initialize_Setup_Time_Screen(void) 
{ 
    fillScreen(ST7735_BLACK);                                    // Fills background of screen with color passed to it
 
    strcpy(txt, " ECE3301L Spring 2019\0");                      // Text displayed 
    drawtext(start_x , start_y, txt, ST7735_WHITE  , ST7735_BLACK, TS_1);
// X and Y coordinates of where the text is to be displayed
    strcpy(txt, "Time Setup\0");                                // Text displayed 
    drawtext(start_x+3 , start_y+15, txt, ST7735_MAGENTA, ST7735_BLACK, TS_2); 
       
    strcpy(txt, "Time");
    drawtext(time_x  , time_y , txt, ST7735_BLUE   , ST7735_BLACK, TS_1);
    
    fillRect(data_time_x-1, data_time_y+16, 25,2,ST7735_CYAN);
    strcpy(txt, "Date");
    drawtext(date_x  , date_y , txt, ST7735_RED    , ST7735_BLACK, TS_1);
}
void Update_Setup_Time_Screen(void)
{
    setup_time[0]  = (setup_hour/10)  + '0';
    setup_time[1]  = (setup_hour%10)  + '0';  
    setup_time[3]  = (setup_minute/10)  + '0';
    setup_time[4]  = (setup_minute %10)+ '0';
    setup_time[6]  = (setup_second/10)  + '0';
    setup_time[7]  = (setup_second %10)+ '0'; 
    setup_date[0]  = (setup_month/10)  + '0';
    setup_date[1]  = (setup_month %10)+ '0';  
    setup_date[3]  = (setup_day/10)  + '0';
    setup_date[4]  = (setup_day %10)+ '0';
    setup_date[6]  = (setup_year/10)  + '0';
    setup_date[7]  = (setup_year %10)+ '0';
    drawtext(data_time_x, data_time_y, setup_time, ST7735_CYAN, ST7735_BLACK, TS_2);
    drawtext(data_date_x, data_date_y, setup_date, ST7735_GREEN, ST7735_BLACK, TS_2);
}
void Update_Setup_Screen_Cursor(char cursor_position)
{
    char xinc = 36;
    char yinc = 30;   
    switch (cursor_position)
    {
        case 0:
            fillRect(data_time_x-1+2*xinc, data_time_y+16+yinc, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1+2*xinc, data_time_y+16, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1, data_time_y+16, 25,2,ST7735_CYAN);
            break;
            
        case 1:
            fillRect(data_time_x-1, data_time_y+16, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1+xinc, data_time_y+16, 25,2,ST7735_CYAN);
            break; 
            
        case 2:
            fillRect(data_time_x-1+xinc, data_time_y+16, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1+2*xinc, data_time_y+16, 25,2,ST7735_CYAN);
            break;  
            
        case 3:
            fillRect(data_time_x-1+2*xinc, data_time_y+16, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1, data_time_y+16+yinc, 25,2,ST7735_CYAN);
            break;   
            
       case 4:
            fillRect(data_time_x-1, data_time_y+16+yinc, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1+xinc, data_time_y+16+yinc, 25,2,ST7735_CYAN);
            break; 
       case 5:
            fillRect(data_time_x-1+xinc, data_time_y+16+yinc, 25,2,ST7735_BLACK);
            fillRect(data_time_x-1+2*xinc, data_time_y+16+yinc, 25,2,ST7735_CYAN);
            break;              
    }
}
void Do_Setup_Time(void)
{
    char Key_Next_Flag, Key_Up_Flag, Key_Dn_Flag, Key_Abort_Flag, Key_Done_Flag;
    char Select_Field, Setup_Mode;    
    Setup_Mode = 1;
    Select_Field = 0;
    DS3231_Read_Time(DS3231_ID);
    setup_second = bcd_2_dec(second);
    setup_minute = bcd_2_dec(minute);
    setup_hour = bcd_2_dec(hour);
    setup_day = bcd_2_dec(day);
    setup_month = bcd_2_dec(month);
    setup_year = bcd_2_dec(year);
    Initialize_Setup_Time_Screen();
    Update_Setup_Time_Screen();
    while (Setup_Mode == 1)
    {
        if (nec_ok == 1)
        { 
            INTCON3bits.INT2IE = 1;                 // Enable external interrupt
            INTCON2bits.INTEDG2 = 0;                // Edge programming for INT0 falling edge
            nec_ok = 0;
            
            if (nec_code == Key_Next)   Key_Next_Flag = 1;  // '>>|' key
            if (nec_code == Key_Up)     Key_Up_Flag = 1;    // 'CH+' key
            if (nec_code == Key_Dn)     Key_Dn_Flag = 1;    // 'CH-' key
            if (nec_code == Key_Abort)  Key_Abort_Flag = 1; // '|<<' key
            if (nec_code == Key_Done)   Key_Done_Flag = 1;  // '>||' key'         
    
            if (Key_Next_Flag == 1 )
            {
                Key_Next_Flag = 0;
                Select_Field++;
                if (Select_Field == 6) Select_Field = 0;
                Update_Setup_Screen_Cursor(Select_Field);
                Key_Next_Flag = 0;
            }
            if (Key_Up_Flag == 1 )
            {
                switch (Select_Field)
                {
                    case 0:
                        setup_hour++;
                        if (setup_hour == 24) setup_hour = 0; 
                        break;
                    case 1:
                        setup_minute++;
                        if (setup_minute == 60) setup_minute = 0;
                        break;
                    case 2:
                       setup_second++;
                        if (setup_second == 60) setup_second = 0;                       
                        break;  
                    case 3:
                        setup_month++;
                        if (setup_month == 13) setup_month = 1;
                        break;  
                    case 4:
                        setup_day++;
                        if (setup_day == 32) setup_day = 1;
                        break;   
                    case 5:
                        setup_year++;
                        if (setup_year == 100) setup_year = 0;
                        break;   
                    default:
                        break;
                }    
                Update_Setup_Time_Screen();
                Key_Up_Flag = 0;
            }
            if (Key_Dn_Flag == 1 )
            {
                switch (Select_Field)
                {
                    case 0:
                        if (setup_hour == 0) setup_hour = 23;
                        else --setup_hour;
                        break;
                    case 1:
                        if (setup_minute == 0) setup_minute = 59;
                        else --setup_minute;                              
                        break;
                    case 2:
                        if (setup_second == 0) setup_second = 59;
                        else --setup_second;                       
                        break;  
                    case 3:
                        if (setup_month == 01) setup_month = 12;
                        else --setup_month;
                        break;  
                    case 4:
                        if (setup_day == 1) setup_day = 31;
                        else --setup_day;
                        break;   
                    case 5:
                        if (setup_year == 0) setup_year = 99;
                        else --setup_year;
                        break;   
                    default:
                    break;
                }
                
                Update_Setup_Time_Screen();
                Key_Dn_Flag = 0;                      
            }
            if (Key_Done_Flag == 1 )
            {
                second = dec_2_bcd(setup_second);
                minute = dec_2_bcd(setup_minute);
                hour = dec_2_bcd(setup_hour);
                dow = 0;
                day = dec_2_bcd(setup_day);
                month = dec_2_bcd(setup_month);
                year = dec_2_bcd(setup_year);
                Key_Done_Flag = 0;
                DS3231_Write_Time(DS3231_ID);
                DS3231_Read_Time(DS3231_ID);
                Initialize_Main_Screen();
                Setup_Mode = 0;               
            }
            
            if (Key_Abort_Flag == 1 )
            {
                Key_Abort_Flag = 0;
                DS3231_Read_Time(DS3231_ID);
                Initialize_Main_Screen();
                Setup_Mode = 0;               
            }
        }    
    }
}
    
void Initialize_Setup_Alarm_Screen(void) 
{ 
    fillScreen(ST7735_BLACK);                                   // Fills background of screen with color passed to it
 
    strcpy(txt, " ECE3301L Spring 2019\0");                     // Text displayed 
    drawtext(start_x , start_y, txt, ST7735_WHITE  , ST7735_BLACK, TS_1);
                                                                // X and Y coordinates of where the text is to be displayed
    strcpy(txt, "Alrm Setup");                                  // Text displayed 
    drawtext(start_x+5 , start_y+15, txt, ST7735_MAGENTA, ST7735_BLACK, TS_2); 
    
    strcpy(txt, "Time");
    drawtext(time_x  , time_y , txt, ST7735_BLUE   , ST7735_BLACK, TS_1);
    fillRect(data_time_x-1, data_time_y+16, 25,2,ST7735_CYAN);
}
void Update_Setup_Alarm_Screen(void)
{
    setup_alarm_time[0]  = (setup_alarm_hour/10)  + '0';
    setup_alarm_time[1]  = (setup_alarm_hour%10)  + '0';  
    setup_alarm_time[3]  = (setup_alarm_minute/10)  + '0';
    setup_alarm_time[4]  = (setup_alarm_minute %10)+ '0';
    setup_alarm_time[6]  = (setup_alarm_second/10)  + '0';
    setup_alarm_time[7]  = (setup_alarm_second %10)+ '0'; 
   
    drawtext(data_time_x, data_time_y, setup_alarm_time, ST7735_CYAN, ST7735_BLACK, TS_2);
}
void Do_Setup_Alarm_Time(void)
{
    char Key_Next_Flag, Key_Up_Flag, Key_Dn_Flag, Key_Abort_Flag, Key_Done_Flag;
    char Select_Field, Setup_Mode;    
    Setup_Mode = 1;
    Select_Field = 0;
    DS3231_Read_Alarm_1_Time(DS3231_ID);
    
    setup_alarm_second = bcd_2_dec(alarm_second);
    setup_alarm_minute = bcd_2_dec(alarm_minute);
    setup_alarm_hour = bcd_2_dec(alarm_hour);
  
    Initialize_Setup_Alarm_Screen();
    Update_Setup_Alarm_Screen();
    
    while (Setup_Mode == 1)
    {
        if (nec_ok == 1)
        { 
            INTCON3bits.INT2IE = 1;                 // Enable external interrupt
            INTCON2bits.INTEDG2 = 0;                // Edge programming for INT0 falling edge
            nec_ok = 0;
            
            if (nec_code == Key_Next)   Key_Next_Flag = 1;  // '>>|' key
            if (nec_code == Key_Up)     Key_Up_Flag = 1;    // 'CH+' key
            if (nec_code == Key_Dn)     Key_Dn_Flag = 1;    // 'CH-' key
            if (nec_code == Key_Abort)  Key_Abort_Flag = 1; // '|<<' key
            if (nec_code == Key_Done)   Key_Done_Flag = 1;  // '>||' key'         
    
            if (Key_Next_Flag == 1 )
            {
                Key_Next_Flag = 0;
                Select_Field++;
                if (Select_Field == 3) Select_Field = 0;
                Update_Setup_Screen_Cursor(Select_Field);
                Key_Next_Flag = 0;
            }
            if (Key_Up_Flag == 1 )
            {
                switch (Select_Field)
                {
                    case 0:
                        setup_alarm_hour++;  //setup_alarm_hour
                        if (setup_alarm_hour == 24) setup_alarm_hour = 0; 
                        break;
                    case 1:
                        setup_alarm_minute++;
                        if (setup_alarm_minute == 60) setup_alarm_minute = 0;
                        break;
                    case 2:
                       setup_alarm_second++;
                        if (setup_alarm_second == 60) setup_alarm_second = 0;                       
                        break;  
                    default:
                        break;
                }    
                Update_Setup_Alarm_Screen();   //Update_Setup_Alarm_Screen();
                Key_Up_Flag = 0;
            }
            if (Key_Dn_Flag == 1 )
            {
                switch (Select_Field)
                {
                    case 0:
                        if (setup_alarm_hour == 0) setup_alarm_hour = 23; //setup_alarm_hour
                        else --setup_alarm_hour;
                        break;
                    case 1:
                        if (setup_alarm_minute == 0) setup_alarm_minute = 59;  //setup_alarm_minute
                        else --setup_alarm_minute;                              
                        break;
                    case 2:
                        if (setup_alarm_second == 0) setup_alarm_second = 59;
                        else --setup_alarm_second;                       
                        break;  
                    default:
                    break;
                }
                
                Update_Setup_Alarm_Screen();  //Update_Setup_Alarm_Screen();
                Key_Dn_Flag = 0;                      
            }
            if (Key_Done_Flag == 1 )
            {
                alarm_second = dec_2_bcd(setup_alarm_second);
                alarm_minute = dec_2_bcd(setup_alarm_minute);
                alarm_hour = dec_2_bcd(setup_alarm_hour);
                
                Key_Done_Flag = 0;
                DS3231_Write_Alarm_1_Time(DS3231_ID);
                DS3231_Read_Alarm_1_Time(DS3231_ID);
                Initialize_Main_Screen();
                Setup_Mode = 0;               
            }
            
            if (Key_Abort_Flag == 1 )
            {
                Key_Abort_Flag = 0;
                DS3231_Read_Time(DS3231_ID);
                Initialize_Main_Screen();
                Setup_Mode = 0;               
            }
        }    
    }
}    
 
void Do_Setup_Heater_Temp(void)
{
    // Put your code here
    Initialize_Setup_Heater_Screen();
    char Key_Up_Flag, Key_Dn_Flag, Key_Abort_Flag, Key_Done_Flag;
    char Select_Field, Setup_Mode;    
    Setup_Mode = 1;
    Select_Field = 0;
    Update_Setup_Heater_Screen();
    setup_heater_temp=set_heater_temp;
    
    while (Setup_Mode == 1)
    {
        if (nec_ok == 1)
        { 
            INTCON3bits.INT2IE = 1;                 // Enable external interrupt
            INTCON2bits.INTEDG2 = 0;                // Edge programming for INT0 falling edge
            nec_ok = 0;
            
            //if (nec_code == Key_Next)   Key_Next_Flag = 1;  // '>>|' key
            if (nec_code == Key_Up)     Key_Up_Flag = 1;    // 'CH+' key
            if (nec_code == Key_Dn)     Key_Dn_Flag = 1;    // 'CH-' key
            if (nec_code == Key_Abort)  Key_Abort_Flag = 1; // '|<<' key
            if (nec_code == Key_Done)   Key_Done_Flag = 1;  // '>||' key'         
    
           
            if (Key_Up_Flag == 1 )
            {
                set_heater_temp++;
                Key_Up_Flag = 0;
                Update_Setup_Heater_Screen();
                
                    
             }
            if (Key_Dn_Flag == 1 )
            {
                set_heater_temp--;
                Key_Dn_Flag = 0;
                Update_Setup_Heater_Screen();
                                   
            }
            if (Key_Done_Flag == 1 )
            {
                Key_Done_Flag = 0;
                Initialize_Main_Screen();
                Setup_Mode = 0;     
                             
            }
            
            if (Key_Abort_Flag == 1 )
            {
                set_heater_temp=setup_heater_temp;
                Key_Abort_Flag = 0;
                Initialize_Main_Screen();
                Setup_Mode = 0;                  
            }
        }    
    }
}  
    
void Initialize_Setup_Heater_Screen(void) 
{ 
    fillScreen(ST7735_BLACK);                                    // Fills background of screen with color passed to it
 
    strcpy(txt, " ECE3301L Spring 2019\0");                      // Text displayed 
    drawtext(start_x , start_y, txt, ST7735_WHITE  , ST7735_BLACK, TS_1);   // X and Y coordinates of where the text is to be displayed
    strcpy(txt, "Heater\0"); // Text displayed 
    drawtext(start_x+25 , start_y+15, txt, ST7735_MAGENTA, ST7735_BLACK, TS_2);     
    strcpy(txt, "Setup\0");
    drawtext(start_x+25 , start_y+35, txt, ST7735_MAGENTA, ST7735_BLACK, TS_2);                           
    strcpy(txt, "Heater Set Temp");
    drawtext(S_HTR_S_x , S_HTR_S_y, txt, ST7735_YELLOW  , ST7735_BLACK, TS_1);
    drawCircle(S_cirf_HTR_S_x, S_cirf_HTR_S_y , 2  , ST7735_BLUE);
    strcpy(txt, "F");
    drawtext(S_tempf_HTR_S_x , S_tempf_HTR_S_y, txt, ST7735_BLUE , ST7735_BLACK, TS_2);
}
    
void Update_Setup_Heater_Screen(void)
{
    setup_heater_S_text[0]  = set_heater_temp/10  + '0';
 setup_heater_S_text[1]  = set_heater_temp%10  + '0';
    drawtext(S_data_HTR_S_x, S_data_HTR_S_y ,setup_heater_S_text, ST7735_BLUE, ST7735_BLACK, TS_2);
}
void DS3231_Init(char device)
{
char control;
  control = I2C_Write_Address_Read_One_Byte(device, DS3231_Add_0E);
  control = control & 0x01;
  control = control | 0x25;
  I2C_Write_Address_Write_One_Byte(device, DS3231_Add_0E, control);
  DS3231_Read_Alarm_1_Time(DS3231_ID);  //     
  alarm_second = alarm_second &0x7f;
  alarm_minute = alarm_minute &0x7f;
  alarm_hour   = alarm_hour   &0x7f;
  alarm_date   = 0x80;
  DS3231_Write_Alarm_1_Time(DS3231_ID);
}
void DS3231_Turn_On_Alarm(char device)
{
    INTCON3bits.INT1IE = 1;
    I2C_Start();                        // Start I2C protocol
    I2C_Write((device << 1) | 0);       // DS3231 address
    I2C_Write(DS3231_Add_0E);           // Send starting register address
                        
    I2C_Write(0x07);  // alarm enable
    I2C_Write(0x80);                    // alarm flag = 0
    I2C_Stop();
    delay_ms(20);                       // Wait 20 ms 
    
      
}
void DS3231_Turn_Off_Alarm(char device)
{
    INTCONbits.INT0IF = 0;
    INTCON3bits.INT1IE = 0;
    I2C_Start();                        // Start I2C protocol
    I2C_Write((device << 1) | 0);       // DS3231 address
    I2C_Write(DS3231_Add_0E);           // Send starting register address
    I2C_Write(0x06);                    // alarm enable = 0
    I2C_Write(0x80);                    // alarm flag = 0
    I2C_Stop();
    delay_ms(20);                       // Wait 20 ms 
}
 
void DS3231_Read_Time(char Device)
{
    I2C_Start();                        // Start I2C protocol
    I2C_Write((Device << 1) | 0);       // DS3231 address
    I2C_Write(DS3231_Add_00);           // Send register address
    I2C_ReStart();                      // Restart I2C
    I2C_Write((Device << 1) | 1);       // Initialize data read
    second = I2C_Read(ACK);             // Read seconds from register 0
    minute = I2C_Read(ACK);             // Read minutes from register 1
    hour   = I2C_Read(ACK);             // Read hour from register 2
    dow    = I2C_Read(ACK);             // Read day of week register 3
    day    = I2C_Read(ACK);             // Read day from register 4
    month  = I2C_Read(ACK);             // Read month from register 5
    year   = I2C_Read(NAK);             // Read year from register 6
    I2C_Stop();                         // Stop I2C protocol
    delay_ms(50);
}
void DS3231_Read_Alarm_1_Time(char Device)
{
    I2C_Start();                        // Start I2C protocol
    I2C_Write((Device << 1) | 0);       // DS3231 address
    I2C_Write(DS3231_Add_07);           // Send register address
    I2C_ReStart();                      // Restart I2C
    I2C_Write((Device << 1) | 1);       // Initialize data read
    alarm_second = I2C_Read(ACK);             // Read seconds from register 0
    alarm_minute = I2C_Read(ACK);             // Read minutes from register 1
    alarm_hour   = I2C_Read(NAK);             // Read hour from register 2
    I2C_Stop();                         // Stop I2C protocol
    delay_ms(50);
}
int DS3231_Read_Temp(char Device)
{
char T_lo, T_hi;
int temp;
    I2C_Start();                        // Start I2C protocol
    I2C_Write((Device << 1) | 0);       // DS3231 address
    I2C_Write(DS3231_Add_11);           // Send register address
    I2C_ReStart();                      // Restart I2C
    I2C_Write((Device << 1) | 1);       // Initialize data read
    T_hi = I2C_Read(ACK);               // read temp hi
    T_lo = I2C_Read(NAK);               // read temp low
    I2C_Stop();                         // Stop I2C protocol 
    delay_ms(50);
    if ((T_hi & 0x80) == 0x80) temp = -(0x80 - (T_hi & 0x7f));
    else temp = (T_hi & 0x7f);
    return (temp);
}
void DS3231_Write_Time(char Device)
{
    I2C_Start();                        // Start I2C protocol
    I2C_Write((Device << 1) | 0);       // DS3231 address
    I2C_Write(DS3231_Add_00);           // Send starting register address
    I2C_Write(second);                  // Reset seconds as 0 and start oscillator
    I2C_Write(minute);                  // Write minute value to DS3231
    I2C_Write(hour);                    // Write hour value to DS3231
    I2C_Write(dow);                     // Write day of week to DS3231
    I2C_Write(day);                     // Write date value to DS3231
    I2C_Write(month);                   // Write month value to DS3231
    I2C_Write(year);                    // Write year value to DS3231       //No I2C_Stop(); possible error?
    delay_ms(20);                       // Wait 20 ms   
}
void DS3231_Write_Alarm_1_Time(char Device)
{
    I2C_Start();                        // Start I2C protocol
    I2C_Write((Device << 1) | 0);       // DS3231 address
    I2C_Write(DS3231_Add_07);           // Send starting register address
    I2C_Write(alarm_second);                  // Reset seconds as 0 and start oscillator
    I2C_Write(alarm_minute);                  // Write minute value to DS3231
    I2C_Write(alarm_hour);                    // Write hour value to DS3231
    I2C_Write(alarm_date);
    delay_ms(20);
}   
void delay_ms(int ms)
{          
#define COUNT_PER_MS    _XTAL_FREQ/4000     // Count per ms = 2000             
#define COUNT_SCALED    COUNT_PER_MS/32     //
    int count;
    count = COUNT_SCALED * ms;
    count = (0xffff - count);
    T0CON = 0x04;                       // Timer 0, 16-bit mode, pre-scaler 1:32
        
    TMR0L = count & 0x00ff;             // set the lower byte of TMR
    TMR0H = count >> 8;                 // set the upper byte of TMR
    INTCONbits.TMR0IF = 0;              // clear the Timer 0 flag
    T0CONbits.TMR0ON = 1;               // Turn on the Timer 0
    while (INTCONbits.TMR0IF == 0); 
    // wait for the Timer Flag to be 1 for done
    T0CONbits.TMR0ON = 0;               // turn off the Timer 0
}

int bcd_2_dec (char bcd)
{
    int dec;
    dec = ((bcd>> 4) * 10) + (bcd & 0x0f);
    return dec;
}
int dec_2_bcd (char dec)
{
    int bcd;
    bcd = ((dec / 10) << 4) + (dec % 10);
    return bcd;
    
}
void gen_2khz_sound()
{
PR2 = 0b01111100 ;
T2CON = 0b00000101 ;
CCPR2L = 0b00111110 ;
CCP2CON = 0b00011100 ;
}
void turn_off_sound()
{
    CCP2CON = 0x0;
    BEEP = 0;      
}
unsigned int Get_Full_ADC(void)
{
int result;
     ADCON0bits.GO=1;                       // Start Conversion
     while(ADCON0bits.DONE==1);             // Wait for conversion to be completed (DONE=0)
     result = (ADRESH * 0x100) + ADRESL;    // Combine result of upper byte and lower byte into
     return result;                         // return the most significant 8- bits of the result.
}
void Select_ADC(char ADC_Channel)
{
    ADCON0 = 0x01 + ADC_Channel * 4;
}
void Do_LED_Out(char Volt)
{
    if (Volt > 2.5)
    {
        
            PORTDbits.RD3 = 1;
            PORTDbits.RD4 = 1;
            PORTDbits.RD5 = 1;
        
    } 
    else
    {
        PORTDbits.RD3 = 0;
        PORTDbits.RD4 = 0;
        PORTDbits.RD5 = 0;
    }    
}
void Do_LED_Up()
{
    if(LED_Intensity == 90) LED_Intensity = 0;
    else LED_Intensity = LED_Intensity + 10;
    SPI_out(LED_Intensity + 100);
}
void Do_LED_Down()
{
    if(LED_Intensity == 0) LED_Intensity = 0;
    else LED_Intensity = LED_Intensity - 10;
    SPI_out(LED_Intensity + 100);
}
void  SPI_out(unsigned char SPI_data)
{
    int i;
    char First_byte, Second_byte;
    First_byte = (SPI_data & 0xf0) >> 4;   // take the upper nibble of data and >> 4
                                            //times 
    First_byte = 0x30 | First_byte;  // set the upper nibble with 0x30 
    Second_byte = (SPI_data & 0x0f) <<4; // take the lower nibble of data and << 4 times
    Chip_Select = 0;   // set the chip select of the D/A chip to be low
    SSPBUF = First_byte;   // output the first byte to the SPI bus
    while (SSPSTATbits.BF == 0);  // wait for status done
    for ( i=0;i<1;i++);   // small delay
    SSPBUF = Second_byte;  // output the second byte to the SPI bus
    while(SSPSTATbits.BF == 0);  // wait for status done
    for ( i=0;i<1;i++);   // small delay
    Chip_Select = 1;   // raise chip select high
}
float Read_Volt_In(void)
{
    float STEP;
    Select_ADC(0);
    STEP = Get_Full_ADC();
    Volt = (STEP * 4.0/20000);
    return Volt;
}
void LED_Alarm(void)
{
    count++;
    if(count == 1)
    {
        D1RED   = 1;
        D1GREEN = 0;
        D1BLUE  = 0;
    }
    
    if(count == 2)
    {
        D1RED   = 0;
        D1GREEN = 1;
        D1BLUE  = 0;
    }
    if(count == 3)
    {
        D1RED   = 1;
        D1GREEN = 1;
        D1BLUE  = 0;
    }
    
    if(count == 4)
    {    
        D1RED   = 0;
        D1GREEN = 0;
        D1BLUE  = 1;
    }
    
    if (count == 5)
    {
        D1RED   = 1;
        D1GREEN = 0;
        D1BLUE  = 1;
    }
    
    if (count == 6)
    {
        D1RED   = 0;
        D1GREEN = 1;
        D1BLUE  = 1;
    }
    
    if(count == 7)
    {
        D1RED   = 1;
        D1GREEN = 1;
        D1BLUE  = 1;
    count = 0;
    }
}
