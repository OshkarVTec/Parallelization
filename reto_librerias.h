#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Tecnologico de Monterrey
//Campus Puebla
//Oskar Adolfo Villa Lopez
//Cruz Daniel Perez Jimenez
//David Alberto Alvarado Cabrero
//Mayo 2025

extern void grey_scale_img(char mask[10], char path[80]){       
     FILE *image, *outputImage; //Transformacion de imagen
     char add_char[80] = "./img/";
     strcat(add_char, mask);
     strcat(add_char, ".bmp");
     printf("%s\n", add_char);
     image = fopen(path,"rb");          //Imagen original a transformar
     outputImage = fopen(add_char,"wb");    //Imagen transformada

     if (image == NULL || outputImage == NULL) {
          perror("Error opening file");
          return;
     }

     unsigned char header[54]; // BMP header
     fread(header, sizeof(unsigned char), 54, image); // Read header
     fwrite(header, sizeof(unsigned char), 54, outputImage); // Write header

     int width = *(int*)&header[18];
     int height = *(int*)&header[22];
     int padding = (4 - (width * 3) % 4) % 4;

     unsigned char *pixelData = malloc((width * 3 + padding) * height);
     fread(pixelData, sizeof(unsigned char), (width * 3 + padding) * height, image);

     #pragma omp parallel for
     for (int i = 0; i < height; i++) {
          for (int j = 0; j < width * 3; j += 3) {
               int index = i * (width * 3 + padding) + j;
               unsigned char b = pixelData[index];
               unsigned char g = pixelData[index + 1];
               unsigned char r = pixelData[index + 2];

               if (r <= 0.80 && g <= 0.80 && b <= 0.80) {
                    pixelData[index] = 205;
                    pixelData[index + 1] = 205;
                    pixelData[index + 2] = 205;
               } else {
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