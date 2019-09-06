

#ifndef SEMIDUPLEXSERIAL_h
#define SEMIDUPLEXSERIAL_h

#include <Arduino.h>

class SemiduplexSerial
{  
public:
   
    unsigned char* ubtColorProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
    unsigned char ubtColorIdProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
    
    unsigned char ubtButtonProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
    unsigned char ubtButtonStateProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
    unsigned char ubtButtonIdProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
    unsigned long ubtButtonVersionProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
    unsigned char ubtButtonUpdateProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
    
    unsigned short ubtInfraredProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
    unsigned char  ubtInfraredIdProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
    uint16_t ubtInfraredProtocols(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
    
    unsigned char ubtEyelightProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
    unsigned char ubtEyelightIdProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
    
    unsigned short ubtUltrasonicProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
     unsigned char ubtUltrasonicIdProtocol(unsigned char Head,unsigned char len,unsigned char CMD,unsigned char * Data);
    
    unsigned short ubtServoProtocol(unsigned char Head,unsigned char ServoNO,unsigned char CMD,unsigned char * Data);
    unsigned char ubtServoIdProtocol(unsigned char Head,unsigned char ServoNO,unsigned char CMD,unsigned char * Data);
    unsigned char ubtServoActionProtocol(unsigned char Head,unsigned char ServoNO,unsigned char CMD,unsigned char * Data);
     unsigned short ubtServoProtocol1M(unsigned char Head,unsigned char ServoNO,unsigned char CMD,unsigned char * Data);

    unsigned short ubtSoundProtocol(unsigned char len,unsigned char CMD,unsigned char * Data);
    unsigned char ubtSoundIdProtocol(unsigned char len,unsigned char CMD,unsigned char * Data);
    
    unsigned short ubtHumitureProtocol(unsigned char len,unsigned char CMD,char choice,unsigned char * Data);
    unsigned char ubtHumitureIdProtocol(unsigned char len,unsigned char CMD,char choice,unsigned char * Data);
    
    unsigned short ubtLightProtocol(unsigned char len,unsigned char CMD,unsigned char * Data);
    unsigned char ubtLightIdProtocol(unsigned char len,unsigned char CMD,unsigned char * Data);


    short ubtMotorProtocol(unsigned char len,unsigned char CMD,unsigned char * Data);
    unsigned char ubtMotorIdProtocol(unsigned char len,unsigned char CMD,unsigned char * Data);
   
    unsigned char ubtMotorActionProtocol(unsigned char len,unsigned char CMD,unsigned char * Data);
  
    unsigned char Cheak_Sum(unsigned char len, unsigned char *buf);
    unsigned long TXD(unsigned char Head,unsigned char ServoNO,unsigned char len,unsigned char CMD,unsigned char RGB,unsigned char * Data);
    unsigned long TXD(unsigned char Head,unsigned char ServoNO,unsigned char len,unsigned char CMD,unsigned char * Data);
    unsigned char * TXDRandom(unsigned char Head,unsigned char ServoNO,unsigned char len,unsigned char CMD,unsigned char * Data);
    unsigned long TXD(unsigned char len,unsigned char * Data);
    unsigned long MTXD(unsigned char len,unsigned char * Data);
    unsigned char IdTxd(unsigned char len,unsigned char * Data);
   
    signed long TXD(unsigned char len,unsigned char choice,unsigned char * Data);
  
    
    unsigned char Get_Servo_list(unsigned char Servo_NO);  //检查舵机存在不 ? 正不正常
    void Set_Servo_list(unsigned char Servo_NO);
    void Clr_Servo_list(unsigned char Servo_NO);
    void check_servo();
    
    static uint32_t crc8(uint32_t crc, const uint8_t *vptr, uint32_t len);
    uint32_t crc8_itu(const uint8_t *pBuf, uint32_t len);
    static uint8_t _crc8(unsigned short data);
    unsigned char redvalue,greenvalue,bluevalue=0;
    
 
private:
  #define tems(val) (val*87*110/100/400)
  #define POLY    (0x1070U << 3)
  #define SERVO_NUMER_MAX    16  //支持的最多舵机
  //置位变量的某位
  #define  Clr_bit(val, bit)  (val &= ~(1 << bit))
  //清位变量的某位
  #define  Set_bit(val, bit)  (val |= (1 << bit))
  //读变量的某位
  #define RXD_OFFSET  10  //接收消息偏移
  //#define  Read_bit(val, bit)  (val & (1 << bit))  //结果拿去计算有问题
  #define  Read_bit(val, bit)  ((val & (1 << bit)) ? 1 : 0)
  #ifndef NULL
  #define  NULL         ((void *) 0)
  #endif

  #ifndef swab16
  #define swab16(x) ((x&0x00ff) << 8 | (x&0xff00) >> 8)
  #endif
 
  #ifndef swab8
  #define swab8(x) ((x&0x0f) << 4 | (x&0xf0) >> 4)
  #endif
  typedef struct _ACTION_PACK_Struct_ //结构顺序不可调
  {
    unsigned  char Angle[SERVO_NUMER_MAX];  //
    unsigned  short Run_time16; //运行时间,分辨率ms
  }ACTION_PACK;
  typedef  struct _ANGLE_PACK_V002_Struct_ //结构顺序不可调,可在后面添加 V0.02
  {
    unsigned  char Angle[SERVO_NUMER_MAX];  //1~20舵机角度
    unsigned  short Run_time16; //运行时间,分辨率ms
  }ANGLE_PACK_V002;
    unsigned char gServos = SERVO_NUMER_MAX;  //舵机个数
    unsigned char gServo_list[SERVO_NUMER_MAX / 8]; //舵机列表,位表示
    unsigned char Usart3_Rx_Buf[120];
    unsigned char Usart3_Rx_Buf_count=0;
    volatile ANGLE_PACK_V002 gsSave_Angle;    //存角度,相同角度不再下发

};
#endif
