/*

 * I2C Shell V1.0 
 * Copyright (C) 2014 by mes3hacklab
 * This file is part of Ardiuno php i2c utility
 * 
 * I2 CShell is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this source code; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
 Hadrware informations:
 
  Board               I2C / TWI pins
  Uno, Ethernet       A4 (SDA), A5 (SCL)
  Mega2560            20 (SDA), 21 (SCL)
  Leonardo            2 (SDA), 3 (SCL)
  Due                 20 (SDA), 21 (SCL), SDA1, SCL1
  
  Supported I2C EEPROM:
    24C00
    24C01
    24C02
    24C04
    24C08
    24C16
    24C32
    24C64
    24C65
    24C128
    24C256
    24C512
    24C1024
  */

  #include <Wire.h> //I2C library
  
  boolean longAddr = true;
  byte icAddress = 0x50;
  byte buffer[16];
  unsigned int length = 0;
  unsigned int lastAddress = 0;
  int LEDPIN = 13;  
  int addressMode = 0;
  int icId=0;
  int hiShift=8;
  int chipSet=2;
  boolean lastok=true;
    
  void setChipSet(int id) {
    icAddress = 0x50;
    addressMode=0;
    hiShift=16;
    chipSet=2;
    icId=0;
    longAddr=false;
    
    if (
      id==0 ||
      id==1 ||
      id==2 
      ) chipSet=id;
    
    if (id==4) {
        addressMode=4;
        hiShift=8;
        chipSet=4;
        }
    
    if (id==8) {
      addressMode=8;
      hiShift=8;
      chipSet=8;
      }
     
    if (id==16) {
      addressMode=16;
      hiShift=8;
      chipSet=16;
      }
     
    if (
      id == 32 ||
      id == 64 ||
      id == 128 ||
      id == 256 ||
      id == 512
       ) {
       chipSet=id;
       longAddr=true;
       addressMode=0;
       hiShift=16;
     } 
    
    if (id==1024) {
      chipSet=1024;
      longAddr=true;
      addressMode=4;
      hiShift=16;
      }

  }
  
  int getDeviceAddress(unsigned int hiAddress) {
    hiAddress = hiAddress>>hiShift;
    int da = 0;
    if (addressMode == 0) da=(icId&7); //Ind. Direct 8 or 16 bits.
    if (addressMode == 4) da=(icId&3)<<1 | (hiAddress&1);
    if (addressMode == 8) da=(icId&1)<<2 | (hiAddress&3);
    if (addressMode == 16) da = hiAddress&7;
    return icAddress | (da<<1);
  }

  void i2c_poke( unsigned int eeaddress, byte data ) {
    int deviceaddress = getDeviceAddress(eeaddress);
    int rdata = data;
    Wire.beginTransmission(deviceaddress);
    if (longAddr) Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.write(rdata);
    Wire.endTransmission();
  }
  
  void i2c_stosb(unsigned int eeaddresspage, byte* data, byte length ) {
    int deviceaddress = getDeviceAddress(eeaddresspage);
    Wire.beginTransmission(deviceaddress);
    if (longAddr) Wire.write((int)(eeaddresspage >> 8)); // MSB
    Wire.write((int)(eeaddresspage & 0xFF)); // LSB
    for (byte c = 0; c < length; c++) {
      Wire.write(data[c]);
      }
    Wire.endTransmission();
  }

  byte i2c_peek(unsigned int eeaddress ) {
    int deviceaddress = getDeviceAddress(eeaddress);
    byte rdata = 0xFF;
    Wire.beginTransmission(deviceaddress);
    if (longAddr) Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,1);
    if (Wire.available()) rdata = Wire.read(); else lastok=false;
    return rdata;
  }

  void detect() {
  int oldc = chipSet;
  int oldi = icId;
  for (int ic = 0; ic < 7; ic++) {
    for (int it = 2 ; it < 11; it++) {
      lastok=true;
      setChipSet(1<<it);
      icId = ic;
      i2c_peek(0);
      if (lastok) {
        Serial.print(ic);
        Serial.print(1<<it);
        Serial.println("P");
        break;
        }
      }
    }
  setChipSet(oldc);
  icId = oldi;
  Serial.print("K");
  }

  void i2c_lodsb(unsigned int eeaddress, byte *buffer, int length ) {
    int deviceaddress = getDeviceAddress(eeaddress);
    Wire.beginTransmission(deviceaddress);
    if (longAddr) Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,length);
    int c = 0;
    for ( c = 0; c < length; c++ )
      if (Wire.available()) buffer[c] = Wire.read(); else lastok=false;
  }

int serialRead() {
  unsigned long time;
  unsigned long tim0;
  int cnt=0;
  tim0=millis();
  while(cnt<2000) {
    time=millis();
    if (Serial.available()) return Serial.read();
    if (time!=tim0) {
      cnt++;
      tim0=time;
      }
    }
  Serial.println("T");  
}

void writeHex(int dw,int sz) {
  sz--;
  for (int i = 0; i<=sz; i++) {
	int n = (sz-i)*4;
	n = 15 & (dw>>n);
        char q = 0;
	if (n<10) q=48+n; else q=55+n;
        Serial.write(q);
	}
}
  int readHex(int len) {
    int rs=0;
    boolean ok=true;

    for (int i = 0; i<len; i++) {
	if (ok) rs=rs<<4;
	ok=false;
        int c = serialRead();
       
	if (c>47 && c<58) { 
            rs|=(c-48); 
            ok=true; 
          }
          
	if (c>64 && c<71) { 
            rs|=(c-55); 
            ok=true; 
          }
          
	if (c>96 && c<104) {
           rs|=(c-87); 
           ok=true; 
          }
      }
    return rs;
   }
