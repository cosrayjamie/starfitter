# -*- coding: utf-8 -*-
"""
Created on Wed May 29 12:58:34 2019

@author: opyob
"""

import numpy as np

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

from subprocess import Popen, PIPE
import shlex

from skimage import io
from skimage.filters.rank import equalize, minimum, mean_bilateral, noise_filter, mean, maximum
from skimage.measure import label, regionprops
from skimage.morphology import closing, disk
from skimage import exposure
from skimage.draw import circle
from skimage.color import label2rgb

import math, random


# Data conventions: A point is a pair of floats (x, y). A circle is a triple of floats (center x, center y, radius).

# Returns the smallest circle that encloses all the given points. Runs in expected O(n) time, randomized.
# Input: A sequence of pairs of floats or ints, e.g. [(0,5), (3.1,-2.7)].
# Output: A triple of floats representing a circle.
# Note: If 0 points are given, None is returned. If 1 point is given, a circle of radius 0 is returned.
# 
# Initially: No boundary points known
def make_circle(points):
	# Convert to float and randomize order
	shuffled = [(float(x), float(y)) for (x, y) in points]
	random.shuffle(shuffled)
	
	# Progressively add points to circle or recompute circle
	c = None
	for (i, p) in enumerate(shuffled):
		if c is None or not is_in_circle(c, p):
			c = _make_circle_one_point(shuffled[ : i + 1], p)
	return c


# One boundary point known
def _make_circle_one_point(points, p):
	c = (p[0], p[1], 0.0)
	for (i, q) in enumerate(points):
		if not is_in_circle(c, q):
			if c[2] == 0.0:
				c = make_diameter(p, q)
			else:
				c = _make_circle_two_points(points[ : i + 1], p, q)
	return c


# Two boundary points known
def _make_circle_two_points(points, p, q):
	circ = make_diameter(p, q)
	left  = None
	right = None
	px, py = p
	qx, qy = q
	
	# For each point not in the two-point circle
	for r in points:
		if is_in_circle(circ, r):
			continue
		
		# Form a circumcircle and classify it on left or right side
		cross = _cross_product(px, py, qx, qy, r[0], r[1])
		c = make_circumcircle(p, q, r)
		if c is None:
			continue
		elif cross > 0.0 and (left is None or _cross_product(px, py, qx, qy, c[0], c[1]) > _cross_product(px, py, qx, qy, left[0], left[1])):
			left = c
		elif cross < 0.0 and (right is None or _cross_product(px, py, qx, qy, c[0], c[1]) < _cross_product(px, py, qx, qy, right[0], right[1])):
			right = c
	
	# Select which circle to return
	if left is None and right is None:
		return circ
	elif left is None:
		return right
	elif right is None:
		return left
	else:
		return left if (left[2] <= right[2]) else right


def make_diameter(a, b):
	cx = (a[0] + b[0]) / 2.0
	cy = (a[1] + b[1]) / 2.0
	r0 = math.hypot(cx - a[0], cy - a[1])
	r1 = math.hypot(cx - b[0], cy - b[1])
	return (cx, cy, max(r0, r1))


def make_circumcircle(a, b, c):
	# Mathematical algorithm from Wikipedia: Circumscribed circle
	ox = (min(a[0], b[0], c[0]) + max(a[0], b[0], c[0])) / 2.0
	oy = (min(a[1], b[1], c[1]) + max(a[1], b[1], c[1])) / 2.0
	ax = a[0] - ox;  ay = a[1] - oy
	bx = b[0] - ox;  by = b[1] - oy
	cx = c[0] - ox;  cy = c[1] - oy
	d = (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by)) * 2.0
	if d == 0.0:
		return None
	x = ox + ((ax*ax + ay*ay) * (by - cy) + (bx*bx + by*by) * (cy - ay) + (cx*cx + cy*cy) * (ay - by)) / d
	y = oy + ((ax*ax + ay*ay) * (cx - bx) + (bx*bx + by*by) * (ax - cx) + (cx*cx + cy*cy) * (bx - ax)) / d
	ra = math.hypot(x - a[0], y - a[1])
	rb = math.hypot(x - b[0], y - b[1])
	rc = math.hypot(x - c[0], y - c[1])
	return (x, y, max(ra, rb, rc))


