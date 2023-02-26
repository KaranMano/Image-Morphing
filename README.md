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
![single line pair](./Media/one-line-pair.png)
<br>
`u = ((X - P) . (Q - P)) / ||Q - P||^2`
<br>
`v = ((X - P) . perpendicular(Q - P)) / ||Q - P||`

