# `Qt版本`的简易课程表程序

目的是取代黑板上写课表和值日班长等一大堆信息，留出更多的黑板空间，撕掉黑板上的一坨贴纸啥的

## 最近更新：

- 重置ui绘制逻辑
- 添加托盘菜单，隐藏任务栏
- 全新的没做完的设置界面
- 未来更高的可扩展性

## 功能

- 实时显示时间和对应课程，支持更改显示的文本格式（包括内容，字体，颜色，字号）
- 支持从csv导入课程信息（可以用Excel软件将xlsx另存为csv文件）
- 当然在一定程度上能防止老师拖课（可能对于初中有点用吧）
- 距离指定日期的倒计时（精确到秒）
- 获取下一节课信息和倒计时
- 更多的自定义选项，如背景颜色和背景图片
- 支持在单独窗口显示当日所有课程并突出显示当前课程（v3.0.0及以后砍掉）
-  ***实验性：（目前已禁用）*** 支持开启主窗体的亚克力效果（目前只在Windows 11上成功开启过）（有概率开启失败，一般最小化后再恢复就能开启）

注意：目前所有的配置文件都是使用***UTF*** 编码，而不是***ANSI*** 编码，另存为csv的时候请选择`CSV UTF-8(逗号分隔)(.csv)`而不是`CSV(逗号分隔)(.csv)`***(Excel似乎会自动保存为`UTF-8 with BOM`的格式，所以第一行以后都不会读取)***

## 鸽子画饼

- 兼容qt5以及Windows 7（目前提供qt5和qt6两个版本，其中qt5支持Windows 7，但对缩放的支持不好，qt6支持缩放但仅支持Windows 10+（mmp为啥2024年了还有用Windows 7这个老古董系统的啊，维护起来真的麻烦））
- 将支持从xlsx（csv）和txt文件导入（目前仅支持csv）（以后支持xlsx时尽量做一个按行列的第一个单元格确定星期和时间的读取方式）
- 支持提前显示下一节课的信息（支持了，但是和理想的效果差距比较大）
- 虽然这个项目几年后不知道还会不会更新，到时候再看吧
- 然后就是无尽的咕咕咕

-----

## 下载

