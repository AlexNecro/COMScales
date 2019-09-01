//НВТ-9: "wn0001.000kg\r\n"
void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  pinMode(2, INPUT);
  digitalWrite(2,HIGH);//add pull-up resistor, logic inverts! (HIGH == off, LOW == on)
  pinMode(3, INPUT);
  digitalWrite(3,HIGH);
  pinMode(4, INPUT);
  digitalWrite(4,HIGH);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }  
}

void loop() {    
  int mode = getMode();  
  int model = (mode & 0x06) >> 1;
  mode = mode & 0x01;    
  switch (model) {
    case 0: SendHBT9(mode); break;
    case 1: SendToledo(mode); break;
    case 2: SendBU6342M1(mode); break;
    case 3: SendCAS6000(mode); break;
    case 4: SendFT11(mode); break;
  }  
  delay(50);
}

int getMode() {
  byte pin2 = digitalRead(2);
  byte pin3 = digitalRead(3);
  byte pin4 = digitalRead(4);
  return ((pin2 == LOW)?1:0) + ((pin3 == LOW)?2:0) + ((pin4 == LOW)?4:0);
}

void SendHBT9(int mode)
{  
  //HBT-9:
  switch (mode) {
    case 0: Serial.write("wn0001.000kg\r\n"); break;    
    case 1: Serial.write("wn0012.146kg\r\n"); break;
  }   
}

void SendToledo(int mode)
{
  //Mettler Toledo:  
  #define sizeToledo 17   
  byte buf[sizeToledo ] = {0x01 ,0x29 ,0x30 ,0x21 ,0x20 ,0x20 ,0x31 ,0x33 ,0x38 ,0x30 ,0x20 ,0x20 ,0x20 ,0x20 ,0x30 ,0x30 ,0x0D}; //1380
  Serial.write(buf, sizeToledo );
}

void SendBU6342M1(int mode)
{
  //BU6342M1:  
  #define sizeBU6342M1 12
  byte buf[sizeBU6342M1] = {0x23, 0x30, 0x03, 0x33, 0x00, 0x24, 0x23, 0x4E, 0x94, 0x00, 0x00, 0x24}; //1480
  Serial.write(buf, sizeBU6342M1);
}

void SendCAS6000(int mode)
{
  //CAS6000:  
  #define sizeCAS6000 22
  byte buf0[sizeCAS6000] = {0x53 ,0x54 ,0x2C ,0x47 ,0x53 ,0x2C ,0x44 ,0xC5 ,0x2C ,0x20 ,0x20 ,0x20 ,0x20 ,0x30 ,0x2E ,0x30 ,0x30 ,0x00 ,0x20 ,0x74 ,0x0D ,0x0A}; //0
  byte buf133[sizeCAS6000] = {0x53 ,0x54 ,0x2C ,0x47 ,0x53 ,0x2C ,0x44 ,0xC4 ,0x2C ,0x20 ,0x20 ,0x20 ,0x20 ,0x31 ,0x2E ,0x33 ,0x33 ,0x00 ,0x20 ,0x74 ,0x0D ,0x0A}; //1.33                          
  switch (mode) {
    case 0: Serial.write(buf0, sizeCAS6000); break;    
    case 1: Serial.write(buf133, sizeCAS6000); break;
  }  
}

void SendFT11(int mode)
{
  //FT11:  
  #define sizeFT11 18   
  byte buf[sizeFT11 ] = {0x0E  ,0x02  ,0x69  ,0x70  ,0x30  ,0x20  ,0x20  ,0x31  ,0x33  ,0x36  ,0x30  ,0x20  ,0x20  ,0x20  ,0x20  ,0x20  ,0x30  ,0x0D}; //1360
  Serial.write(buf, sizeFT11 );
}
