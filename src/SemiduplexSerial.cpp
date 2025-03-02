
#include"SemiduplexSerial.h"

// ****************************
// **** modified block from original file **** //
// ****************************
#include "HardwareSerial.h"
#define u8 unsigned char
#define Txd3 27
#define Rxd3 12
#define Txd2 13
#define Rxd2 14

//HardwareSerial Serial2(1);
HardwareSerial Serial3(2);


// -	Modify the all the call to Serial2.write(buf,length) in all functions: SemiduplexSerial::ubtServoProtocol(…)
// SemiduplexSerial::ubtHumitureProtocol(..) …
// Important: Need to add Serial2.flush() to wait for all the data written out before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work  (although when use with Arduino Mega, this requirement is not needed)

// ****************************
// **** end of modified block from original file **** //
// ****************************

SoftwareSerial mySoftSerial1(VISION_SOFTSERIAL_RXPIN,VISION_SOFTSERIAL_TXPIN);

/**@brief EN:JIMU Servo checksum/CN:JIMU舵机校验
 *
 * @param[in] len EN:Data length/CN：数据长度.
 * @param[in] len EN:Data buffer/CN:数据缓冲区.
 *
 * @returns sum EN: checksum value/CN:校验值.
 */
unsigned char SemiduplexSerial::Cheak_Sum(unsigned char len, unsigned char *buf){
  unsigned char i, sum = 0;
  if(len > 254) return 0;
  for(i = 0; i < len; i++){
    sum += buf[i];
  }
  return  (uint8_t)(sum);
}

unsigned char* SemiduplexSerial::ubtColorProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[20];
  unsigned char Usart3_Rx_Ack_Len=0;
  //unsigned char *rxBuf= new unsigned char[3];  
  static unsigned char rxBuf[3]={0,0,0};
  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
 
 
  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
  if(CMD==0x04){
    Usart3_Rx_Ack_Len = 16; //应答消息长度 
  }
  else{
    Usart3_Rx_Ack_Len = 7; //应答消息长度 
  }
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len + 16) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  if(CMD==0x06){
    delay(10);
  } 
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
  if(tRet == 0){ //没有接收到消息 
    if( tCnt < 2){
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else{ //接收到消息

    if(Usart3_Rx_Buf[len+1]==0xE8 && Usart3_Rx_Buf[len+2]==0x8E && Usart3_Rx_Buf[len+5]-0xAA==Data[0]){
      switch(CMD){      
        case 0x01://重启颜色传感器
          rxBuf[0]=0xAA;  //成功信息
          rxBuf[1]=0; 
          rxBuf[2]=0;           
          break;   
        case 0x02://开启颜色传感器传输功能
          rxBuf[0]=0xAA;  //成功信息
          rxBuf[1]=0; 
          rxBuf[2]=0;          
          break;     
        case 0x03://关闭颜色传感器传输功能
          rxBuf[0]=0xAA;  //成功信息
          rxBuf[1]=0; 
          rxBuf[2]=0;        
          break;    
        case 0x04://读取颜色传感器数据
          rxBuf[0]=Usart3_Rx_Buf[len+6];//R   
          rxBuf[1]=Usart3_Rx_Buf[len+7];//G  
          rxBuf[2]=Usart3_Rx_Buf[len+8];//B  

          break; 
        case 0x06://修改ID
          rxBuf[0]=Usart3_Rx_Buf[len+5]-Data[0];  //成功信息 
          rxBuf[1]=Usart3_Rx_Buf[len+5]-0xAA; //老ID
          rxBuf[2]=0;   
         
          break;              
        case 0x07://读取ID
          rxBuf[0]=Usart3_Rx_Buf[len+5]-0xAA;  //ID
          rxBuf[1]=0; 
          rxBuf[2]=0;        
          break;                                         
      }
      
    }
    else if(Usart3_Rx_Buf[len+1]==0xE8 && Usart3_Rx_Buf[len+2]==0x8E || Usart3_Rx_Buf[len+5]-0xEE==Data[0]){
      rxBuf[0]=Data[0]+0xec;
      rxBuf[1]=0; 
      rxBuf[2]=0;   
    }
    else if((Usart3_Rx_Buf[len+1]!=0xE8 && Usart3_Rx_Buf[len+1]!=0) || (Usart3_Rx_Buf[len+2]!=0x8E && Usart3_Rx_Buf[len+2]!=0) || (Usart3_Rx_Buf[len+5]-Data[0]!=0xAA && Usart3_Rx_Buf[len+5]!=0)){
      rxBuf[0]=Data[0]+0xec;
      rxBuf[1]=0; 
      rxBuf[2]=0;   
    }

    else{
      rxBuf[0]=0;
      rxBuf[1]=0; 
      rxBuf[2]=0;   
      
    }

   
  
  }
  return rxBuf;
 
}
unsigned char SemiduplexSerial::ubtColorIdProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[20]={0};
  unsigned char Usart3_Rx_Ack_Len=0;
  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
 
 
  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
  Usart3_Rx_Ack_Len = 16; //应答消息长度 
  
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len+5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息

    if(Usart3_Rx_Buf[len+1]==0xE8 && Usart3_Rx_Buf[len+2]==0x8E && Usart3_Rx_Buf[len+5]-0xAA==Data[0] && Cheak_Sum( (Usart3_Rx_Buf[len+3] - 3),(u8*)&Usart3_Rx_Buf[len+3])==Usart3_Rx_Buf[len+14]){
      switch(CMD){      
        case 0x01://重启传感器
          tRet=0xAA;  //成功信息         
          break;   
        case 0x02://开启传感器传输功能
          tRet=0xAA;  //成功信息             
          break;     
        case 0x03://关闭传感器传输功能
          tRet=0xAA;  //成功信息         
          break;    
        case 0x04://读取传感器数据
          tRet=Usart3_Rx_Buf[len+6];  //成功信息   
          break; 
        case 0x06://修改ID
          tRet=Usart3_Rx_Buf[len+5]-Data[0];  //成功信息                   
          break;              
        case 0x07://读取ID
          tRet=Usart3_Rx_Buf[len+5]-0xAA;  //ID       
          break;       
        case 0x10://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
        case 0x11://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
        case 0x12://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
        case 0x13://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
          
                                            
      }
      
    }
    else if(Usart3_Rx_Buf[len+1]==0 && Usart3_Rx_Buf[len+2]==0 && Usart3_Rx_Buf[len+3]==0 && Usart3_Rx_Buf[len+4]==0 && Usart3_Rx_Buf[len+14]==0){
      tRet=0;
  
    }

    else{
      tRet=Data[0]+0xec;
      
    }

   
  
  
  if(tRet==128){
 
    tRet=0;
  }
  return tRet;
 
}

unsigned char SemiduplexSerial::ubtButtonProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[8]={0};
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 8; //应答消息长度 
  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len + 5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息
  if(CMD==0x06){
    delay(20);
  } 
  else if(CMD==0x10){
    delay(110);
    
  }
  
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
  if(tRet == 0){ //没有接收到消息 
    if( tCnt < 2){
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else{ //接收到消息
  
    if(Usart3_Rx_Buf[len+1]==0xF7 && Usart3_Rx_Buf[len+2]==0x7F && Usart3_Rx_Buf[len+5]-0xAA==Data[0]){
      switch(CMD){      
        case 0x01://重启传感器
          tRet=0xAA;  //成功信息         
          break;   
        case 0x02://开启传感器传输功能
          tRet=0xAA;  //成功信息             
          break;     
        case 0x03://关闭传感器传输功能
          tRet=0xAA;  //成功信息         
          break;    
        case 0x04://读取传感器数据
          tRet=Usart3_Rx_Buf[len+6];  //成功信息   
          break; 
        case 0x06://修改ID
          tRet=Usart3_Rx_Buf[len+5]-Data[0];  //成功信息                   
          break;              
        case 0x07://读取ID
          tRet=Usart3_Rx_Buf[len+5]-0xAA;  //ID       
          break;       
        case 0x10://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
        case 0x11://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
        case 0x12://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
        case 0x13://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
          
                                            
      }
      
    }

    else if(Usart3_Rx_Buf[len+1]==0xF7 && Usart3_Rx_Buf[len+2]==0x7F || Usart3_Rx_Buf[len+5]-0xEE==Data[0]){
      tRet=Data[0]+0xec;
  
    }
       else if((Usart3_Rx_Buf[len+1]!=0xF7 && Usart3_Rx_Buf[len+1]!=0) || (Usart3_Rx_Buf[len+2]!=0x7F && Usart3_Rx_Buf[len+2]!=0 ) ||(Usart3_Rx_Buf[len+5]-Data[0]!=0xAA && Usart3_Rx_Buf[len+5]!=0) ){
      tRet=Data[0]+0xec;
  
    }

    else{
      tRet=0;  
      
    }
  
   
  
  }
  return tRet;
}

unsigned char SemiduplexSerial::ubtButtonStateProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char buf[8]={0},Rx_Buf[8]={0};
  unsigned char Usart3_Rx_Ack_Len=8;

  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 

  Serial2.end();  //关闭串口2,否则会影响接收消息 
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(2);
  Serial3.readBytes(Rx_Buf, Usart3_Rx_Ack_Len); //接收应
  Serial3.end();  //关闭串口3,否则会影响接收消息
    if(Rx_Buf[0]==0xF7 && Rx_Buf[1]==0x7F && Rx_Buf[4]-0xAA==Data[0]){
      switch(CMD){      
        case 0x02://开启传感器传输功能
          tRet=0xAA;  //成功信息             
          break;     
        case 0x03://关闭传感器传输功能
          tRet=0xAA;  //成功信息         
          break;    
        case 0x04://读取传感器数据
          tRet=Rx_Buf[5];  //成功信息   
          break;       
                                            
      }
      
    }


    else if(Usart3_Rx_Buf[len+1]==0xF7 && Usart3_Rx_Buf[len+2]==0x7F || Usart3_Rx_Buf[len+5]-0xEE==Data[0]){
      tRet=Data[0]+0xec;
  
    }
       else if((Usart3_Rx_Buf[len+1]!=0xF7 && Usart3_Rx_Buf[len+1]!=0) || (Usart3_Rx_Buf[len+2]!=0x7F && Usart3_Rx_Buf[len+2]!=0 ) ||(Usart3_Rx_Buf[len+5]-Data[0]!=0xAA && Usart3_Rx_Buf[len+5]!=0) ){
      tRet=Data[0]+0xec;
  
    }

    else{
      tRet=0;  
      
    }
  
   
  
  
  return tRet;
}
unsigned char SemiduplexSerial::ubtButtonIdProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[8]={0};
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 16; //应答消息长度 
  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len + 5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息 
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息

  
    if(Usart3_Rx_Buf[len+1]==0xF7 && Usart3_Rx_Buf[len+2]==0x7F && Usart3_Rx_Buf[len+5]-0xAA==Data[0] && Cheak_Sum( (Usart3_Rx_Buf[len+3] - 3),(u8*)&Usart3_Rx_Buf[len+3])==Usart3_Rx_Buf[len+14]){
      switch(CMD){      
        case 0x01://重启传感器
          tRet=0xAA;  //成功信息         
          break;   
        case 0x02://开启传感器传输功能
          tRet=0xAA;  //成功信息             
          break;     
        case 0x03://关闭传感器传输功能
          tRet=0xAA;  //成功信息         
          break;    
        case 0x04://读取传感器数据
          tRet=Usart3_Rx_Buf[len+6];  //成功信息   
          break; 
        case 0x06://修改ID
          tRet=Usart3_Rx_Buf[len+5]-Data[0];  //成功信息                   
          break;              
        case 0x07://读取ID
          tRet=Usart3_Rx_Buf[len+5]-0xAA;  //ID       
          break;       
        case 0x10://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
        case 0x11://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
        case 0x12://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
        case 0x13://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
          
                                            
      }
      
    }
    else if(Usart3_Rx_Buf[len+1]==0 && Usart3_Rx_Buf[len+2]==0 && Usart3_Rx_Buf[len+3]==0 && Usart3_Rx_Buf[len+4]==0 && Usart3_Rx_Buf[len+14]==0){
      tRet=0;
  
    }

    else{
      tRet=Data[0]+0xec;
      
    }
  
   
  
  
  return tRet;
}
unsigned char SemiduplexSerial::ubtButtonUpdateProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[90];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 10; //应答消息长度 
  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len + 10) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息
  if(CMD==0x06){
    delay(20);
  } 
  else if(CMD==0x11){
    delay(5);
    
  }
  
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息

    if(Usart3_Rx_Buf[len+1]==0xF7 && Usart3_Rx_Buf[len+2]==0x7F && Usart3_Rx_Buf[len+5]-0xAA==Data[0]){
      switch(CMD){      
        case 0x01://重启传感器
          tRet=0xAA;  //成功信息         
          break;   
        case 0x02://开启传感器传输功能
          tRet=0xAA;  //成功信息             
          break;     
        case 0x03://关闭传感器传输功能
          tRet=0xAA;  //成功信息         
          break;    
        case 0x04://读取传感器数据
          tRet=Usart3_Rx_Buf[len+6];  //成功信息   
          break; 
        case 0x06://修改ID
          tRet=Usart3_Rx_Buf[len+5]-Data[0];  //成功信息                   
          break;              
        case 0x07://读取ID
          tRet=Usart3_Rx_Buf[len+5]-0xAA;  //ID       
          break;       
        case 0x10://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
        case 0x11://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
        case 0x12://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
        case 0x13://进入升级模式
          tRet=0xAA;  //成功信息 
          break;  
          
                                            
      }
      
    }

    else if(Usart3_Rx_Buf[len+1]==0xF7 && Usart3_Rx_Buf[len+2]==0x7F || Usart3_Rx_Buf[len+5]-0xEE==Data[0]){
      tRet=0xee;
  
    }

    else{
      tRet=0;  
      
    }
  
   
  
  
  return tRet;
}

