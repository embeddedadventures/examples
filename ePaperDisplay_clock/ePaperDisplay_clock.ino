/*

Copyright (c) 2016, Embedded Adventures, www.embeddedadventures.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

- Neither the name of Embedded Adventures nor the names of its contributors
  may be used to endorse or promote products derived from this software
  without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.

Contact us at admin [at] embeddedadventures.com
*/

#include <SPI.h>
#include <Wire.h>
#include <draw.h>
#include <SSD1607.h>
#include <TMP275.h>
#include <M41T81S.h>

#define HOUR_HAND_LEN   30
#define MIN_HAND_LEN    35
#define CLOCK_CENTER_X  100
#define CLOCK_CENTER_Y  100
#define CLOCK_RADIUS    50

const uns8 dcPin = 12;
const uns8 csPin = 2;
const uns8 busyPin = 16;
const uns8 rstPin = 15;

SSD1607 epd(csPin, dcPin, busyPin, rstPin);

int hour_x_offset[12];
int hour_y_offset[12];
int min_x_offset[60];
int min_y_offset[60];
int hour_marker_x[12];
int hour_marker_y[12];

String monthNames[12] = {"JAN", "FEB", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUG", "SEPT", "OCT", "NOV", "DEC"};
int sec, mins, hrs, dow, date, month, year, psec;
float celsius;

void setup() {
  Serial.begin(115200);
  Serial.println("Welcome to the Embedded Adventures demo sketch for the EPD-200200B and MOD1001 RTC and Temperature Sensor");
  Serial.println("The display updates every minute with a temperature measurement and time");
  Wire.begin();
  
  tmp275.init();
  rtc.init();
  tmp275.setResolution(4);
  tmp275.enableShutdownMode(true);
  
  draw_init(EPD_HT, EPD_WD, 1);
  draw_fonts_init();
  
  epd.invert(true);
  epd.init();
  initHourOffsets();
  initMinuteOffsets();
  
  drawLogo(70, 175);
  //setRTC(); //Update RTC contents once, then comment it out and upload again so RTC isn't stuck in time
  delay(2000);
  tempOneShot();
  updateClock();
  print_serial();
  updateDisplay();
}

void loop() {
  if (mins != rtc.getMinutes()) {
    tempOneShot();
    updateClock();
    print_serial();
    updateDisplay();
  }
}

void tempOneShot() { 
  tmp275.enableOS();
  celsius = tmp275.getTemperature();
}

void updateClock() { 
  psec = rtc.getPartSeconds(); 
  sec = rtc.getSeconds();

  mins = rtc.getMinutes();
  hrs = rtc.getHours();
  dow = rtc.getDayOfWeek();
  date = rtc.getDate();
  month = rtc.getMonth();
  year = rtc.getYear();
}

void print_serial() {
  Serial.println("hrs:mins:sec:ms - ");
  Serial.print(hrs);
  Serial.print(":");
  Serial.print(mins);
  Serial.print(":");
  Serial.print(sec);
  Serial.print(":");
  Serial.println(psec);
  Serial.println();
  Serial.print("Temperature, Celsius:\t");
  Serial.println(celsius);
}

void testHours() {
  for (int i = 0; i < 12; i++) {
    drawHourHand(i);
  }
}

void testMinutes() {
  for (int i = 0; i < 60; i++) {
    drawMinuteHand(i);
  }
}

void drv_paint() {
  epd.displayFullRev(draw_buffer);
}

double deg2rad(double deg) {
  return (deg) * (PI / 180);
}

void updateDisplay() {
  String str;
  draw_clear_screen();
  delay(1);
  //Date
  str = (String)date + + " " + monthNames[month - 1] + " 20" + (String)year;
  draw_fonts_print_str(DRAW_FONT_10DOUBLE_ID, 125, 185, 128, 0, 2, str.c_str());
  //Temperature
  str = (String)celsius + " CELSIUS";
  draw_fonts_print_str(DRAW_FONT_12DOUBLE_ID, 8, 170, 128, 0, 2, str.c_str());
  //Time
  str = (String)hrs + ":" + (String)mins;
  draw_fonts_print_str(DRAW_FONT_10DOUBLE_ID, 8, 185, 128, 0, 2, str.c_str());
  //Clock
  drawClockOutline();
  drawHourHand(hrs);
  drawMinuteHand(mins);
  draw_paint();
}

void drawLogo(draw_x_type x, draw_y_type y) {
  draw_clear_screen();
  delay(100);
  draw_bitmap(x, y-60, 1, embedded_bitmap);
  draw_bitmap(x, y-75, 1, adventures_bitmap);
  draw_bitmap(x, y, 1, e_big_bitmap);
  draw_bitmap(x+30, y, 1, a_big_bitmap);
  draw_fonts_print_str(DRAW_FONT_10NORMAL_ID, x-5, y - 100, 128, 0, 2, "EPD-200200B");
  draw_fonts_print_str(DRAW_FONT_10NORMAL_ID, x-30, y - 115, 128, 0, 2, "200X200 EPAPER DISPLAY");
  draw_paint();
}

void drawClockOutline() {
  int x_end, y_end;
  
  draw_circle(CLOCK_CENTER_X, CLOCK_CENTER_Y, CLOCK_RADIUS, 1);
  draw_filled_circle(CLOCK_CENTER_X, CLOCK_CENTER_Y, 3, 1);
  
  for (int i = 0; i < 12; i++) {
    if (i <= 6)
      x_end = CLOCK_CENTER_X + hour_marker_x[i];
    else
      x_end = CLOCK_CENTER_X - hour_marker_x[i];
      
    if (i >= 9 || i < 3)
      y_end = CLOCK_CENTER_Y + hour_marker_y[i];
    else
      y_end = CLOCK_CENTER_Y - hour_marker_y[i];   
    draw_filled_circle(x_end, y_end, 2, 1);
  }
}

void drawHourHand(uns8 hour) {
  hour = hour % 12;
  int x_end, y_end;
  if (hour <= 6)
    x_end = CLOCK_CENTER_X + hour_x_offset[hour];
  else
    x_end = CLOCK_CENTER_X - hour_x_offset[hour];
    
  if (hour >= 9 || hour < 3)
    y_end = CLOCK_CENTER_Y + hour_y_offset[hour];
  else
    y_end = CLOCK_CENTER_Y - hour_y_offset[hour];
  draw_line(CLOCK_CENTER_X, CLOCK_CENTER_Y, x_end, y_end, 1);
  delay(1); 
}

void drawMinuteHand(uns8 minute) {
  minute = minute % 60;   //0-59 only
  int x_end, y_end;
  
  if (minute <= 30)
    x_end = CLOCK_CENTER_X + min_x_offset[minute];
  else
    x_end = CLOCK_CENTER_X - min_x_offset[minute];
    
  if (minute >= 45 || minute < 15)
    y_end = CLOCK_CENTER_Y + min_y_offset[minute];
  else
    y_end = CLOCK_CENTER_Y - min_y_offset[minute];
  draw_line(CLOCK_CENTER_X, CLOCK_CENTER_Y, x_end, y_end, 1); //12
}

void initHourOffsets() {
  for (int i = 0; i < 12; i++) {
    if (i == 0 || i == 6) {
      hour_x_offset[i] = 0;
      hour_y_offset[i] = HOUR_HAND_LEN;
      hour_marker_x[i] = 0;
      hour_marker_y[i] = CLOCK_RADIUS - 4;
    }
    else if (i == 3 || i == 9) {
      hour_y_offset[i] = 0;
      hour_x_offset[i] = HOUR_HAND_LEN;
      hour_marker_x[i] = CLOCK_RADIUS - 4;
      hour_marker_y[i] = 0;
    }
    else if ((i % 2) == 1) {
      hour_x_offset[i] = (int)(HOUR_HAND_LEN * ((cos(deg2rad(60)))));
      hour_y_offset[i] = (int)(HOUR_HAND_LEN * ((sin(deg2rad(60)))));
      hour_marker_x[i] = (int)((CLOCK_RADIUS - 4) * ((cos(deg2rad(60)))));
      hour_marker_y[i] = (int)((CLOCK_RADIUS - 4) * ((sin(deg2rad(60)))));
    }
    else {
      hour_x_offset[i] = (int)(HOUR_HAND_LEN * ((cos(deg2rad(30)))));
      hour_y_offset[i] = (int)(HOUR_HAND_LEN * ((sin(deg2rad(30)))));
      hour_marker_x[i] = (int)((CLOCK_RADIUS - 4) * ((cos(deg2rad(30)))));
      hour_marker_y[i] = (int)((CLOCK_RADIUS - 4) * ((sin(deg2rad(30)))));
    }
  }
}

void initMinuteOffsets() {
  int angle; 
  for (int i = 0; i < 60; i++) {
    angle = 6 * (i % 15);   //Give the angle in Quadrnt 1
    
    if (i == 0 || i == 30) {
      min_x_offset[i] = 0;
      min_y_offset[i] = MIN_HAND_LEN;
    }
    else if (i == 15 || i == 45) {
      min_y_offset[i] = 0;
      min_x_offset[i] = MIN_HAND_LEN;
    }
    else if  (i < 15) {
      min_x_offset[i] = (int)(MIN_HAND_LEN * ((sin(deg2rad(angle)))));
      min_y_offset[i] = (int)(MIN_HAND_LEN * ((cos(deg2rad(angle)))));
    }
    else if (i < 30 && i > 15) {
      min_x_offset[i] = (int)(MIN_HAND_LEN * ((cos(deg2rad(angle)))));
      min_y_offset[i] = (int)(MIN_HAND_LEN * ((sin(deg2rad(angle)))));
    }
    else if (i > 30 && i < 45) {
      min_x_offset[i] = (int)(MIN_HAND_LEN * ((sin(deg2rad(angle)))));
      min_y_offset[i] = (int)(MIN_HAND_LEN * ((cos(deg2rad(angle)))));
    }
    else {
      min_x_offset[i] = (int)(MIN_HAND_LEN * ((cos(deg2rad(angle)))));
      min_y_offset[i] = (int)(MIN_HAND_LEN * ((sin(deg2rad(angle)))));
    }
  }
}

void setRTC() {
  rtc.setYear(16);
  rtc.setMonth(10);
  rtc.setDate(14);
  rtc.setDayOfWeek(6);
  rtc.setHours(14);
  rtc.setMinutes(12);
  rtc.setSeconds(5);
}

