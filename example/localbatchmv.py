#!/usr/bin/python
from os import system
fromlist = range(914635,914639)
head = ''
tono = ''
todir = '$HOME/rctr-wuvel%s/' % tono
reshead = 'result-k200-fixed-t000a-p'
for i,ele in enumerate(fromlist):
    cmd = 'mv sdae_mult%d %s%s%s%d' % (ele,todir,reshead,head,ele)
    system(cmd)