unsigned long SemiduplexSerial::ubtButtonVersionProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned long tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[8];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 16; //应答消息长度 
  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len + 5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息
  if(CMD==0x06){
    delay(20);
  } 
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
  if(tRet == 0){ //没有接收到消息 
    if( tCnt < 2){
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else{ //接收到消息
  
    if(Usart3_Rx_Buf[len+1]==0xF7 && Usart3_Rx_Buf[len+2]==0x7F && Usart3_Rx_Buf[len+5]-0xAA==Data[0]){
      switch(CMD){  
        case 0x07://读取ID
          tRet=(Usart3_Rx_Buf[len+6]*10000000)+(Usart3_Rx_Buf[len+7]*100000)+(Usart3_Rx_Buf[len+8]*1000)+(Usart3_Rx_Buf[len+9]*10);  //ID
          tRet=tRet/10; 
         
          break;                                         
      }
      
    }

    else if(Usart3_Rx_Buf[len+1]==0xF7 && Usart3_Rx_Buf[len+2]==0x7F || Usart3_Rx_Buf[len+5]-0xEE==Data[0]){
      tRet=0xee;
  
    }

    else{
      tRet=0;  
      
    }
  
   
  
  }
  return tRet;
}
unsigned short SemiduplexSerial::ubtInfraredProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned short tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[8];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 8; //应答消息长度 
  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len+12) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息
  if(CMD==0x06){
    delay(25);
  }  
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len+2); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
  if(tRet == 0){ //没有接收到消息 
    if( tCnt < 2){
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else{ //接收到消息
    if(Usart3_Rx_Buf[len+1]==0xF8 && Usart3_Rx_Buf[len+2]==0x8F && Usart3_Rx_Buf[len+5]-0xAA==Data[0]){
      switch(CMD){      
        case 0x01://重启传感器
          tRet=0xAA;  //成功信息         
          break;   
        case 0x02://开启传感器传输功能
          tRet=0xAA;  //成功信息             
          break;     
        case 0x03://关闭传感器传输功能
          tRet=0xAA;  //成功信息         
          break;    
        case 0x04://读取传感器数据
          tRet = (Usart3_Rx_Buf[len + 6] << 8) + Usart3_Rx_Buf[len + 7];       
          break; 
        case 0x06://修改ID
          tRet=Usart3_Rx_Buf[len+5]-Data[0];  //成功信息                   
          break;              
        case 0x07://读取ID
          tRet=Usart3_Rx_Buf[len+5]-0xAA;  //ID       
          break;                                         
      }
      
    }

    else if(Usart3_Rx_Buf[len+1]==0xF8 && Usart3_Rx_Buf[len+2]==0x8F || Usart3_Rx_Buf[len+5]-0xEE==Data[0]){
      tRet=Data[0]+0xec;
  
    }
   else if((Usart3_Rx_Buf[len+1]!=0xF8 && Usart3_Rx_Buf[len+1]!=0) || (Usart3_Rx_Buf[len+2]!=0x8F && Usart3_Rx_Buf[len+2]!=0 ) ||(Usart3_Rx_Buf[len+5]-Data[0]!=0xAA && Usart3_Rx_Buf[len+5]!=0) ){
      tRet=Data[0]+0xec;

}
    else{
      tRet=0;  
      
    }
  
   
  
  }
  return tRet;
}
unsigned char SemiduplexSerial::ubtInfraredIdProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[8];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 16; //应答消息长度 
  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len+5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len+2); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
    if(Usart3_Rx_Buf[len+1]==0xF8 && Usart3_Rx_Buf[len+2]==0x8F && Usart3_Rx_Buf[len+5]-0xAA==Data[0] && Cheak_Sum( (Usart3_Rx_Buf[len+3] - 3),(u8*)&Usart3_Rx_Buf[len+3])==Usart3_Rx_Buf[len+14]){
      switch(CMD){      
        case 0x07://读取ID
          tRet=Usart3_Rx_Buf[len+5]-0xAA;  //ID       
          break;       
      }
      
    }
    else if(Usart3_Rx_Buf[len+1]==0 && Usart3_Rx_Buf[len+2]==0 && Usart3_Rx_Buf[len+3]==0 && Usart3_Rx_Buf[len+4]==0 && Usart3_Rx_Buf[len+14]==0){
      tRet=0;
  
    }

    else{
      tRet=Data[0]+0xec;
      
    }
 
  
  
  return tRet;
}
uint16_t SemiduplexSerial::ubtInfraredProtocols(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  
  uint16_t tRet=0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[8]={0};
  unsigned char Usart3_Rx_Ack_Len=0;
  unsigned char tCnt = 0, tCnt1 = 0;
  static uint16_t tLastData = 0;
  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 9; //应答消息长度 
  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len+5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len+1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息

  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len+1); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
  
  if((tRet != 16)&&(tCnt < 3))
  {
    tCnt++;
    goto Retry_Servo;
  }

    if(Usart3_Rx_Buf[len+1]==0xF8 && Usart3_Rx_Buf[len+2]==0x8F && Usart3_Rx_Buf[len+5]-0xAA==Data[0]){
      switch(CMD){      
        case 0x01://重启传感器
          tRet=0xAA;  //成功信息         
          break;   
        case 0x02://开启传感器传输功能
          tRet=0xAA;  //成功信息             
          break;     
        case 0x03://关闭传感器传输功能
          tRet=0xAA;  //成功信息         
          break;    
        case 0x04://读取传感器数据
          tRet = (Usart3_Rx_Buf[len + 6] << 8) + Usart3_Rx_Buf[len + 7]; 
          
          break; 
        case 0x06://修改ID
          tRet=Usart3_Rx_Buf[len+5]-Data[0];  //成功信息                   
          break;              
        case 0x07://读取ID
          tRet=Usart3_Rx_Buf[len+5]-0xAA;  //ID       
          break;                                         
      }
      
    }

    else if(Usart3_Rx_Buf[len+1]==0xF8 && Usart3_Rx_Buf[len+2]==0x8F || Usart3_Rx_Buf[len+5]-0xEE==Data[0]){
      tRet=Data[0]+0xec;
  
    }
   else if((Usart3_Rx_Buf[len+1]!=0xF8 && Usart3_Rx_Buf[len+1]!=0) || (Usart3_Rx_Buf[len+2]!=0x8F && Usart3_Rx_Buf[len+2]!=0 ) ||(Usart3_Rx_Buf[len+5]-Data[0]!=0xAA  && Usart3_Rx_Buf[len+5]!=0) ){
      tRet=Data[0]+0xec;

}

    else{
      tRet=0;  
      
    }

  if((abs(tRet - tLastData) > 300)&&(tCnt1 < 3))
  {
    tCnt1++;
    goto Retry_Servo;
  }
  tLastData = tRet;
  return tRet;
}
unsigned char SemiduplexSerial::ubtEyelightProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[40];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 8; //应答消息长度 
  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len+12) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息
  if(CMD==0x06){
    delay(10);
  }  
  
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len+1); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
  if(tRet == 0){ //没有接收到消息 
    if( tCnt < 2){
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else{ //接收到消息
    if(Usart3_Rx_Buf[len+1]==0xF4 && Usart3_Rx_Buf[len+2]==0x4F && Usart3_Rx_Buf[len+5]-0xAA==Data[0]){
      switch(CMD){      
        case 0x01://重启传感器
          tRet=0xAA;  //成功信息         
          break;   
        case 0x02://开启传感器传输功能
          tRet=0xAA;  //成功信息             
          break;     
        case 0x03://关闭传感器传输功能
          tRet=0xAA;  //成功信息         
          break;    
        case 0x04://读取传感器数据
          tRet = (Usart3_Rx_Buf[len + 6] << 8) + Usart3_Rx_Buf[len + 7];       
          break; 
        case 0x06://修改ID
          tRet=Usart3_Rx_Buf[len+5]-Data[0];  //成功信息                   
          break;              
        case 0x07://读取ID
          tRet=Usart3_Rx_Buf[len+5]-0xAA;  //ID       
          break;                                         
      }
      
    }

    else if(Usart3_Rx_Buf[len+1]==0xF4 && Usart3_Rx_Buf[len+2]==0x4F || Usart3_Rx_Buf[len+5]-0xEE==Data[0]){
      tRet=Data[0]+0xec;
  
    }
   else if((Usart3_Rx_Buf[len+1]!=0xF4 && Usart3_Rx_Buf[len+1]!=0) || (Usart3_Rx_Buf[len+2]!=0x4F && Usart3_Rx_Buf[len+2]!=0 ) ||(Usart3_Rx_Buf[len+5]-Data[0]!=0xAA && Usart3_Rx_Buf[len+5]!=0) ){
      tRet=Data[0]+0xec;

}
    else{
      tRet=0;  
      
    }
  
   
  
  }
  return tRet;

}
unsigned char SemiduplexSerial::ubtEyelightIdProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[40];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 16; //应答消息长度 
  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len+5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息

//   for(int i=0;i<Usart3_Rx_Ack_Len+len;i++){
//    Serial.print(Usart3_Rx_Buf[i],HEX);
//    Serial.print(",");
//   }
//   Serial.println();
//   Serial.println(crc8_itu(&Usart3_Rx_Buf[len+1], Usart3_Rx_Buf[len+2]+2),HEX);
//   Serial.println(Usart3_Rx_Buf[len+16],HEX);

    if(Usart3_Rx_Buf[len+1]==0xF4 && Usart3_Rx_Buf[len+2]==0x4F && Usart3_Rx_Buf[len+5]-0xAA==Data[0] && Cheak_Sum( (Usart3_Rx_Buf[len+3] - 3),(u8*)&Usart3_Rx_Buf[len+3])==Usart3_Rx_Buf[len+14]){
      switch(CMD){      
        case 0x01://重启传感器
          tRet=0xAA;  //成功信息         
          break;   
        case 0x02://开启传感器传输功能
          tRet=0xAA;  //成功信息             
          break;     
        case 0x03://关闭传感器传输功能
          tRet=0xAA;  //成功信息         
          break;    
        case 0x04://读取传感器数据
          tRet = (Usart3_Rx_Buf[len + 6] << 8) + Usart3_Rx_Buf[len + 7];       
          break; 
        case 0x06://修改ID
          tRet=Usart3_Rx_Buf[len+5]-Data[0];  //成功信息                   
          break;              
        case 0x07://读取ID
          tRet=Usart3_Rx_Buf[len+5]-0xAA;  //ID       
          break;                                         
      }
      
    }

    else if(Usart3_Rx_Buf[len+1]==0xF4 && Usart3_Rx_Buf[len+2]==0x4F || Usart3_Rx_Buf[len+5]-0xEE==Data[0]){
      tRet=Data[0]+0xec;
  
    }
   else if((Usart3_Rx_Buf[len+1]!=0xF4 && Usart3_Rx_Buf[len+1]!=0) || (Usart3_Rx_Buf[len+2]!=0x4F && Usart3_Rx_Buf[len+2]!=0 ) ||(Usart3_Rx_Buf[len+5]-Data[0]!=0xAA && Usart3_Rx_Buf[len+5]!=0) || (Cheak_Sum( (Usart3_Rx_Buf[len+3] - 3),(u8*)&Usart3_Rx_Buf[len+3])!=Usart3_Rx_Buf[len+14] && Usart3_Rx_Buf[len+14]!=0) ){
      tRet=Data[0]+0xec;

}
    else{
      tRet=0;  
      
    }
  
   
  
  
  return tRet;
}

