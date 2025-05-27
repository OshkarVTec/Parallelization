#include <mpi.h>

int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int kernel_size;
    if (rank == 0)
    {
        do
        {
            printf("Enter the kernel size for blurring (55 to 150): ");
            fflush(stdout);
            scanf("%d", &kernel_size);
            if (kernel_size < 55 || kernel_size > 150)
            {
                printf("Invalid kernel size. Please enter a value between 55 and 150.\n");
            }
        } while (kernel_size < 55 || kernel_size > 150);
    }
    // Broadcast kernel_size to all processes
    MPI_Bcast(&kernel_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Obtener lista de archivos solo en el proceso 0
    char *img_folder = "./img/";
    char command[256];
    FILE *fp;
    int IMAGE_COUNT = 0;
    char **filenames = NULL;

    if (rank == 0)
    {
        snprintf(command, sizeof(command), "ls %s | wc -l", img_folder);
        fp = popen(command, "r");
        if (fp == NULL || fscanf(fp, "%d", &IMAGE_COUNT) != 1)
        {
            perror("Failed to count images");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        pclose(fp);

        filenames = malloc(IMAGE_COUNT * sizeof(char *));
        snprintf(command, sizeof(command), "ls %s", img_folder);
        fp = popen(command, "r");
        if (fp == NULL)
        {
            perror("Failed to list images");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        char filename[128];
        int i = 0;
        while (fgets(filename, sizeof(filename), fp) != NULL && i < IMAGE_COUNT)
        {
            filename[strcspn(filename, "\n")] = '\0';
            filenames[i++] = strdup(filename);
        }
        pclose(fp);
    }

    // Broadcast IMAGE_COUNT to all processes
    MPI_Bcast(&IMAGE_COUNT, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Distribuir nombres de archivos a todos los procesos
    int files_per_proc = IMAGE_COUNT / size;
    int remainder = IMAGE_COUNT % size;
    int my_count = files_per_proc + (rank < remainder ? 1 : 0);
    int *counts = NULL, *displs = NULL;
    if (rank == 0)
    {
        counts = malloc(size * sizeof(int));
        displs = malloc(size * sizeof(int));
        int offset = 0;
        for (int i = 0; i < size; i++)
        {
            counts[i] = files_per_proc + (i < remainder ? 1 : 0);
            displs[i] = offset;
            offset += counts[i];
        }
    }

    // Preparar buffer para recibir mis nombres de archivos
    char my_filenames[my_count][128];
    if (rank == 0)
    {
        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < counts[i]; j++)
            {
                if (i == 0)
                    strncpy(my_filenames[j], filenames[displs[i] + j], 128);
                else
                    MPI_Send(filenames[displs[i] + j], 128, MPI_CHAR, i, 0, MPI_COMM_WORLD);
            }
        }
    }
    else
    {
        for (int j = 0; j < my_count; j++)
            MPI_Recv(my_filenames[j], 128, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Cada proceso procesa sus imágenes
    omp_set_num_threads(find_optimal_threads());
    double start_time = omp_get_wtime();
#pragma omp parallel for schedule(dynamic)
    for (int j = 0; j < my_count; j++)
    {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s%s", img_folder, my_filenames[j]);
        // Aquí puedes copiar el código de procesamiento de una imagen,
        // usando my_filenames[j] como nombre de archivo.
        // Por simplicidad, puedes extraer el cuerpo del ciclo for de process_all_images()
        // y adaptarlo aquí.
    }
    double end_time = omp_get_wtime();
    double my_time = end_time - start_time;
    printf("Rank %d: Execution time: %.2f seconds\n", rank, my_time);

    // Obtener el tiempo máximo entre todos los procesos
    double max_time;
    MPI_Reduce(&my_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (rank == 0)
    {
        printf("Tiempo total de procesamiento (máximo entre todos los procesos): %.2f segundos\n", max_time);
    }

    if (rank == 0)
    {
        for (int i = 0; i < IMAGE_COUNT; i++)
            free(filenames[i]);
        free(filenames);
        free(counts);
        free(displs);
    }

    MPI_Finalize();
    return 0;
}