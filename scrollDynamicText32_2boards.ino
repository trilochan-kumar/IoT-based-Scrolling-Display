// Includes
#include <DMD32.h>
/*#include "fonts/SystemFont5x7.h"*/
#include "fonts/Arial_black_16.h"<arial_black_16.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

WiFiMulti WiFiMulti;

const char* ssid     = "2o22";
const char* password = "19o82oo2";
const char* rss_url = "https://1-updates.blogspot.com/feeds/posts/default?alt=rss";

//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

//Timer setup || create a hardware timer  of ESP32
hw_timer_t * timer = NULL;

/*--------------------------------------------------------------------------------------
  Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, this gets
  called at the period set in Timer1.initialize();
--------------------------------------------------------------------------------------*/
void IRAM_ATTR triggerScan()
{
  dmd.scanDisplayBySPI();
}

void scrollText(String dispString) 
{
  dmd.clearScreen( true );
  dmd.selectFont( Arial_Black_16 );
  char newString[256];
  int sLength = dispString.length();
  dispString.toCharArray( newString, sLength+1 );
  dmd.drawMarquee(newString,sLength,( 32*DISPLAYS_ACROSS )-1 , 0 );
  long start=millis();
  long timer=start;
  long timer2=start;
  boolean ret=false;
  while(!ret)
  {
    if ( ( timer+20 ) < millis() ) 
    {
      ret=dmd.stepMarquee( -1 , 0 );
      timer=millis();
      delay(30);
    }
  }
}

void setup(void)
{

  Serial.begin(9600);

  WiFiMulti.addAP(ssid, password);
  while (WiFiMulti.run() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  pinMode(2,OUTPUT);

  uint8_t cpuClock = ESP.getCpuFreqMHz();
  timer = timerBegin(0, cpuClock, true);
  timerAttachInterrupt(timer, &triggerScan, true);
  timerAlarmWrite(timer, 300, true);
  timerAlarmEnable(timer);
  dmd.clearScreen( true );

  scrollText("All Set!");

}

void loop()
{
  while (WiFiMulti.run() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  String text = fetchRssFeed();
  Serial.println(text);

  digitalWrite(2,HIGH);
  delay(100);
  digitalWrite(2,LOW);


  //String text = "narendra thrillu abhi";
  scrollText(text);
  Serial.println("Text is set!\n");
}

String fetchRssFeed() 
{
  // Make an HTTP request to fetch the RSS feed data
  HTTPClient http;
  http.begin(rss_url);
  int httpCode = http.GET();

  // Check if the request was successful
  if (httpCode == HTTP_CODE_OK) {
    String xmlString = http.getString();

    // Find the second and third occurrences of the <title> tag in the XML string
    int titleStartIndex = 0;
    int titleEndIndex = 0;
    int entryCount = 0;
    String topEntry;
    while (entryCount < 3 && titleStartIndex >= 0 && titleEndIndex >= 0) 
    {
      titleStartIndex = xmlString.indexOf("<title>", titleEndIndex);
      titleEndIndex = xmlString.indexOf("</title>", titleStartIndex);
      if (titleStartIndex >= 0 && titleEndIndex >= 0) 
      {
        if (entryCount > 0) 
        {
          topEntry += xmlString.substring(titleStartIndex + 7, titleEndIndex);
          break;
        }
        entryCount++;
      }
    }

    return topEntry;
  }
  else {
    Serial.printf("HTTP error code: %d", httpCode);
    return "";
  }

  http.end();
}
