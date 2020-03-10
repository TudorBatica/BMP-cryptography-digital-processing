# BMP-cryptography-digital-processing
Developed using C, this project consists of 2 parts.  
The first part is cryptography related, in particular encrypting and decrypting a Bitmap.  
The second part is a template matching algorithm used for detecting digits on a Bitmap.

## Motivation
This project was developed during a Procedural Programming course I took in my first year studying Computer Science at the University of Bucharest.

## How do I use it?
Simply copy the main.c source file and the BMP images. Then, compile and run the program.  
If you want to encrypt another BMP or if you want to test the template matching algorithm on another bitmap, place the BMP in the same directory as the main.c source file and use its name when calling the functions in the main function.

## How does the cryptography part work?
In order to encrypt the given BMP the following algorithm is used:  
A sequence of random numbers is generated using the XORSHIFT32 generator.  
A random permutation is also generated using Durstenfeld's algorithm. The bitmap's pixels are then permuted using the permuation.
Finally, we obtain the ciphered image by substituting the pixels using the following substitution relation:  

In order to decrypt the encrypted the BMP we basically mirror the encryption algorithm:  
A sequence of random numbers is generated using the XORSHIFT32 generator.  
We generate the sequence random of numbers using the XORSHIFT32 generator.  
A random permutation is also generated using Durstenfeld's algorithm and we compute its inverse.  
The pixels are substituted using the inverse substitution relation:  
Finally, the original image is obtained by permuting the pixels.  




