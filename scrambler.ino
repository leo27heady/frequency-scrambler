#include <SPI.h>

int slaveSelect = 2;
int delayTime = 50;

String inString = ""; 
int input_counter = 0;

bool sq_input = true;
int negative = 0;
String inStringK = ""; 

int data[64];
unsigned short Key = 65280;

float out_r[64]={};   //real part of transform
float out_im[64]={};  //imaginory part of transform

//int data[64]={14, 30, 35, 34, 34, 40, 46, 45, 30, 4,  -26,  -48,  -55,  -49,  -37,
//-28,  -24,  -22,  -13,  6,  32, 55, 65, 57, 38, 17, 1,  -6, -11,  -19,  -34, 
//-51,  -61,  -56,  -35,  -7, 18, 32, 35, 34, 35, 41, 46, 43, 26, -2, -31,  -50,
//-55,  -47,  -35,  -27,  -24,  -21,  -10,  11, 37, 58, 64, 55, 34, 13, -1, -7
//};
//*/

void scrambler(){
  unsigned short shift_check = 2;
  int second_part = (64 / 2);
  float left_first;
  float left_second;

  float right_first;
  float right_second;
  
  for(int i = 1; i < 15; i++ ) {
    if ((shift_check & Key) > 0){
      ///////////REAL///////////
      left_first = out_r[i];
      left_second = out_r[second_part - i];

      right_first = out_r[i + second_part];
      right_second = out_r[(second_part*2) - i];

      out_r[i] = left_second;
      out_r[second_part - i ] = left_first;
      out_r[i + second_part] = right_second;
      out_r[(second_part*2) - i] = right_first;


      ///////////IMAGE///////////
      left_first = out_im[i];
      left_second = out_im[second_part - i];

      right_first = out_im[i + second_part];
      right_second = out_im[(second_part*2) - i];

      out_im[i] = left_second;
      out_im[second_part - i] = left_first;
      out_im[i + second_part] = right_second;
      out_im[(second_part*2) - i] = right_first;
    }
    shift_check <<= 1;
  }

//    for(int i = 15; i >= 0; i--) {
//      float left_first = out_r[i];
//      float left_second = out_r[31-i];
//
//      float right_first = out_r[i+32];
//      float right_second = out_r[32+31-i];
//
//      out_r[i] = left_second;
//      out_r[31-i] = left_first;
//      out_r[i+32] = right_second;
//      out_r[32+31-i] = right_first;
//
//
//      left_first = out_im[i];
//      left_second = out_im[31-i];
//
//      right_first = out_im[i+32];
//      right_second = out_im[32+31-i];
//
//      out_im[i] = left_second;
//      out_im[31-i] = left_first;
//      out_im[i+32] = right_second;
//      out_im[32+31-i] = right_first;
//    }
  
  Serial.println("\n\nReal + Image transformed sequence AFTER scrambling");
  for(int i=0;i<64;i++){
    Serial.print(float(out_r[i])/64);
    Serial.print(' ');                                     // uncomment to print RAW o/p    
    Serial.print(float(out_im[i])/64); 
    Serial.print('/');      
  }
}

void setup(){
  Serial.begin(9600);
  pinMode(slaveSelect, OUTPUT);
  SPI.begin();
  SPI.setBitOrder(LSBFIRST);  
  
  Serial.print("Input sequential: ");         
}

        
void loop(){

while (Serial.available() > 0) {
  if (sq_input){
    int inChar = Serial.read();
    if ((char)inChar == '-'){
        negative = 1;
    }
    
    if (isDigit(inChar)) {
      inString += (char)inChar; 
    }

    if (inChar == ' ') {
      data[input_counter] = inString.toInt();
      if (negative == 1){
        data[input_counter] = -1 * data[input_counter];
      }
      inString = "";
      input_counter++;
      negative = 0;
    }

    if (inChar == '\n') {
      data[input_counter] = inString.toInt();
      if (negative == 1){
        data[input_counter] = -1 * data[input_counter];
      }

      for(int i=0;i<64;i++){
        Serial.print(data[i]);
        Serial.print(' ');                                    
      }
      Serial.print("\nSequence received successfully\n");
          
      negative = 0;
      inString = "";
      input_counter = 0;
      sq_input = false;
      Serial.print("\nKey value: ");
    }
  }

  if (!sq_input){
    while (Serial.available() > 0) {
      int inChar = Serial.read();
      if (isDigit(inChar)) {
        inStringK += (char)inChar; 
      }
      
      if (inChar == '\n') {
        Key = inStringK.toInt();
        Serial.println(Key);
        Serial.print("Key received successfully\n");

        inStringK = "";
        sq_input = true;
        
        FFT(data,64);
        scrambler();
        IFFT(64);
        
//        SPI.transfer(output_num+1); 
        for(int i=0;i<64;i++){
          digitalWrite(slaveSelect, LOW);
          SPI.transfer(int(float(out_r[i])/64));
          digitalWrite(slaveSelect, HIGH);
          delay(50);
        }
        digitalWrite(slaveSelect, LOW);
        SPI.transfer(255);
        digitalWrite(slaveSelect, HIGH);
       
        
        Serial.print("\n\n\nInput sequential: ");
      }
    }
  }
}


//  delay(99999);
            
}



//-----------------------------FFT Function----------------------------------------------//

