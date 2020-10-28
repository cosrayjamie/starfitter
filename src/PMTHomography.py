# -*- coding: utf-8 -*-
"""
Created on Tue Oct 30 12:47:01 2018

@author: Joshua Peterson
"""

import numpy as np
#read in and store measured PMT coordinates

PMTLocations = []
#need to have file given as a line command
file = open("TranPMT.txt","r")

cntr = 0
crap = file.readlines()
for line in crap:
    PMTInfo = line.split()
    PMTLocations.append([])
    for i in range(0,len(PMTInfo)):
        PMTLocations[cntr].append(PMTInfo[i])
    cntr += 1
    
file.close()
#print("PMT measured locations:")
#print(PMTLocations)
                

#read in and store ideal PMT coordinates

PMTIdeals = []

file = open("IdealPMT.txt","r")

cntr = 0
crap = file.readlines()
for line in crap:
    PMTInfo = line.split()
    PMTIdeals.append([])
    for i in range(0,len(PMTInfo)):
        PMTIdeals[cntr].append(PMTInfo[i])
    cntr += 1
    
file.close()
#print("PMT ideal locations:")
#print(PMTIdeals)

#create a matrix of zeroes that we will use to create the A matrix
AMat = np.zeros((2*len(PMTLocations),9))

for k in range(0,len(PMTLocations)):
    for l in range(0,len(PMTIdeals)):
        if(int(PMTLocations[k][0])==int(PMTIdeals[l][0])):
                    AMat[2*k][3] = -float(PMTLocations[k][1])
                    AMat[2*k][4] = -float(PMTLocations[k][2])
                    AMat[2*k][5] = -1.0
                    AMat[2*k][6] = float(PMTIdeals[l][2])*float(PMTLocations[k][1])
                    AMat[2*k][7] = float(PMTIdeals[l][2])*float(PMTLocations[k][2])
                    AMat[2*k][8] = float(PMTIdeals[l][2])
                    AMat[2*k + 1][0] = float(PMTLocations[k][1])
                    AMat[2*k + 1][1] = float(PMTLocations[k][2])
                    AMat[2*k + 1][2] = 1.0
                    AMat[2*k + 1][6] = -float(PMTIdeals[l][1])*float(PMTLocations[k][1])
                    AMat[2*k + 1][7] = -float(PMTIdeals[l][1])*float(PMTLocations[k][2])
                    AMat[2*k + 1][8] = -float(PMTIdeals[l][1])
    
#print("The A matrix is:")                
#print(AMat)

#implement singular value decomposition
u,s,vh = np.linalg.svd(AMat, full_matrices=False)
#print("vh is:")
#print(vh)
#solution is last column of v which is the last row of vh
h = vh[len(vh)-1]
#print("h is")
for i in range(len(h)):
    print h[i],
#h[0], h[1], h[2], h[3], h[4], h[5], h[6], h[7], h[8]
#this would be the end of the program. It would output the entries of h.
"""
H = np.zeros((3,3))

for i in range(0,3):
    for j in range(0,3):
        H[i][j]=h[j + 3*i]
        
print("H is")
print(H)

p1 = np.dot(H,[7.543,-5.244,1.0])
print(p1/p1[2]) #should be 2.0,2.0
p2 = np.dot(H,[6.026,-4.236,1.0])
print(p2/p2[2]) #should be 0.0,4.0
p3 = np.dot(H,[9.881,-3.532,1.0])
print(p3/p3[2]) #should be 4.0,3.0
"""
