// Tecnologico de Monterrey
// Campus Puebla
// Oskar Adolfo Villa Lopez
// Cruz Daniel Perez Jimenez
// David Alberto Alvarado Cabrero
// Mayo 2025

// IMPORTANT run: gcc reto.c -o reto -lm -fopenmp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#include "reto_librerias.h"

int NUM_THREADS = 4; // Number of threads

int main()
{
    omp_set_num_threads(NUM_THREADS);
    grey_scale_img("greyscale_1", "./img/sample1.bmp");
    blur_img("blur_1", "./img/sample1.bmp", 55);
    horizontal_mirror_color_img("mirrorHorizontal_1", "./img/sample1.bmp");
    vertical_mirror_color_img("mirrorVertical_2", "./img/sample1.bmp");
#pragma omp parallel
    {
#pragma omp single
        {
            printf("Number of threads used: %d\n", omp_get_num_threads());
        }
    }
    return 0;
}