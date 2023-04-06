
//ADD DEFAULT CITY VAL

include <SPI.h>
#include <TFT.h>
#include <DHT.h>

#define Type DHT11
int sensePin = 5;
DHT HT(sensePin, Type);

const int button1_Pin = 2;
const int button2_Pin = 3;
const int button3_Pin = 4;
int button1 = 0;
int button2 = 0;
int button3 = 0;
int alarm_screen = 0;
int set_hour = 0;
int set_minutes = 0;
int set_ampm = 0;
int set_confirm = 0;

const int buzzer = 6;
bool ringing = 0;
bool alarm_is_set = 0;

int local_temp;
int local_humidity;
int remote_temp =0;
int remote_humidity = 0;
int month = 0;
int day = 0;
int year = 0;
int read_hour;
int hour = 12;
int minutes = 0;  
String am_pm = "AM";
int alarm_hour = 12;  //KEEP THIS 
int alarm_minutes = 0; //KEEP THIS 
String alarm_ampm = "AM"; //KEEP THIS
String alarm_confirm = "Y"; //KEEP THIS
String city;
String state;

#define PURPLE  0x780F      
#define CYAN    0xFF00      
#define LIGHTGREY   0xC618
#define RED     0x00FF 
#define GREEN   0x07E0
#define ORANGE  0xA5FF
#define YELLOW  0x07FF
#define PINK    0x807F
#define BLACK  0x0000
#define WHITE  0xFFFF
#define cs 10
#define dc 9
#define rst 8

TFT screen = TFT(cs, dc, rst);

  int i = 0;
  char msg[30];

void setup() {
  Serial.begin(115200);
  
  screen.begin();
  HT.begin();
  
  //set background color to black 
  screen.background(0,0,0); 
   
  pinMode(button1_Pin, INPUT);
  pinMode(button2_Pin, INPUT);
  pinMode(button3_Pin, INPUT);

  pinMode(buzzer, OUTPUT);


  
}

void loop() { 
  
  /*month = 10;
  day = 13;
  year = 23;
  */
  city = "";
  state = "";
  local_temp = HT.readTemperature(true);
  local_humidity = HT.readHumidity();
  //remote_temp = 85;
  //remote_humidity = 52;

  soundAlarm();

  button1 = digitalRead(button1_Pin);
  button2 = digitalRead(button2_Pin);
  button3 = digitalRead(button3_Pin);

  alarm();

  recieve();
  formulateTime();
  
  delay(200); // 200
}

void recieve() { 
  memset(msg,0,30);
  i = 0;
  // put your main code here, to run repeatedly:
    while(Serial.available()){
      char c = Serial.read();
      if(isDigit(c)){
        msg[i] = c;
        i++;
      }
       
    }
   /* if(i == 6){
      int num1 = msg[0]-48;
      int num2 = msg[1]-48;
      int num3 = msg[2]-48;
      int num4 = msg[3]-48;
    //  hour = num1 * 10 + num2;
     read_hour = num1 * 10 + num2;
      minutes = num3*10 + num4;
    
    
    }
    */
    if(i == 18){
      int num1 = msg[0]-48;
      int num2 = msg[1]-48;
      int num3 = msg[2]-48;
      int num4 = msg[3]-48;

      //temp
      int tempNum1 = msg[6] - 48;
      int tempNum2 = msg[7] - 48;

      //humid
      int humNum1 = msg[8] - 48;
      int humNum2 = msg[9] - 48;

      //month
      int M1 = msg[10] - 48;
      int M2 = msg[11] - 48;

      month = M1 * 10 + M2;
      //month = M1;
      //day
      int D1 = msg[12] - 48;
      int D2 = msg[13] - 48;

      day = D1 * 10 + D2;
      //day = D1;
      //year
     // int Y1 = msg[14] - 48;
      //int Y2 = msg[15] - 48;
      int Y3 = msg[16] - 48;
      int Y4 = msg[17] - 48;

      year = (Y3 * 10) + Y4;
      //year = 2;
      


      remote_temp = tempNum1 * 10 + tempNum2;
      remote_humidity = humNum1 * 10 + humNum2;
      
    //  hour = num1 * 10 + num2;
      read_hour = num1 * 10 + num2;
      minutes = num3*10 + num4;
      
    }
    memset(msg,0,30);

    if(read_hour < 12) {
      am_pm = "AM";
    }
    else if(read_hour >= 12) {
      am_pm = "PM";
    }

    hour = read_hour; 
    
    delay(1000);
}

