import cv2
import numpy as np
from matplotlib import pyplot as plt
img699 = cv2.imread('C:/Users/JAZ/github/Starfitter/src/img_0098.jpg')
img872 = cv2.imread('C:/Users/JAZ/github/Starfitter/src/img_0149.jpg')
shp699 = img699.shape
shp872 = img872.shape
#print(shp699)
#print(shp872)


#cv2.imshow('img_0699.jpg',img699)
#cv2.imshow('img_0872.jpg',img872)
#cv2.waitKey(0)
#cv2.destroyAllWindows()

b699, g699, r699 = cv2.split(img699)
b872, g872, r872 = cv2.split(img872)
#cv2.imshow('b699',b699)
#cv2.imshow('g699',g699)
#cv2.imshow('r699',r699)
#cv2.imshow('b872',b872)
#cv2.imshow('g872',g872)
#cv2.imshow('r872',r872)
#cv2.waitKey(0)
#cv2.destroyAllWindows()



# Initiate ORB detector
orb = cv2.ORB_create(nfeatures=50000, scoreType=cv2.ORB_FAST_SCORE)

# Convert to grayscale

img699_bw = cv2.cvtColor(img699,cv2.COLOR_BGR2GRAY)
img872_bw = cv2.cvtColor(img872,cv2.COLOR_BGR2GRAY)

# Detect keypoints and compute the descriptors with ORB

kp699, des699 = orb.detectAndCompute(img699_bw, None)
kp872, des872 = orb.detectAndCompute(img872_bw, None)

# Initialize the Matcher for matching 
# the keypoints and then match the 
# keypoints 

matcher = cv2.BFMatcher() 
matches = matcher.match(des699, des872)
matches = sorted(matches, key = lambda x:x.distance)


# draw the matches to the final image 
# containing both the images the drawMatches() 
# function takes both images and keypoints 
# and outputs the matched query image with 
# its train image 

final_img = cv2.drawMatches(img699, kp699, img872, kp872, matches[:200], None)

final_img = cv2.resize(final_img, (2000,1300)) 
  
# Show the final image 
cv2.imshow("Matches", final_img)

# draw only keypoints location, not size and orientation

#img2699 = cv2.drawKeypoints(img699, kp699, None, color=(0,255,0), flags=0)
#plt.imshow(img2699), plt.show()

#img2872 = cv2.drawKeypoints(img872, kp872, None, color=(0,255,0), flags=0)
#plt.imshow(img2872), plt.show()

cv2.waitKey(0)
cv2.destroyAllWindows()
