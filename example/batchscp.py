#!/usr/bin/python
from os import system
fromlist = range(914611,914615)
head = ''
tono = ''
todir = 'hwangaz@lcpu3.cse.ust.hk:/csproject/dygroup2/wanghao/rctr-wuvel%s/' % tono
reshead = 'result-k200-fixed-t000a-p'
for i,ele in enumerate(fromlist):
    cmd = 'scp -r sdae_mult%d/. %s%s%s%d' % (ele,todir,reshead,head,ele)
    system(cmd)