void mainScreen() {
  screen.drawLine(90, 74, 90, 122, CYAN); //VERTICAL
  printDate();
  printTime();
  printLocalTemp();
  printLocalHumidity();
  printCity();
  printRemoteTemp();
  printRemoteHumidity();
  
}
/*
void printDate() {
   screen.setTextSize(1);
   screen.stroke(225, 225, 225); //WHITE
   screen.setCursor(7,11);
   screen.print(month);
   screen.print("-");
   screen.print(day);
   screen.print("-");
   screen.print(year);
   screen.setTextColor(WHITE, BLACK);
   screen.setCursor(63,8);
   screen.setTextSize(2);
   screen.print(am_pm);
}*/
void printDate() {
   screen.setTextSize(1);
   screen.stroke(225, 225, 225); //WHITE
   screen.setTextColor(WHITE, BLACK);
   screen.setCursor(7,11);
   screen.print(month);
   screen.print("-");
   screen.print(day);
   screen.print("-");
   screen.print(year);
   screen.setCursor(63,8);
   screen.setTextSize(2);
   screen.print(am_pm);
}

void formulateTime() {
  //hour 0 becomes 12 
  if(hour == 0) {
      hour = 12;
    }
  else if(hour > 13) {
    hour = hour - 12;
  }
}

void printTime(){
  screen.setTextSize(3);
  screen.stroke(225, 225, 0); //CYAN
  screen.rect(0,31,94,33); 
  screen.setTextColor(WHITE, BLACK);
  screen.setCursor(3,37);
  if(hour < 10) {
    screen.print("0");
  }
  screen.print(hour);
  screen.print(":");
  if(minutes < 10) {
    screen.print("0");
  }
  screen.print(minutes);
}

void printLocalTemp() {
  screen.setTextSize(1);
  screen.stroke(0, 0, 225); //RED
  screen.text("HOME",35,75);
  screen.setTextColor(YELLOW, BLACK);
  screen.setTextSize(2);
  screen.setCursor(25,87);
  screen.print(local_temp);
  screen.stroke(0, 255, 255); //YELLOW
  screen.circle(55,88,2);
  screen.setCursor(61,87);
  screen.print("F");
}

void printLocalHumidity() {
  screen.setTextSize(1);
  screen.setTextColor(RED, BLACK);  //ORANGE
  screen.setTextSize(2);
  screen.setCursor(30,107);
  screen.print(local_humidity);
  screen.print("%");
}
void printCity() {
  screen.setTextSize(1);
  screen.setCursor(100,10); 
  screen.print(city);
  screen.print(",");
  screen.setCursor(120, 20);
  screen.print(state);
}
void printRemoteTemp() {
  screen.setTextSize(1);
  screen.stroke(0, 225, 0); //GREEN
  screen.text("Temp",118,37);
  screen.drawLine(108, 47, 150, 47, CYAN);
  screen.setTextColor(PINK, BLACK);
  screen.setTextSize(2);
  screen.setCursor(108,52);
  screen.print(remote_temp);
  screen.stroke(197, 30, 255); //PINK  
  screen.circle(136,52,2);
  screen.setCursor(141,52);
  screen.print("F");
}
void printRemoteHumidity(){
  screen.setTextSize(1);
  screen.stroke(0, 225, 0); //GREEN
  screen.text("Humidity",107,80);
  screen.drawLine(104, 90, 155, 90, CYAN);
  screen.setTextColor(PURPLE, BLACK);
  screen.setTextSize(2);
  screen.setCursor(112,95); 
  screen.print(remote_humidity);
  screen.print("%");
}

