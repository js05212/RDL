#!/usr/bin/python
from threading import Thread
from os import system
import thread
from datetime import datetime
import re
def worker(pline,tempfile,tmpsh):
    pline = pline.replace('\n','')
    segs = re.split(',',pline)
    nump = len(segs)    # num of variable/parameter
    ft = open(tempfile)
    tmpsh = tmpsh+segs[2]
    fw = open(tmpsh,'w')
    tlines = ft.readlines()
    for i,tline in enumerate(tlines):
        if i >= 1 and i<=nump:
            secs = re.split('=',tline)
            tstr = '%s=\'%s\'\n' % (secs[0],segs[i-1])
            fw.write(tstr)
        else:
            fw.write(tline)
    fw.close()
    system('sh '+tmpsh)

def multipro(tempfile,parafile,tmpsh):
    tlist = list()
    b = datetime.now()
    fp = open(parafile)
    plines = fp.readlines()
    numt = len(plines)
    for i in range(numt):
        t = Thread(target = worker,args = (plines[i],tempfile,tmpsh))
        tlist.append(t)
    for i in range(numt):
        tlist[i].start()
    for i in range(numt):
        tlist[i].join()
    e = datetime.now()
    print 'total time: ',str((e-b).seconds)

if __name__=='__main__':
    multipro('template.sh.a','para1112a','tmpsh')

