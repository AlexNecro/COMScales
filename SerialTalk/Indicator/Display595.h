#define Display595_MAXDIGITS 8

#define LED_A 10
#define LED_Z LED_A + ('z'-'a')
#define LED_SPACE LED_Z+1
#define LED_GRAD LED_SPACE+1
#define LED_EXCL LED_GRAD+1
#define LED_POINT LED_EXCL+1
#define LED_UNDER LED_POINT+1
#define LED_MINUS LED_UNDER+1
#define LED_OVER LED_MINUS+1

const byte digit[] = {
      0b11000000, // 0
      0b11111001, // 1
      0b10100100, // 2
      0b10110000, // 3
      0b10011001, // 4
      0b10010010, // 5
      0b10000010, // 6
      0b11111000, // 7
      0b10000000, // 8
      0b10010000, // 9       
      0b10001000, // 10 A
      0b10000011, //  b
      0b11000110, //  C
      0b10100001, //  d
      0b10000110, //  E
      0b10001110, //  f
      0b11000010, //  G
      0b10001011, //  h
      0b00010000, // 0 i
      0b00111000, //  j
      0b00100111, //k
      0b11000111, //  L
      0b01010100, //m
      0b00010101, //n
      0b10100011, // o
      0b01100111, //p
      0b01110011, //q
      0b10101111, //  r
      0b10010010, //  S
      0b00001111, //t
      0b00111110, //u
      0b00011100, //v
      0b00101011, //w
      0b00110111, //x 
      0b00111011, //y
      0b10100100, //z
      0b11111111, // space
      0b10011100, //  gradus
      0b01111101, // !      
      0b01111111, // .      
      0b11110111, // _            
      0b10111111, //  -
      0b11111110, //  ^      
};
//AbCdEfGh
/*const byte alpha[26] = {
  
}*/

class Display595 {
  public:
    Display595();
    void Init(byte DIO /* data */, byte SCK /* clock */, byte RCK /* latch */, byte nDigits);//after that mask for each digit must be set!
    void SetMask(byte  pos, byte mask);
    byte Decode(char ch);
    void ShowChar(byte pos, char code, byte withPoint = 0);
    void ShowString(int pos, const char* string);//pos CAN be negative!    
    void Clear();

  protected:
    byte mask[Display595_MAXDIGITS];
    byte DIO;
    byte SCK;
    byte RCK;
    byte Digits;
};//class Display595

Display595::Display595() {
  
}

void Display595::Init(byte DIO, byte SCK, byte RCK, byte nDigits) {
  this->DIO = DIO;
  this->SCK = SCK;
  this->RCK = RCK;
  Digits = nDigits;  

  pinMode(DIO, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(RCK, OUTPUT);
  
  mask[0] = 0b00010000;
  mask[1] = 0b00100000;
  mask[2] = 0b01000000;
  mask[3] = 0b00000001;
  mask[4] = 0b00000010;
  mask[5] = 0b00000100;
  mask[6] = 0b00001000;
  mask[7] = 0b10000000;
}

void Display595::SetMask(byte pos, byte mask){
  Display595::mask[pos] = mask;
}

byte Display595::Decode(char ch) {
  if (ch >='0' && ch <= '9') return digit[ch-'0'];
  if (ch >='a' && ch <= 'z') return digit[LED_A+ch-'a'];
  if (ch >='A' && ch <= 'Z') return digit[LED_A+ch-'A'];
  switch (ch) {    
    case '!': return digit[LED_EXCL]; break;
    case '\'':return digit[LED_GRAD]; break;
    case '.': return digit[LED_POINT]; break;
    case '-': return digit[LED_MINUS]; break;
    case '^': return digit[LED_OVER]; break;
    case ' ': return digit[LED_SPACE]; break;
    default: return digit[LED_UNDER];
  }
}

void Display595::ShowChar(byte pos, char code, byte withPoint) {
  digitalWrite(RCK, LOW);//start transmission  
  if (withPoint)
    shiftOut(DIO, SCK, MSBFIRST, Decode(code) & digit[LED_POINT]); //char
  else
    shiftOut(DIO, SCK, MSBFIRST, Decode(code)); //char
  shiftOut(DIO, SCK, MSBFIRST, mask[pos]);//pos  
  digitalWrite(RCK, HIGH);//end trans
}

void Display595::Clear() {
  for (int pos=0;pos<Digits;pos++)
    ShowChar(pos, ' ');
}

void Display595::ShowString(int pos, const char* string) {        
  if (pos>=Digits) {
    Clear();
    return;
  }
  if (pos<0) {
    string-=pos;
    pos = 0;
  }
  while (*string && (pos<Digits)) {
    if (*(string+1)=='.') {
      ShowChar(pos, *string, 1);
      string++;//skip point
    } else
      ShowChar(pos, *string, 0);
    pos++;
    string++;
  }
}