unsigned short SemiduplexSerial::ubtUltrasonicProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned short tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[40];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 8; //应答消息长度 
  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len+30) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息
  if(CMD==0x06){
    delay(10);
  }  
  
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len+1); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
  if(tRet == 0){ //没有接收到消息 
    if( tCnt < 2){
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else{ //接收到消息
    if(Usart3_Rx_Buf[len+1]==0xF5 && Usart3_Rx_Buf[len+2]==0x5F && Usart3_Rx_Buf[len+5]-0xAA==Data[0]){
      switch(CMD){      
        case 0x01://重启传感器
          tRet=0xAA;  //成功信息         
          break;   
        case 0x02://开启传感器传输功能
          tRet=0xAA;  //成功信息             
          break;     
        case 0x03://关闭传感器传输功能
          tRet=0xAA;  //成功信息         
          break;    
        case 0x04://读取传感器数据
          tRet = (Usart3_Rx_Buf[len + 6] << 8) + Usart3_Rx_Buf[len + 7];       
          break; 
        case 0x06://修改ID
          tRet=Usart3_Rx_Buf[len+5]-Data[0];  //成功信息                   
          break;              
        case 0x07://读取ID
          if(Usart3_Rx_Buf[len+3]==14 && Usart3_Rx_Buf[len+4]==7){
          tRet=Usart3_Rx_Buf[len+5]-0xAA;  //ID       
          }
          else{
            tRet=Data[0]+0xec;
          }
          break;          
        case 0x08://灯管
          tRet=Usart3_Rx_Buf[len+5]-0xAA;  //ID       
          break;     
        case 0x0F://关灯
          tRet=Usart3_Rx_Buf[len+5]-0xAA;  //ID       
          break;                                    
      }
      
    }

    else if(Usart3_Rx_Buf[len+1]==0xF5 && Usart3_Rx_Buf[len+2]==0x5F || Usart3_Rx_Buf[len+5]-0xEE==Data[0]){
      tRet=Data[0]+0xec;
  
    }
   else if((Usart3_Rx_Buf[len+1]!=0xF5 && Usart3_Rx_Buf[len+1]!=0) || (Usart3_Rx_Buf[len+2]!=0x5F && Usart3_Rx_Buf[len+2]!=0 ) ||(Usart3_Rx_Buf[len+5]-Data[0]!=0xAA && Usart3_Rx_Buf[len+5]!=0) ){
      tRet=Data[0]+0xec;
  
    }

    else{
      tRet=0;  
      
    }

   
  
  }
  return tRet;
}

unsigned char SemiduplexSerial::ubtUltrasonicIdProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[40];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 16; //应答消息长度 
  buf[0] = Head;  //协议头
  buf[1] = swab8(Head);
  buf[2] = len;
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len-5);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
    
  temp = (Usart3_Rx_Ack_Len+5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len +1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len+1); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息

  if(Usart3_Rx_Buf[len+1]==0xF5 && Usart3_Rx_Buf[len+2]==0x5F && Usart3_Rx_Buf[len+5]-0xAA==Data[0] && Cheak_Sum( (Usart3_Rx_Buf[len+3] - 3),(u8*)&Usart3_Rx_Buf[len+3])==Usart3_Rx_Buf[len+14]){
    switch(CMD){         
      case 0x07://读取ID
        tRet=Usart3_Rx_Buf[len+5]-0xAA;  //ID       
        break;                                            
    }
    
  }
  else if(Usart3_Rx_Buf[len+1]==0 && Usart3_Rx_Buf[len+2]==0 && Usart3_Rx_Buf[len+3]==0 && Usart3_Rx_Buf[len+4]==0 && Usart3_Rx_Buf[len+14]==0){
    tRet=0;

  }

  else{
    tRet=Data[0]+0xec;
    
  }
  
  return tRet;
}

