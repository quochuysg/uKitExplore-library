# modified uKit Explore Library v1.2.15m
## For used with ESP32 development board
- file changes: 
    1. SemiduplexSerial.cpp
       - include HardwareSerial.h
       - define pins to used with hardware serial
       - add Serial2.flush(); before Serail.end() call to make sure all     data is writen
    2. uKitID.cpp
      - Comment out #include "avr/boot.h"
    3. Comment out all calls to FlexiTmer2 in these files:
       - kuka.cpp
       - ucode.cpp
       - uCodeOneMode.cpp
    4. delete FlexiTmer2.h and .cpp due to incompatable
    5. Temporally empty newTone() and noNewTone() functions in Newtone.cpp

# uKit Explore Library v1.2.15

```
git clone git@github.com:UBTEDU/uKit-Explore-library.git
```

## Arduino Library for uKit Explore Modules

### Library Dependencies:
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [Arduino-IRremote](https://github.com/z3t0/Arduino-IRremote/)


### How to use:

1. Download the source from the github 
![image](https://github.com/UBTEDU/uKitExplore-library/blob/master/img/downloadzip.png)

2. Importing a .zip Library. In the Arduino IDE, navigate to Sketch > Include Library > Add .ZIP Library. At the top of the drop down list, select the option to "Add .ZIP Library''.
![image](https://github.com/UBTEDU/uKitExplore-library/blob/master/img/add_library.png)

3. Open the Arduino Application. (If it's already open, you will need to restart it to see changes.)

4. Click "File-> Examples". Here are some test programs in "uKit Explore->"

### Links
- [Arduino IDE](https://www.arduino.cc/en/Main/Software)
- [Drive](http://www.wch.cn/downfile/65)
- [Instructions](https://easydoc.xyz/?#/s/44498285)  

### Update

#### V1.2.15更新说明
1. 修改example代码中编译错误问题

#### V1.2.14更新说明:
1.修改imu接口，用户不再需要读取数据前调用read()

#### V1.2.13更新说明:
1.更新人车互变接口名，当前接口与STL接口重名

#### V1.2.12更新说明:
1.修正文件头的保护宏

#### V1.2.11更新说明:

1.修改红外传感器接口，修复恒定距离数值不定问题

#### V1.2.10更新说明：

1.修改软件串口函数名，原函数名与ucode 串口积木块重名

#### V1.2.9更新说明

1.修改蓝线距离

#### V1.2.8更新说明：
1. 修改超声波传感器值范围

#### 2020-02-11 更新说明：

1. 新增了 打 tag 自动推送到 npm 仓库
2. 原来的默认目录移动到  lib 下面
3. 所有逻辑是在 Jenkins 下完成的

#### V1.2.7更新说明：
1. 修复一直扫描不到wifi
2. 修复获取不到设备id

#### V1.2.6.200107更新说明：
1. 修复扫描不到视觉模块id
2. 修复超时时间不准
3. 增加视觉模块wifi接口

#### V1.2.6更新说明：
1. 增加视觉模块
2. 增加偏移量接口

#### V1.2.4更新说明：
1. 修复SN烧录返回code无法正确指示问题
2. 修复偶尔读取不到SN问题


#### V1.2.3更新说明：
1. 修复蓝牙连接不上的问题
2. 修复按压传感器拔插后无法读取问题


#### V1.2.2更新说明：
1. 修复离线模式初始化程序不执行问题
2. 修复ucode库离线模式中断不能用BUG
3. 修复电机pwm堵转导致ID获取重复BUG

#### V1.2.1更新说明：
1. 修复离线模式读取乱码丢码问题
2. 修复舞台模式自定义眼灯失效问题
3. 去掉了蓝牙波特率设置并修改为115200

#### V1.2.0更新说明：
1. 蓝牙和USB串口通讯速率修改为1M
2. 新增单独获取舵机外设ID指令
3. 新增获取眼灯外设ID指令
4. 新增获取电机外设ID指令
5. 新增电机恒速群发指令
6. 新增电机pwm群发指令
7. 新增舵机角度模式群发指令
8. 新增舵机轮模式群发指令
9. 新增亮起眼灯群发指令
10. 新增眼灯表情群发指令
11. 新增情景灯群发指令
12. 新增自定义灯瓣群发指令

#### V1.1.8更新说明：
1. 自定义眼灯时间改回100-9999ms
2. 修复了自定义眼灯不能关闭的问题
3. 优化了所有传感器重复ID检测不到的问题
4. 修复了偶尔修改不了颜色传感器ID问题
5. 修复舞台模式打印陀螺仪数据导致死机问题

#### V1.1.7更新说明：
1. 新增眼灯表情阻塞功能
2. 新增情景灯阻塞功能
3. 恢复开机打印设备类型和版本号
4. 灯光时长持续时间优化为3600s
5. 修复编译警告
6. uCode库加入眼灯阻塞等功能


#### V1.1.6更新说明：
1. 修复蜂鸣器与IR冲突 BUG
2. 取消空格分隔符通讯机制，改为长度和协议头
3. 加入蜂鸣器阻塞功能
4. 新增自定义灯瓣颜色功能
5. 修复升级固件失败问题
6. 修复重复按压传感器，出现颜色传感器问题


#### V1.1.5更新说明
1. 加入检查设备ID重复问题
2. 修复重复ID不显示设备问题
3. 修复颜色传感器误识别问题
4. 修复上传模式陀螺仪打印失败问题
5. 修复眼灯关闭后不能亮问题
6. 修复上传模式初始化程序不能用问题
7. 打印双精度输出修改为后两位
8. 优化红外传感器数值问题

#### V1.1.4更新说明
1. 修复上传模式切换到舞台模式，舵机没有停止
2. 修复红外传感器距离算法与ukit app端不一致
3. 修复打印电机转速不可显示
4. 修复字符串打印不出来(整数或者double类型的可以打印)

#### V1.1.3更新说明
1. 修复在线模式板载按键延迟问题
2. 新增板载按键长按保持功能
3. 替换分包符号，使用协议中极少使用的空格作为分包符号
4. 修复自定义灯瓣不能只亮一个灯瓣 BUG
5. 修复自定义灯光嵌套到重复执行里面不能再次点亮 BUG
6. 修复红外传感器距离算法与ukit app端不一致问题
7. 优化红外传感器执行速度
8. 修复打印舵机角度值只显示一次
9. 加入获取CPU 唯一码功能​

#### V1.1.2更新说明
1. 修复了初次打开串口时，读取设备信息提示有时缺少的问题
2. 修复打印四个颜色传感器【B】值（仅连接三个颜色传感器）时，未连接的传感器ID打印值不为0的问题
3. 修复了频繁打印串口时，开始的几个数据会乱码的问题
4. 加入开机获取设备信息
5. 加入通讯烧录二合一框架
6. 加入ucode支持
7. 加入日志模式，新增consoleLog函数
8. 加入获取芯片UUID功能
9. 新增泰文支持

#### V1.1.1更新说明
1. 修复初次打开串口时，读取设备信息提示有时缺少
2. 修复接入两个颜色传感器时只有一个可以使用
3. 调整了一些翻译问题

#### V1.1.0更新说明
1. 修复长时间打印颜色传感器会读取不出来的问题
2. 解决颜色传感器多次读取r值时，有时获取的数值为0的问题
3. 增加英文库，uKitExploreEn.h/uKitExplore2En.h
4. 修复ID获取声音传感器为电机的BUG
5. 优化频繁打印串口时，开始的几个数据会乱码的情况


#### V1.0.9更新说明
1. 开机获取ID按照设备类型排序
2. 优化颜色传感器读取，修复颜色传感器多次读取r值时，有时获取的数值为0的问题
3. 亮度传感器最大值设定在4000
4. 新增复位关闭所有设备
5. 加入陀螺仪姿态融合获取

#### V1.0.8更新说明
1. 优化颜色识别算法
2. 进一步优化Json解析库
3. 亮度传感器最大值设定在4000
4. 优化灰度传感器识别灵敏度

#### V1.0.7更新说明
1. 温湿度传感器读取数据改为四舍五入，减小误差
2. 加入RGB转HSB算法，优化颜色识别
3. 修改ID输入小数、中文等均能提示
4. 加入json解析库并适配
5. 优化了灯瓣的逻辑和操作，解决不同嵌入式版本时间问题
6. 修改了一些传感器名称不统一的问题

#### V1.0.6更新说明
1. 修复了获取ID字符缺漏的问题
2. 修复了颜色传感器检测颜色问题，并支持10种uKit颜色识别
3. 增加uKit Explore 2.0主控支持
4. 加入眼灯灯瓣功能
5. 修复陀螺仪读取问题

#### V1.0.5更新说明
1. 规范化函数命名方式
2. 若已安装了本程序，安装前加入卸载提示功能
2. 修复和优化了颜色传感器的读取
3. 开放舵机速度，修改底层实现真正的舵机运行时间。（不占用CPU任务）
4. 声音传感器的ADC由0~4069改为0~1023 

#### V1.0.4更新说明：
1. 加入开机获取设备ID功能，烧录新程序后，打开串口会显示当前接入的设备和ID号（重复ID暂时无法检测）。
2. 加入修改ID功能
3. 加入FlexiTimer2库，支持定时器功能。
4. 添加电机pwm调速

#### V1.0.3更新说明：
1. 修复了接入多个RGB传感器打印数据覆盖问题
2. 修复了电机逆时针读速度错误问题
3. 修复了NOC块车型不能动问题
4. 修复了板载按键长按错误问题
5. 超声波读取数据由毫米转给厘米
6. 蜂鸣器的节拍去掉，改为可输入的响亮时间
7. 新增uKitExploreBlockly库，解决Arduino IDE开发和Blockly开发版本不一致互相影响问题。

#### V1.0.2更新说明：
1. 修改了舵机正反转错误问题
2. 增加了声响传感器、光感传感器支持；传感器已全支持
3. 修复舵机回读大于118的问题
4. 舵机轮模式速度改为0~255
5. 修复眼灯模块、陀螺仪、变量不可用问题
6. 修复红外传感器读取数值跳变问题
7. 修复舵机速度不变化问题




### Learn more from UBTECH official website: www.ubtrobot.com
