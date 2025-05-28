// Tecnologico de Monterrey
// Campus Puebla
// Oskar Adolfo Villa Lopez
// Cruz Daniel Perez Jimenez
// David Alberto Alvarado Cabrero
// Mayo 2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#include <mpi.h>
#include "reto_librerias.h"
#include <stdint.h>

void process_all_images(int kernel_size, int mpi_rank, int mpi_size)
{
    char *img_folder = "./Parallelization/img/";
    char command[256];
    FILE *fp;
    FILE *operations_count;
    char **filenames = NULL;
    int IMAGE_COUNT = 0;

    if (mpi_rank == 0)
    {
        // Obtener lista de archivos y contar el número de imágenes
        snprintf(command, sizeof(command), "ls %s | wc -l", img_folder);
        fp = popen(command, "r");
        if (fp == NULL)
        {
            perror("Failed to count images");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        if (fscanf(fp, "%d", &IMAGE_COUNT) != 1)
        {
            perror("Failed to read image count");
            pclose(fp);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        pclose(fp);

        // Asignar memoria para los nombres de los archivos
        filenames = malloc(IMAGE_COUNT * sizeof(char *));
        if (filenames == NULL)
        {
            perror("Memory allocation failed for filenames");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Obtener lista de archivos
        snprintf(command, sizeof(command), "ls %s", img_folder);
        fp = popen(command, "r");
        if (fp == NULL)
        {
            perror("Failed to list images");
            free(filenames);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Leer nombres de archivos
        char filename[128];
        int i = 0;
        while (fgets(filename, sizeof(filename), fp) != NULL && i < IMAGE_COUNT)
        {
            filename[strcspn(filename, "\n")] = '\0'; // quitar salto de línea
            filenames[i++] = strdup(filename);        // duplicar el nombre
        }
        pclose(fp);
    }

    // Broadcast IMAGE_COUNT a todos los procesos
    MPI_Bcast(&IMAGE_COUNT, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Distribuir nombres de archivos a todos los procesos
    if (mpi_rank != 0)
    {
        filenames = malloc(IMAGE_COUNT * sizeof(char *));
        for (int i = 0; i < IMAGE_COUNT; i++)
        {
            filenames[i] = malloc(128);
        }
    }
    for (int i = 0; i < IMAGE_COUNT; i++)
    {
        if (mpi_rank == 0)
        {
            MPI_Bcast(filenames[i], 128, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Bcast(filenames[i], 128, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
    }

    double start_time = omp_get_wtime();

    // Inicializar archivo (vacío) por proceso
    char operations_filename[64];
    snprintf(operations_filename, sizeof(operations_filename), "operations_count_rank%d.txt", mpi_rank);
    operations_count = fopen(operations_filename, "w");
    if (operations_count == NULL)
    {
        perror("Failed to open operations_count.txt");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    fclose(operations_count);

    // Procesar imágenes asignadas a este proceso
#pragma omp parallel for schedule(dynamic)
    for (int j = mpi_rank; j < IMAGE_COUNT; j += mpi_size)
    {
        char *filename = filenames[j];
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s%s", img_folder, filename);

        // Crear nombres de salida
        char filename_copy[128];
        strncpy(filename_copy, filename, sizeof(filename_copy));
        char *dot = strrchr(filename_copy, '.');
        if (dot && strcmp(dot, ".bmp") == 0)
        {
            *dot = '\0';
        }

        char greyscale_output[256], blur_output[256], mirror_horizontal_output[256];
        char mirror_vertical_output[256], mirror_vertical_bw_output[256], mirror_horizontal_bw_output[256];

        snprintf(greyscale_output, sizeof(greyscale_output), "%s_greyscale.bmp", filename_copy);
        snprintf(blur_output, sizeof(blur_output), "%s_blur.bmp", filename_copy);
        snprintf(mirror_horizontal_output, sizeof(mirror_horizontal_output), "%s_mirrorHorizontal.bmp", filename_copy);
        snprintf(mirror_vertical_output, sizeof(mirror_vertical_output), "%s_mirrorVertical.bmp", filename_copy);
        snprintf(mirror_vertical_bw_output, sizeof(mirror_vertical_bw_output), "%s_mirrorVerticalBW.bmp", filename_copy);
        snprintf(mirror_horizontal_bw_output, sizeof(mirror_horizontal_bw_output), "%s_mirrorHorizontalBW.bmp", filename_copy);

        FILE *file = fopen(filepath, "rb");
        if (!file)
        {
            printf("Error abriendo el archivo BMP\n");
            continue;
        }

        BITMAPFILEHEADER file_header;
        BITMAPINFOHEADER info_header;

        fread(&file_header, sizeof(BITMAPFILEHEADER), 1, file);
        fread(&info_header, sizeof(BITMAPINFOHEADER), 1, file);

        // Skip to pixel data
        fseek(file, file_header.bfOffBits, SEEK_SET);

        int width = info_header.biWidth;
        int height = info_header.biHeight;
        int abs_height = abs(height); // to handle top-down bitmaps
        int row_padded = (width * 3 + 3) & (~3);

        unsigned char *row = malloc(row_padded);
        RGB *image = malloc(width * abs_height * sizeof(RGB));

        if (info_header.biBitCount != 24 || info_header.biCompression != 0)
        {
            printf("Unsupported BMP format: bit count = %d, compression = %u\n",
                   info_header.biBitCount, info_header.biCompression);
            fclose(file);
            free(row);
            free(image);
            continue;
        }

        if (!row || !image)
        {
            printf("Memory allocation failed\n");
            fclose(file);
            free(row);
            free(image);
            continue;
        }

        for (int i = 0; i < abs_height; i++)
        {
            if (fread(row, sizeof(unsigned char), row_padded, file) != row_padded)
            {
                printf("Error reading row data\n");
                fclose(file);
                free(row);
                free(image);
                continue;
            }

            int row_index = (height > 0) ? (abs_height - 1 - i) : i;

            for (int k = 0; k < width; k++)
            {
                int index = row_index * width + k;
                image[index].blue = row[k * 3];
                image[index].green = row[k * 3 + 1];
                image[index].red = row[k * 3 + 2];
            }
        }

        free(row);

        int total_pixels_read = width * height * 3; // Total de píxeles leídos
        int total_pixels_written = width * height * 3 * 6;
        int padding = (4 - (width * sizeof(RGB)) % 4) % 4; // Calculate padding for BMP format

#pragma omp critical
        {
            operations_count = fopen(operations_filename, "a");
            if (operations_count != NULL)
            {
                fprintf(operations_count, "File: %s\n", filename_copy);
                fprintf(operations_count, "Total pixels read: %d\n", total_pixels_read);
                fprintf(operations_count, "Total pixels written: %d\n\n", total_pixels_written);
                fclose(operations_count);
            }
        }

        grey_scale_img(image, width, height, padding, greyscale_output, file_header, info_header);
        blur_img(image, width, height, kernel_size, padding, blur_output, file_header, info_header);
        horizontal_mirror_color_img(image, width, height, padding, mirror_horizontal_output, file_header, info_header);
        vertical_mirror_color_img(image, width, height, padding, mirror_vertical_output, file_header, info_header);
        horizontal_mirror_bw_img(image, width, height, padding, mirror_horizontal_bw_output, file_header, info_header);
        vertical_mirror_bw_img(image, width, height, padding, mirror_vertical_bw_output, file_header, info_header);

        free(image);
        free(filenames[j]);
    }

    double end_time = omp_get_wtime();
    if (mpi_rank == 0)
    {
        printf("Total execution time for processing all images: %.2f seconds\n", end_time - start_time);
    }

    free(filenames);
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

#pragma omp parallel for
        for (int i = 0; i < 1000000; i++)
        {
            double temp = sqrt(i) * sin(i); // Dummy computation
        }

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

int main(int argc, char **argv)
{
    int mpi_rank, mpi_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    int kernel_size;
    if (mpi_rank == 0)
    {
        do
        {
            printf("Enter the kernel size for blurring (55 to 150): ");
            fflush(stdout);
            scanf("%d", &kernel_size);
            if (kernel_size < 55 || kernel_size > 155)
            {
                printf("Invalid kernel size. Please enter a value between 55 and 150.\n");
            }
        } while (kernel_size < 55 || kernel_size > 150);
    }
    MPI_Bcast(&kernel_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int optimal_threads = find_optimal_threads();
    if (mpi_rank == 0)
        printf("Optimal number of threads: %d\n", optimal_threads);
    omp_set_num_threads(optimal_threads);

    process_all_images(kernel_size, mpi_rank, mpi_size);

    if (mpi_rank == 0)
    {
#pragma omp parallel
        {
#pragma omp single
            {
                printf("Number of threads used: %d\n", omp_get_num_threads());
            }
        }
    }
    MPI_Finalize();
    return 0;
}