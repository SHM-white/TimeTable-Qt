# `Qt版本`的简易课程表程序

目的是取代黑板上写课表和值日班长等一大堆信息，留出更多的黑板空间，撕掉黑板上的一坨贴纸啥的

## 功能

- 实时显示时间和对应课程，支持更改显示的文本格式（包括内容，字体，颜色，字号）
- 支持从csv导入课程信息（可以用Excel软件将xlsx另存为csv文件）
- 当然在一定程度上能防止老师拖课（可能对于初中有点用吧）

注意：目前所有的配置文件都是使用***ANSI***编码，而不是***UTF***编码，另存为csv的时候请选择`CSV(逗号分隔)(.csv)`而不是`CSV UTF-8(逗号分隔)(.csv)`

## 鸽子画饼

- 将支持从xlsx（csv）和txt文件导入（目前仅支持csv）
- 距离指定日期的倒计时（精确到秒吧）（做完了）
- 更多的自定义选项，如背景颜色和背景图片（咕咕咕）
- 更换界面库为qt（已经在换了，功能啥的慢慢移植吧）
- 值日班长等信息将支持在主窗口显示
- 然后就是无尽的咕咕咕

-----

## 下载

[点此](https://github.com/SHM-white/TimeTable-Qt/releases)下载Qt版本

[点此](https://github.com/SHM-white/TimeTable/releases)跳转至旧版本的Release下载使用Windows API的版本

## 配置文件说明

### `config.json`

```
{
   "About" : "该程序的目的是取代每天写在黑板上的课表和值日表等信息，可以放在教室的电脑上，也可网课期间自己使用\r\n如有bug或建议请到GitHub反馈",
   "Settings" : {
      "Window" : {
         "LessonNull" : "",
         "BackGroundColor" : [255,255,255],
         "BackGroundImg" : "BackGround.png",
         "LessonInLine" : 2,
         "TextFormat" : [ 
            ["%a-%H:%M:%S","Minecraft AE",[2,22],15,[0,220,0]],
            ["当前课程：","Minecraft AE",[2,44],15,[0,220,0]],
            ["距离春节还有：","Minecraft AE",[2,66],15,[0,220,0]],
            ["%02d月%02d天-%02d:%02d:%02d","Minecraft AE",[2,88],15,[255,0,0]]
         ],
         "WindowSize" : [ 250,96 ],
         "WindowLocation" : [ 1200, 40 ],
         "LessonInfoFile" : "Lessons.json",
         "CountDownDay" : [ 2023 , 1 , 22 , 0 , 0 , 0],
         "CountDownDayInLine" : 4
      }
   },
   "futures" : "饼：\r\n1.将支持从xlsx（csv）和txt文件导入\r\n2.更多的自定义选项，如背景颜色（目前有点小问题）\r\n3.（然后应该暂时没有了）\r\n（开源库好烦啊，还是自己写个类读csv吧）\r\n4.值日班长等信息将支持在主窗口显示"
}
```

`About`是显示在关于对话框文本框中的内容，懒得改代码就做成读配置文件了

```
"About" : "该程序的目的是取代每天写在黑板上的课表和值日表等信息，可以放在教室的电脑上，也可网课期间自己使用\r\n如有bug或建议请到GitHub反馈"
```

`futures`同理，在鸽子画饼对话框显示
```
"futures" : "饼：\r\n1.将支持从xlsx（csv）和txt文件导入\r\n2.更多的自定义选项，如背景颜色（目前有点小问题）\r\n3.（然后应该暂时没有了）\r\n（开源库好烦啊，还是自己写个类读csv吧）\r\n4.值日班长等信息将支持在主窗口显示"
```

#### `Settings`块：

```
"LessonNull" : ""
```
当前时刻没有任何课程时的显示文字

```
"BackGroundColor" : [255,255,255]
```
***暂时无用***，为已经废弃的更换背景颜色做的文件接口

```
"BackGroundImg" : "BackGround.png"
```
设置窗口背景图片，若找不到则默认为白色窗口

```
"LessonInLine" : 2
```
当前的课程显示的行数，即在TextFormat的第几项的末尾显示

```
"WindowSize" : [ 250,96 ]
```
```
"WindowLocation" : [ 1200, 40 ]
```
主窗口的大小和位置

```
"LessonInfoFile" : "Lessons.json"
```
存储课程信息的json文件路径

```
"CountDownDay" : [ 2023 , 1 , 22 , 0 , 0 , 0],
```
```
"CountDownDayInLine" : 4
```
倒数日的时间`[ 年 , 月 , 日 , 时 , 分 , 秒 ]`以及所在文字项

#### TextFormat块

```
"TextFormat" : [ 
            ["%a-%H:%M:%S","Minecraft AE",[2,22],15,[0,220,0]],
            ["当前课程：","Minecraft AE",[2,44],15,[0,220,0]],
            ["距离春节还有：","Minecraft AE",[2,66],15,[0,220,0]],
            ["%02d月%02d天-%02d:%02d:%02d","Minecraft AE",[2,88],15,[255,0,0]]
         ]
```
```
"TextFormat" : [ 
            文字项1,
            文字项2,
            文字项3
         ]
```
每个文字项由以下部分构成：

`[内容,字体(需要在电脑上安装),[显示的位置x,显示的位置y],字体大小,(字体颜色)[R,G,B]]`

***注意：倒数时间所在的行内容里需有5个`%d`字符依次存储倒数的月、日、小时、分钟、秒***<br>
***若不需要显示倒数时间，请将`"CountDownDayInLine"`设置为比较大的值（至少大于所设置的文字项数）***
