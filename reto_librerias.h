#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/types.h>

// Tecnologico de Monterrey
// Campus Puebla
// Oskar Adolfo Villa Lopez
// Cruz Daniel Perez Jimenez
// David Alberto Alvarado Cabrero
// Mayo 2025

extern void grey_scale_img(char mask[10], char path[80])
{
     FILE *image, *outputImage; // Transformacion de imagen
     char add_char[80] = "./out/";
     strcat(add_char, mask);
     strcat(add_char, ".bmp");
     printf("%s\n", add_char);

     // Create the folder "out" if it does not exist
     struct stat st = {0};
     if (stat("./out", &st) == -1)
     {
          mkdir("./out", 0700);
     }

     image = fopen(path, "rb");           // Imagen original a transformar
     outputImage = fopen(add_char, "wb"); // Imagen transformada

     if (image == NULL || outputImage == NULL)
     {
          perror("Error opening file");
          return;
     }

     unsigned char header[54];                               // BMP header
     fread(header, sizeof(unsigned char), 54, image);        // Read header
     fwrite(header, sizeof(unsigned char), 54, outputImage); // Write header

     int width = *(int *)&header[18];
     int height = *(int *)&header[22];
     int padding = (4 - (width * 3) % 4) % 4;

     unsigned char *pixelData = malloc((width * 3 + padding) * height);
     fread(pixelData, sizeof(unsigned char), (width * 3 + padding) * height, image);

#pragma omp parallel for
     for (int i = 0; i < height; i++)
     {
          for (int j = 0; j < width * 3; j += 3)
          {
               int index = i * (width * 3 + padding) + j;
               unsigned char b = pixelData[index];
               unsigned char g = pixelData[index + 1];
               unsigned char r = pixelData[index + 2];

               if (r <= 0.80 && g <= 0.80 && b <= 0.80)
               {
                    pixelData[index] = 205;
                    pixelData[index + 1] = 205;
                    pixelData[index + 2] = 205;
               }
               else
               {
                    unsigned char pixel = 0.21 * r + 0.72 * g + 0.07 * b;
                    pixelData[index] = pixel;
                    pixelData[index + 1] = pixel;
                    pixelData[index + 2] = pixel;
               }
          }
     }

     fwrite(pixelData, sizeof(unsigned char), (width * 3 + padding) * height, outputImage);

     free(pixelData);
     fclose(image);
     fclose(outputImage);
}

int matrixBlurring(unsigned char *image, long width, long height, int kernelSize, int position, int padding)
{
     int mean = 0, row = 0, pivot = 0, pixel = 0;
     int blurringLevel = (kernelSize - 1) / 2; // Calculate blurring level from kernel size

     for (int i = -blurringLevel; i <= blurringLevel; i++)
     { // Iterate through the rows of the blur matrix
          pivot = position + i * (width + padding);
          if (pivot >= 0 && pivot < height * (width + padding))
          { // Ensure the pivot row is within the image bounds
               row = pivot / (width + padding);
               for (int j = -blurringLevel; j <= blurringLevel; j++)
               { // Iterate through the columns of the blur matrix
                    pixel = pivot + j;

                    if (pixel >= row * (width + padding) && pixel < (row + 1) * (width + padding))
                    { // Ensure the pixel is in the same row as the pivot
                         mean += image[pixel];
                    }
               }
          }
     }
     mean = mean / (kernelSize * kernelSize); // Normalize by the total number of pixels in the kernel
     return mean;
}

extern void blur_img(char mask[10], char path[80], int kernelSize)
{
     FILE *inputImage, *outputImage;
     inputImage = fopen(path, "rb"); // Original image to transform
     char add_char[80] = "./out/";
     strcat(add_char, mask);
     strcat(add_char, ".bmp");
     printf("%s\n", add_char);

     // Create the folder "out" if it does not exist
     struct stat st = {0};
     if (stat("./out", &st) == -1)
     {
          mkdir("./out", 0700);
     }

     outputImage = fopen(add_char, "wb"); // Transformed image
     long width, height;
     int padding;

     unsigned char header[54];
     fread(header, sizeof(unsigned char), 54, inputImage);   // Read BMP header
     fwrite(header, sizeof(unsigned char), 54, outputImage); // Write BMP header

     width = *(int *)&header[18];
     height = *(int *)&header[22];
     padding = (4 - (width * 3) % 4) % 4;

     unsigned char *arr_original = (unsigned char *)malloc((width * 3 + padding) * height * sizeof(unsigned char));
     unsigned char *arr_blurred = (unsigned char *)malloc((width * 3 + padding) * height * sizeof(unsigned char));

     unsigned char *originalBlue = (unsigned char *)malloc(width * height * sizeof(unsigned char));
     unsigned char *originalGreen = (unsigned char *)malloc(width * height * sizeof(unsigned char));
     unsigned char *originalRed = (unsigned char *)malloc(width * height * sizeof(unsigned char));
     unsigned char *blurredBlue = (unsigned char *)malloc(width * height * sizeof(unsigned char));
     unsigned char *blurredGreen = (unsigned char *)malloc(width * height * sizeof(unsigned char));
     unsigned char *blurredRed = (unsigned char *)malloc(width * height * sizeof(unsigned char));

     fread(arr_original, sizeof(unsigned char), (width * 3 + padding) * height, inputImage);

     int j = 0;
     for (int i = 0; i < height; i++)
     {
          for (int k = 0; k < width * 3; k += 3)
          {
               int index = i * (width * 3 + padding) + k;
               originalBlue[j] = arr_original[index];
               originalGreen[j] = arr_original[index + 1];
               originalRed[j] = arr_original[index + 2];
               j++;
          }
     }

#pragma omp parallel
     {
#pragma omp for
          for (int i = 0; i < width * height; i++)
          {
               blurredBlue[i] = matrixBlurring(originalBlue, width, height, kernelSize, i, padding);
               blurredRed[i] = matrixBlurring(originalRed, width, height, kernelSize, i, padding);
               blurredGreen[i] = matrixBlurring(originalGreen, width, height, kernelSize, i, padding);
          }
     }

     j = 0;
     for (int i = 0; i < height; i++)
     {
          for (int k = 0; k < width * 3; k += 3)
          {
               int index = i * (width * 3 + padding) + k;
               arr_blurred[index] = blurredBlue[j];
               arr_blurred[index + 1] = blurredGreen[j];
               arr_blurred[index + 2] = blurredRed[j];
               j++;
          }
          // Add padding bytes
          for (int p = 0; p < padding; p++)
          {
               arr_blurred[i * (width * 3 + padding) + width * 3 + p] = 0;
          }
     }

     fwrite(arr_blurred, sizeof(unsigned char), (width * 3 + padding) * height, outputImage);

     free(arr_original);
     free(arr_blurred);
     free(originalBlue);
     free(originalGreen);
     free(originalRed);
     free(blurredBlue);
     free(blurredGreen);
     free(blurredRed);

     fclose(inputImage);
     fclose(outputImage);
}