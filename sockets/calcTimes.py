#! usr/bin/python
import re
import os
import sys



def num_files(dire):
    return len([name for name in os.listdir(dire)])

def suma_Exec(nom):
    s = 0
    clients_cor = 0
    numClients = num_files(nom) - 1 # -1 per el arxiu 'launch_info'
    for i in range(0, numClients-1):
        f = open(nom +"/client_" + str(i), "rw")
        l = f.readline()
        m = re.search('Time\ ',l)
        f.close()
        if not m is None:
            s = s +  int(l[m.end():])
            clients_cor += 1
    avg = s / clients_cor
    print avg


def main():
    s = sys.argv[1] #folder name (i-j-k) ../sockets/random/prova-i-j-k
    suma_Exec(s)

if __name__ == "__main__":
    main()
