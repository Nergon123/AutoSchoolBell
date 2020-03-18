/*******************************************************
Project : AutoSchoolBell
Version : 1.0
Date    : 22.02.2020
Author  : Petro Chazov
Company : Chazov Inc.
Chip type               : ATmega328
Program type            : Application
AVR Core Clock frequency: 8,000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 512
*******************************************************/


#include <mega328.h>

// I2C Bus functions
#include <i2c.h>
#include <stdio.h>
// DS1307 Real Time Clock functions
#include <ds1307.h>
#include <delay.h>


// Alphanumeric LCD functions
#include <alcd.h>
#define profmax 20


//Переменые
// Declare your global variables here
flash unsigned char led_posx[]={2,5,10,13,16};
eeprom unsigned char charwhat;
eeprom struct eeprom_structure
{
unsigned char hour[21];
unsigned char min[21];
unsigned char TimeBell[21];
unsigned char BellMode[21];
}receeprom[6];
struct stram
{
unsigned char hour[21];
unsigned char min[21];
unsigned char TimeBell[21];
unsigned char BellMode[21];
}st;
eeprom struct monthh1
{
unsigned char days[31];
}montth[12];
//eeprom unsigned char eprofile;
unsigned char profile;
unsigned char hour,min,sec,week_day,day,month,year,oldmin; //oldmin - старая минута
unsigned char menu1;
unsigned char maxdays;
unsigned char yearsend,monthsend;
unsigned char display_buffer[20];
unsigned char weekday(unsigned char year1,unsigned char month1,unsigned char day1);

/*функции*/

void settime(void);
void menu(void);
void mainmenu(unsigned char pos);
void schbell(unsigned char pos);
void menusch(void);
void timedit(unsigned char tob);
void calendar(unsigned char yeartemp,unsigned char monthtemp);
//void prof_sel(void);

//===========Чтение структуры eeprom=======================

unsigned char saveprofile (unsigned char number)
{
unsigned char i,error;
number--;
error=0;
i=0;
do {
receeprom[number].hour[i]=st.hour[i];
receeprom[number].min[i]=st.min[i];
receeprom[number].TimeBell[i]=st.TimeBell[i];
receeprom[number].BellMode[i]=st.BellMode[i];
i++;
}
while(i<21);
return error;
}
//================Запись структуры eeprom========================

unsigned char loadprofile (unsigned char number)
{
unsigned char i,error;
number--;
error=0;
i=0;

do {
st.hour[i]=receeprom[number].hour[i];
if(st.hour[i]>23){error=1;st.hour[i]=23;};
st.min[i]=receeprom[number].min[i];
if(st.min[i]>59){error=2;st.min[i]=59;};
st.TimeBell[i]=receeprom[number].TimeBell[i];
if(st.TimeBell[i]>99){error=3;st.TimeBell[i]=99;};
st.BellMode[i]=receeprom[number].BellMode[i];
if(st.BellMode[i]>99){error=4;st.BellMode[i]=99;};
i++;
}
while(i<21);
return error;
}
//=========================================
void deleteprofile(void)
{
unsigned char i;
i=0;
while(i<21)
{
st.hour[i]=0;
st.min[i]=0;
st.TimeBell[i]=0;
st.BellMode[i]=0;
i++;
}
saveprofile(profile);
}


//============Вывод времени================

void print_time(void)
{
rtc_get_time(&hour,&min,&sec);
rtc_get_date(&week_day,&day,&month,&year);
lcd_gotoxy(1,0);
sprintf(display_buffer,"%2u:%02u %02u/%02u/20%02u %1u",hour,min,day,month,year,profile);
lcd_puts(display_buffer);
//lcd_gotoxy(xlcd,ylcd);
}

//===================================================

/* function used to KEY PRESSED */
unsigned char keypressed (unsigned char char1)
{
unsigned char temp;
do {
temp= PIND;
if(temp==0xFF)char1=0xFF;
if(temp==char1)temp=0xFF;
}
while(temp == 0xFF);
return temp;
}

