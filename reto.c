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

void process_all_images()
{
    char *img_folder = "./img/";
    char command[256];
    FILE *fp;

    // Use a system command to list all files in the img folder
    snprintf(command, sizeof(command), "ls %s", img_folder);
    fp = popen(command, "r");
    if (fp == NULL)
    {
        perror("Failed to list images");
        exit(EXIT_FAILURE);
    }

    char filename[128];
    while (fgets(filename, sizeof(filename), fp) != NULL)
    {
        // Remove newline character from filename
        filename[strcspn(filename, "\n")] = '\0';

        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s%s", img_folder, filename);

        // Generate output filenames based on the original filename
        char greyscale_output[256], blur_output[256], mirror_horizontal_output[256], mirror_vertical_output[256];

        // Remove the .bmp extension from the filename
        char *dot = strrchr(filename, '.');
        if (dot && strcmp(dot, ".bmp") == 0)
        {
            *dot = '\0'; // Terminate the string before the extension
        }

        snprintf(greyscale_output, sizeof(greyscale_output), "greyscale_%s", filename);
        snprintf(blur_output, sizeof(blur_output), "blur_%s", filename);
        snprintf(mirror_horizontal_output, sizeof(mirror_horizontal_output), "mirrorHorizontal_%s", filename);
        snprintf(mirror_vertical_output, sizeof(mirror_vertical_output), "mirrorVertical_%s", filename);

        // Process the image
        grey_scale_img(greyscale_output, filepath);
        blur_img(blur_output, filepath, 55);
        horizontal_mirror_color_img(mirror_horizontal_output, filepath);
        vertical_mirror_color_img(mirror_vertical_output, filepath);
    }

    pclose(fp);
}

int find_optimal_threads()
{
    int max_threads = omp_get_max_threads();
    int optimal_threads = 1;
    double min_time = INFINITY;

    for (int num_threads = 1; num_threads <= max_threads; num_threads++)
    {
        omp_set_num_threads(num_threads);

        double start_time = omp_get_wtime();
        grey_scale_img("test_greyscale", "./img/Image01.bmp");
        double end_time = omp_get_wtime();
        double elapsed_time = end_time - start_time;
        if (elapsed_time < min_time)
        {
            min_time = elapsed_time;
            optimal_threads = num_threads;
        }
    }

    return optimal_threads;
}

int main()
{
    /*     grey_scale_img("greyscale_1", "./img/Image01.bmp");
        blur_img("blur_1", "./img/Image01.bmp", 55);
        horizontal_mirror_color_img("mirrorHorizontal_1", "./img/Image01.bmp");
        vertical_mirror_color_img("mirrorVertical_1", "./img/Image01.bmp"); */
    int optimal_threads = find_optimal_threads();
    printf("Optimal number of threads: %d\n", optimal_threads);
    omp_set_num_threads(optimal_threads);
    process_all_images();
#pragma omp parallel
    {
#pragma omp single
        {
            printf("Number of threads used: %d\n", omp_get_num_threads());
        }
    }
    return 0;
}