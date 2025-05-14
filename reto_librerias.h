#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/types.h>
#include <stdint.h>

// Tecnologico de Monterrey
// Campus Puebla
// Oskar Adolfo Villa Lopez
// Cruz Daniel Perez Jimenez
// David Alberto Alvarado Cabrero
// Mayo 2025

#pragma pack(push, 1)
typedef struct
{
     uint16_t bfType;
     uint32_t bfSize;
     uint16_t bfReserved1;
     uint16_t bfReserved2;
     uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct
{
     uint32_t biSize;
     int32_t biWidth;
     int32_t biHeight;
     uint16_t biPlanes;
     uint16_t biBitCount;
     uint32_t biCompression;
     uint32_t biSizeImage;
     int32_t biXPelsPerMeter;
     int32_t biYPelsPerMeter;
     uint32_t biClrUsed;
     uint32_t biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

typedef struct
{
     uint8_t blue;
     uint8_t green;
     uint8_t red;
} RGB;

void create_folder(char *path)
{
     // Create the folder "out" if it does not exist
     struct stat st = {0};
     if (stat(path, &st) == -1)
     {
#ifdef _WIN32
          mkdir(path);
#else
          mkdir(path, 0700);
#endif
     }
}

extern void grey_scale_img(RGB *image, int width, int height, int padding, char mask[10], BITMAPFILEHEADER file_header, BITMAPINFOHEADER info_header)
{
     FILE *outputImage; // Transformed image
     char add_char[80] = "./out/";
     strcat(add_char, mask);

     printf("%s\n", add_char);

     create_folder("./out");
     // Create the folder "out" if it does not exist

     outputImage = fopen(add_char, "wb"); // Transformed image

     if (outputImage == NULL)
     {
          perror("Error opening output file");
          return;
     }

     fwrite(&file_header, sizeof(BITMAPFILEHEADER), 1, outputImage);
     fwrite(&info_header, sizeof(BITMAPINFOHEADER), 1, outputImage);

     // Create a temporary array for the grayscale image
     RGB *grayscale_image = (RGB *)malloc(width * height * sizeof(RGB));
     if (!grayscale_image)
     {
          perror("Memory allocation failed");
          fclose(outputImage);
          return;
     }

     // Process each pixel to convert to grayscale
     for (int i = 0; i < height; i++)
     {
          for (int j = 0; j < width; j++)
          {
               int index = i * width + j;
               unsigned char r = image[index].red;
               unsigned char g = image[index].green;
               unsigned char b = image[index].blue;

               // Convert to grayscale using luminosity formula
               unsigned char grayscale = (unsigned char)(0.21 * r + 0.72 * g + 0.07 * b);

               // Update the temporary array with the grayscale value
               grayscale_image[index].red = grayscale;
               grayscale_image[index].green = grayscale;
               grayscale_image[index].blue = grayscale;
          }
     }

     for (int i = height - 1; i >= 0; i--)
     {
          fwrite(&grayscale_image[i * width], sizeof(RGB), width, outputImage);
          for (int p = 0; p < padding; p++)
               fputc(0, outputImage);
     }

     free(grayscale_image);
     fclose(outputImage);
}

void blur_img(RGB *image, int width, int height, int kernel_size, int padding, char mask[10], BITMAPFILEHEADER file_header, BITMAPINFOHEADER info_header)
{
     int offset = kernel_size / 2;
     RGB *temp = malloc(width * height * sizeof(RGB));
     RGB *blurred_image = malloc(width * height * sizeof(RGB));
     if (!temp || !blurred_image)
     {
          perror("Memory allocation failed");
          free(temp);
          free(blurred_image);
          return;
     }

// Horizontal pass
#pragma omp parallel for
     for (int y = 0; y < height; y++)
     {
          for (int x = 0; x < width; x++)
          {
               int sum_r = 0, sum_g = 0, sum_b = 0;
               int count = 0;

               for (int k = -offset; k <= offset; k++)
               {
                    int px = x + k;
                    if (px >= 0 && px < width)
                    {
                         RGB pixel = image[y * width + px];
                         sum_r += pixel.red;
                         sum_g += pixel.green;
                         sum_b += pixel.blue;
                         count++;
                    }
               }

               temp[y * width + x].red = sum_r / count;
               temp[y * width + x].green = sum_g / count;
               temp[y * width + x].blue = sum_b / count;
          }
     }

// Vertical pass
#pragma omp parallel for
     for (int y = 0; y < height; y++)
     {
          for (int x = 0; x < width; x++)
          {
               int sum_r = 0, sum_g = 0, sum_b = 0;
               int count = 0;

               for (int k = -offset; k <= offset; k++)
               {
                    int py = y + k;
                    if (py >= 0 && py < height)
                    {
                         RGB pixel = temp[py * width + x];
                         sum_r += pixel.red;
                         sum_g += pixel.green;
                         sum_b += pixel.blue;
                         count++;
                    }
               }

               blurred_image[y * width + x].red = sum_r / count;
               blurred_image[y * width + x].green = sum_g / count;
               blurred_image[y * width + x].blue = sum_b / count;
          }
     }

     free(temp);

     // Output
     char add_char[80] = "./out/";
     strcat(add_char, mask);
     printf("%s\n", add_char);

     create_folder("./out");

     FILE *outputImage = fopen(add_char, "wb");
     if (!outputImage)
     {
          perror("Error opening output file");
          free(blurred_image);
          return;
     }

     fwrite(&file_header, sizeof(BITMAPFILEHEADER), 1, outputImage);
     fwrite(&info_header, sizeof(BITMAPINFOHEADER), 1, outputImage);

     for (int i = height - 1; i >= 0; i--)
     {
          fwrite(&blurred_image[i * width], sizeof(RGB), width, outputImage);
          for (int p = 0; p < padding; p++)
               fputc(0, outputImage);
     }

     free(blurred_image);
     fclose(outputImage);
}

extern void horizontal_mirror_color_img(RGB *image, int width, int height, int padding, char mask[10], BITMAPFILEHEADER file_header, BITMAPINFOHEADER info_header)
{
     FILE *outputImage; // Imagen transformada
     char add_char[80] = "./out/";
     strcat(add_char, mask);

     printf("%s\n", add_char);

     create_folder("./out");
     // Crear la carpeta "out" si no existe

     outputImage = fopen(add_char, "wb"); // Imagen transformada

     if (outputImage == NULL)
     {
          perror("Error opening output file");
          return;
     }

     fwrite(&file_header, sizeof(BITMAPFILEHEADER), 1, outputImage);
     fwrite(&info_header, sizeof(BITMAPINFOHEADER), 1, outputImage);

     // Crear un arreglo para la imagen espejada
     RGB *mirrored_image = (RGB *)malloc(width * height * sizeof(RGB));
     if (!mirrored_image)
     {
          perror("Memory allocation failed");
          fclose(outputImage);
          return;
     }

     // Aplicar el efecto espejo horizontal
     for (int i = 0; i < height; i++)
     {
          for (int j = 0; j < width; j++)
          {
               int src_idx = i * width + j;
               int dst_idx = i * width + (width - 1 - j);

               // Copiar los valores RGB al píxel espejado
               mirrored_image[dst_idx] = image[src_idx];
          }
     }

     // Escribir los datos de píxeles en el archivo de salida
     for (int i = height - 1; i >= 0; i--)
     {
          fwrite(&mirrored_image[i * width], sizeof(RGB), width, outputImage);

          // Escribir los bytes de padding
          for (int p = 0; p < padding; p++)
          {
               fputc(0, outputImage);
          }
     }

     // Liberar recursos
     free(mirrored_image);
     fclose(outputImage);
}

extern void vertical_mirror_color_img(RGB *image, int width, int height, int padding, char mask[10], BITMAPFILEHEADER file_header, BITMAPINFOHEADER info_header)
{
     FILE *outputImage; // Imagen transformada
     char add_char[80] = "./out/";
     strcat(add_char, mask);

     printf("%s\n", add_char);

     create_folder("./out");
     // Crear la carpeta "out" si no existe

     outputImage = fopen(add_char, "wb"); // Imagen transformada

     if (outputImage == NULL)
     {
          perror("Error opening output file");
          return;
     }

     fwrite(&file_header, sizeof(BITMAPFILEHEADER), 1, outputImage);
     fwrite(&info_header, sizeof(BITMAPINFOHEADER), 1, outputImage);

     // Crear un arreglo para la imagen espejada
     RGB *mirrored_image = (RGB *)malloc(width * height * sizeof(RGB));
     if (!mirrored_image)
     {
          perror("Memory allocation failed");
          fclose(outputImage);
          return;
     }

     // Aplicar el efecto espejo vertical
     for (int i = 0; i < height; i++)
     {
          for (int j = 0; j < width; j++)
          {
               int src_idx = i * width + j;
               int dst_idx = (height - 1 - i) * width + j;

               // Copiar los valores RGB al píxel espejado
               mirrored_image[dst_idx] = image[src_idx];
          }
     }

     // Escribir los datos de píxeles en el archivo de salida
     for (int i = height - 1; i >= 0; i--)
     {
          fwrite(&mirrored_image[i * width], sizeof(RGB), width, outputImage);

          // Escribir los bytes de padding
          for (int p = 0; p < padding; p++)
          {
               fputc(0, outputImage);
          }
     }

     // Liberar recursos
     free(mirrored_image);
     fclose(outputImage);
}

extern void horizontal_mirror_bw_img(RGB *image, int width, int height, int padding, char mask[10], BITMAPFILEHEADER file_header, BITMAPINFOHEADER info_header)
{
     FILE *outputImage; // Imagen transformada
     char add_char[80] = "./out/";
     strcat(add_char, mask);

     printf("%s\n", add_char);

     create_folder("./out");
     // Crear la carpeta "out" si no existe

     outputImage = fopen(add_char, "wb"); // Imagen transformada

     if (outputImage == NULL)
     {
          perror("Error opening output file");
          return;
     }

     fwrite(&file_header, sizeof(BITMAPFILEHEADER), 1, outputImage);
     fwrite(&info_header, sizeof(BITMAPINFOHEADER), 1, outputImage);

     // Crear un arreglo para la imagen espejada
     RGB *mirrored_image = (RGB *)malloc(width * height * sizeof(RGB));
     if (!mirrored_image)
     {
          perror("Memory allocation failed");
          fclose(outputImage);
          return;
     }

     // Aplicar el efecto espejo horizontal y convertir a escala de grises
     for (int i = 0; i < height; i++)
     {
          for (int j = 0; j < width; j++)
          {
               int src_idx = i * width + j;
               int dst_idx = i * width + (width - 1 - j);

               // Convertir a escala de grises usando la fórmula de luminosidad
               unsigned char grayscale = (unsigned char)(0.21 * image[src_idx].red +
                                                         0.72 * image[src_idx].green +
                                                         0.07 * image[src_idx].blue);

               // Asignar el valor en escala de grises al píxel espejado
               mirrored_image[dst_idx].red = grayscale;
               mirrored_image[dst_idx].green = grayscale;
               mirrored_image[dst_idx].blue = grayscale;
          }
     }

     // Escribir los datos de píxeles en el archivo de salida
     for (int i = height - 1; i >= 0; i--)
     {
          fwrite(&mirrored_image[i * width], sizeof(RGB), width, outputImage);

          // Escribir los bytes de padding
          for (int p = 0; p < padding; p++)
          {
               fputc(0, outputImage);
          }
     }

     // Liberar recursos
     free(mirrored_image);
     fclose(outputImage);
}

extern void vertical_mirror_bw_img(RGB *image, int width, int height, int padding, char mask[10], BITMAPFILEHEADER file_header, BITMAPINFOHEADER info_header)
{
     FILE *outputImage; // Imagen transformada
     char add_char[80] = "./out/";
     strcat(add_char, mask);

     printf("%s\n", add_char);

     create_folder("./out");
     // Crear la carpeta "out" si no existe

     outputImage = fopen(add_char, "wb"); // Imagen transformada

     if (outputImage == NULL)
     {
          perror("Error opening output file");
          return;
     }

     fwrite(&file_header, sizeof(BITMAPFILEHEADER), 1, outputImage);
     fwrite(&info_header, sizeof(BITMAPINFOHEADER), 1, outputImage);

     // Crear un arreglo para la imagen espejada
     RGB *mirrored_image = (RGB *)malloc(width * height * sizeof(RGB));
     if (!mirrored_image)
     {
          perror("Memory allocation failed");
          fclose(outputImage);
          return;
     }

     // Aplicar el efecto espejo vertical y convertir a escala de grises
     for (int i = 0; i < height; i++)
     {
          for (int j = 0; j < width; j++)
          {
               int src_idx = i * width + j;
               int dst_idx = (height - 1 - i) * width + j;

               // Convertir a escala de grises usando la fórmula de luminosidad
               unsigned char grayscale = (unsigned char)(0.21 * image[src_idx].red +
                                                         0.72 * image[src_idx].green +
                                                         0.07 * image[src_idx].blue);

               // Asignar el valor en escala de grises al píxel espejado
               mirrored_image[dst_idx].red = grayscale;
               mirrored_image[dst_idx].green = grayscale;
               mirrored_image[dst_idx].blue = grayscale;
          }
     }

     // Escribir los datos de píxeles en el archivo de salida
     for (int i = height - 1; i >= 0; i--)
     {
          fwrite(&mirrored_image[i * width], sizeof(RGB), width, outputImage);

          // Escribir los bytes de padding
          for (int p = 0; p < padding; p++)
          {
               fputc(0, outputImage);
          }
     }

     // Liberar recursos
     free(mirrored_image);
     fclose(outputImage);
}