//=========Вывод содержания профиля=================
void print_profile(unsigned char pos,unsigned char number)
{
unsigned char temp;
unsigned char temp1;
unsigned char temp2;
lcd_gotoxy(0,1);
temp = ' ';
temp1 = ' ';
//pos=1;
if(pos == 1){
temp = '>';
temp1= '<';
}
temp2=number+1-pos;
sprintf(display_buffer,"%02u.%c%02u:%02u D:%02u M:%02u%c",temp2,temp,st.hour[temp2],st.min[temp2],st.TimeBell[temp2],st.BellMode[temp2],temp1);
lcd_puts(display_buffer);
lcd_gotoxy(0,2);
temp = ' ';
temp1 = ' ';
if(pos == 2){
temp = '>';
temp1= '<';
}
temp2=number+2-pos;
sprintf(display_buffer,"%02u.%c%02u:%02u D:%02u M:%02u%c",temp2,temp,st.hour[temp2],st.min[temp2],st.TimeBell[temp2],st.BellMode[temp2],temp1);
lcd_puts(display_buffer);
temp = ' ';
temp1 = ' ';
lcd_gotoxy(0,3);
if(pos == 3){
temp = '>';
temp1= '<';
}
temp2=number+3-pos;
sprintf(display_buffer,"%02u.%c%02u:%02u D:%02u M:%02u%c",temp2,temp,st.hour[temp2],st.min[temp2],st.TimeBell[temp2],st.BellMode[temp2],temp1);
lcd_puts(display_buffer);

}


//==========================Выбор профилля============================

void prof_sel(void)
{
unsigned char prof;
unsigned char key;
unsigned char pos;
unsigned char profil;
unsigned char err;
profil=profile;
delay_ms(100);
prof=1;
pos=1;
lcd_clear();
lcd_gotoxy(0,0);
sprintf(display_buffer,"      Profile:%1u",profil);
lcd_puts(display_buffer);

print_profile(pos,prof);
key=keypressed(0b11110111);
key=0xFF;
do
{
key=keypressed(key);

switch(key)
{
case 0b11111110:
key = 0;
break;
case 0b11111101:
if(profil>=1)profil--;
lcd_clear();
lcd_gotoxy(0,0);
sprintf(display_buffer,"      Profile:%1u",profil);
lcd_puts(display_buffer);
loadprofile(profil);
//pos=1;
//prof=1;
profile=profil;
print_profile(pos,prof);

break;

case 0b11111011:
if(pos>1){pos--;prof--;}else if(prof>1)prof--;
print_profile(pos,prof);
break;

case 0b11110111:
//SELECT BUTTON
timedit(prof);
lcd_init(20);
lcd_clear();
lcd_gotoxy(0,0);
sprintf(display_buffer,"      Profile:%1u",profil);
lcd_puts(display_buffer);
//loadprofile(profil);
//pos=1;
//prof=1;
print_profile(pos,prof);
key=keypressed(0b11111110);
key=0xFF;
break;

case 0b11101111:
if(pos<3){pos++;prof++;}else if(prof<profmax)prof++;
print_profile(pos,prof);

break;

case 0b11011111:
if(profil<5)profil++;
lcd_clear();
lcd_gotoxy(0,0);
sprintf(display_buffer,"      Profile:%1u",profil);
lcd_puts(display_buffer);
err=loadprofile(profil);
//pos=1;
//prof=1;
profile=profil;
print_profile(pos,prof);
break;

case 0b10111111:
break;
}

}
while(key);


}
//=========================================================
void te_print(unsigned char pos, unsigned char data)
{ unsigned char d_buff[2];
 lcd_gotoxy(0,1);
 lcd_putsf("                   ");
 lcd_gotoxy(0,3);
 lcd_putsf("                   ");
 switch(pos){
 case 1:
 pos=4;
 break;
 case 2:
 pos=7;
 break;
 case 3:
 pos=12;
 break;
 case 4:
 pos=17;
 break;
 }
 lcd_gotoxy(pos,1);
 lcd_putsf("vv");
 lcd_gotoxy(pos,3);
 lcd_putsf("^^");
 sprintf(d_buff,"%02u",data);
 lcd_gotoxy(pos,2);
 lcd_puts(d_buff);
}
 //=============Редактирование Времени дзвонка==============