_MULTIPLICATIVE_EPSILON = 1 + 1e-14

def is_in_circle(c, p):
	return c is not None and math.hypot(p[0] - c[0], p[1] - c[1]) <= c[2] * _MULTIPLICATIVE_EPSILON


# Returns twice the signed area of the triangle defined by (x0, y0), (x1, y1), (x2, y2).
def _cross_product(x0, y0, x1, y1, x2, y2):
	return (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0)


#this function sorts four corners into upper left, upper right, lower right,
# and lower left
def FindCorn(corners):
        
    temp = 0.0
    ind = 1000  
    
    for i in range(len(corners)):
        mag = np.sqrt(corners[i][0]**2 + corners[i][1]**2)
        if mag > temp :
            temp = mag
            ind = i
    LR = corners[ind]
    LRi = ind
            
    temp = 10.0**6
    ind = 1000
            
    for i in range(len(corners)):
        mag = np.sqrt(corners[i][0]**2 + corners[i][1]**2)
        if mag < temp :
            temp = mag
            ind = i
    UL = corners[ind]
    ULi = ind
                    
    for i in range(len(corners)):
        if i==ULi or i==LRi:
            continue
        if np.abs(corners[i][0] - LR[0]) < np.abs(corners[i][0] - UL[0]):
            UR = corners[i]
        else:
            LL = corners[i]
            
    return UL, UR, LR, LL

#this function makes a mask with a circle
def DrawCircle(centrow,centcol,radius,x,y):
    
    rr, cc = circle(centrow, centcol, radius)     
    circled = np.zeros((y,x))
    circled[rr, cc] = 1
    
    return circled

#this fuction thresholds the image in such a way that stars can be identified
def ThreshPhoto(gimage,mask):
    #Run the image through many image filters until we can threshold the image
    
    print("Thresholding Photo")
    print("")
    
    y,x = gimage.shape

    smooth = equalize(gimage, disk(250), mask=np.logical_not(mask))
    #smooth = 1.0*smooth
    #smooth = exposure.rescale_intensity(smooth)
    #io.imshow(smooth)

    mini = minimum(smooth, disk(1), mask=np.logical_not(mask))
    #mini = 1.0*mini
    #mini = exposure.rescale_intensity(mini)
    #io.imshow(mini)

    meany = mean(mini, disk(10), mask=np.logical_not(mask))
    meany = 1.0*meany
    meany = exposure.rescale_intensity(meany)
    #io.imshow(meany)

    maxo = maximum(meany, disk(2), mask=np.logical_not(mask))
    maxo = 1.0*maxo
    maxo = exposure.rescale_intensity(maxo)
    #io.imshow(maxo)

    #threshold the processed image based on a good intensity cut
    #use 0.52 for the MD telescopes
    checkerino = np.full((y,x),0.425)
    mask2 = maxo > checkerino
    #maxo[mask2] = 0.0
    io.imshow(mask2)

    #denoise = noise_filter(meany, disk(100), mask=np.logical_not(mask))
    #denoise = 1.0*denoise
    #denoise = exposure.rescale_intensity(denoise)
    #io.imshow(denoise)
    
    #cyborg = enhance_contrast(smooth,disk(5),mask=np.logical_not(mask))
    #cyborg = 1.0*cyborg
    #cyborg = exposure.rescale_intensity(cyborg)
    
    #io.imshow(cyborg)
    #this was to cut out the top part of the screen that was bright
    #anothermask = (row > 1000)
    #tempmask = np.logical_and(mask2, anothermask)
    #io.imshow(tempmask)
    
    cool = closing(mask2, disk(25))
    #io.imshow(cool)
    
    print("Thresholding Completed")
    print("")
    
    return cool