unsigned short SemiduplexSerial::ubtSoundProtocol(unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned short tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[40];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 8; //应答消息长度 
  
  buf[0] = 0xFB;//帧头
  buf[1] = 0x10;//设备类型
  buf[2] = len-4;//长度
  buf[3] = CMD;//命令号
  memcpy((void *)&buf[4],(void *)Data,len-4);
  buf[len-1] = crc8_itu(&buf[1], buf[2]+2);
  
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len+12) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes(Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
  if(tRet == 0){ //没有接收到消息 
    if( tCnt < 2){
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else{ //接收到消息
    
    if(Usart3_Rx_Buf[len]==0xAC  && Usart3_Rx_Buf[len+5]==0){
      switch(CMD){      
        case 0x05:
          if(Usart3_Rx_Buf[len+1]==16){
            tRet=(Usart3_Rx_Buf[len+6]<<8) |(Usart3_Rx_Buf[len+7] & 0xff);   
          }
          else{
            tRet=Data[0]+0xec;   
          }
          break;   
        case 0x06:
          tRet=0xAA;        
          break;   
       
      }
      
    }

    else if(Usart3_Rx_Buf[len]==0xAC  &&  Usart3_Rx_Buf[len+5]==1){
      tRet=Data[0]+0xec;
  
    }
    else if((Usart3_Rx_Buf[len]!=0xAC && Usart3_Rx_Buf[len]!=0) ||(Usart3_Rx_Buf[len+1]!=16 && Usart3_Rx_Buf[len+1]!=0) ||  Usart3_Rx_Buf[len+5]!=0){//重复ID
      tRet=Data[0]+0xec;
    
    }

    else{
      tRet=0;  
      
    }
//  Serial.print("head[0]:");
//  Serial.print(Usart3_Rx_Buf[len]);
//  Serial.print(",head[1]:");
//  Serial.println(Usart3_Rx_Buf[len+1]);//fixme
   
  
  }
  return tRet;
}

unsigned char SemiduplexSerial::ubtSoundIdProtocol(unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[40];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 17; //应答消息长度 
  
  buf[0] = 0xFB;//帧头
  buf[1] = 0x10;//设备类型
  buf[2] = len-4;//长度
  buf[3] = CMD;//命令号
  memcpy((void *)&buf[4],(void *)Data,len-4);
  buf[len-1] = crc8_itu(&buf[1], buf[2]+2);
   
  
  temp = (Usart3_Rx_Ack_Len+5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes(Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息

    
    if(Usart3_Rx_Buf[len]==0xAC  && Usart3_Rx_Buf[len+1]==0x10 && Usart3_Rx_Buf[len+3]==0x05 && Usart3_Rx_Buf[len+5]==0 && Usart3_Rx_Buf[len+16]==crc8_itu(&Usart3_Rx_Buf[len+1], Usart3_Rx_Buf[len+2]+2)){
    tRet=Usart3_Rx_Buf[len+4];    
    } 
      
   else if((Usart3_Rx_Buf[len]!=0xAC && Usart3_Rx_Buf[len]!=0) ||(Usart3_Rx_Buf[len+1]!=0x10 && Usart3_Rx_Buf[len+1]!=0)|| Usart3_Rx_Buf[len+5]!=0 || ( Usart3_Rx_Buf[len+16]!=crc8_itu(&Usart3_Rx_Buf[len+1], Usart3_Rx_Buf[len+2]+2) &&Usart3_Rx_Buf[len+16]!=0 )){//重复ID
      tRet=Data[0]+0xec;
    
    }
   else if(Usart3_Rx_Buf[len]==0  && Usart3_Rx_Buf[len+1]==0 && Usart3_Rx_Buf[len+2]==0 && Usart3_Rx_Buf[len+3]==0 && Usart3_Rx_Buf[len+4]==0 && Usart3_Rx_Buf[len+16]==0){
    tRet=0;  
       
   }
   else{
    tRet=Data[0]+0xec;
   }

  return tRet;
}


unsigned short SemiduplexSerial::ubtHumitureProtocol(unsigned char len,unsigned char CMD,char choice,unsigned char * Data){
  unsigned short tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[10];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  if(Data[2]==0x01 && Data[4]==0x05){
    Usart3_Rx_Ack_Len = 8; //应答消息长度 
  }
  Usart3_Rx_Ack_Len = 10; //应答消息长度 
  
  buf[0] = 0xFB;//帧头
  buf[1] = 0x05;//设备类型
  buf[2] = len-4;//长度
  buf[3] = CMD;//命令号
  memcpy((void *)&buf[4],(void *)Data,len-4);
  buf[len-1] = crc8_itu(&buf[1], buf[2]+2);
  
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len+16) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
  delay(1);
  if(tRet == 0){ //没有接收到消息 
    if( tCnt < 2){
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else{ //接收到消息
   
    if(Usart3_Rx_Buf[len]==0xAC  && Usart3_Rx_Buf[len+5]==0){
      switch(CMD){      
        case 0x05:
          if(Usart3_Rx_Buf[len+1]==5){
            switch(choice){
              case 'C':
                tRet=ceil(((Usart3_Rx_Buf[len+6]<<8) | (Usart3_Rx_Buf[len+7] & 0xff))/10.0); 
                break;
              case 'F':
                tRet=ceil(((Usart3_Rx_Buf[len+6]<<8) | (Usart3_Rx_Buf[len+7] & 0xff))/10.0);
                tRet*=1.8;
                tRet+=32;
                break;
              case 'H':
                tRet=(Usart3_Rx_Buf[len+8]<<8)+Usart3_Rx_Buf[len+9];
                break;
              default:
                tRet=((Usart3_Rx_Buf[len+6]<<8) | (Usart3_Rx_Buf[len+7] & 0xff));
                break;          
            }
          }
          else{
            tRet=Data[0]+0xec; 
          }
        
          break;   
        case 0x06:
          tRet=0xAA;        
          break;   
       
      }
      
    }

    else if(Usart3_Rx_Buf[len]==0xAC  &&  Usart3_Rx_Buf[len+5]==1){
      tRet=Data[0]+0xec;
  
    }
    else if((Usart3_Rx_Buf[len]!=0xAC && Usart3_Rx_Buf[len]!=0) ||(Usart3_Rx_Buf[len+1]!=5 && Usart3_Rx_Buf[len+1]!=0) || Usart3_Rx_Buf[len+5]!=0){//重复ID
      tRet=Data[0]+0xec;
    
    }

    else{
      tRet=0;  
      
    }
//  Serial.print("head[0]:");
//  Serial.print(Usart3_Rx_Buf[len]);
//  Serial.print(",head[1]:");
//  Serial.println(Usart3_Rx_Buf[len+1]);//fixme
  
   
  
  }
 
  
  return tRet;
}
unsigned char SemiduplexSerial::ubtHumitureIdProtocol(unsigned char len,unsigned char CMD,char choice,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[10];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 17; //应答消息长度 
  
  buf[0] = 0xFB;//帧头
  buf[1] = 0x05;//设备类型
  buf[2] = len-4;//长度
  buf[3] = CMD;//命令号
  memcpy((void *)&buf[4],(void *)Data,len-4);
  buf[len-1] = crc8_itu(&buf[1], buf[2]+2);
     
  
  temp = (Usart3_Rx_Ack_Len+5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t 
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息

   
    if(Usart3_Rx_Buf[len]==0xAC  && Usart3_Rx_Buf[len+1]==0x05 && Usart3_Rx_Buf[len+3]==0x05 && Usart3_Rx_Buf[len+5]==0 && Usart3_Rx_Buf[len+16]==crc8_itu(&Usart3_Rx_Buf[len+1], Usart3_Rx_Buf[len+2]+2)){
    tRet=Usart3_Rx_Buf[len+4];    
    } 
      
   else if((Usart3_Rx_Buf[len]!=0xAC && Usart3_Rx_Buf[len]!=0) ||(Usart3_Rx_Buf[len+1]!=0x05 && Usart3_Rx_Buf[len+1]!=0)|| Usart3_Rx_Buf[len+5]!=0 || ( Usart3_Rx_Buf[len+16]!=crc8_itu(&Usart3_Rx_Buf[len+1], Usart3_Rx_Buf[len+2]+2) &&Usart3_Rx_Buf[len+16]!=0 )){//重复ID
      tRet=Data[0]+0xec;
    
    }
   else if(Usart3_Rx_Buf[len]==0  && Usart3_Rx_Buf[len+1]==0 && Usart3_Rx_Buf[len+2]==0 && Usart3_Rx_Buf[len+3]==0 && Usart3_Rx_Buf[len+4]==0 && Usart3_Rx_Buf[len+16]==0){
    tRet=0;  
       
   }
   else{
    tRet=Data[0]+0xec;
   }
  
   
  
  
 
  
  return tRet;
}
unsigned short SemiduplexSerial::ubtLightProtocol(unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned short tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[40];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 8; //应答消息长度 
  
  buf[0] = 0xFB;//帧头
  buf[1] = 0x06;//设备类型
  buf[2] = len-4;//长度
  buf[3] = CMD;//命令号
  memcpy((void *)&buf[4],(void *)Data,len-4);
  buf[len-1] = crc8_itu(&buf[1], buf[2]+2);
  
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len+5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
  if(tRet == 0){ //没有接收到消息 
    if( tCnt < 2){
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else{ //接收到消息

    if(Usart3_Rx_Buf[len]==0xAC  && Usart3_Rx_Buf[len+5]==0){
      switch(CMD){      
        case 0x05:
          tRet=(Usart3_Rx_Buf[len+6]<<8) |(Usart3_Rx_Buf[len+7] & 0xff);        
          break;   
        case 0x06:
          tRet=0xAA;        
          break;   
       
      }
      
    }

    else if(Usart3_Rx_Buf[len]==0xAC  &&  Usart3_Rx_Buf[len+5]==1){
      tRet=Data[0]+0xec;
  
    }
    else if((Usart3_Rx_Buf[len]!=0xAC && Usart3_Rx_Buf[len]!=0) ||(Usart3_Rx_Buf[len+1]!=6 && Usart3_Rx_Buf[len+1]!=0)||  Usart3_Rx_Buf[len+5]!=0  ){//重复ID
      tRet=Data[0]+0xec;
    
    }

    else{
      tRet=0;  
      
    }
    

  
   
  
  }
  return tRet;
}


unsigned char SemiduplexSerial::ubtLightIdProtocol(unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[40];
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 17; //应答消息长度 
  
  buf[0] = 0xFB;//帧头
  buf[1] = 0x06;//设备类型
  buf[2] = len-4;//长度
  buf[3] = CMD;//命令号
  memcpy((void *)&buf[4],(void *)Data,len-4);
  buf[len-1] = crc8_itu(&buf[1], buf[2]+2);

  temp = (Usart3_Rx_Ack_Len+10) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
//   for(int i=0;i<Usart3_Rx_Ack_Len+len;i++){
//    Serial.print(Usart3_Rx_Buf[i],HEX);
//    Serial.print(",");
//   }
//   Serial.println();
//   Serial.println(crc8_itu(&Usart3_Rx_Buf[len+1], Usart3_Rx_Buf[len+2]+2),HEX);
//   Serial.println(Usart3_Rx_Buf[len+16],HEX);

   if(Usart3_Rx_Buf[len]==0xAC  && Usart3_Rx_Buf[len+1]==6 && Usart3_Rx_Buf[len+3]==0x05 && Usart3_Rx_Buf[len+5]==0 && Usart3_Rx_Buf[len+16]==crc8_itu(&Usart3_Rx_Buf[len+1], Usart3_Rx_Buf[len+2]+2)){
    tRet=Usart3_Rx_Buf[len+4];    
    } 
      
   else if((Usart3_Rx_Buf[len]!=0xAC && Usart3_Rx_Buf[len]!=0) ||(Usart3_Rx_Buf[len+1]!=6 && Usart3_Rx_Buf[len+1]!=0)|| Usart3_Rx_Buf[len+5]!=0 || ( Usart3_Rx_Buf[len+16]!=crc8_itu(&Usart3_Rx_Buf[len+1], Usart3_Rx_Buf[len+2]+2) &&Usart3_Rx_Buf[len+16]!=0 )){//重复ID
      tRet=Data[0]+0xec;
    
    }
   else if(Usart3_Rx_Buf[len]==0  && Usart3_Rx_Buf[len+1]==0 && Usart3_Rx_Buf[len+2]==0 && Usart3_Rx_Buf[len+3]==0 && Usart3_Rx_Buf[len+4]==0 && Usart3_Rx_Buf[len+16]==0){
    tRet=0;  
       
   }
   else{
    tRet=Data[0]+0xec;
   }


  
  return tRet;
}

short SemiduplexSerial::ubtMotorProtocol(unsigned char len,unsigned char CMD,unsigned char * Data){
  short tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 0; //2ms 发完
  unsigned char buf[16]={0};
  unsigned char Usart3_Rx_Ack_Len=0;

  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  if(Data[2]==0x07 && Data[4]==0x01){//回读
    Usart3_Rx_Ack_Len = 8; //应答消息长度 
  }
  else{
    Usart3_Rx_Ack_Len = 6; //应答消息长度 
  }
  
  buf[0] = 0xFB;//帧头
  buf[1] = 0x03;//设备类型
  buf[2] = len-4;//长度
  buf[3] = CMD;//命令号
  memcpy((void *)&buf[4],(void *)Data,len-4);
  buf[len-1] = crc8_itu(&buf[1], buf[2]+2);
  
    
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len+8) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes(Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
 
  if(Usart3_Rx_Buf[len]==0x00){
    Serial3.begin(114200);  //uart3
    Serial3.setTimeout(temp*87*110/100/400);  //设置超时ms
    Serial2.begin(114200);  //设置波特率
    Serial2.write(buf,len);  //发送消息
    Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
    Serial2.end();  //关闭串口2,否则会影响接收消息
    tRet =Serial3.readBytes(Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
    Serial3.end();  //关闭串口3,否则会影响接收消息
  }

  if(tRet == 0){ //没有接收到消息 
    if( tCnt < 2){
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else{ //接收到消息
    if(Usart3_Rx_Buf[len]==0xAC  && Usart3_Rx_Buf[len+5]==0){
      switch(CMD){      
        case 0x05:
          if(Data[2]==0x09 && Data[4]==0x05){
            tRet=Usart3_Rx_Buf[len+4];
          }
          else{
            tRet=(Usart3_Rx_Buf[len+6]<<8) |(Usart3_Rx_Buf[len+7] & 0xff); 
          }
                 
          break;   
        case 0x06:
          tRet=0xAA;        
          break;   
       
      }
      
    }

    else if(Usart3_Rx_Buf[len]==0xAC  &&  Usart3_Rx_Buf[len+5]==1){
      tRet=Data[0]+0xec;
  
    }
   else if((Usart3_Rx_Buf[len]!=0xAC && Usart3_Rx_Buf[len]!=0) ||(Usart3_Rx_Buf[len+1]!=3 && Usart3_Rx_Buf[len+1]!=0)|| Usart3_Rx_Buf[len+5]!=0){//重复ID
      tRet=Data[0]+0xec;
    
    }
    

    else{
      tRet=0;  
      
    }
    
    
  
   
  
  }
  return tRet;
}

unsigned char SemiduplexSerial::ubtMotorIdProtocol(unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 0; //2ms 发完
  unsigned char buf[16]={0};
  unsigned char Usart3_Rx_Ack_Len=0;
  unsigned char err=0;
  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 17; //应答消息长度 

  
  buf[0] = 0xFB;//帧头
  buf[1] = 0x03;//设备类型
  buf[2] = len-4;//长度
  buf[3] = CMD;//命令号
  memcpy((void *)&buf[4],(void *)Data,len-4);
  buf[len-1] = crc8_itu(&buf[1], buf[2]+2);
   
  temp = (Usart3_Rx_Ack_Len+5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes(Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
 
  if(Usart3_Rx_Buf[len]==0x00){
    //tone(300,100);
    Serial3.begin(114200);  //uart3
    Serial3.setTimeout(temp*87*110/100/400);  //设置超时ms
    Serial2.begin(114200);  //设置波特率
    Serial2.write(buf,len);  //发送消息
    Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
    Serial2.end();  //关闭串口2,否则会影响接收消息
    tRet =Serial3.readBytes(Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
    Serial3.end();  //关闭串口3,否则会影响接收消息
  } 
  
//   for(int i=len;i<Usart3_Rx_Ack_Len+len;i++){
//    Serial.print(i-len);
//    Serial.print(":");
//    Serial.print(Usart3_Rx_Buf[i],HEX);
//    Serial.println( );
//   }
//  Serial.println((0xFFFF & ((Usart3_Rx_Buf[len+6] << 8) + Usart3_Rx_Buf[len+7] + (Usart3_Rx_Buf[len+8] << 8) + Usart3_Rx_Buf[len+9] + (Usart3_Rx_Buf[len+10] << 8) + \
//                                Usart3_Rx_Buf[len+11] + (Usart3_Rx_Buf[len+12] << 8) + Usart3_Rx_Buf[len+13])));
//  Serial.println(((Usart3_Rx_Buf[len+14] << 8) + Usart3_Rx_Buf[len+15]));
//  
  
if((0xFFFF & ((Usart3_Rx_Buf[len+6] << 8) + Usart3_Rx_Buf[len+7] + (Usart3_Rx_Buf[len+8] << 8) + Usart3_Rx_Buf[len+9] + (Usart3_Rx_Buf[len+10] << 8) + \
                                Usart3_Rx_Buf[len+11] + (Usart3_Rx_Buf[len+12] << 8) + Usart3_Rx_Buf[len+13])) == ((Usart3_Rx_Buf[len+14] << 8) + Usart3_Rx_Buf[len+15])){

  
   if(Usart3_Rx_Buf[len]==0xAC  && Usart3_Rx_Buf[len+1]==0x03 && Usart3_Rx_Buf[len+2]==0x0b && Usart3_Rx_Buf[len+3]==0x05 && Usart3_Rx_Buf[len+5]==0  && Usart3_Rx_Buf[len+16]==crc8_itu(&Usart3_Rx_Buf[len+1], Usart3_Rx_Buf[len+2]+2)){
    tRet=Usart3_Rx_Buf[len+4];    
    } 
   else if(Usart3_Rx_Buf[len]==0xAC && Usart3_Rx_Buf[len+1]==0x03&&  Usart3_Rx_Buf[len+2]==0x0b  && Usart3_Rx_Buf[len+3]==0x05 && (Usart3_Rx_Buf[len+5]==0 || Usart3_Rx_Buf[len+5]==0x80)){
    tRet=Usart3_Rx_Buf[len+4];    
    }     
   else if(Usart3_Rx_Buf[len]==0xAC  && Usart3_Rx_Buf[len+1]==0x03 && Usart3_Rx_Buf[len+2]==0x0D && Usart3_Rx_Buf[len+3]==0x05 && (Usart3_Rx_Buf[len+5]==0 || Usart3_Rx_Buf[len+5]==0x80) && Usart3_Rx_Buf[len+16]==crc8_itu(&Usart3_Rx_Buf[len+1], Usart3_Rx_Buf[len+2]+2)){
    tRet=Usart3_Rx_Buf[len+4];    
    } 
   else if(Usart3_Rx_Buf[len]==0xAC && Usart3_Rx_Buf[len+1]==0x03&&  Usart3_Rx_Buf[len+2]==0x0D  && Usart3_Rx_Buf[len+3]==0x05 && (Usart3_Rx_Buf[len+5]==0 || Usart3_Rx_Buf[len+5]==0x80) ){
    tRet=Usart3_Rx_Buf[len+4];    
    }   
    else{
      tRet=0;
    }
}
   else if(Usart3_Rx_Buf[len]==0  && Usart3_Rx_Buf[len+1]==0 && Usart3_Rx_Buf[len+2]==0 && Usart3_Rx_Buf[len+3]==0 && Usart3_Rx_Buf[len+4]==0 && Usart3_Rx_Buf[len+16]==0){
    tRet=0;  
       
   }

   else{
    tRet=Data[0]+0xec;
    
    
   }


    
    
  
   
  
  

  return tRet;
}

unsigned char SemiduplexSerial::ubtMotorActionProtocol(unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned char tRet=0;
  unsigned char tCnt = 0;
  unsigned long temp = 0; //2ms 发完
  unsigned char buf[16]={0};
  unsigned char Usart3_Rx_Ack_Len=0;  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 7; //应答消息长度 

  
  buf[0] = 0xFB;//帧头
  buf[1] = 0x03;//设备类型
  buf[2] = len-4;//长度
  buf[3] = CMD;//命令号
  memcpy((void *)&buf[4],(void *)Data,len-4);
  buf[len-1] = crc8_itu(&buf[1], buf[2]+2);
   
  temp = (Usart3_Rx_Ack_Len+5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes(Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
 
  if(Usart3_Rx_Buf[len]==0x00){
    //tone(300,100);
    Serial3.begin(114200);  //uart3
    Serial3.setTimeout(temp*87*110/100/400);  //设置超时ms
    Serial2.begin(114200);  //设置波特率
    Serial2.write(buf,len);  //发送消息
    Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
    Serial2.end();  //关闭串口2,否则会影响接收消息
    tRet =Serial3.readBytes(Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
    Serial3.end();  //关闭串口3,否则会影响接收消息
  } 
  
  return tRet;
}

unsigned short SemiduplexSerial::ubtServoProtocol(unsigned char Head,unsigned char ServoNO,unsigned char CMD,unsigned char * Data){
  unsigned short tRet = 0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[10];
  unsigned char len = 9; //9+1
  unsigned char Usart3_Rx_Ack_Len=0; 
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf)); 
  buf[0] = Head;  //填充协议头
  buf[1] = swab8(Head);
  buf[2] = ServoNO; //舵机好
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,4);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;

  if((CMD == 0x01&& Head!=0xFC) || CMD==0x04 ||CMD==0xCD ){    
    Usart3_Rx_Ack_Len = 1;  //1,4命令只应答一个字节
  }
  else if(CMD == 0x02 || CMD==0x03){    
    Usart3_Rx_Ack_Len = 8;  
  }
  else if((CMD == 0x01&& Head==0xFC)){
    Usart3_Rx_Ack_Len = 4; 
  }
      
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len+ 5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+10); //接收应答
  
  Serial3.end();  //关闭串口3,否则会影响接收消息

  if(tRet == 0) //没有接收到消息
  {
    if( tCnt < 2)
    {
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else  //接收到消息
  { 
    
    Usart3_Rx_Buf_count = tRet;
    tRet = 0;
    switch(CMD){
      case 0x01:
        if(Head==0xFC){
          if(Usart3_Rx_Buf[len+1]==0xfc && Usart3_Rx_Buf[len+2]==0xcf && Usart3_Rx_Buf[len+4]==0xaa){
            tRet=Usart3_Rx_Buf[len+3];   
          }
          else if((Usart3_Rx_Buf[len+1]!=0xfc && Usart3_Rx_Buf[len+1]!=0) || (Usart3_Rx_Buf[len+2]!=0xcf && Usart3_Rx_Buf[len+2]!=0) || (Usart3_Rx_Buf[len+4]!=0xAA && Usart3_Rx_Buf[len+4]!=0)){
            tRet=ServoNO+0xec;
          }
          else{
            tRet=0;
          }
        }
        else{
          tRet=Usart3_Rx_Buf[len+1]-0xAA-ServoNO; 
        }
       
        break;
      case 0x02:
        tRet=(Usart3_Rx_Buf[len+7]<<8) |(Usart3_Rx_Buf[len+8] & 0xff); 
        break;
      case 0x03:
        tRet=(Usart3_Rx_Buf[len+7]<<8) |(Usart3_Rx_Buf[len+8] & 0xff); 
        break;    
      
    }
    
    
    


  }
  return tRet;
}
unsigned char SemiduplexSerial::ubtServoIdProtocol(unsigned char Head,unsigned char ServoNO,unsigned char CMD,unsigned char * Data){
  unsigned char tRet = 0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[10];
  unsigned char len = 9; //9+1
  unsigned char Usart3_Rx_Ack_Len=0; 
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf)); 
  buf[0] = Head;  //填充协议头
  buf[1] = swab8(Head);
  buf[2] = ServoNO; //舵机好
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,4);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
  
  Usart3_Rx_Ack_Len = 11;  
     
  temp = (Usart3_Rx_Ack_Len+ 5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+len); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息

//   for(int i=0;i<Usart3_Rx_Ack_Len+len;i++){
//    Serial.print(Usart3_Rx_Buf[i],HEX);
//    Serial.print(",");
//   }
//   Serial.println();
//   Serial.println(Cheak_Sum( (len - 3),(u8*)&Usart3_Rx_Buf[len+3]),HEX);
//   Serial.println(Usart3_Rx_Buf[len+9],HEX);

  if(Usart3_Rx_Buf[len+1]==0xFC && Usart3_Rx_Buf[len+2]==0xCF && Usart3_Rx_Buf[len+4]==0xAA && Cheak_Sum( (len - 3),(u8*)&Usart3_Rx_Buf[len+3])==Usart3_Rx_Buf[len+9]){
       tRet=Usart3_Rx_Buf[len+3];      
    
  }
  else if(Usart3_Rx_Buf[len+1]==0 && Usart3_Rx_Buf[len+2]==0 && Usart3_Rx_Buf[len+3]==0 && Usart3_Rx_Buf[len+4]==0 && Usart3_Rx_Buf[len+9]==0){
    tRet=0;

  }

  else{
    tRet=ServoNO+0xec;
    
  }

   

  
  return tRet;
}
unsigned char SemiduplexSerial::ubtServoActionProtocol(unsigned char Head,unsigned char ServoNO,unsigned char CMD,unsigned char * Data){
  unsigned char tRet = 0;
  unsigned char buf[10]={0};
  const unsigned char len = 9; //9+1
  const unsigned char Usart3_Rx_Ack_Len=1; 
  unsigned char Rx_Buf[11]={0};
  buf[0] = Head;  //填充协议头
  buf[1] = swab8(Head);
  buf[2] = ServoNO; //舵机好
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,4);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(tems((long)Usart3_Rx_Ack_Len+1));  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes(Rx_Buf, Usart3_Rx_Ack_Len+len+1); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息 
  return tRet;
}
unsigned short SemiduplexSerial::ubtServoProtocol1M(unsigned char Head,unsigned char ServoNO,unsigned char CMD,unsigned char * Data){
  unsigned short tRet = 0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[10];
  unsigned char len = 9; //9+1
  unsigned char Usart3_Rx_Ack_Len=0; 
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf)); 
  buf[0] = Head;  //填充协议头
  buf[1] = swab8(Head);
  buf[2] = ServoNO; //舵机好
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,4);
  buf[len - 1] = Cheak_Sum( (len - 3),(u8*)&buf[2]);
  buf[len] = 0xED;

  if((CMD == 0x01&& Head!=0xFC) || CMD==0x04 ||CMD==0xCD ){    
    Usart3_Rx_Ack_Len = 1;  //1,4命令只应答一个字节
  }
  else if(CMD == 0x02 || CMD==0x03){    
    Usart3_Rx_Ack_Len = 8;  
  }
  else if((CMD == 0x01&& Head==0xFC)){
    Usart3_Rx_Ack_Len = 4; 
  }
      
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len+ 5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(1000000);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(1000000);  //设置波特率
  Serial2.write(buf,len + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+10); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息

  if(tRet == 0) //没有接收到消息
  {
    if( tCnt < 2)
    {
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else  //接收到消息
  { 
    
    Usart3_Rx_Buf_count = tRet;
    tRet = 0;
    switch(CMD){
      case 0x01:
        if(Head==0xFC){
          tRet=Usart3_Rx_Buf[len+3]; 
        }
        else{
          tRet=Usart3_Rx_Buf[len+1]-0xAA-ServoNO; 
        }
       
        break;
      case 0x02:
        tRet=(Usart3_Rx_Buf[len+7]<<8) |(Usart3_Rx_Buf[len+8] & 0xff); 
        break;
      case 0x03:
        tRet=(Usart3_Rx_Buf[len+7]<<8) |(Usart3_Rx_Buf[len+8] & 0xff); 
        break;    
      
    }
    
    


  }
  return tRet;
}



/**@brief EN:Communication protocol sending and receiving functions/CN:通讯协议发送和接受函数.
 *
 * @param[in] Head EN:Protocol head/CN:协议头.
 * @param[in] ServoNO EN:Servo id/CN:舵机号.
 * @param[in] len EN:data length/CN:数据长度.
 * @param[in] CMD EN:CMD code/CN:CMD码.
 * @param[in] Data EN:data buffer/CN:数据缓冲区.
 *
 * @returns tRet EN:Accept return value/CN:接受返回值.
 */


unsigned long SemiduplexSerial::TXD(unsigned char Head,unsigned char ServoNO,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned long tRet = 0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[60];
  unsigned char length = 9; //9+1

  unsigned char Usart3_Rx_Ack_Len=0;
  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 17; //应答消息长度
  
  buf[0] = Head;  //填充协议头
  buf[1] = swab8(Head);
  if(Head >= 0xFA)
  {
    buf[2] = ServoNO; //舵机好
  }
  else if(Head > 0x00)  //变长协议
  {
    length = len + 5; //长度
    buf[2] = length;
  }
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len);
  buf[length - 1] = Cheak_Sum( (length - 3),(u8*)&buf[2]);
  buf[length ] = 0xED;
  
  
  if(Head == 0xFA)
  {
    if(CMD == 1 )
    {
      if(ServoNO == 0)
      {
        //这句有问题 memset 不支持双字节的填充
        //memset((void *)&gsSave_Angle,0xFF,sizeof(gsSave_Angle));  //保存所有角度
        //memset((void *)&gsSave_Angle,Data,sizeof(gsSave_Angle));  //保存所有角度
        memset((void *)&gsSave_Angle,Data[0],sizeof(gsSave_Angle)); //保存所有角度
      }
      else
      {
        if(Data[0] == gsSave_Angle.Angle[ServoNO - 1])  return  0;  //不发送,直接返回
        else
        {
          gsSave_Angle.Angle[ServoNO - 1] = Data[0];  //保存单个角度,以后可能双字节角度
        }
      }
      Usart3_Rx_Ack_Len = 1;  //1,4命令只应答一个字节
    }
    else if( CMD == 4)  Usart3_Rx_Ack_Len = 1;  //1,4命令只应答一个字节
  }
  
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len + 5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 / 400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,length + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+10); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
  if(tRet > RXD_OFFSET) //打印返回消息
  {
      //Serial.write(Usart3_Rx_Buf + RXD_OFFSET,tRet - RXD_OFFSET);
      //Serial.print("\r\n");
  }
  
  if(tRet == 0) //没有接收到消息
  {
    if( tCnt < 2)
    {
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else  //接收到消息
  {
  Usart3_Rx_Buf_count = tRet;
    tRet = 0;
    if(Head == 0xFA)
    {
      if( (CMD == 1) || (CMD == 4) )
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 1] == 0xAA + ServoNO) )
        {
          tRet = ServoNO;
        }
      }
      else if(CMD == 2 | CMD==3 |CMD==8)
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 3] == 0xAA) && (Usart3_Rx_Buf[RXD_OFFSET + 2] == ServoNO) )
        {
          tRet = Usart3_Rx_Buf[10+6] << 8;
          tRet |= Usart3_Rx_Buf[10+7] << 0;
        }
      }

      
      else if(CMD == 0xCD )
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 5] == ServoNO) )
        {
          tRet = Usart3_Rx_Buf[RXD_OFFSET + 5];
        }
      }
      else if(CMD == 0xD2 )
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 3] == 0xAA) && (Usart3_Rx_Buf[RXD_OFFSET + 2] == ServoNO) )
        {
          tRet = 1;
        }
      }
      else if(CMD == 0xD4 )
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 3] == 0xAA) && (Usart3_Rx_Buf[RXD_OFFSET + 2] == ServoNO) )
        {
          tRet = Usart3_Rx_Buf[RXD_OFFSET + 6] << 8;
          tRet |= Usart3_Rx_Buf[RXD_OFFSET + 7] << 0;
        }
      }
      
    }
    else if(Head == 0xFC)
    {
      if(CMD == 1 )
      {
        if((Usart3_Rx_Buf[RXD_OFFSET + 3] == 0xAA)  )//版本号改为读ID
        {
          tRet=Usart3_Rx_Buf[RXD_OFFSET + 2];
         
        }
      }
    }
    else if( (Head < 0xFA) && (Head > 0x00) && (Usart3_Rx_Buf[length + 4] == CMD) )//传感器
    {
      if(CMD == 0x02 )
      {
        tRet = Usart3_Rx_Buf[length + 5] - 0xAA;
        
      }
            else if(CMD == 0x03 )
      {
        tRet = Usart3_Rx_Buf[length + 5] - 0xAA;
        
      }
      else if(CMD == 0x04 )
      {
        //Serial.println(Usart3_Rx_Buf[length + 3]);
        if(Usart3_Rx_Buf[length + 3] == 7)
          tRet = ((Usart3_Rx_Buf[length + 5] - 0xAA) << 8) + Usart3_Rx_Buf[length + 6];                
        else   if(Usart3_Rx_Buf[length + 3] == 8)
          tRet = (Usart3_Rx_Buf[length + 6] << 8) + Usart3_Rx_Buf[length + 7];
        else   if(Usart3_Rx_Buf[length + 3] == 9)
          tRet = (Usart3_Rx_Buf[length + 7] << 8) + Usart3_Rx_Buf[length + 8];
        else   if(Usart3_Rx_Buf[length + 3] == 17)
        {    
            
             tRet = ((Usart3_Rx_Buf[length + 5] - 0xAA) << 8) + Usart3_Rx_Buf[length + 6];   
             redvalue = Usart3_Rx_Buf[length + 6];
             greenvalue = Usart3_Rx_Buf[length + 7];
             bluevalue = Usart3_Rx_Buf[length + 8];
        }
          
      }
      else if(CMD==0x06|CMD==0x07)
      {
        if(Usart3_Rx_Buf[length + 4]==7 && Usart3_Rx_Buf[length + 5]-0xAA!=Data[0]){
           tRet =254;
        }
        else if(Usart3_Rx_Buf[length + 4]==7&& Usart3_Rx_Buf[length + 5]-0xAA==Data[0]){    
          tRet = Usart3_Rx_Buf[length + 5] - 0xAA;
        }
        else{
          tRet = 0;
        }
        
      }
      
    }

  }
  return tRet;
}
unsigned long SemiduplexSerial::TXD(unsigned char Head,unsigned char ServoNO,unsigned char len,unsigned char CMD,unsigned char RGB,unsigned char * Data){
  unsigned long tRet = 0;
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[40];
  unsigned char length = 9; //9+1

  unsigned char Usart3_Rx_Ack_Len=0;
  
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 17; //应答消息长度
  
  buf[0] = Head;  //填充协议头
  buf[1] = swab8(Head);
  if(Head >= 0xFA)
  {
    buf[2] = ServoNO; //舵机好
  }
  else if(Head > 0x00)  //变长协议
  {
    length = len + 5; //长度
    buf[2] = length;
  }
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len);
  buf[length - 1] = Cheak_Sum( (length - 3),(u8*)&buf[2]);
  buf[length ] = 0xED;
  
  
  if(Head == 0xFA)
  {
    if(CMD == 1 )
    {
      if(ServoNO == 0)
      {
        //这句有问题 memset 不支持双字节的填充
        //memset((void *)&gsSave_Angle,0xFF,sizeof(gsSave_Angle));  //保存所有角度
        //memset((void *)&gsSave_Angle,Data,sizeof(gsSave_Angle));  //保存所有角度
        memset((void *)&gsSave_Angle,Data[0],sizeof(gsSave_Angle)); //保存所有角度
      }
      else
      {
        if(Data[0] == gsSave_Angle.Angle[ServoNO - 1])  return  0;  //不发送,直接返回
        else
        {
          gsSave_Angle.Angle[ServoNO - 1] = Data[0];  //保存单个角度,以后可能双字节角度
        }
      }
      Usart3_Rx_Ack_Len = 1;  //1,4命令只应答一个字节
    }
    else if( CMD == 4)  Usart3_Rx_Ack_Len = 1;  //1,4命令只应答一个字节
  }
  
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len + 5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 /400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,length + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+10); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
  if(tRet > RXD_OFFSET) //打印返回消息
  {
      //Serial.write(Usart3_Rx_Buf + RXD_OFFSET,tRet - RXD_OFFSET);
      //Serial.print("\r\n");
  }
  
  if(tRet == 0) //没有接收到消息
  {
    if( tCnt < 2)
    {
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else  //接收到消息
  {
  Usart3_Rx_Buf_count = tRet;
    tRet = 0;
    if(Head == 0xFA)
    {
      if( (CMD == 1) || (CMD == 4) )
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 1] == 0xAA + ServoNO) )
        {
          tRet = ServoNO;
        }
      }
      else if(CMD == 2 | CMD==3)
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 3] == 0xAA) && (Usart3_Rx_Buf[RXD_OFFSET + 2] == ServoNO) )
        {
          tRet = Usart3_Rx_Buf[10+6] << 8;
          tRet |= Usart3_Rx_Buf[10+7] << 0;
        }
      }

      
      else if(CMD == 0xCD )
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 5] == ServoNO) )
        {
          tRet = Usart3_Rx_Buf[RXD_OFFSET + 5];
        }
      }
      else if(CMD == 0xD2 )
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 3] == 0xAA) && (Usart3_Rx_Buf[RXD_OFFSET + 2] == ServoNO) )
        {
          tRet = 1;
        }
      }
      else if(CMD == 0xD4 )
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 3] == 0xAA) && (Usart3_Rx_Buf[RXD_OFFSET + 2] == ServoNO) )
        {
          tRet = Usart3_Rx_Buf[RXD_OFFSET + 6] << 8;
          tRet |= Usart3_Rx_Buf[RXD_OFFSET + 7] << 0;
        }
      }
      
    }
    else if(Head == 0xFC)
    {
      if(CMD == 1 )
      {
        if((Usart3_Rx_Buf[RXD_OFFSET + 3] == 0xAA) && (Usart3_Rx_Buf[RXD_OFFSET + 2] == ServoNO) )
        {
          tRet = Usart3_Rx_Buf[RXD_OFFSET + 6] << 8;
          tRet |= Usart3_Rx_Buf[RXD_OFFSET + 7] << 0;
        }
      }
    }
    else if( (Head < 0xFA) && (Head > 0x00) && (Usart3_Rx_Buf[length + 4] == CMD) )
    {
      if(CMD == 0x02 )
      {
        tRet = Usart3_Rx_Buf[length + 5] - 0xAA;
        
      }
            else if(CMD == 0x03 )
      {
        tRet = Usart3_Rx_Buf[length + 5] - 0xAA;
        
      }
      else if(CMD == 0x04 )
      {
        //Serial.println(Usart3_Rx_Buf[length + 3]);
        if(Usart3_Rx_Buf[length + 3] == 7)
          tRet = ((Usart3_Rx_Buf[length + 5] - 0xAA) << 8) + Usart3_Rx_Buf[length + 6];                
        else   if(Usart3_Rx_Buf[length + 3] == 8)
          tRet = (Usart3_Rx_Buf[length + 6] << 8) + Usart3_Rx_Buf[length + 7];
        else   if(Usart3_Rx_Buf[length + 3] == 9)
          tRet = (Usart3_Rx_Buf[length + 7] << 8) + Usart3_Rx_Buf[length + 8];
        else   if(Usart3_Rx_Buf[length + 3] == 17)
        {    
              if(RGB == 'R')    
                tRet = Usart3_Rx_Buf[length + 6]; 
              else if(RGB == 'G')   
                tRet = Usart3_Rx_Buf[length + 7];
              else if(RGB=='B')
                tRet = Usart3_Rx_Buf[length + 8]; 
        }
        else
          tRet=0;
          
      }
      else if(CMD==0x06|CMD==0x07)
      {
        tRet = Usart3_Rx_Buf[length + 5] - 0xAA;
      }
      
    }

  }
  return tRet;
}

