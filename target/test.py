#!python
# -*- coding: UTF-8 -*-
import cgi
import os
import sys
import codecs
sys.stdout = codecs.getwriter('utf8')(sys.stdout.buffer)

os.popen('chcp 65001')
print("Content-Type: text/html; charset=utf-8", end='\r\n')
print("", end='\r\n')
print("<meta charset=\"utf-8\">")
print("<b>环境变量</b><br>")
print("<ul>")
for key in os.environ.keys():
    print("<li><span style='color:green'>%30s </span> : %s </li>" % (key,os.environ[key]))
output = os.popen('chcp')
chcp = "<li>"+output.read()+"</li>"
print(chcp)
print("</ul>")
print("hello world")