float FFT(int in[],byte N){
  
  unsigned int data[13]={1,2,4,8,16,32,64,128,256,512,1024,2048};
  int a,c1,f,o,x;
  a=N;  
  
  //calculating the levels                          
  for(int i = 0; i < 12; i++){
    if(data[i] <= a){
      o=i;
    }
  }
        
  int in_ps[data[o]]={};     //input for sequencing

             
  x = 0;  
  // bit reversal
  for(int b = 0; b < o; b++){
    c1 = data[b];
    f = data[o] / (c1 + c1);
      for(int j = 0; j < c1; j++){ 
        x++;
        in_ps[x]= in_ps[j] + f;
      }
   }
  
  // update input array as per bit reverse order
  for(int i = 0; i < data[o]; i++){
    if(in_ps[i] < a){
      out_r[i] = in[in_ps[i]];
    }
    if(in_ps[i] > a){
      out_r[i] = in[in_ps[i] - a];
    }      
  }


  int i10,i11,n1;
  float e,c,s,tr,ti;

  //fft
  for(int i = 0; i < o; i++){
    i10=data[i];              // overall values of sine/cosine  :
    i11=data[o]/data[i+1];    // loop with similar sine cosine:
    e=6.283185307/data[i+1];
    e=0-e;
    n1=0;
  
    for(int j=0;j<i10;j++){
      c=cos(e*j);
      s=sin(e*j);    
      n1=j;
  
      for(int k=0;k<i11;k++){
        tr=c*out_r[i10+n1]-s*out_im[i10+n1];
        ti=s*out_r[i10+n1]+c*out_im[i10+n1];
    
        out_r[n1+i10]=out_r[n1]-tr;
        out_r[n1]=out_r[n1]+tr;
    
        out_im[n1+i10]=out_im[n1]-ti;
        out_im[n1]=out_im[n1]+ti;          
    
        n1=n1+i10+i10;
      }       
    }
  }

//  /*
  Serial.println("\n\nReal + Image transformed sequence BEFORE scrambling");
  for(int i=0;i<data[o];i++){
    Serial.print(float(out_r[i])/N);
    Serial.print(' ');                                     // uncomment to print RAW o/p    
    Serial.print(float(out_im[i])/N); 
    Serial.print('/');      
  }
//  */
  

  
} // END FFT function
    

//-----------------------------IFFT Function----------------------------------------------//

float IFFT(byte N){
  
  unsigned int data[13]={1,2,4,8,16,32,64,128,256,512,1024,2048};
  int a,c1,f,o,x;
  a=N;  

 
  for(int i = 0; i < N; i++){
    out_im[i] = -1 * out_im[i];
  }

  
  //calculating the levels                          
  for(int i = 0; i < 12; i++){
    if(data[i] <= a){
      o=i;
    }
  }
        
  int in_ps[data[o]]={};     //input for sequencing
  float out_r_cp[data[o]]={};   //real part of transform
  float out_im_cp[data[o]]={};  //imaginory part of transform
             
  x = 0;  
  // bit reversal
  for(int b = 0; b < o; b++){
    c1 = data[b];
    f = data[o] / (c1 + c1);
      for(int j = 0; j < c1; j++){ 
        x++;
        in_ps[x] = in_ps[j] + f;
      }
   }
  
  // update input array as per bit reverse order
  for(int i = 0; i < data[o]; i++){
    if(in_ps[i] < a){
      out_r_cp[i] = out_r[in_ps[i]];
      out_im_cp[i] = out_im[in_ps[i]];
    }
    if(in_ps[i] > a){
      out_r_cp[i] = out_r[in_ps[i] - a];
      out_im_cp[i] = out_im[in_ps[i] - a];
    }      
  }

  for(int i = 0; i < N; i++){
    out_r[i] = out_r_cp[i];
    out_im[i] = out_im_cp[i];
  }



  int i10,i11,n1;
  float e,c,s,tr,ti;

  //ifft
  for(int i = 0; i < o; i++){
    i10=data[i];              // overall values of sine/cosine  :
    i11=data[o]/data[i+1];    // loop with similar sine cosine:
    e=6.283185307/data[i+1];
    e=0-e;
    n1=0;
  
    for(int j=0;j<i10;j++){
      c=cos(e*j);
      s=sin(e*j);    
      n1=j;
  
      for(int k=0;k<i11;k++){
        tr=c*out_r[i10+n1]-s*out_im[i10+n1];
        ti=s*out_r[i10+n1]+c*out_im[i10+n1];
    
        out_r[n1+i10]=out_r[n1]-tr;
        out_r[n1]=out_r[n1]+tr;
    
        out_im[n1+i10]=out_im[n1]-ti;
        out_im[n1]=out_im[n1]+ti;          
    
        n1=n1+i10+i10;
      }       
    }
  }


  for(int i = 0; i < data[o]; i++){
    out_im[i] = -1 * out_im[i];
  }
  
//  /*
  Serial.println("\n\nData sequence AFTER scrambling");
  for(int i=0;i<data[o];i++){
    Serial.print(int(float(out_r[i])/N));
    Serial.print(' ');                                     // uncomment to print RAW o/p    
//    Serial.print(float(out_im[i])/N); 
//    Serial.print('/');      
  }
//  */
    
} // END IFFT function