- [点此](https://github.com/SHM-white/TimeTable-Qt/releases)下载Qt版本
- [点此](https://github.com/SHM-white/TimeTable-Qt/actions)跳转至Action体验正在开发的版本
- [点此](https://github.com/SHM-white/TimeTable/releases)跳转至旧版本的Release下载使用Windows API的版本



## 配置文件详解

默认配置文件如下：

```
{
   "ConfigFile" : "config.json",
   "ItemScrollSpeed" : 20,
   "LessonInfoFile" : "Lessons.json",
   "MainWindow" : {
      "AcrylicEffect" : false,
      "AutoOpen" : true,
      "AutoResize" : false,
      "BackGroundColor" : [ 255, 255, 255, 100 ],
      "BackGroundImg" : "BackGround.png",
      "FPS" : 1,
      "ImgAsBackGround" : false,
      "Moveable" : true,
      "TopMost" : true,
      "UIElements" : [],
      "WindowLocation" : [ 0, 0 ],
      "WindowSize" : [ 0, 0 ]
   },
   "Windows" : [
      {
         "AcrylicEffect" : false,
         "AutoOpen" : true,
         "AutoResize" : true,
         "BackGroundColor" : [ 255, 255, 255, 100 ],
         "BackGroundImg" : "BackGround.png",
         "FPS" : 60,
         "ImgAsBackGround" : false,
         "Moveable" : true,
         "Name" : "Window 1",
         "TopMost" : true,
         "UIElements" : [
            {
               "Data" : [
                  {
                     "Data" : null,
                     "Font" : "微软雅黑",
                     "TextColor" : [ 0, 0, 0, 255 ],
                     "TextFormat" : "%a-%H:%M:%S",
                     "TextSize" : 16,
                     "Type" : 1
                  }
               ],
               "ElementType" : 0,
               "Location" : [ 0, 0 ],
               "Size" : [ 160, 30 ]
            },
            {
               "Data" : [
                  {
                     "Data" : null,
                     "Font" : "微软雅黑",
                     "TextColor" : [ 0, 0, 0, 255 ],
                     "TextFormat" : "当前课程:",
                     "TextSize" : 16,
                     "Type" : 2
                  },
                  {
                     "Data" : null,
                     "Font" : "微软雅黑",
                     "TextColor" : [ 0, 0, 0, 255 ],
                     "TextFormat" : "值日:",
                     "TextSize" : 16,
                     "Type" : 6
                  },
                  {
                     "Data" : {
                        "TargetTime" : [ 2024, 6, 18, 0, 0, 0 ]
                     },
                     "Font" : "微软雅黑",
                     "TextColor" : [ 0, 0, 0, 255 ],
                     "TextFormat" : "距离高考%03d天%02d小时%02d分%02d秒",
                     "TextSize" : 16,
                     "Type" : 5
                  },
                  {
                     "Data" : {
                        "APIKey" : "",
                        "Adcode" : "",
                        "Address" : "",
                        "City" : ""
                     },
                     "Font" : "微软雅黑",
                     "TextColor" : [ 0, 0, 0, 255 ],
                     "TextFormat" : "",
                     "TextSize" : 16,
                     "Type" : 9
                  }
               ],
               "ElementType" : 1,
               "Location" : [ 160, 0 ],
               "Size" : [ 300, 30 ]
            }
         ],
         "WindowLocation" : [ 600, 0 ],
         "WindowSize" : [ 0, 0 ]
      }
   ],
   "version" : [ 3, 0, 0 ]
}

```

### 根

```
{
   "ConfigFile" : "config.json",
   "ItemScrollSpeed" : 20,
   "LessonInfoFile" : "Lessons.json",
   "MainWindow" : {
     ...
   },
   "Windows" : [...],
   "version" : [ 3, 0, 0 ]
}
```

均为字面义，***"Windows"*** 中可以设置多个窗口

每个窗口的属性包括如下几部分：

```
    {
      "AcrylicEffect" : false,
      "AutoOpen" : true,
      "AutoResize" : false,
      "BackGroundColor" : [ 255, 255, 255, 100 ],
      "BackGroundImg" : "BackGround.png",
      "FPS" : 1,
      "ImgAsBackGround" : false,
      "Moveable" : true,
      "TopMost" : true,
      "UIElements" : [...],
      "WindowLocation" : [ 0, 0 ],
      "WindowSize" : [ 0, 0 ]
   }
```

根据需要进行设置

### ***UIElements*** 列表：

每个项目为如下格式：

```
            {
               "Data" : [
                  {
                     "Data" : null,
                     "Font" : "微软雅黑",
                     "TextColor" : [ 0, 0, 0, 255 ],
                     "TextFormat" : "%a-%H:%M:%S",
                     "TextSize" : 16,
                     "Type" : 1
                  }
               ],
               "ElementType" : 0,
               "Location" : [ 0, 0 ],
               "Size" : [ 160, 30 ]
            }
```

其中，***"ElementType"*** 对应如下类型：

|类型（"ElementType"）|对应数字|
|----|----|
|单一元素(SingleItem)|0|
|按顺序显示的多个元素(MultiItemInOrder)|1|
|同时显示的多个元素(MultiItemAllDisplay)***暂不支持***|2|

***"Data"*** 中可以保存一个或多个元素

一个元素包括如下属性：

```
                  {
                     "Data" : null,
                     "Font" : "微软雅黑",
                     "TextColor" : [ 0, 0, 0, 255 ],
                     "TextFormat" : "%a-%H:%M:%S",
                     "TextSize" : 16,
                     "Type" : 1
                  }
```

这里的 `"Data"` 根据 `"Type"` 需包含不同内容，`"Type"`对应的类型和数字如下表（标注为`*`的类型已实现）：

|类型|对应数字|
|--|--|
|	*Default              |0|
|	*CurrentTime          |1|
|	*CurrentLesson        |2|
|	*NextLesson           |3|
|	*NextLessonTime       |4|
|	*CountDownDay         |5|
|	*TodayInfo            |6|
|	News                 |7|
|	DailyWord            |8|
|	*Weather              |9|
|	AllLessons           |10|
|	CurrentLessonProgress|11|
|	WeatherBlock         |12|
|	Clock                |13|

#### 需要特殊`"Data"`的元素：

##### CountDownDay(5):

```
"TargetTime" : [ 2024, 6, 18, 0, 0, 0 ]
```
示例代表目标日期为2024年6月18日0时0分
***注意：格式`"TextFormat"`中需使用`%d`字符依次替代实际的倒数的日、小时、分钟、秒***

##### Weather(9):

```
                        "APIKey" : "",
                        "Adcode" : "",
                        "Address" : "",
                        "City" : ""
```
- `"APIKey"`需要到[高德开放平台](https://console.amap.com/dev/key/app)获取
- `"Adcode"`可留空，需要时会根据填写的"Address"和"City"自动获取
- "Address"和"City"需准确以确保获取的Adcode准确，具体填写标准见[高德开放平台官方文档](https://lbs.amap.com/api/webservice/guide/api/georegeo)

## csv文件格式说明

表格文件A列为星期，B列为课程名称，C列为开始时间（24小时制，4位数字，精确到分钟，如`1234`表示`12时34分`），D列为结束时间，格式同开始时间

***注意：目前由于Excel导出的csv文件为UTF-8 with BOM的编码，第一行用作标题***

示例：

| A | B | C | D |
|---|---|---|---|
|星期|名称|开始时间|结束时间|
|Mon|Test|1000|1100|

导入之后为星期一的上午10点到11点名为Test的课程

## 时间格式
```
    %a 星期几的简写
    %A 星期几的全称
    %b 月份的简写
    %B 月份的全称
    %c 标准的日期的时间串
    %C 年份的后两位数字
    %d 十进制表示的每月的第几天
    %D 月/天/年
    %e 在两字符域中，十进制表示的每月的第几天
    %F 年-月-日
    %g 年份的后两位数字，使用基于周的年
    %G 年分，使用基于周的年
    %h 简写的月份名
    %H 24小时制的小时
    %I 12小时制的小时
    %j 十进制表示的每年的第几天
    %m 十进制表示的月份
    %M 十时制表示的分钟数
    %n 新行符
    %p 本地的AM或PM的等价显示
    %r 12小时的时间
    %R 显示小时和分钟：hh:mm
    %S 十进制的秒数
    %t 水平制表符
    %T 显示时分秒：hh:mm:ss
    %u 每周的第几天，星期一为第一天 （值从0到6，星期一为0）
    %U 第年的第几周，把星期日作为第一天（值从0到53）
    %V 每年的第几周，使用基于周的年
    %w 十进制表示的星期几（值从0到6，星期天为0）
    %W 每年的第几周，把星期一作为第一天（值从0到53）
    %x 标准的日期串
    %X 标准的时间串
    %y 不带世纪的十进制年份（值从0到99）
    %Y 带世纪部分的十进制年份
    %z，%Z 时区名称，如果不能得到时区名称则返回空字符。
    %% 百分号
```
