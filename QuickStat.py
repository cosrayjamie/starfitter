# -*- coding: utf-8 -*-
"""
Created on Thu Mar 21 18:33:06 2019

@author: Josh Peterson
"""

import numpy as np

MirNumb = [28,11,11,8]


#read in the geometry file
MirGeo = []

file = open("mirror_geometry.nparm.stats", "r")

cntr = 0
crap = file.readlines()
for line in crap:
    GeoInfo = line.split()
    MirGeo.append([])
    for i in range(0,len(GeoInfo)):
        MirGeo[cntr].append(GeoInfo[i])
    cntr += 1
file.close()

#We will find an average and error with the standard method and the inverse variance method.

#initialize all of the variables we will use.
ntheta = 0.0
nphi = 0.0
npsi = 0.0
nD0 = 0.0

nthetaD = 0.0
nphiD = 0.0
npsiD = 0.0
nD0D = 0.0

thetan = 0.0
phin = 0.0
psin = 0.0
D0n = 0.0

itheta = 0.0
iphi = 0.0
ipsi = 0.0
iD0 = 0.0

ithetan = 0.0
iphin = 0.0
ipsin = 0.0
iD0n = 0.0

flagtheta = 0
flagphi = 0
flagpsi = 0
flagD0 = 0

flag = 0

file = open("TAx4_Mirror_Geometry.txt","w+")