void timedit(unsigned char tob)
{
unsigned char key;
unsigned char pos;
lcd_clear();
lcd_gotoxy(1,2);
sprintf(display_buffer,"%02u.%02u:%02u D:%02u M:%02u",tob,st.hour[tob],st.min[tob],st.TimeBell[tob],st.BellMode[tob]);
lcd_puts(display_buffer);
key=0xFF;

pos=1;
te_print(pos,st.hour[tob]);
do{
key=keypressed(key);
switch(key){
case 0b11111110:
key=0;
break;
case 0b11111101:
if(pos>1)pos--;
if(pos==1)te_print(pos,st.hour[tob]);
if(pos==2)te_print(pos,st.min[tob]);
if(pos==3)te_print(pos,st.TimeBell[tob]);
if(pos==4)te_print(pos,st.BellMode[tob]);
break;
case 0b11111011:
if(pos==1)if (st.hour[tob]<23)st.hour[tob]++;
if(pos==2)if (st.min[tob]<59)st.min[tob]++;
if(pos==3)if (st.TimeBell[tob]<99)st.TimeBell[tob]++;
if(pos==4)if (st.BellMode[tob]<99)st.BellMode[tob]++;
if(pos==1)te_print(pos,st.hour[tob]);
if(pos==2)te_print(pos,st.min[tob]);
if(pos==3)te_print(pos,st.TimeBell[tob]);
if(pos==4)te_print(pos,st.BellMode[tob]);
break;
case 0b11110111:
//SELECT
break;
case 0b11101111:
if(pos==1)if (st.hour[tob]>0)st.hour[tob]--;
if(pos==2)if (st.min[tob]>0)st.min[tob]--;
if(pos==3)if (st.TimeBell[tob]>0)st.TimeBell[tob]--;
if(pos==4)if (st.BellMode[tob]>0)st.BellMode[tob]--;
if(pos==1)te_print(pos,st.hour[tob]);
if(pos==2)te_print(pos,st.min[tob]);
if(pos==3)te_print(pos,st.TimeBell[tob]);
if(pos==4)te_print(pos,st.BellMode[tob]);

break;
case 0b11011111:
if(pos<4)pos++;
if(pos==1)te_print(pos,st.hour[tob]);
if(pos==2)te_print(pos,st.min[tob]);
if(pos==3)te_print(pos,st.TimeBell[tob]);
if(pos==4)te_print(pos,st.BellMode[tob]);
break;
case 0b10111111:
break;
}

}
while(key);
}
//=================================================================

void profdel(void)
{
unsigned char key;
//key=0xFF;
lcd_init(20);
lcd_clear();
lcd_gotoxy(0,0);
sprintf(display_buffer,"Delete profile %1u?",profile);
lcd_puts(display_buffer);
lcd_gotoxy(0,3);
lcd_putsf("Cancel       Confirm");
key=0xFF;
do
{
key=keypressed(key);
if(key==0b11111110)
{
key=0;
}
if(key==0b10111111)
{
lcd_clear();
lcd_putsf("DELETING...");
deleteprofile();
delay_ms(1000);
lcd_clear();
key=0;
}
}
while(key);
}
//==================================================================

void st_print(unsigned char pos, unsigned char data)
{ unsigned char d_buff[2];
 lcd_gotoxy(0,1);
 lcd_putsf("                   ");
 lcd_gotoxy(0,3);
 lcd_putsf("                   ");
 lcd_gotoxy(led_posx[pos],1);
 lcd_putsf("vv");
 lcd_gotoxy(led_posx[pos],3);
 lcd_putsf("^^");
 sprintf(d_buff,"%02u",data);
 lcd_gotoxy(led_posx[pos],2);
 lcd_puts(d_buff);
}

//============================================================

unsigned char maxday(unsigned char mm, unsigned char yy)
{
maxdays=30;
if(yy/4*10 == yy*10/4)
{
if(mm == 2)
{
maxdays=29;
}

}
else maxdays=28;

if(mm == 1|mm==3|mm==5|mm==7|mm==8|mm==9|mm==10|mm==12)
{
maxdays=31;
}



 return mm;
}

//==============================================================