#this function takes a lights on, doors open photo and finds the screen in the
#photo
def FindCover(gimage):
    
    y,x = gimage.shape

    #First we find a typical range of intensities for the screen
    Sample = DrawCircle(y/2.0, x/2.0, 500.0, x, y)

    #io.imshow(Sample)

    label_sample = label(Sample)

    for region in regionprops(label_sample, intensity_image = gimage):
        minwhite = region.min_intensity
        maxwhite = region.max_intensity

    print("screen minimum:",minwhite)
    print("screen maximum:",maxwhite)
    print("")

    meanmatlower = np.full((y,x),minwhite)
    meanmathigher = np.full((y,x),maxwhite)
    
    #Now we create a mask that only includes pixels with the typical screen range
    screen = gimage >= (meanmatlower - 0.1)
    #screen2 = gimage <= meanmathigher
    #screen = np.logical_and(screen1, screen2)
    
    copy = np.copy(gimage)
    
    copy[screen] = 1.0
    copy[np.logical_not(screen)] = 0.0
    
    #io.imshow(copy)
    
    print("Cover Photo Thresholded")
    print("")
    
    #Now we run the program through image filters to remove everything except
    #the screen
    counter = 0
    disksize = 20
    
    while (counter != 1):
    
        counter = 0
        print("Attempting to Remove Artifacts")
        print("")
        mini = minimum(copy, disk(disksize))
    
        mask = mini > 0
        
        labelmask = label(mask)
        
        for region in regionprops(labelmask):
            counter += 1
            
        print(counter-1,"Artifacts Found")
        print("")
        disksize += 5
    
    print("Background Artifacts Removed")
    print("")
      
    print("Closing Holes")
    print("")
    mask = closing(mask, disk(50))
    
    print("Fixing Loss of Screen Size")
    print("")
    mask = maximum(mask, disk(disksize-10))
    
    #io.imshow(mask)
    
    return mask

#this takes a number of points and computes the moment of inertia about
#an axis in the z direction passing through the centroid, assuming each
#pixel has a mass of 1

def MOI(points, cx, cy):
    
    center = []
    for i in range(len(points)):
        center.append((cx,cy))
        
    scooby = np.subtract(points,center)
        
    I = np.trace(np.dot(scooby,np.transpose(scooby)))
    
    return I

#main program

#here we need to run the find command and iterate over every image
"""
command1 = 'find ./Telescope_Mirror_Survey/TAx4 LIGHTON.jpg'
args1 = shlex.split(command1)

pipe1 = Popen(args1, shell=True, stdout=PIPE)

for line in pipe1.stdout:
    
    limage = io.imread(line)
    
    photopath = line.split("/")
    
    path = photopath[0]
    for i in range(1,len(photopath)-1)
        path = path+'/'+photopath[i]
        
    glimage = np.mean(limage, axis=2)

    glimage = exposure.rescale_intensity(glimage, out_range = (0.0,1.0))

    print("Converted Screen Image to Greyscale")
    print("")

    #io.imshow(glimage)

    #Here we find the screen using the image with the lights on and the door open.
    #Everything but the screen in taken out of the photo

    mask = FindCover(glimage)
    #io.imshow(mask)

"""
limage = io.imread('LIGHTSON32.jpg')
glimage = np.mean(limage, axis=2)
glimage = exposure.rescale_intensity(glimage, out_range = (0.0,1.0))
mask = FindCover(glimage)
mask = np.logical_not(mask)
#io.imshow(mask)
#read in the image we are going to process

image = io.imread('IMG_0029.jpg')

y,x,other = image.shape

#io.imshow(image)
#here we convert to greyscale with equal weighting.  Most greyscale conversions
#weight with respect to luminance, which is a measure dependent on human
#perception of brightness, not intensity.

