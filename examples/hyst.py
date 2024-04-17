import csv
import numpy as np
import os
import matplotlib.pyplot as plt

def evaluate(Sh, data, wp):

    with open(path+'/sat.csv', 'w', newline='') as file:
        for S in Sh:        
            for s in S:        
                file.write(str(s)+"\n")

    pathCall = "~/workspace/opm/opm-common/build/bin/hysteresis " + data +".DATA sat.csv relperms.csv " + wp + " 0"
    os.system(pathCall)

    krw = []
    krnw = []
    krM = []
    sT = []

    with open(path+'/relperms.csv', newline='') as csvfile:
        reader = csv.reader(csvfile, delimiter=',')
        for row in reader:
            #S2.append(float(row[0]))
            krnw.append(float(row[2]))
            krw.append(float(row[1]))
            krM.append(float(row[3]))
            sT.append(float(row[4]))

    i = 0
    start = 0
    end = 0
    for S in Sh:
        end = len(S) + start
        input = "D" + str(i)
        if S[0] > S[-1]:
            input = "I" + str(i)
            i = i + 1
        
        #plt.plot(S, krnw[start:end], label = "KRNW"+input)
        plt.plot(S, krw[start:end],label = "KRW"+input)
        start = end

def evaluate2(Sh, data, wp):

    #with open(path+'/sat.csv', 'w', newline='') as file:
    #    for S in Sh:        
    #        for s in S:        
    #            file.write(str(s)+"\n")
    smax = .0
    krnw = []

    for S in Sh:        
       for s in S:
            smax = max(s,smax)
            pathCall = "~/workspace/opm/opm-common/build/bin/hysteresis2 " + data +".DATA " + str(s) + " " + str(smax) + " " + wp + " 0 > output.txt"
            x = os.system(pathCall)
            if os.path.exists('output.txt'):
                fp = open('output.txt', "r")
                output = fp.read()
                fp.close()
                os.remove('output.txt')
                krnw.append(float(output))

    print(krnw)
    i = 0
    start = 0
    end = 0
    for S in Sh:
        end = len(S) + start
        input = "D" + str(i)
        if S[0] > S[-1]:
            input = "I" + str(i)
            i = i + 1
        
        plt.plot(S, krnw[start:end], label = "KRNW"+input)
        start = end

swl = 0.05
path = os.getcwd()
S = np.linspace(0.0, 1.0-swl, 10)
#S2 = 1.0 - S - swl; 
#evaluate( [S, S2], "")
#evaluate([S], "1D_3PHASE_KILLOUGH_BOTH_MOD", "GW")

S1 = np.linspace(0.0, 0.5, 100)
S2 = np.linspace(0.5, 0.15, 100)
evaluate([S1,S2], "1D_3PHASE_KILLOUGH_BOTH", "GW")
evaluate2([S1,S2], "1D_3PHASE_KILLOUGH_BOTH", "GW")

#S1 = np.linspace(0.0, 0.5, 100)
#S2 = np.linspace(0.5, 0.15, 100)
#S3 = np.linspace(0.15, 0.7, 50)
#S4 = np.linspace(0.7, 0.12, 50)
#evaluate([S1,S2,S3,S4], "1D_3PHASE_KILLOUGH_BOTH", "WO")

plt.legend()
#plt.savefig("CARLSON.png")
plt.savefig("Killough.png")
plt.show()