void settime(void)
{

unsigned char data[5];
unsigned char pos;
unsigned char key;
unsigned char datamax;
unsigned char datamin;

rtc_get_time(&hour,&min,&sec);
rtc_get_date(&week_day,&day,&month,&year);
data[0]=hour;
data[1]=min;
data[2]=day;
data[3]=month;
data[4]=year;
lcd_clear();
lcd_gotoxy(6,0);
lcd_putsf("Set Time");
lcd_gotoxy(2,2);
sprintf(display_buffer,"%02u:%02u   %02u/%02u/%02u",data[0],data[1],data[2],data[3],data[4]);
lcd_puts(display_buffer);
pos=0;
key=0xFF;
do
{
st_print(pos,data[pos]);
key=keypressed(key);
switch(key)
{
case 0b11111110:
lcd_gotoxy(0,3);
lcd_clear();
lcd_putsf("Saved");
delay_ms(100);
key = 0;
break;
case 0b11111101:
if(pos>0)pos--;
break;
case 0b11111011:
if(pos==0)datamax=23;
if(pos==1)datamax=59;
if(pos==2)
{
maxday(data[3],data[4]);
datamax=maxdays;
}
if(pos==3)datamax=11;
if(pos==4)datamax=99;
if(data[pos]<datamax)data[pos]++;
break;
case 0b11110111:

break;
case 0b11101111:
datamin=0;
if(pos==3)datamin=1;
if(pos==2)datamin=1;
if(data[pos]>datamin)data[pos]--;
break;
case 0b11011111:
if(pos<4)pos++;
break;
case 0b10111111:
break;
}

}
while(key);
rtc_set_time(data[0],data[1],0);
rtc_set_date(0,data[2],data[3],data[4]);
}

//==============================================================

void menu(void)
{
unsigned char key;

menu1=1;
mainmenu(menu1);
key=0xFF;
do{
key=keypressed(key);
switch(key)
{
case 0b11111110:
key = 0;
break;
case 0b11111101:
break;
case 0b11111011:
if(menu1 > 1)menu1--;
break;
case 0b11110111:
if(menu1 == 1)settime();
if(menu1 == 2)menusch();
if(menu1 == 3)calendar(year,month);
key=keypressed(0xFE);
key=0xFF;
menu1=1;
break;
case 0b11101111:
if(menu1<4)menu1++;
break;
case 0b11011111:
break;
case 0b10111111:
break;
}
mainmenu(menu1);
}
while(key);
lcd_clear();
print_time();
}

//===================================================================

void menusch(void)
{
unsigned char key;
unsigned char pos;

pos=1;
schbell(pos);
key=keypressed(0b11110111);
key=0xFF;
do{
key=keypressed(key);
switch(key)
{
case 0b11111110:
key = 0;
break;
case 0b11111101:
break;
case 0b11111011:
if(pos > 1)pos--;
break;
case 0b11110111:
if(pos == 1)prof_sel();
if(pos == 2)profdel();
schbell(pos);

key=keypressed(0b11111110);
key=0xFF;
//pos=0;
break;
case 0b11101111:
if(pos<2)pos++;
break;
case 0b11011111:
break;
case 0b10111111:
break;
}
schbell(pos);
}
while(key);
lcd_clear();

//print_time();
}

//====================================================================

void schbell(unsigned char pos)
{
unsigned char temp;
unsigned char temp1;
lcd_clear();
lcd_gotoxy(6,0);
lcd_putsf("Bell Times");
lcd_gotoxy(0,1);
temp = ' ';
temp1 = ' ';
//pos=1;
if(pos == 1){
temp = '>';
temp1= '<';
}
sprintf(display_buffer,"%cEdit Profile%c",temp,temp1);
lcd_puts(display_buffer);
lcd_gotoxy(0,2);
temp = ' ';
temp1 = ' ';
if(pos == 2){
temp = '>';
temp1= '<';
}
sprintf(display_buffer,"%cClear Profile%c",temp,temp1);
lcd_puts(display_buffer);


}

//====================================================================

