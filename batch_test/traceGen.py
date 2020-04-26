import sys
import numpy as np
import random
import os

HEXLETTERS = ['1','2','3','4','5','6','7','8','9','a','b','c','d','e','f']

if __name__ == "__main__":
    # print(f"Arguments count: {len(sys.argv)}")
    # for i, arg in enumerate(sys.argv):
    #     print(f"Argument {i:>6}: {arg}")
    if (len(sys.argv) != 5 and len(sys.argv) != 4):
        print("""Please have command line arguments in the following format
             [Executable] [Lambda (expected timing interval)] [Transac. Count] [Write ratio] xx[Addr Range]xx [dirName (optional)]
             == Assuming no cache thus no spatial locality applies
             == Assuming request size irrelevant, small amount of extra clock cycles not going to make significant difference
          """)
        exit(-1)

    CMDARGLIST = []
    for i, arg in enumerate(sys.argv):
        CMDARGLIST.append(arg)
    
    EXECUTABLE = CMDARGLIST[0]
    LAMBDA = float(CMDARGLIST[1])
    TRANCNT = int(CMDARGLIST[2])
    WRRATIO = float(CMDARGLIST[3])
    FILEPATH = CMDARGLIST[4]

    outfile_filename = ""
    if (len(sys.argv) == 5):
        outfile_filename = FILEPATH + "/" +  "k6_TraceGen_"+str(LAMBDA)+"_"+str(TRANCNT)+"_"+str(int(WRRATIO*100))+".trc"
    else :
        outfile_filename = "k6_TraceGen_"+str(LAMBDA)+"_"+str(TRANCNT)+"_"+str(int(WRRATIO*100))+".trc"
    outfile = open(outfile_filename,"w") 

    counter = 1
    time = 1

    while (counter != TRANCNT):
        # address
        address = "0x0"
        for i in range(0,7):
            address += random.choice(HEXLETTERS)

        # read/write requests
        write_flag = None
        readwrite_rand = np.random.random_sample()
        if (readwrite_rand > WRRATIO):
            write_flag = False
        else:
            write_flag = True

        # intervals
        interval = np.random.poisson(LAMBDA, None)
        time+=interval

        # cmd
        cmd = address.upper() + " "
        if write_flag:
            cmd += "P_MEM_WR "
        else:
            cmd += "P_MEM_RD "
        cmd += str(time)

        # print(cmd)
        outfile.write(cmd+"\n")

        counter+=1
    outfile.close()