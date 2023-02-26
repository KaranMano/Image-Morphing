# Introduction
The main objective of the project is to morph a given source image into a given target image based on features. The morphing is done using Feature-based Image Metamorphosis [1]. To achieve this, the traditional methods are to define the features using manual correspondence markers. This process is tedious and would be impossible for a large number of images. We will try to automate the process of defining the correspondence markers as done by Covell [2].
To make this, we will first be implementing manual correspondence, after which we will move on to defining the features automatically using Feature-based Match estimation. We will also give users manual control over tweaking the features detected using Autocorrespondence.

# Implementation
To implement Feature Based Image metamorphosis, we followed Feaeture Based image metamorphosis[1], in which they went step by step, increasing the complexity of transformations at every step. 
We divided our implementation into the following steps:
- Take user input for both the source and the target images
- Take user input for the correspondence markers on the image (line segments)
- Transforming an image using one line pair
- Transforming an image using multiple line pairs
- Transforming a human face
- Reverse Image Transformation
- In Between Frames generation
- Crossfading for In Between Frames
- Automatic correspondence marker generation

We are using ImGui and OpenGL to build our GUI for input/output, and we use stb for storing and manipulating images and their data. 

## Storing the correspondence markers data
The correspondence markers are defined as the `Edge` class in the code. The `Edge` class consists of three attributes which are:
- Head coordinate
- Tail coordinate
- IsDisplaying boolean

The usage of the attributes is self-explanatory. The correspondence markers are stored as a tuple of their head and tail coordinates in two files named `source.edges` and `target.edges`.

## Transforming with one line pair
A pair of lines (one defined relative to the source image, the other defined relative to the destination image) defines a mapping from one image to the other[1]. 
We first create an empty image with the same image type and dimensions as the source image. This image will act as the destination image i.e. the output image after morphing the source image. We now iterate over every pixel of this empty image and find the corresponding pixel value in the source image using the following equations:
<img src="./Media/one-line-pair.png" height="250"/>
<br>
`u = ((X - P) . (Q - P)) / ||Q - P||^2`
<br>
`v = ((X - P) . perpendicular(Q - P)) / ||Q - P||`
<br>
`X’ = P’ + u . (Q’ - P’) + (v . perpendicular(Q’ - P’))/||Q’ - P’||`

Where:
- X’ is the value of the corresponding pixel value in the source image
- X is the pixel value of destination image
- perpendicular() function returns a vector perpendicular to a given vector

We recreated some of the examples from the single line pair correspondence and got the same results as the Feature based Image Metamorphosis[1].
<br>
<br>
<img src="./Media/one-line-pair-examples.png" height="350" />
<br>
<img src="./Media/one-line-pair-result.png" height="550" />


## Transforming with multiple line pairs
To Implement multiple line pair transformations, for every pixel in the destination image, we need to iterate over all the correspondence markers and calculate their weight in the final displacement of the pixel position from the source image to the destination image.
We do this by calculating the weight of every correspondence marker pair, and take a weighted average of all the displacements.

<img src="./Media/multiple-line-pair.png" height="250" />

Where:
- length = len(P’Q’)
- dist is defined as:
    - dist = |v|    for 1 > u > 0,
    - len(XQ)       for u >= 1,
    - len(XP)       for u <= 0
    
Here a, b and p are constants for controlling the warping. These constants will come in handy when we transform a human face since it will need tweaking to get their optimal values for correct morph generation.
We also recreated the example of transformation with multiple line pairs as shown below

<img src="./Media/multiple-line-pair-example.png" height="350" />
<br>
<img src="./Media/multiple-line-pair-result.png" height="550" />


