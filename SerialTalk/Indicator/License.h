class License {
  public:
    License();
    void Init(unsigned long key, int seed);
    void Encode(char* buffer, int size);//in-place
    void Decode(char* buffer, int size);

  protected:
    char key[4];
};

License::License() {
  
}

void License::Init(unsigned long key, int seed){
  this->key[0] = (key>>24)&0xff;
  this->key[1] = (key>>16)&0xff;
  this->key[2] = (key>>8)&0xff;
  this->key[3] = (key)&0xff;
}

void License::Encode(char* buffer, int size){
  for (int i=0; i<size; i++) {
    buffer[i] = buffer[i]^key[i%4];
  }
}

void License::Decode(char* buffer, int size){
  for (int i=0; i<size; i++) {
    buffer[i] = buffer[i]^key[i%4];
  }
}