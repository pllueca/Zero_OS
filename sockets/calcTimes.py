#! usr/bin/python
import re
import os

dirBase = '/home/brinon/Zero_OS/sockets/'

def num_files(dire):
    return len([name for name in os.listdir(dirBase + dire)])

def suma_Exec(nom):
    s = 0
    numClients = num_files(nom) - 1 # -1 per el arxiu 'launch_info'
    for i in range(0, numClients-1):
        f = open(nom +"/client_" + str(i), "rw")
        l = f.readline()
        m = re.search('Time\ ',l)
        s = s +  int(l[m.end():])
    avg = s / numClients
    return s

print dirBase + 'p1'
print num_files('p1')
print suma_Exec('p1')