void mainmenu(unsigned char pos)
{
unsigned char temp;
unsigned char temp1;
lcd_init(20);
lcd_clear();
lcd_gotoxy(8,0);
lcd_putsf("Menu");
lcd_gotoxy(0,1);
temp = ' ';
temp1 = ' ';
if(pos == 1){
temp = '>';
temp1= '<';
}
sprintf(display_buffer,"%cSet Time%c",temp,temp1);
lcd_puts(display_buffer);
lcd_gotoxy(0,2);
temp = ' ';
temp1 = ' ';
if(pos == 2){
temp = '>';
temp1= '<';
}
sprintf(display_buffer,"%cSet Bell Times%c",temp,temp1);
lcd_puts(display_buffer);
temp = ' ';
temp1 = ' ';
lcd_gotoxy(0,3);
if(pos == 3){
temp = '>';
temp1= '<';
}
sprintf(display_buffer,"%cCalendar%c",temp,temp1);
lcd_puts(display_buffer);


}
///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void calendgoto(unsigned char yeartemp,unsigned char monthtemp)
{
unsigned char pos;
unsigned char key;
pos=1;
yearsend=yeartemp;
monthsend=monthtemp;
lcd_clear();
sprintf(display_buffer,"Year:20%02u Month:%02u",yeartemp,monthtemp);
lcd_gotoxy(0,2);
lcd_puts(display_buffer);
key=0xFF;
do
{
key=keypressed(key);
if(pos==1)
{
lcd_gotoxy(5,1);
lcd_putsf("vvvv");
lcd_gotoxy(5,3);
lcd_putsf("^^^^");
lcd_gotoxy(16,1);
lcd_putsf("  ");
lcd_gotoxy(16,3);
lcd_putsf("  ");
}
if(pos==2)
{
lcd_gotoxy(16,1);
lcd_putsf("vv");
lcd_gotoxy(16,3);
lcd_putsf("^^");
lcd_gotoxy(5,1);
lcd_putsf("    ");
lcd_gotoxy(5,3);
lcd_putsf("    ");

}
switch(key)
{
case 0b11111110:
key=0;
break;
case 0b11111101:
if(pos==2)pos--;
break;
case 0b11111011:
if(pos==1)
{
if(yeartemp<99)yeartemp++;
}
if(pos==2)
{
monthtemp++;
if(monthtemp==13)monthtemp=1;
}
sprintf(display_buffer,"Year:20%02u Month:%02u",yeartemp,monthtemp);
lcd_gotoxy(0,2);
lcd_puts(display_buffer);
break;
case 0b11101111:
if(pos==1)
{
if(yeartemp<1)yeartemp--;
}
if(pos==2)
{
monthtemp--;
if(monthtemp==0)monthtemp=12;
}
sprintf(display_buffer,"Year:20%02u Month:%02u",yeartemp,monthtemp);
lcd_gotoxy(0,2);
lcd_puts(display_buffer);
break;
case 0b11011111:
if(pos==1)pos++;
break;
}
}
while(key);
monthsend=monthtemp;
yearsend=yeartemp;
delay_ms(500);

}
///////////////////////////////////////////////////////////////////
void calendar(unsigned char yeartemp,unsigned char monthtemp)