unsigned char  * SemiduplexSerial::TXDRandom(unsigned char Head,unsigned char ServoNO,unsigned char len,unsigned char CMD,unsigned char * Data){
  unsigned long tRet = 0;
  static unsigned char data[3]={0,0,0};  
  unsigned char tCnt = 0;
  unsigned long temp = 2; //2ms 发完
  unsigned char buf[40];
  unsigned char length = 9; //9+1
  
  unsigned char Usart3_Rx_Ack_Len=0;
   memset((void *)data,0,sizeof(data));
  memset((void *)Usart3_Rx_Buf,0,sizeof(Usart3_Rx_Buf));
  memset((void *)buf,0,sizeof(buf));
  Usart3_Rx_Ack_Len = 30; //应答消息长度
  
  buf[0] = Head;  //填充协议头
  buf[1] = swab8(Head);
  if(Head >= 0xFA)
  {
    buf[2] = ServoNO; //舵机好
  }
  else if(Head > 0x00)  //变长协议
  {
    length = len + 5; //长度
    buf[2] = length;
  }
  buf[3] = CMD;
  memcpy((void *)&buf[4],(void *)Data,len);
  buf[length - 1] = Cheak_Sum( (length - 3),(u8*)&buf[2]);
  buf[length ] = 0xED;
  
  
  if(Head == 0xFA)
  {
    if(CMD == 1 )
    {
      if(ServoNO == 0)
      {
        //这句有问题 memset 不支持双字节的填充
        //memset((void *)&gsSave_Angle,0xFF,sizeof(gsSave_Angle));  //保存所有角度
        //memset((void *)&gsSave_Angle,Data,sizeof(gsSave_Angle));  //保存所有角度
        memset((void *)&gsSave_Angle,Data[0],sizeof(gsSave_Angle)); //保存所有角度
      }
      else
      {
        if(Data[0] == gsSave_Angle.Angle[ServoNO - 1])  return  0;  //不发送,直接返回
        else
        {
          gsSave_Angle.Angle[ServoNO - 1] = Data[0];  //保存单个角度,以后可能双字节角度
        }
      }
      Usart3_Rx_Ack_Len = 1;  //1,4命令只应答一个字节
    }
    else if( CMD == 4)  Usart3_Rx_Ack_Len = 1;  //1,4命令只应答一个字节
  }
  
Retry_Servo:
  
  temp = (Usart3_Rx_Ack_Len + 5) ;  //接收消息长度,用于计算接收时间,1个字节 0.087ms,预留5个空闲,10%误差
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(temp*87*110/100 /400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(buf,length + 1);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  
  tRet = Serial3.readBytes( Usart3_Rx_Buf, Usart3_Rx_Ack_Len+10); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
 
  
  if(tRet > RXD_OFFSET) //打印返回消息
  {
      //Serial.write(Usart3_Rx_Buf + RXD_OFFSET,tRet - RXD_OFFSET);
      //Serial.print("\r\n");
  }
  
  if(tRet == 0) //没有接收到消息
  {
    if( tCnt < 2)
    {
      tCnt ++;  //重试
      goto  Retry_Servo;
    }
  }
  else  //接收到消息
  {
  Usart3_Rx_Buf_count = tRet;
    tRet = 0;
    if(Head == 0xFA)
    {
      if( (CMD == 1) || (CMD == 4) )
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 1] == 0xAA + ServoNO) )
        {
          tRet = ServoNO;
        }
      }
      else if(CMD == 2 | CMD==3)
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 3] == 0xAA) && (Usart3_Rx_Buf[RXD_OFFSET + 2] == ServoNO) )
        {
          tRet = Usart3_Rx_Buf[10+6] << 8;
          tRet |= Usart3_Rx_Buf[10+7] << 0;
        }
      }

      
      else if(CMD == 0xCD )
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 5] == ServoNO) )
        {
          tRet = Usart3_Rx_Buf[RXD_OFFSET + 5];
        }
      }
      else if(CMD == 0xD2 )
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 3] == 0xAA) && (Usart3_Rx_Buf[RXD_OFFSET + 2] == ServoNO) )
        {
          tRet = 1;
        }
      }
      else if(CMD == 0xD4 )
      {
        if( (Usart3_Rx_Buf[RXD_OFFSET + 3] == 0xAA) && (Usart3_Rx_Buf[RXD_OFFSET + 2] == ServoNO) )
        {
          tRet = Usart3_Rx_Buf[RXD_OFFSET + 6] << 8;
          tRet |= Usart3_Rx_Buf[RXD_OFFSET + 7] << 0;
        }
      }
      
    }
    else if(Head == 0xFC)
    {
      if(CMD == 1 )
      {
        if((Usart3_Rx_Buf[RXD_OFFSET + 3] == 0xAA) && (Usart3_Rx_Buf[RXD_OFFSET + 2] == ServoNO) )
        {
          tRet = Usart3_Rx_Buf[RXD_OFFSET + 6] << 8;
          tRet |= Usart3_Rx_Buf[RXD_OFFSET + 7] << 0;
        }
      }
    }
    else if( (Head < 0xFA) && (Head > 0x00) && (Usart3_Rx_Buf[length + 4] == CMD) )
    {
      if(CMD == 0x02 )
      {
        tRet = Usart3_Rx_Buf[length + 5] - 0xAA;
        
      }
            else if(CMD == 0x03 )
      {
        tRet = Usart3_Rx_Buf[length + 5] - 0xAA;
        
      }
      else if(CMD == 0x04 )
      {
        //Serial.println(Usart3_Rx_Buf[length + 3]);
        if(Usart3_Rx_Buf[length + 3] == 7)
          tRet = ((Usart3_Rx_Buf[length + 5] - 0xAA) << 8) + Usart3_Rx_Buf[length + 6];                
        else   if(Usart3_Rx_Buf[length + 3] == 8)
          tRet = (Usart3_Rx_Buf[length + 6] << 8) + Usart3_Rx_Buf[length + 7];
        else   if(Usart3_Rx_Buf[length + 3] == 9)
          tRet = (Usart3_Rx_Buf[length + 7] << 8) + Usart3_Rx_Buf[length + 8];
        else   if(Usart3_Rx_Buf[length + 3] == 17)
        {          
          data[0] = Usart3_Rx_Buf[length + 6]; //R
          data[1] = Usart3_Rx_Buf[length + 7]; //G           
          data[2] = Usart3_Rx_Buf[length + 8]; //B       
                
        }
        else{
          data[0]=0;
          data[1]=0;
          data[2]=0;
        }
          tRet=0;
          
      }
      else if(CMD==0x06|CMD==0x07)
      {
        tRet = Usart3_Rx_Buf[length + 5] - 0xAA;
      }
      
    }

  }
  
  return data;

  
}
unsigned long SemiduplexSerial::TXD(unsigned char len,unsigned char * Data){
   unsigned char Rx_Buf[23];
  unsigned long tRet = 0,lens=0;
  memset((void *)Rx_Buf,0,sizeof(Rx_Buf));
  lens=len+5;
  Serial3.begin(115200,SERIAL_8N1);  //uart3
  Serial3.setTimeout(lens*87*110/100/400);  //设置超时ms
  Serial2.begin(115200,SERIAL_8N1);  //设置波特率
  Serial2.write(Data,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  Serial3.readBytes(Rx_Buf,23); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息

  if((Rx_Buf[6]==4 |Rx_Buf[6]==6 | Rx_Buf[6]==3) &( Rx_Buf[8]==3 | Rx_Buf[8]==1)){//move&stop&setid//getsoundid   
     if(Rx_Buf[len+3]==0x05 & Rx_Buf[len+5]==0){   
      tRet= Rx_Buf[len+6]<<8 | Rx_Buf[len+7]&0xff; 
    }
    else{
      tRet=0;
    }
    
  }
  else if((Rx_Buf[6]==7)  & (Rx_Buf[8]==1)){//readspeed
    if(Rx_Buf[len+3]==0x05 & Rx_Buf[len+5]==0){
      tRet=abs((Rx_Buf[len+6]<<8) | (Rx_Buf[len+7] &0xff));
    }
    else{
      tRet=0;//停止成功返回0 
      
    }
  }  
    else if((Rx_Buf[6]==0)  & (Rx_Buf[8]==1)){//sound read
    if(Rx_Buf[len+3]==0x05 & Rx_Buf[len+5]==0){
       tRet=(Rx_Buf[len+6]<<8) |(Rx_Buf[len+7] & 0xff);
    }
    else{
      tRet=0;//停止成功返回0 
      
    }
  }  
    else if(Rx_Buf[6]==0x0e  & Rx_Buf[8]==0x01){//readspeed
    if(Rx_Buf[len+3]==0x06 & Rx_Buf[len+5]==0){
        tRet=0;//停止成功返回0

    }
    else{
      tRet=0;//停止成功返回0 
      
    }
  }

  memset((void *)Data,0,sizeof(Data));
  return tRet;
}
unsigned long SemiduplexSerial::MTXD(unsigned char len,unsigned char * Data){
   unsigned char Rx_Buf[23];
  unsigned long tRet = 0,lens=0;
  memset((void *)Rx_Buf,0,sizeof(Rx_Buf));
  lens=len+5;
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(lens*87*110/100/400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(Data,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  Serial3.readBytes(Rx_Buf,23); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息

  if(Rx_Buf[len+4]==0){
    Serial3.begin(114200);  //uart3
    Serial3.setTimeout(lens*88*110/100/400);  //设置超时ms
    Serial2.begin(114200);  //设置波特率
    Serial2.write(Data,len);  //发送消息
    Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
    Serial2.end();  //关闭串口2,否则会影响接收消息
    Serial3.readBytes(Rx_Buf,23); //接收应答
    Serial3.end();  //关闭串口3,否则会影响接收消息
  }

  if((Rx_Buf[6]==4 |Rx_Buf[6]==6 | Rx_Buf[6]==3 ) &( Rx_Buf[8]==3 | Rx_Buf[8]==1)){//move&stop&setid//getsoundid   
     if(Rx_Buf[len+3]==0x05 & Rx_Buf[len+5]==0){   
      tRet= Rx_Buf[len+6]<<8 | Rx_Buf[len+7]&0xff; 
    }
    else{
      tRet=0;
    }
    
  }
  else if(Rx_Buf[6]==9  && Rx_Buf[8]==5){//id
    
    if(Rx_Buf[len+3]==0x05 & Rx_Buf[len+5]==0){
      tRet=Rx_Buf[len+4];
          
    }
    else{
      tRet=0;
      
    }
  }
  else if((Rx_Buf[6]==7)  & (Rx_Buf[8]==1)){//readspeed
    if(Rx_Buf[len+3]==0x05 & Rx_Buf[len+5]==0){
      tRet=abs((Rx_Buf[len+6]<<8) | (Rx_Buf[len+7] &0xff));
    }
    else{
      tRet=0;//停止成功返回0 
      
    }
  }  
    else if((Rx_Buf[6]==0)  & (Rx_Buf[8]==1)){//sound read
    if(Rx_Buf[len+3]==0x05 & Rx_Buf[len+5]==0){
       tRet=(Rx_Buf[len+6]<<8) |(Rx_Buf[len+7] & 0xff);
    }
    else{
      tRet=0;//停止成功返回0 
      
    }
  }  
    else if(Rx_Buf[6]==0x0e  & Rx_Buf[8]==0x01){//readspeed
    if(Rx_Buf[len+3]==0x06 & Rx_Buf[len+5]==0){
        tRet=0;//停止成功返回0

    }
    else{
      tRet=0;//停止成功返回0 
      
    }
  }

  memset((void *)Data,0,sizeof(Data));
  return tRet;
}
unsigned char SemiduplexSerial::IdTxd(unsigned char len,unsigned char * Data){
   unsigned char Rx_Buf[23];
  unsigned char tRet = 0,tCnt=0;
  unsigned long lens=0;
  memset((void *)Rx_Buf,0,sizeof(Rx_Buf));
  lens=len+5;
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(lens*88*110/100/400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(Data,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  Serial3.readBytes(Rx_Buf,23); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息

if(Rx_Buf[len+4]==0){
    Serial3.begin(114200);  //uart3
  Serial3.setTimeout(lens*88*110/100/400);  //设置超时ms
  Serial2.begin(114200);  //设置波特率
  Serial2.write(Data,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  Serial3.readBytes(Rx_Buf,23); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
}
    
    if(Rx_Buf[len+3]==0x05 && Rx_Buf[len+5]==0){
      tRet=Rx_Buf[len+4];
          
    }
    else{
      tRet=0;
      
    }

  
  

  //memset((void *)Data,0,sizeof(Data));
  return tRet;
}
signed long SemiduplexSerial::TXD(unsigned char len,unsigned char choice,unsigned char * Data){
  unsigned char Rx_Buf[26];
  signed long tRet = 0;
  float fRet=0;
  memset((void *)Rx_Buf,0,sizeof(Rx_Buf));
  
  Serial3.begin(115200);  //uart3
  Serial3.setTimeout(len+5*87*110/100/400);  //设置超时ms
  Serial2.begin(115200);  //设置波特率
  Serial2.write(Data,len);  //发送消息
  Serial2.flush(); // add by lqh on 2022-05-28 to for all the data written out 
                  // before calling Serial2.end(). Otherwise, data is missing and protocol doesn’t work
  Serial2.end();  //关闭串口2,否则会影响接收消息
  Serial3.readBytes(Rx_Buf,26); //接收应答
  Serial3.end();  //关闭串口3,否则会影响接收消息
  if(Rx_Buf[6]==0  &Rx_Buf[8]==2){//温湿度
    if(Rx_Buf[len+3]==0x05 & Rx_Buf[len+5]==0){
      
      if(choice=='C')
      {
        tRet=ceil(((Rx_Buf[len+6]<<8) | (Rx_Buf[len+7] & 0xff))/10.0);
     
       
        
      }
      else if(choice=='F'){
       tRet=ceil(((Rx_Buf[len+6]<<8) | (Rx_Buf[len+7] & 0xff))/10.0);
        tRet*=1.8;
        tRet+=32;
            
      }       
      else if(choice=='H')
        tRet=(Rx_Buf[len+8]<<8)+Rx_Buf[len+9];
    }
    else{
      tRet=0;//停止成功返回0 
      
    }
  }
 
  memset((void *)Data,0,sizeof(Data));
  return tRet;
}

/**@brief EN:Communication protocol sending and receiving functions/CN:检查舵机是否存在。
 *
 * @param[in] Head EN:servo id/CN:舵机号.
 *
 * @returns tRet EN:Accept return value/CN:接受返回值.
 */
unsigned char SemiduplexSerial::Get_Servo_list(unsigned char Servo_NO){
  return(Read_bit(gServo_list[Servo_NO / 8],Servo_NO % 8 ));
}

void SemiduplexSerial::Set_Servo_list(unsigned char Servo_NO){
  Set_bit(gServo_list[Servo_NO / 8],Servo_NO % 8 );
}

void SemiduplexSerial::Clr_Servo_list(unsigned char Servo_NO){
  Clr_bit(gServo_list[Servo_NO / 8],Servo_NO % 8 );
}

/**@brief EN:Check the number of the servo/CN:检查舵机个数
 */
void SemiduplexSerial::check_servo(){//花两秒时间
  unsigned char tID = 1;
  unsigned char tData[4] = {0,0,0,0};
  
  memset((void*)gServo_list,0,sizeof(gServo_list));
  gServos = 0;
  TXD(0xFC,tID,4,0x01,tData ); //预先测试一下
  for(tID = 1; tID < SERVO_NUMER_MAX + 1; tID++)
  {
    if( TXD(0xFC,tID,4,0x01,tData ) != 0 )
    {
      Set_Servo_list(tID - 1);  //设置舵机列表,后面多舵机使用
      gServos++;
    }
    else
    {
      Clr_Servo_list(tID - 1);
    }
  }
 Serial.print("\tTotal Servo:");
  Serial.print(gServos, DEC);
Serial.print("\r\n");
}

static uint8_t SemiduplexSerial::_crc8(unsigned short data){
  for(int i = 0; i < 8; i++)
  {
    if(data & 0x8000)
    {
      data = data ^ POLY;
    }
    
    data = data << 1;
  }

  return (uint8_t)(data >> 8);
}

static uint32_t SemiduplexSerial::crc8(uint32_t crc, const uint8_t *vptr, uint32_t len){
  for(int i = 0; i < len; i++)
  {
    crc = _crc8((crc ^ vptr[i]) << 8);
  }

  return crc;
}

/**@brief EN:JIMU MOTOR Calculate the CRC8 checksum/CN:JIMU电机计算CRC8校验
 *
 * @param[in] pBuf EN:Data buffer/CN:数据缓冲区
 * @param[in] len EN:Data length/CN：数据长度.
 *
 * @returns crc EN:CRC8 checksum value/CN:CRC8校验值.
 */
uint32_t SemiduplexSerial::crc8_itu(const uint8_t *pBuf, uint32_t len){
  uint32_t crc;
  crc = crc8(0 ,pBuf, len);
  crc ^= 0x55;
  
  return crc;
}

/**************************************************
 * @name        UbtExploreSendTo
 * @brief       explore send out data
 * @param[in]   ucFlag   flag
 * @param[in]   stHead head of protolcol
 * @param[in]   ucDataLen data len
 * @param[in]   ptrData data
 * @return      int Length of data sent,-1 error
 * @author      chenglong.xiong
 * @date        2019/12/18
 * @version     1.2.6
**************************************************/
int SemiduplexSerial::UbtExploreSend(unsigned char ucFlag, stVisionHead * stHead, unsigned char ucDataLen, unsigned char * ptrData)
{
  unsigned char strSendBuf[VISION_SERIAL_BUFSIZE] = {0};
  uint32_t           iLen = 0, uiRet = 0;

  /*build packet*/
  strSendBuf[0] = ucFlag;
  strSendBuf[1] = sizeof(stVisionHead);
  memcpy(strSendBuf + 2, stHead, sizeof(stVisionHead));
  iLen = 2 + sizeof(stVisionHead);
  strSendBuf[iLen] = ucDataLen;
  iLen += 1;
  if(ucDataLen)
  {
    memcpy(strSendBuf + iLen, ptrData, ucDataLen);
    iLen += ucDataLen;
  }
  
  strSendBuf[iLen] = crc8_itu(strSendBuf, iLen);
  iLen += 1;

  /*send data  #vision use serial1 communicate with explore*/
  //Serial1.begin(115200);
  uiRet = mySoftSerial1.write(strSendBuf, iLen);
  #ifdef DEBUG_PRINT_TAG
    int i = 0;
    LOG_PRINT("send========================ret:");
    LOG_PRINTLN(uiRet);
    Serial2.begin(115200);
    for(; i < (iLen - 1); i++)
    {
      Serial2.print(strSendBuf[i], HEX);
      Serial2.print("#");
    }
    Serial2.println(strSendBuf[i], HEX);
    Serial2.end();
  #endif
  #if 0
  /*charley test*/
  int i = 0;
  Serial.print("send========================ret:");
  Serial.println(uiRet);
  for(; i < (iLen - 1); i++)
  {
    Serial.print(strSendBuf[i],HEX);
    Serial.print("#");
  }
  Serial.println(strSendBuf[i],HEX);
  #endif
  return uiRet;
}

/**************************************************
 * @name        UbtExploreRead
 * @brief       explore read data from vision
 * @param[out]  ptrBuf return data from serial
 * @param[in]   iBufLen data len
 * @param[in]   iTimeOut time out
 * @return      int Length of data read,-1 error
 * @author      chenglong.xiong
 * @date        2019/12/20
 * @version     1.2.6
**************************************************/
int SemiduplexSerial::UbtExploreRead(unsigned char * ptrBuf, int iBufLen, int iTimeOutData, int iWaitTime)
{
  uint8_t iData = 0;
  int iLen = 0;
  int iTime = iTimeOutData;
  //int iTimeOut = VISION_READ_TIMEOUT;
  
  /*time out*/
  #if 1
  while(iTime--)
  {
    if(mySoftSerial1.available() > 0)
    {
      break;
    }
    delay(1);
  }
  #endif

  #ifdef DEBUG_PRINT_TAG
    LOG_PRINT("read========================iTime:");
    LOG_PRINTLN(iTime);
  #endif

  //Serial.print("read========================iTime:");
  //Serial.println(iTime);
  /*read from serial until buf empty*/
  //while((iData = mySoftSerial1.read()) != -1)
  iTime = iWaitTime;
  //while(mySoftSerial1.available() > 0)
  //iTime = iTimeOut;
  while(1)
  {
    #if 1
    /*time out*/
    while(mySoftSerial1.available() <= 0)
    {
      if((iTime--) <= 0)
      {
        break;
      }
      delay(1);
    }
    
    if(iTime <= 0)
    {
      break;
    }
    #endif
    iData = mySoftSerial1.read();
    if(iLen >= iBufLen)
    {
      return -1;
    }
    ptrBuf[iLen++] = iData;
    
    iTime = iWaitTime;
  }
  
  #ifdef DEBUG_PRINT_TAG
    int i = 0;
    LOG_PRINT("read========================:");
    LOG_PRINTLN(iLen);
    Serial2.begin(115200);
    for(; i < (iLen - 1); i++)
    {
      Serial2.print(ptrBuf[i], HEX);
      Serial2.print("#");
    }
    Serial2.println(ptrBuf[i], HEX);
    Serial2.end();
  #endif

  #if 0
  /*charley test*/
  int i = 0;
  Serial.print("read========================:");
  Serial.println(iLen);
  for(; i < (iLen - 1); i++)
  {
    Serial.print(ptrBuf[i],HEX);
    Serial.print("#");
  }
  Serial.println(ptrBuf[i],HEX);
  #endif
  return iLen;
}

/**************************************************
 * @name        UbtSerialBegin
 * @brief       Serial Begin
 * @param[in]   iId serial id
 * @param[in]   uBitRate serial bitrate
 * @return      0 sucess,-1 error
 * @author      chenglong.xiong
 * @date        2019/12/20
 * @version     1.2.6
**************************************************/
int SemiduplexSerial::UbtSerialBegin(int iId, uint32_t uBitRate)
{
  int ret = 0;
  /*multi serial*/
  switch(iId)
  {
    case 0:
        Serial.begin(uBitRate);
      break;
    case 1:
        Serial1.begin(uBitRate);
      break;
    case 2:
        Serial2.begin(uBitRate);
      break;
    case 3:
        Serial3.begin(uBitRate);
      break;
    case 6463:
        mySoftSerial1.begin(uBitRate);
      break;
    default:
        ret = -1;
      break;
  }
  return ret;
}

/**************************************************
 * @name        UbtSerialEnd
 * @brief       Serial end
 * @param[in]   iId serial id
 * @return      0 sucess,-1 error
 * @author      chenglong.xiong
 * @date        2019/12/20
 * @version     1.2.6
**************************************************/
int SemiduplexSerial::UbtSerialEnd(int iId)
{
  int ret = 0;
  /*multi serial*/
  switch(iId)
  {
    case 0:
        Serial.end();
      break;
    case 1:
        Serial1.end();
      break;
    case 2:
        Serial2.end();
      break;
    case 3:
        Serial3.end();
      break;
    case 6463:
        mySoftSerial1.end();
      break;
    default:
        ret = -1;
      break;
  }
  return ret;
}
