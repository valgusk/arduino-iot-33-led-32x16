#include <Arduino.h>
#include <WiFi.h>

int status = WL_DISCONNECTED;
WiFiClient wifiClient;

uint8_t buffer[768];

//uint8_t* buffer = defaultbuffer;

WiFiServer server(80);
WiFiClient client;

// actual pin numbers
#define RA 14  // A0 register selector a
#define RB 15  // A1 register selector b
#define RC 16  // A2 register selector c
#define RD 17  // A3 register selector d
#define RF 2   // red first byte
#define RS 5   // red second byte
#define BF 4   // blue first byte
#define BS 7   // blue second byte
#define GF 3   // green first byte
#define GS 6   // green second byte
#define LAT 18 // A4 data latch
#define CLK 8  // clock signal
#define OE 9   // output enable


#define FREQ 15

void setup() {
  // put your setup code here, to run once:
  pinMode(RA, OUTPUT);
  pinMode(RB, OUTPUT);
  pinMode(RC, OUTPUT);
  pinMode(RD, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(RF, OUTPUT);
  pinMode(RS, OUTPUT);
  pinMode(GF, OUTPUT);
  pinMode(GS, OUTPUT);
  pinMode(BF, OUTPUT);
  pinMode(BS, OUTPUT);
  pinMode(LAT, OUTPUT);
  pinMode(OE, OUTPUT);

  Serial.begin(9600);

  for(int i = 0; i < 768; i++) buffer[i] = 1;
}

uint8_t step = 0;
int time = 0;
int val = 0;
int readbytes = 0;

void loop() {
  // put your main code here, to run repeatedly:
  
  time++;

  step = step + 1;
  if(step > FREQ) {
    step = 0;

    if(time > FREQ * 10) {
      outputOff();
      ensureConnection();
      readMessage();
      
      time = 0;
  
//      delay(300);
    }

    readClient();
    
//    Serial.println((char*)&buffer);
  }
  
  val = (step == FREQ);
  uint pixid = 0;

  uint8_t* addr1 = &buffer[0];
  uint8_t* addr2 = addr1 + 384;

  for(uint8_t line_pair = 0; line_pair < 8; line_pair++) {
    setLine(line_pair - 1);
    outputOff();
    
    for(int pixquad = 0; pixquad < 16; pixquad++) {
      uint8_t starta = *addr1;
      uint8_t middla = *(addr1 + 1);
      uint8_t enda = *(addr1 + 2);

      uint8_t startb = *addr2;
      uint8_t middlb = *(addr2 + 1);
      uint8_t endb = *(addr2 + 2);

      uint8_t ra1 = starta >> 4;
      uint8_t ga1 = starta & 15;
      uint8_t ba1 = middla >> 4;

      uint8_t rb1 = startb >> 4;
      uint8_t gb1 = startb & 15;
      uint8_t bb1 = middlb >> 4;

      setVal(ra1, ga1, ba1, rb1, gb1, bb1);

      uint8_t ra2 = middla & 15;
      uint8_t ga2 = enda >> 4;
      uint8_t ba2 = enda & 15;

      uint8_t rb2 = middlb & 15;
      uint8_t gb2 = endb >> 4;
      uint8_t bb2 = endb & 15;

      setVal(ra2, ga2, ba2, rb2, gb2, bb2);
      
      addr1 = addr1 + 3;
      addr2 = addr2 + 3;
    }

    latch();
    outputOn();
    
  }
}

void setVal(uint8_t ra, uint8_t ga, uint8_t ba, uint8_t rb, uint8_t gb, uint8_t bb) {
  // RF
  if(ra > step) {
    REG_PORT_OUTSET1 = PORT_PB10;
  } else {
    REG_PORT_OUTCLR1 = PORT_PB10;
  }
  
  // GF
  if(ga > step) {
    REG_PORT_OUTSET1 = PORT_PB11;
  } else {
    REG_PORT_OUTCLR1 = PORT_PB11;
  }

  // BF
  if(ba > step) {
    REG_PORT_OUTSET0 = PORT_PA07;
  } else {
    REG_PORT_OUTCLR0 = PORT_PA07;
  }
  
  // RS
  if(rb > step) {
    REG_PORT_OUTSET0 = PORT_PA05;
  } else {
    REG_PORT_OUTCLR0 = PORT_PA05;
  }
  
  // GS
  if(gb > step) {
    REG_PORT_OUTSET0 = PORT_PA04;
  } else {
    REG_PORT_OUTCLR0 = PORT_PA04;
  }

  // BS
  if(bb > step) {
    REG_PORT_OUTSET0 = PORT_PA06;
  } else {
    REG_PORT_OUTCLR0 = PORT_PA06;
  }
  
  clock();
}

void latch() {
  REG_PORT_OUTSET1 = PORT_PB08;
  REG_PORT_OUTCLR1 = PORT_PB08;
}

void clock() {
  REG_PORT_OUTSET0 = PORT_PA18;
  REG_PORT_OUTCLR0 = PORT_PA18;
}

void output() {
  outputOn();
  outputOff();
}

void outputOff() {
  REG_PORT_OUTCLR0 = PORT_PA20;
}

void outputOn() {
  REG_PORT_OUTSET0 = PORT_PA20;
}

void setLine(uint8_t num) {
  // todo according to num
  digitalWrite(RA, num & 1);
  digitalWrite(RB, (num & 2) >> 1);
  digitalWrite(RC, (num & 4) >> 2);
  digitalWrite(RD, (num & 8) >> 3);
}

// WIFI


void ensureConnection() {
  int oldStatus = status;
  status = WiFi.status();
  
  if (status != WL_CONNECTED || status != oldStatus || status == WL_DISCONNECTED) {
    if(status != WL_CONNECTED) {
      Serial.print(status);
      Serial.println("- Wifi not connected!");
      status = WiFi.begin("xxxxxxxx", "xxxxxxxx");

      if(status == WL_CONNECTED) {
        Serial.println("Wifi connected!");
        server.begin();
      }
    } else if(status == WL_CONNECTED) {
      Serial.println("Wifi connected!");
      server.begin();
    }
  }
}

void readClient() {
//  Serial.println(client.available());
  int availableBytes = client.available();
  int bytesToRead = min(availableBytes, 768 - readbytes);

//  if()
  
  if(availableBytes > 0) {
    client.read(buffer + readbytes, bytesToRead);
//    Serial.println((char*)buffer);
    readbytes += bytesToRead;
    if(readbytes == 768) {
      client.write((uint8_t)1);
      client.flush();
      readbytes = 0;
    }
  }
}

void readMessage() {
  if(status != WL_CONNECTED) return;

  if(client.connected()) {
//    readClient();
  } else {
    readbytes = 0;
    
    client.stop();
    
    client = server.available();
    
    if (client) {
      if (client.connected()) {
        Serial.println("client connected!");
//        readClient();
//        Serial.println((char*)&buffer);
      }
  
      // close the connection:
    }
  }
}
