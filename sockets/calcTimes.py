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


# a l'arxiu f nomes hi ha linies 'Time 123123'
# retorna la mitjana del temps
def suma_temps(nom):
    s = 0
    n = 0
    f = op



def calc(k,i,j):
    s = "prova-"+str(k)+"-"+str(i)+"-"+str(j)
    f = open(s+"/resumTimes.txt","rw")
    x = suma_Exec(s)
    f.write('Average Time: '+str(x)+'\n')


def main():
    for i in [1, 10, 50, 100, 200, 250]:
        for j in [1, 50, 200, 300, 500, 750, 1000, 2000]:
            for k in [1,2,3]:
                calc(k,i,j)
    print "End!\n"

if __name__ == "__main__":
    main()