{
unsigned char weekofday[7];
unsigned char pos;
unsigned char key;
unsigned char temprof[7];
unsigned char daytemp;
char i;
unsigned char a;
unsigned char week;
daytemp=day;
yeartemp=yearsend;
monthtemp=monthsend;
i=0;
a=0;
week=1;
lcd_clear();
lcd_gotoxy(0,0);
sprintf(display_buffer,"Year:20%2u  Month:%02u",yeartemp,monthtemp);
lcd_puts(display_buffer);
lcd_gotoxy(0,1);
lcd_putsf("MO TU WE TH FR SA SU");
key=0xFF;
do
{
key=keypressed(key);
{//week
if(week==1)
{
if(weekday(yeartemp,monthtemp,1)==0)
{
weekofday[0]=0;
weekofday[1]=0;
weekofday[2]=0;
weekofday[3]=0;
weekofday[4]=0;
weekofday[5]=0;
weekofday[6]=1;
}
if(weekday(yeartemp,monthtemp,1)==1)
{
weekofday[0]=1;
weekofday[1]=2;
weekofday[2]=3;
weekofday[3]=4;
weekofday[4]=5;
weekofday[5]=6;
weekofday[6]=7;
}
if(weekday(yeartemp,monthtemp,1)==2)
{
weekofday[0]=0;
weekofday[1]=1;
weekofday[2]=2;
weekofday[3]=3;
weekofday[4]=4;
weekofday[5]=5;
weekofday[6]=6;
}
if(weekday(yeartemp,monthtemp,1)==3)
{
weekofday[0]=0;
weekofday[1]=0;
weekofday[2]=1;
weekofday[3]=2;
weekofday[4]=3;
weekofday[5]=4;
weekofday[6]=5;
}
if(weekday(yeartemp,monthtemp,1)==4)
{
weekofday[0]=0;
weekofday[1]=0;
weekofday[2]=0;
weekofday[3]=1;
weekofday[4]=2;
weekofday[5]=3;
weekofday[6]=4;
}
if(weekday(yeartemp,monthtemp,1)==5)
{
weekofday[0]=0;
weekofday[1]=0;
weekofday[2]=0;
weekofday[3]=0;
weekofday[4]=1;
weekofday[5]=2;
weekofday[6]=3;
}
if(weekday(yeartemp,monthtemp,1)==6)
{
weekofday[0]=0;
weekofday[1]=0;
weekofday[2]=0;
weekofday[3]=0;
weekofday[4]=0;
weekofday[5]=1;
weekofday[6]=2;
}
}
if(week==2)
{
i=2;
weekofday[1]=weekofday[0]+1;
a=weekofday[1];
do
{
weekofday[i]=weekofday[i]+1;
}
while(i<7);
}
if(week==3)
{
i=2;
weekofday[1]=weekofday[0]+1;
a=weekofday[1];
do
{
weekofday[i]=weekofday[i]+1;
}
while(i<7);
}
if(week==4)
{
i=2;
weekofday[1]=weekofday[0]+1;
a=weekofday[1];
do
{
weekofday[i]=weekofday[i]+1;
}
while(i<7);
}
if(week==5)
{
if(monthtemp==1||monthtemp==3||monthtemp==5||monthtemp==7||monthtemp==8||monthtemp==10||monthtemp==12)
{
i=2;
weekofday[1]=weekofday[0]+1;
a=weekofday[1];
do
{
weekofday[i]=weekofday[i]+1;
if(weekofday[i]>31)
{
weekofday[i]=0;
}
}
while(i<7);
}
if(monthtemp==4||monthtemp==6||monthtemp==9||monthtemp==11)
{
i=2;
weekofday[1]=weekofday[0]+1;
a=weekofday[1];
do
{
weekofday[i]=weekofday[i]+1;
if(weekofday[i]>30)
{
weekofday[i]=0;
}
}
while(i<7);
}
if(month==2)
{
i=2;
weekofday[1]=weekofday[0]+1;
a=weekofday[1];
do
{
weekofday[i]=weekofday[i]+1;
if(yeartemp/4*10 == yeartemp*10/4)
{
if(weekofday[i]>29)
{
weekofday[i]=0;
}
}else
{
if(weekofday[i]>28)
{
weekofday[i]=0;
}
}


}
while(i<7);
}

}
}
lcd_gotoxy(0,2);
sprintf(display_buffer,"%2u %2u %2u %2u %2u %2u %2u",weekofday[0],weekofday[1],weekofday[2],weekofday[3],weekofday[4],weekofday[5],weekofday[6]);
lcd_puts(display_buffer);
{
if(pos==1)
{
lcd_gotoxy(0,3);
lcd_putsf("^");
lcd_gotoxy(3,3);
lcd_putsf(" ");
}
if(pos==2)
{
lcd_gotoxy(3,3);
lcd_putsf("^");
lcd_gotoxy(0,3);
lcd_putsf(" ");
lcd_gotoxy(6,3);
lcd_putsf(" ");
}
if(pos==3)
{
lcd_gotoxy(6,3);
lcd_putsf("^");
lcd_gotoxy(9,3);
lcd_putsf(" ");
lcd_gotoxy(3,3);
lcd_putsf(" ");
}
if(pos==4)
{
lcd_gotoxy(9,3);
lcd_putsf("^");
lcd_gotoxy(6,3);
lcd_putsf(" ");
lcd_gotoxy(12,3);
lcd_putsf(" ");
}
if(pos==5)
{
lcd_gotoxy(12,3);
lcd_putsf("^");
lcd_gotoxy(15,3);
lcd_putsf(" ");
lcd_gotoxy(9,3);
lcd_putsf(" ");
}
if(pos==6)
{
lcd_gotoxy(15,3);
lcd_putsf("^");
lcd_gotoxy(18,3);
lcd_putsf(" ");
lcd_gotoxy(12,3);
lcd_putsf(" ");
}
if(pos==7)
{
lcd_gotoxy(18,3);
lcd_putsf("^");
lcd_gotoxy(15,3);
lcd_putsf(" ");
}
}
switch(key){
case 0b11111110:
lcd_clear();
lcd_putsf("Saved");
delay_ms(100);
key=0;
break;
case 0b11111101:
if(pos>1)pos--;
break;
case 0b11111011:
if(temprof[pos-1]<5)temprof[pos-1]++;
break;
case 0b11110111:
//select
key=0xFF;

break;
case 0b11101111:
if(temprof[pos-1]>5)temprof[pos-1]--;
break;
case 0b11011111:
if(pos<7)pos++;
break;
case 0b10111111:
//////////////////////////////////////////////////
//MENU
calendgoto(yeartemp,monthtemp);
lcd_clear();
lcd_gotoxy(0,0);
sprintf(display_buffer,"Year:20%2u  Month:%02u",yeartemp,monthtemp);
lcd_puts(display_buffer);
lcd_gotoxy(0,1);
lcd_putsf("MO TU WE TH FR SA SU");
sprintf(display_buffer,"%2u %2u %2u %2u %2u %2u %2u",weekofday[0],weekofday[1],weekofday[2],weekofday[3],weekofday[4],weekofday[5],weekofday[6]);
lcd_gotoxy(0,2);
lcd_puts(display_buffer);
key=0xff;
/////////////////////////////////////////////////
break;
}

}
while(key);
}
////////////////////////////////////////////////////////////////////
unsigned char weekday(unsigned char year1,unsigned char month1,unsigned char day1)
{
unsigned int year2;
unsigned char weekofday;
weekofday=0;
year2=year1+2000;

if (month < 3)
{
year2=year2 - 1;
month1 = month1 + 10;
}else
{
month1 = month1 - 2;
}

weekofday=(day1 + (31*month1)/12+year2+year2/4-year2/100+year2/400)%7;

return weekofday;
}