int readDec(int len) {
    int rs=0;
    boolean ok=true;

    for (int i = 0; i<len; i++) {
	if (ok) rs=rs*10;
	ok=false;
        int c = serialRead();
       
	if (c>47 && c<58) { 
            rs+=(c-48); 
            ok=true; 
          }
      }
    return rs;
   }
  void setup() 
  {

    Wire.begin(); 
    Serial.begin(57600);
    pinMode(LEDPIN,OUTPUT);
    setChipSet(256);
  }

  void loop() 
  {
   if (Serial.available()) {
     char c = Serial.read();
     if (c == 'V') Serial.println("I2CE0100K");
     
     if (c == 'K') {
       digitalWrite(LEDPIN,HIGH);
       lastok=true;
       setChipSet(256);
       icId=0;
       icAddress=0x50;
       lastAddress=0;
       length=0;
       Wire.endTransmission();
       Serial.flush();
       delay(250);
       digitalWrite(LEDPIN,LOW);
       Serial.println("K");
       }
     
     if (c == 'I') {
       icId = readHex(1)&7;
       Serial.println("K");
       }
     
     if (c == 'Q') {
       Serial.print(icId&7);
       Serial.println("K");
       }
     
     if (c == 'C') {
       int rs = readDec(4);
       setChipSet(rs);
       if (rs == chipSet) Serial.println("K"); else Serial.println("U");
       }
       
     if (c == 'G') {
       Serial.print(chipSet);
       Serial.println("K");
       }
      
     if (c == 'X') detect();
      
     if (c == 'W') {
       lastAddress = readHex(4);
       length = readHex(1);
       if (length==0) length=16;
       for (int i = 0; i < length; i++) {
         int b = readHex(2);
         buffer[i] = (byte) (b&0xff);
         }
       i2c_stosb(lastAddress, (byte *)buffer, length); 
       Serial.println("K");
       }
     
     if (c == 'D') {
       int pag = readHex(4);
       lastAddress = 0;
       
       for (int pc = 0 ; pc < pag ; pc++) {
         lastok=true;
         for (int i = 0; i< 16; i++ ) {
           byte b = i2c_peek(lastAddress);
           lastAddress++;
           writeHex((int)(b&0xff),2);
           }
         if (lastok) Serial.println("P"); else Serial.println("I");
         }
       Serial.println("K");
       }
     
     if (c == 'A') {
       lastAddress = readHex(4);
       length = readHex(4);
       boolean oncr = length == 0;
       if (oncr) length = 512;
       for (int i = 0; i< length; i++ ) {
           byte b = i2c_peek(lastAddress);
           lastAddress++;
           if (oncr && b==0) {
             Serial.println("");
             break;
             }
           Serial.write(b);
           }
       Serial.println("K");
       }
     
     if (c == 'S' || c == 'F') {
       lastAddress = readHex(4);
       length = readHex(4);
       int eof = 13;
       if ( c == 'F' ) eof = serialRead();
       boolean oncr = length == 0;
       if (oncr) length=512;
       for (int i = 0; i< length; i++ ) {
         int c = serialRead();
         if (oncr && c == eof) c=0;
         i2c_poke(lastAddress, (byte) (c&0xff));
         if (oncr && c == 0) break;         
         lastAddress++;
         }
       Serial.println("K");
       }
          
     if (c == 'Z') {
       lastAddress = readHex(4);
       Serial.println("K");      
       }
       
     if (c == 'L') {
       for (int i = 0; i < length; i++) {
         int b = readHex(2);
         buffer[i] = (byte) (b&0xff);
         }
       i2c_stosb(lastAddress, (byte *)buffer, length); 
       Serial.print("Z");
       writeHex(lastAddress,4);
       Serial.println("K");
       }
       
     if (c == 'R') {
       lastAddress = readHex(4);
       length = readHex(1);
       if (length==0) length=16;       
       lastok=true;
       for (int i = 0; i< length; i++ ) {
         byte b = i2c_peek(lastAddress);
         lastAddress++;
         writeHex((int)(b&0xff),2);
       }
       if (lastok) Serial.println("K"); else Serial.println("I");
     } 
     
     if (c == 'M') {
       int rs = readHex(2);
       if (rs == 0xff) {
         icAddress = 0x50;
         longAddr = true;
         lastAddress = 0;
       } else {
         if (rs & 0x10) longAddr=true; else longAddr=false;
         icAddress = 0x50 | (0x0E & rs);
       }
       Serial.println("K");
     }
    
    if (c == 'E') {
      int rs = readHex(1);
      if (rs==0) digitalWrite(LEDPIN,LOW);
      if (rs==1) digitalWrite(LEDPIN,HIGH);
      if (rs>1) {
        rs--;
        for (int i = 0; i<= rs; i++) {
            digitalWrite(LEDPIN,HIGH);
            delay(100);
            digitalWrite(LEDPIN,LOW);
            if (rs!=i) delay(100);            
        }
        Serial.println("K");
      }
    }
     
   }
  }