gimage = np.mean(image, axis=2)

gimage = exposure.rescale_intensity(gimage, out_range = (0.0,1.0))

print("Converted to Greyscale")
print("")

#io.imshow(gimage)

#Here we find the screen using the image with the lights on and the door open.
#Everything but the screen in taken out of the photo
"""
#change gimage in FindCover to the lights on image
mask3 = FindCover(gimage)
#io.imshow(mask3)
#gimage[np.logical_not(FindCover(gimage))] = 0.0

image_overlay = label2rgb(mask3, image=gimage)

io.imshow(image_overlay)
"""

data = []
daeta = []
eigenval1 = []
eigenval2 = []

#here we define a mask that will make it so we won't measure anything that isn't
#on the screen
"""
#finding the corners
corners = [[967.000, 394.333],[3052.125,398.000],[3048.250,2261.333],[961.625,2248.750]] #image 56
#corners = [[937.333, 428.167],[3034.500,393.833],[3066.000,2271.000],[966.333,2295.000]] #image 5
#corners = [[956.667, 391.000],[3054.833,379.667],[3074.500,2256.167],[963.375,2264.125]] #image 37

UL, UR, LR, LL = FindCorn(corners)

#iterate over all of the star photos
#command2 = 'find'+ path + 'IMG_????.jpg' + 'everything else'

args2 = shlex.split(command2)

pipe2 = Popen(args2, shell=True, stdout=PIPE)

for line in pipe2.stdout:
    
    image = io.imread(line)
    
    cronk = line.split(".")
    
    pathnojpg = cronk[0]

    gimage = np.mean(image, axis=2)

    gimage = exposure.rescale_intensity(gimage, out_range = (0.0,1.0))

    print("Converted to Greyscale")
    print("")
 
y,x,other = image.shape
    
row, col = np.ogrid[:y, :x]

#above the upper edge
condition1 = (((UL[0]-UR[0])*(row-5.0) - (UL[1]-UR[1])*col) > (UR[1]*UL[0]-UL[1]*UR[0]))
#below the lower edge
condition2 = ((-1.0*(LR[0]-LL[0])*(row+5.0) + (LR[1]-LL[1])*col) < (-1.0*LL[1]*LR[0]+LR[1]*LL[0]))
#to the left of the left edge
condition3 = ((col-5.0)*(UL[1]-LL[1]) > (row*(UL[0]-LL[0]) + UL[1]*LL[0] - LL[1]*UL[0]))
#to the right of the right edge
condition4 = ((col+5.0)*(UR[1]-LR[1]) < (row*(UR[0]-LR[0]) + UR[1]*LR[0] - LR[1]*UR[0]))
    
condition12 = np.logical_or(condition1, condition2)
condition34 = np.logical_or(condition3, condition4)

mask = np.logical_or(condition12, condition34)
"""

gimage[mask] = 0.0

print("Cut Out Background")
print("")

#io.imshow(gimage)

#Run the image through many image filters until we can threshold the image

cool = ThreshPhoto(gimage,mask)
    
#io.imshow(cool)
    
#identify the regions left in the image
#image_overlay = label2rgb(label_image, image=image)
#io.imshow(image_overlay)
#Here we create the mask we will use to measure the greyscaled image
#The mask puts a bounding circle around every star
    
#Majora = np.full((y,x),False)
    
file = open('doodydata.txt','w+')

for region in regionprops(label_image):
        
    data.append(region.area)
    daeta.append(region.bbox_area)
    cy,cx = region.centroid
    ecc = region.eccentricity
    eulernumber = region.euler_number
    extent = region.extent
    e1, e2 = region.inertia_tensor_eigvals 
    eigenval1.append(e1)
    eigenval2.append(e2)
    #I = MOI(region.coords, cx, cy)
    I=3
    
    """
    and (40000000 < I < 10000000000)   
    """
    if (8000 < region.area < 20000) and (0.250 < region.eccentricity < 0.750) and (0.250 < region.extent < 0.750) and (1100 < e1 < 2300) and (600 < e2 < 1900):
        file.write("%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\tstar\n" % (cy,cx, region.area, ecc, extent, e1, e2, I))
    else:
        file.write("%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\n" % (cy,cx, region.area, ecc, extent, e1, e2, I))