///////////////////////////////////////////////////////////////////
void erasemonth(void)
{
unsigned char i;
lcd_clear();
lcd_putsf("ERASING");
delay_ms(1000);
i=0;
do
{
montth[month-1].days[i]=0;
i++;
}
while(i<31);
lcd_clear();

}
/////////////////////////////////////////////////////////////////////
void check_profile(void)
{
unsigned char error;
if(day==1)erasemonth();
error=0;
profile=montth[month].days[day];
if(profile>5)error=1;
if(error==1)profile=1;
lcd_clear();
lcd_gotoxy(3,2);
lcd_putsf("Checking Profile");
delay_ms(2000);
lcd_clear();
}


/////////////////////////////////////////////////////////////////////
//////////////////////КОНЕЦ_ФУНКЦИЙ////////////////////////////////
/////////////////////////////////////////////////////////////////////

void main(void)
{
// Declare your local variables here


// Crystal Oscillator division factor: 1
#pragma optsize-
CLKPR=(1<<CLKPCE);
CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Input/Output Ports initialization
// Port B initialization
// Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
DDRB=(0<<DDB7) | (0<<DDB6) | (0<<DDB5) | (0<<DDB4) | (1<<DDB3) | (0<<DDB2) | (0<<DDB1) | (0<<DDB0);
// State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T
PORTB=(0<<PORTB7) | (0<<PORTB6) | (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);

// Port C initialization
// Function: Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
DDRC=(0<<DDC6) | (0<<DDC5) | (0<<DDC4) | (0<<DDC3) | (0<<DDC2) | (0<<DDC1) | (0<<DDC0);
// State: Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T
PORTC=(0<<PORTC6) | (0<<PORTC5) | (0<<PORTC4) | (0<<PORTC3) | (0<<PORTC2) | (0<<PORTC1) | (0<<PORTC0);

// Port D initialization
// Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
DDRD=(0<<DDD7) | (0<<DDD6) | (0<<DDD5) | (0<<DDD4) | (0<<DDD3) | (0<<DDD2) | (0<<DDD1) | (0<<DDD0);
// State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T
PORTD=(1<<PORTD7) | (1<<PORTD6) | (1<<PORTD5) | (1<<PORTD4) | (1<<PORTD3) | (1<<PORTD2) | (1<<PORTD1) | (1<<PORTD0);

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: Timer 0 Stopped
// Mode: Normal top=0xFF
// OC0A output: Disconnected
// OC0B output: Disconnected
TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
TCCR0B=(0<<WGM02) | (0<<CS02) | (0<<CS01) | (0<<CS00);
TCNT0=0x00;
OCR0A=0x00;
OCR0B=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: Timer1 Stopped
// Mode: Normal top=0xFFFF
// OC1A output: Disconnected
// OC1B output: Disconnected
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (0<<CS10);
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer2 Stopped
// Mode: Normal top=0xFF
// OC2A output: Disconnected
// OC2B output: Disconnected
ASSR=(0<<EXCLK) | (0<<AS2);
TCCR2A=(0<<COM2A1) | (0<<COM2A0) | (0<<COM2B1) | (0<<COM2B0) | (0<<WGM21) | (0<<WGM20);
TCCR2B=(0<<WGM22) | (0<<CS22) | (0<<CS21) | (0<<CS20);
TCNT2=0x00;
OCR2A=0x00;
OCR2B=0x00;

// Timer/Counter 0 Interrupt(s) initialization
TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (0<<TOIE0);

// Timer/Counter 1 Interrupt(s) initialization
TIMSK1=(0<<ICIE1) | (0<<OCIE1B) | (0<<OCIE1A) | (0<<TOIE1);

// Timer/Counter 2 Interrupt(s) initialization
TIMSK2=(0<<OCIE2B) | (0<<OCIE2A) | (0<<TOIE2);

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
// Interrupt on any change on pins PCINT0-7: Off
// Interrupt on any change on pins PCINT8-14: Off
// Interrupt on any change on pins PCINT16-23: Off
EICRA=(0<<ISC11) | (0<<ISC10) | (0<<ISC01) | (0<<ISC00);
EIMSK=(0<<INT1) | (0<<INT0);
PCICR=(0<<PCIE2) | (0<<PCIE1) | (0<<PCIE0);

// USART initialization
// USART disabled
UCSR0B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (0<<RXEN0) | (0<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);

// Analog Comparator initialization
// Analog Comparator: Off
// The Analog Comparator's positive input is
// connected to the AIN0 pin
// The Analog Comparator's negative input is
// connected to the AIN1 pin


ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIC) | (0<<ACIS1) | (0<<ACIS0);
ADCSRB=(0<<ACME);
// Digital input buffer on AIN0: On
// Digital input buffer on AIN1: On
DIDR1=(0<<AIN0D) | (0<<AIN1D);