#look for data from the same telescope
for i in range(4):
    for j in range(MirNumb[i]+1):
        for k in range(len(MirGeo)):
                if (i == int(MirGeo[k][0]) and j == int(MirGeo[k][1])):
                    flag = 1
                    #azimuth
                    if(float(MirGeo[k][3]) != 0.0):
                        flagtheta = 1
                        ntheta += float(MirGeo[k][2])
                        nthetaD += (float(MirGeo[k][3]))**2
                        itheta += float(MirGeo[k][2]) / (float(MirGeo[k][3]))**2
                        ithetan += 1.0 / (float(MirGeo[k][3]))**2
                        thetan += 1
                    #elevation
                    if(float(MirGeo[k][5]) != 0.0):
                        flagphi = 1
                        nphi += float(MirGeo[k][4])
                        nphiD += (float(MirGeo[k][5]))**2
                        iphi += float(MirGeo[k][4]) / (float(MirGeo[k][5]))**2
                        iphin += 1.0 / (float(MirGeo[k][5]))**2
                        phin += 1
                    #psi
                    if(float(MirGeo[k][7]) != 0.0):
                        flagpsi = 1
                        npsi += float(MirGeo[k][6])
                        npsiD += (float(MirGeo[k][7]))**2
                        ipsi += float(MirGeo[k][6]) / (float(MirGeo[k][7]))**2
                        ipsin += 1.0 / (float(MirGeo[k][7]))**2
                        psin += 1
                    #distance
                    if(float(MirGeo[k][11]) != 0.0):
                        flagD0 = 1
                        nD0 += float(MirGeo[k][10])
                        nD0D += (float(MirGeo[k][11]))**2
                        iD0 += float(MirGeo[k][10]) / (float(MirGeo[k][11]))**2
                        iD0n += 1.0 / (float(MirGeo[k][11]))**2
                        D0n += 1
        

        if(flag == 1):
            
            #computing the inverse variance mean and error with quaduature
            print("Combined geometry of mirror", i, j)
            file.write("Geometry of mirror %d, %d\n" % (i,j))
            
            if(flagtheta == 1):
                itheta /= ithetan
                nthetaD = np.sqrt(nthetaD)
                print("theta =", itheta, "+/-", nthetaD)
                file.write("theta = %.3f +/- %.3f\n" % (itheta, nthetaD))
                
            if(flagphi == 1):
                iphi /= iphin
                nphiD = np.sqrt(nphiD)
                print("phi =", iphi, "+/-", nphiD)
                file.write("phi = %.3f +/- %.3f\n" % (iphi, nphiD))
            
            if(flagpsi == 1):
                ipsi /= ipsin
                nphiD = np.sqrt(nphiD)
                print("psi =", ipsi, "+/-", npsiD)
                file.write("psi = %.3f +/- %.3f\n" % (ipsi, npsiD))
                
            if(flagD0 == 1):
                iD0 /= iD0n
                nD0D = np.sqrt(nD0D)
                print("D0 =", iD0, "+/-", nD0D)
                file.write("D0 = %.3f +/- %.3f\n" % (iD0, nD0D))

            print("")
            file.write("\n")
            
            """
            #computing the standard mean and error with quaduature
            print("Geometry with equal weighting of mirror", i, j)
            file.write("Geometry with equal weighting of mirror %d, %d\n" % (i,j))
            
            if(flagtheta == 1):
                ntheta /= thetan
                nthetaD = np.sqrt(nthetaD)
                print("theta =", ntheta, "+/-", nthetaD)
                file.write("theta = %.3f +/- %.3f\n" % (ntheta, nthetaD))
                
            if(flagphi == 1):
                nphi /= phin
                nphiD = np.sqrt(nphiD)
                print("phi =", nphi, "+/-", nphiD)
                file.write("phi = %.3f +/- %.3f\n" % (nphi, nphiD))
            
            if(flagpsi == 1):
                npsi /= psin
                nphiD = np.sqrt(nphiD)
                print("psi =", npsi, "+/-", npsiD)
                file.write("psi = %.3f +/- %.3f\n" % (npsi, npsiD))
                
            if(flagD0 == 1):
                nD0 /= D0n
                nD0D = np.sqrt(nD0D)
                print("D0 =", nD0, "+/-", nD0D)
                file.write("D0 = %.3f +/- %.3f\n" % (nD0, nD0D))

            print("")
            file.write("\n")
        
            #computing the mean and error with inverse variance
            print("Geometry with inverse variance weighting of mirror", i, j)
            file.write("Geometry with inverse variance weighting of mirror %d, %d\n" % (i,j))
            
            if(flagtheta == 1):
                itheta /= ithetan
                ithetan = 1.0/np.sqrt(ithetan)
                print("theta =", itheta, "+/-", ithetan)
                file.write("theta = %.3f +/- %.3f\n" % (itheta, ithetan))
                
            if(flagphi == 1):    
                iphi /= iphin
                iphin = 1.0/np.sqrt(iphin)
                print("phi =", iphi, "+/-", iphin)
                file.write("phi = %.3f +/- %.3f\n" % (iphi, iphin))
                
            if(flagpsi == 1):
                ipsi /= ipsin
                ipsin = 1.0/np.sqrt(ipsin)
                print("psi =", ipsi, "+/-", ipsin)
                file.write("psi = %.3f +/- %.3f\n" % (ipsi, ipsin))
                
            if(flagD0 == 1):
                iD0 /= iD0n
                iD0n = 1.0/np.sqrt(iD0n)
                print("D0 =", iD0, "+/-", iD0n)
                file.write("D0 = %.3f +/- %.3f\n" % (iD0, iD0n))
            
            print("")
            file.write("\n")
            """
            #now we need to reset all of the variables
            ntheta = 0.0
            nphi = 0.0
            npsi = 0.0
            nD0 = 0.0
            
            nthetaD = 0.0
            nphiD = 0.0
            npsiD = 0.0
            nD0D = 0.0
            
            thetan = 0.0
            phin = 0.0
            psin = 0.0
            D0n = 0.0   
            
            itheta = 0.0
            iphi = 0.0
            ipsi = 0.0
            iD0 = 0.0
            
            ithetan = 0.0
            iphin = 0.0
            ipsin = 0.0
            iD0n = 0.0
            
            flagtheta = 0
            flagphi = 0
            flagpsi = 0
            flagD0 = 0

            flag = 0
        
file.close()