void alarm() {
  if(button1 == HIGH && alarm_screen == 0 && ringing == 0) {
   set_hour = 1;
   screen.fillScreen(0);
   screen.setTextSize(1);
   screen.text("Set the hour",42,30);
   alarm_screen = 1;
  } 
  else if(button1 == HIGH && alarm_screen == 1) {
    set_hour = 0;
    set_minutes = 1;
    screen.fillScreen(0);
    screen.setTextSize(1);
    screen.stroke(0, 225, 0); //GREEN
    screen.text("Set the minutes",36,30);
    alarm_screen = 2;
  }
  else if(button1 == HIGH && alarm_screen == 2) {
    set_minutes = 0;
    set_ampm = 1;
    screen.fillScreen(0);
    screen.setTextSize(1); 
    screen.stroke(0, 225, 0); //GREEN
    screen.text("AM or PM?",50,30);
    alarm_screen = 3;
  }
  else if(button1 == HIGH && alarm_screen == 3) {
    set_ampm = 0;
    set_confirm = 1;
    screen.fillScreen(0);
    screen.setTextSize(1); 
    screen.stroke(0, 225, 0); //GREEN
    screen.text("Confirm?", 45, 30);
    screen.setTextColor(GREEN, BLACK);
    screen.setCursor(100,30); 
    screen.print(alarm_confirm);
    alarm_screen = 4;
  }
  else if(button1 == HIGH && alarm_screen == 4) {
    screen.fillScreen(0);
    set_confirm = 0;
    if(alarm_confirm == "Y") {
      alarm_is_set = 1;
    }
    else if(alarm_confirm == "N") {
      alarm_is_set = 0;
    }
    alarm_screen = 0;
  }
  
  if(alarm_screen == 1 || alarm_screen == 2 || alarm_screen == 3 || alarm_screen == 4) {
    alarmScreen();
  }
  else if(alarm_screen == 0) {
    mainScreen();
  }
}

void alarmScreen() {
  screen.setTextSize(2);
  screen.stroke(225, 225, 0); //CYAN
  screen.text("SET ALARM",28,5);
  screen.setTextSize(1);
  screen.stroke(255, 0, 240); //PURPLE
  screen.text("Middle button --- up",10,50);
  screen.text("Lower button --- down",10,65);
  screen.setTextSize(3);
  screen.setTextColor(WHITE, BLACK);
  screen.setCursor(10,90);
  if(alarm_hour < 10) {
    screen.print("0");
  }
  screen.print(alarm_hour);
  screen.print(":");
  if(alarm_minutes < 10) {
    screen.print("0");
  }
  screen.print(alarm_minutes);
  screen.print(" ");
  screen.print(alarm_ampm);
  setAlarm();

  if(set_confirm == 1) {
    screen.setTextSize(1);
    screen.setTextColor(GREEN, BLACK);
    screen.setCursor(100,30); 
    screen.print(alarm_confirm);
  }
}
void setAlarm() {
  if(button2 == HIGH && set_hour == 1) {
    if(alarm_hour == 12){
      alarm_hour = 1;
    }
    else {
      alarm_hour++;
    }
  }
  else if(button2 == HIGH && set_minutes == 1) {
    if(alarm_minutes == 59) {
      alarm_minutes = 0;
    }
    else {
      alarm_minutes++;
    }
  }
  else if(button2 == HIGH && set_ampm == 1) {
    if(alarm_ampm == "AM") {
      alarm_ampm = "PM";
    }
    else {
      alarm_ampm = "AM";
    }
  }
  else if(button2 == HIGH && set_confirm == 1) {
    if(alarm_confirm == "Y") {
      alarm_confirm = "N";
    }
    else {
      alarm_confirm = "Y";
    }
  }
  else if(button3 == HIGH && set_hour == 1) {
    if(alarm_hour == 1) {
      alarm_hour = 12;
    }
    else {
      alarm_hour--;
    }
  }
  else if(button3 == HIGH && set_minutes == 1) {
    if(alarm_minutes == 0){
      alarm_minutes = 59;
    }
    else {
      alarm_minutes--;
    }
  }
  else if(button3 == HIGH && set_ampm == 1) {
    if(alarm_ampm == "AM") {
      alarm_ampm = "PM";
    }
    else {
      alarm_ampm = "AM";
    }
  }
  else if(button3 == HIGH && set_confirm == 1) {
    if(alarm_confirm == "Y") {
      alarm_confirm = "N";
    }
    else {
      alarm_confirm = "Y";
    }
  }
}

void soundAlarm() {
  //if alarm time matches actual time and the alarm is not already ringing then ring 
  if(alarm_hour == hour && alarm_minutes == minutes && alarm_ampm == am_pm && ringing == 0 && alarm_is_set == 1) {
    screen.fillRect(2, 45, 80, 20, BLACK);
    tone(buzzer, 250);
    ringing = 1;
  }
  else if(ringing == 0) {
    noTone(buzzer);
  } 
  //if the alarm is ringing and user pushes top button then stop ringing 
  else if(ringing == 1 && button1 == HIGH && alarm_is_set == 1) {
    ringing = 0;
    alarm_is_set = 0;
    noTone(buzzer);
  }
  //arm is ringing and has not been turned off so keep ringing
  else if(ringing == 1 && button1 == LOW) {
    screen.fillRect(1, 30, 93, 32, BLACK);
    tone(buzzer, 250);
  }
}