// ADC initialization
// ADC disabled
ADCSRA=(0<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (0<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);

// SPI initialization
// SPI disabled
SPCR=(0<<SPIE) | (0<<SPE) | (0<<DORD) | (0<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);

// TWI initialization
// TWI disabled
TWCR=(0<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWEN) | (0<<TWIE);

// Bit-Banged I2C Bus initialization
// I2C Port: PORTC
// I2C SDA bit: 1
// I2C SCL bit: 0
// Bit Rate: 100 kHz
// Note: I2C settings are specified in the
// Project|Configure|C Compiler|Libraries|I2C menu.
i2c_init();

// DS1307 Real Time Clock initialization
// Square wave output on pin SQW/OUT: Off
// SQW/OUT pin state: 0
rtc_init(0,0,0);
rtc_get_time(&hour,&min,&sec);
rtc_get_date(&week_day,&day,&month,&year);

// Alphanumeric LCD initialization
// Connections are specified in the
// Project|Configure|C Compiler|Libraries|Alphanumeric LCD menu:
// RS - PORTB Bit 0
// RD - PORTB Bit 1
// EN - PORTB Bit 2
// D4 - PORTB Bit 4
// D5 - PORTB Bit 5
// D6 - PORTB Bit 6
// D7 - PORTB Bit 7
// Characters/line: 20
oldmin = charwhat;

lcd_init(20);
oldmin = min;
lcd_clear();
lcd_gotoxy(3,1);
lcd_putsf("AutoSchoolBell");
lcd_gotoxy(3,3);
lcd_putsf("By Petro Chazov");
delay_ms(1000);
lcd_clear();
yearsend=year;
monthsend=month;
check_profile();
print_time();


//////////////////////////////////////////////////
while (1)
      {
       rtc_get_time(&hour,&min,&sec);
       if (oldmin != min) print_time();
       oldmin=min;
       delay_ms(300);
       if (hour==0)if(min==0)if(sec<2){check_profile();print_time();}
       if (PIND.6 == 0) {
       menu();
       }
   //    lcd_gotoxy(3,0);
   //    lcd_write_byte(0xffff,0x0100);
}
}
