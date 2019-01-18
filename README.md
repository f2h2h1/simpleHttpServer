## 简介
- 这是一个简单的 http 服务器
- 运行于 windows 环境下
- 使用 C 语言写成
## 编译
1. 请确保你的电脑已安装好 MinGW ，并且已经把 MinGW 加入了环境变量
2. 运行 build.bat 的批处理文件
## 如何使用
编译完成后，双击生成的exe文件即可
程序会占用2047端口，程序所在的目录会作为网站的根目录
## cgi 脚本
这个程序支持以下五种后缀名的 cgi 脚本，其中cgi后缀的是可执行文件
```
".cgi", ".pl", ".py", ".php", ".rb"
```
使用 cgi 脚本时，请确保文件的第一行以这种格式说明脚本的解释器路径
```
#!C:\Program Files\language\Python36\python.exe
```
如果脚本的解释器已加入环境变量也可以写成这样
```
#!python
```
cgi 脚本输出的第一行需要指定响应头中的 Content-Type，例如这样
```
Content-Type: text/html; charset=utf-8
```
## 目录结构
```
├─target           生成目录
│  ├─test.py       测试用的cgi脚本
│  ├─test.php      测试用的cgi脚本
│  ├─test.cgi      测试用的cgi可执行文件
│  ├─index.html    测试用的html文件
├─src              当前版本源码
│  ├─http.c        程序入口
│  ├─parser.c      解释 http 请求报文
│  ├─linklist.c    链表
│  ├─test.c        test.cgi的源码
├─build.bat        执行makefile的脚本
├─clean.bat        删除生成的文件
├─makefile
```
## 这个程序编写的大致思路
1. tpc的套路，socket，bind，listen，accept
2. 使用 select 模型
3. 在接收到 http 请求后解释 http 请求
4. 通过文件后缀判断是动态请求还是静态请求
5. 通过 cgi 脚本的首行获取解释器的路径
6. 难点在于解释 http 请求和执行 cgi 脚本是的输入输出重定向
## TODO
1. 解码 url 编码
2. 确保，http 进程退出之后，cgi 进程也跟着退出
3. 使用子进程
4. cgi 请求使用 chunked
5. 使用配置文件加载配置