"""
        #check if a region is a star or not with a typical star range
        if 9000 < region.area < 20000:
            print("Found a star!")
            area = region.area
            print("Star Area:", area)
            #here we are going to find an approximate least bounding circle
            #(smallest circle containing all of the region)
            
            points = region.coords
            
            minrow, mincol, maxrow, maxcol = region.bbox
            
            #find the center of the box
            centrow = (minrow + maxrow)/2.0
            centcol = (mincol + maxcol)/2.0
            
            #here we find a circle that is guarunteed to contain the whole star
            radius2 = np.sqrt((maxrow - centrow)**2 + (maxcol - centcol)**2)
            
            outercircle = DrawCircle(centrow, centcol, radius2, x, y)
            
            #we use the bounding circle we found to isolate the star we are considering
            isostar = np.logical_and(outercircle, cool)
            
            #we record every pixel coordinate for the pixels in the star
            starpoints = np.where(isostar)
            
            points = []
            
            for i in range(len(starpoints[0])):
                points.append((starpoints[1][i],starpoints[0][i]))
                
            #Now we find the least bounding circle for the star we are looking at
            ccol, crow, rad = make_circle(points)
            print("Star Center Non-weighted: (",ccol,",",crow,")")
            print("")
            bcircle = DrawCircle(crow, ccol, rad, x, y)
                
            Majora = np.logical_or(Majora, bcircle)
        
    #identify the stars we found
    labellyboy = label(Majora)
      
    #open the star measurement file
    extend = '.smb.csv'
    starfile = pathnojpg+extend
    file = open(starfile,"w+")
    file.write("X\tY\tlabel\n")
            
    for region in regionprops(labellyboy, intensity_image = gimage):

        #measure the centroid of the star
        ulty, ultx = region.weighted_centroid
        print("Star Center: (", ultx, ",", ulty, ")")
        #write the result to file
        file.write("%.3f\t%.3f\tstar\n" % (ultx, ulty))
   
    file.write("%.3f\t%.3f\tcorner\n" % (UL[0], UL[1]))
    file.write("%.3f\t%.3f\tcorner\n" % (UR[0], UR[1]))
    file.write("%.3f\t%.3f\tcorner\n" % (LR[0], LR[1]))
    file.write("%.3f\t%.3f\tcorner\n" % (LL[0], LL[1]))
    file.close()

    #image_overlay = label2rgb(Majora, image=gimage)

    #io.imshow(image_overlay)
"""

file.close()

"""
plt.figure(1)
plt.hist(x=data, bins='auto')
plt.grid(axis='x', alpha=0.75)
plt.xlabel('Blob Area (pixels)')
plt.title('Blob Area Distribution')

plt.figure(2)
plt.hist(x=daeta, bins='auto')
plt.grid(axis='x', alpha=0.75)
plt.xlabel('Blob Bounding Box Area (pixels)')
plt.title('Blob Bounding Box Area Distribution')

plt.figure(3)
plt.hist(x=eigenval1, bins='auto')
plt.grid(axis='x', alpha=0.75)
plt.xlabel('Blob Inertia First Eigenvalue (pixels)')
plt.title('Blob Inertia First Eigenvalue Distribution')

plt.figure(4)
plt.hist(x=eigenval2, bins='auto')
plt.grid(axis='x', alpha=0.75)
plt.xlabel('Blob Inertia Second Eigenvalue (pixels)')
plt.title('Blob Inertia Second Eigenvalue Distribution')

plt.show
"""
