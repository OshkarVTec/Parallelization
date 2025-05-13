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

void process_all_images(int kernel_size)
{
    char *img_folder = "img/";
    char command[256];
    FILE *fp;
    FILE *operations_count;

    // Use a system command to list all files in the img folder
    snprintf(command, sizeof(command), "dir /b \"%s\"", img_folder);
    fp = popen(command, "r");
    if (fp == NULL)
    {
        perror("Failed to list images");
        exit(EXIT_FAILURE);
    }

    // Open the operations_count file to write the number of operations
    operations_count = fopen("operations_count.txt", "w");
    if (operations_count == NULL)
    {
        perror("Failed to open operations_count.txt");
        pclose(fp);
        exit(EXIT_FAILURE);
    }

    char filename[128];
    double start_time = omp_get_wtime(); // Start timing

    while (fgets(filename, sizeof(filename), fp) != NULL)
    {
        // Remove newline character from filename
        filename[strcspn(filename, "\n")] = '\0';

        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s%s", img_folder, filename);

        // Generate output filenames based on the original filename
        char greyscale_output[256], blur_output[256] ,mirror_horizontal_output[256], mirror_vertical_output[256], mirror_vertical_bw_output[256], mirror_horizontal_bw_output[256];

        // Remove the .bmp extension from the filename
        char *dot = strrchr(filename, '.');
        if (dot && strcmp(dot, ".bmp") == 0)
        {
            *dot = '\0'; // Terminate the string before the extension
        }

        // Count the total number of pixels (locations) read from the original image
        int width, height;
        get_image_dimensions(filepath, &width, &height);
        int total_pixels_read = width * height * 6 * 3;
        int total_pixels_written = total_pixels_read;

        // Write the counts to the operations_count file
        fprintf(operations_count, "File: %s\n", filename);
        fprintf(operations_count, "Total pixels read: %d\n", total_pixels_read);
        fprintf(operations_count, "Total pixels written: %d\n\n", total_pixels_written);

        // Generate output filenames
        snprintf(greyscale_output, sizeof(greyscale_output), "%s_greyscale", filename);
        snprintf(blur_output, sizeof(blur_output), "%s_blur", filename);
        snprintf(mirror_horizontal_output, sizeof(mirror_horizontal_output), "%s_mirrorHorizontal", filename);
        snprintf(mirror_vertical_output, sizeof(mirror_vertical_output), "%s_mirrorVertical", filename);
        snprintf(mirror_vertical_bw_output, sizeof(mirror_vertical_bw_output), "%s_mirrorVerticalBW", filename);
        snprintf(mirror_horizontal_bw_output, sizeof(mirror_horizontal_bw_output), "%s_mirrorHorizontalBW", filename);

        // Process the image
        grey_scale_img(greyscale_output, filepath);
        blur_img(blur_output, filepath, kernel_size);
        horizontal_mirror_color_img(mirror_horizontal_output, filepath);
        vertical_mirror_color_img(mirror_vertical_output, filepath);
        horizontal_mirror_bw_img(mirror_horizontal_bw_output, filepath);
        vertical_mirror_bw_img(mirror_vertical_bw_output, filepath);
    }

    double end_time = omp_get_wtime(); // End timing
    printf("Total execution time for processing all images: %.2f seconds\n", end_time - start_time);
    fclose(operations_count);
    pclose(fp);
}

int find_optimal_threads()
{
    int max_threads = 100;
    int optimal_threads = 1;
    double min_time = INFINITY;

    for (int num_threads = 1; num_threads <= max_threads; num_threads += 2)
    {
        omp_set_num_threads(num_threads);

        double start_time = omp_get_wtime();
        grey_scale_img("test_greyscale", "./img/Image03.bmp");
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

    int kernel_size;
    do
    {
        printf("Enter the kernel size for blurring (55 to 150): ");
        scanf("%d", &kernel_size);
        if (kernel_size < 55 || kernel_size > 155)
        {
            printf("Invalid kernel size. Please enter a value between 55 and 150.\n");
        }
    } while (kernel_size < 55 || kernel_size > 150);
    /*     grey_scale_img("greyscale_1", "./img/Image01.bmp");
        blur_img("blur_1", "./img/Image01.bmp", kernel_size);
        horizontal_mirror_color_img("mirrorHorizontal_1", "./img/Image01.bmp");
        vertical_mirror_color_img("mirrorVertical_1", "./img/Image01.bmp"); */
    int optimal_threads = find_optimal_threads();
    printf("Optimal number of threads: %d\n", optimal_threads);
    omp_set_num_threads(optimal_threads);
    process_all_images(kernel_size);
#pragma omp parallel
    {
#pragma omp single
        {
            printf("Number of threads used: %d\n", omp_get_num_threads());
        }
    }
    return 0;
}