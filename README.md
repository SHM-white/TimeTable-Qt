# `Qt版本`的简易课程表程序

目的是取代黑板上写课表和值日班长等一大堆信息，留出更多的黑板空间，撕掉黑板上的一坨贴纸啥的

## 功能

- 实时显示时间和对应课程，支持更改显示的文本格式（包括内容，字体，颜色，字号）
- 支持从csv导入课程信息（可以用Excel软件将xlsx另存为csv文件）
- 当然在一定程度上能防止老师拖课（可能对于初中有点用吧）
- 距离指定日期的倒计时（精确到秒）
- 更多的自定义选项，如背景颜色和背景图片
- 支持在单独窗口显示当日所有课程并突出显示当前课程
-  ***实验性：（目前已禁用）*** 支持开启主窗体的亚克力效果（目前只在Windows 11上成功开启过）（有概率开启失败，一般最小化后再恢复就能开启）

注意：目前所有的配置文件都是使用***UTF*** 编码，而不是***ANSI*** 编码，另存为csv的时候请选择`CSV UTF-8(逗号分隔)(.csv)`而不是`CSV(逗号分隔)(.csv)`***(v2.3.0及以后)***

## 鸽子画饼

- 将支持从xlsx（csv）和txt文件导入（目前仅支持csv）
- 更换界面库为qt（已经在换了，差一个右键菜单待移植）
- 支持提前显示下一节课的信息（已经实现了咕咕咕）
- 然后就是无尽的咕咕咕

-----

## 下载

[点此](https://github.com/SHM-white/TimeTable-Qt/releases)下载Qt版本<br>
最新版本为preview版是由于亚克力效果以及透明背景有概率开启失败，一般最小化后再恢复就能开启

[点此](https://github.com/SHM-white/TimeTable/releases)跳转至旧版本的Release下载使用Windows API的版本

## GUI使用说明

### 菜单

将鼠标指针移至主窗口上方以显示菜单

#### 窗口

- 窗口置顶：切换窗口置顶状态，默认开启
- 最小化：最小化窗口
- 退出：退出程序

#### 文件

- 从文件导入：显示从文件导入对话框
- 设置：显示设置对话框

#### 帮助

- 关于：为两个网页链接及版本信息

### 设置对话框使用方法

#### 课程标签：

此时列表显示4列内容，分别为星期，课程名称，开始时间，结束时间

##### 仅显示当前复选框：

- 若勾选，左侧只会显示所选星期（蓝色方框部分）的课程；
  - 选中左侧列表框内一项，该项属性会显示在右侧课程名称（红色方框部分）、开始时间（绿色方框部分）、结束时间（黄色方框部分）
  - 若点击“添加”按钮，会添加新的课程到列表底部
  - 若点击“修改”按钮，会修改列表所选中课程的课程信息
  - 若点击“排序”按钮，会按照开始时间排序列表所展示的课程 
  - 若点击“删除”按钮，会删除列表所选课程信息
- 若不勾选，则显示所有的课程信息
	- 若点击“添加”、“排序”按钮：行为同上
	- 此时更改左侧列表框选中项目的属性不会在右侧显示
	- 此时“修改”、“删除”按钮不再可用

##### 值日信息标签：

左侧列表显示所选星期的所有信息
- 若点击“添加”按钮，会添加新的值日信息到列表底部
- 若点击“删除”按钮，会删除列表所选值日信息
- 选中左侧列表框内一项，该项内容会显示在右侧文本编辑框内

##### 窗口标签：

- 此时删除按钮无任何作用，列表仅用来展示主窗口显示的文字
- 右侧窗口位置、窗口大小分别用来调整程序启动时的窗口位置以及主窗口的大小
- 倒数日：设置倒数目标时间
- 背景图片编辑框（图中为显示”BackGround.png”的文本编辑框）：修改主窗口显示的图片（会被缩放至主窗口的大小）
- 应用按钮：保持设置并重新加载该界面设置项

##### 文字项标签：用于修改文字项属性

- 单击列表项目会在右侧显示该项的所有属性（文字颜色、文字格式、文字大小、字体名称）
- 添加按钮：在列表底部添加文字项
- 修改按钮：修改列表所选文字项的属性
- 删除按钮：删除列表所选的文字项
- 放弃按钮：放弃刚刚所作的所有改动，重新从配置文件读取配置（只能回退到上一次点击保存时的配置）
- 保存按钮：保存更改至配置文件
- 文字项类型：
	- 时间：将文字格式转换为时间，时间格式见最后
	- 若时间格式错误，则显示原始内容
	- 当前课程：原始格式转换为时间后在末尾加上当前课程
	- 值日信息：原始格式转换为时间后在末尾加上当日的值日信息
	- 倒计时：将原始格式中的1-4个”%d”占位符分别填入剩余的天数、小时、分钟、秒

##### 配置文件标签：

- 配置文件路径：可临时加载其他配置文件
- 课程信息路径：用来保存课程信息的json文件路径

## 配置文件说明

### `config.json`

```
   "version" : [ 2, 3, 0 ]
```

配置文件版本，若小于应用程序版本则不会读取

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

`futures`同理，在鸽子画饼对话框显示（目前在qt版本被砍）
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
主窗口的大小和位置，*2.4.0加了两项为当日所有课程窗口的默认位置*

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
            [
               "%a-%H:%M:%S",
               "微软雅黑",
               [ 4, 22 ],
               18,
               [ 0, 0, 0 ],
               0
            ],
            [
               "当前课程：",
               "微软雅黑",
               [ 4, 44 ],
               18,
               [ 0, 0, 0 ],
               1
            ],
            [
               "距离春节%03d天%02d小时",
               "微软雅黑",
               [ 4, 66 ],
               18,
               [ 0, 0, 0 ],
               2
            ],
            [
               "值日信息：",
               "微软雅黑",
               [ 4, 88 ],
               18,
               [ 0, 0, 0 ],
               3
            ]
         ],
```
```
"TextFormat" : [ 
            文字项1,
            文字项2,
            文字项3
         ]
```
每个文字项由以下部分构成：

```
[
	内容,
	字体(需要在电脑上安装),
	[显示的位置x,显示的位置y],
	字体大小,
	(字体颜色)[R,G,B],
	文字项类型
]
```

***注意：倒数时间所在的行内容里需有5个`%d`字符依次存储倒数的月、日、小时、分钟、秒***<br>
***若不需要显示倒数时间，请将`"CountDownDayInLine"`设置为比较大的值（至少大于所设置的文字项数）***

## csv文件格式说明

表格文件A列为星期，B列为课程名称，C列为开始时间（24小时制，4位数字，精确到分钟，如`1234`表示`12时34分`），D列为结束时间，格式同开始时间

***注意：目前由于一堆奇怪的原因导致第一行读取有bug，所以就直接不读了，直接跳过***

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
