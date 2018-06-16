#!/usr/bin/python
from os import system
fromlist = range(914943,914947)
head = ''
tono = ''
todir = '$HOME/rctr-wuvel%s/' % tono
reshead = 'result-k200-fixed-t000a-p'
for i,ele in enumerate(fromlist):
    cmd = 'cp -rv sdae_mult%d/. %s%s%s%d' % (ele,todir,reshead,head,ele)
    system(cmd)
