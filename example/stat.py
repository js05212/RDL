#!/usr/bin/python

f_in = 'citations.dat'
stamax = 0
file_in = open(f_in)
for line in file_in:
    cur = int(line.strip().split(' ')[0])
    if cur>stamax:
        stamax = cur
print stamax

