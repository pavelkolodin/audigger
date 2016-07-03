#!/usr/bin/python


import os
import sys
import math





if __name__ == '__main__':

    if len(sys.argv) < 2:
        print ( "<program> <steps>" )
        sys.exit(1)


    points = int(sys.argv[1])
    curr = 0
    step = (math.pi/2) / points

    outlist = []
    for val in range(points):
        outlist.append( math.cos( curr ) )
        #outlist.append( math.exp( -curr ) )
        curr = curr + step


    print ("static const float curv_size = " + str(len(outlist) - 1) +";")

    print ("static const float curv[] = {")
    for i in range(1, len(outlist) - 1):
        print ( "\t" + str(outlist[i]) + "," )

    print ( "\t" + str(outlist[ len(outlist)-1 ]) + "};" )
   
    print ("static const float curv_mirr[] = {")
    for i in range(len(outlist) - 1, 1, -1):
        print ( "\t" + str(outlist[i]) + "," )

    print ( "\t" + str(outlist[ 1 ]) + "};" )


