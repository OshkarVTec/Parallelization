//Tecnologico de Monterrey
//Campus Puebla
//Oskar Adolfo Villa Lopez
//Cruz Daniel Perez Jimenez
//David Alberto Alvarado Cabrero
//Mayo 2025

extern void grey_scale_img(char mask[10], char path[80]){       
    FILE *image, *outputImage, *lecturas, *fptr; //Transformacion de imagen
    char add_char[80] = "./img/";
    strcat(add_char, mask);
    strcat(add_char, ".bmp");
    printf("%s\n", add_char);
    image = fopen(path,"rb");          //Imagen original a transformar
    outputImage = fopen(add_char,"wb");    //Imagen transformada

    unsigned char r, g, b;               //Pixel
    
    int i;
    for(i=0; i<54; i++) fputc(fgetc(image), outputImage);   //Copia cabecera a nueva imagen
    while(!feof(image)){                                        //Grises
       b = fgetc(image);
       g = fgetc(image);
       r = fgetc(image);

       if(r <= 0.80 && g <= 0.80 && b <= 0.80)
       {
            fputc(205, outputImage);
            fputc(205, outputImage);
            fputc(205, outputImage);
       }
       else 
       {
            unsigned char pixel = 0.21*r+0.72*g+0.07*b;
            fputc(pixel, outputImage);
            fputc(pixel, outputImage);
            fputc(pixel, outputImage);
       }
    }

    fclose(image);
    fclose(outputImage);
}