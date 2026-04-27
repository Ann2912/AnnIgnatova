#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include "lodepng.h"

// принимаем на вход: имя файла, указатели на int для хранения прочитанной ширины и высоты картинки
// возвращаем указатель на выделенную память для хранения картинки
// Если память выделить не смогли, отдаем нулевой указатель и пишем сообщение об ошибке
unsigned char* load_png(const char* filename, unsigned int* width, unsigned int* height)
{
  unsigned char* image = NULL;
  int error = lodepng_decode32_file(&image, width, height, filename);
  if(error != 0) {
    printf("error %u: %s\n", error, lodepng_error_text(error));
  }
  return (image);
}

// принимаем на вход: имя файла для записи, указатель на массив пикселей,  ширину и высоту картинки
// Если преобразовать массив в картинку или сохранить не смогли,  пишем сообщение об ошибке
void write_png(const char* filename, const unsigned char* image, unsigned width, unsigned height)
{
  unsigned char* png;
  size_t pngsize;
  int error = lodepng_encode32(&png, &pngsize, image, width, height);
  if(error == 0) {
      lodepng_save_file(png, pngsize, filename);
  } else {
    printf("error %u: %s\n", error, lodepng_error_text(error));
  }
  free(png);
}


// вариант огрубления серого цвета в ЧБ
void contrast(unsigned char *col, int bw_size)
{
    int i;
    for(i=0; i < bw_size; i++)
    {
        if(col[i] < 70)
        col[i] = 0;
        if(col[i] > 195)
        col[i] = 255;
    }
    return;
}

// Гауссово размыттие
void Gauss_blur(unsigned char *col, unsigned char *blr_pic, int width, int height)
{
    int i, j;
    for(i=1; i < height-1; i++)
        for(j=1; j < width-1; j++)
        {
            blr_pic[width*i+j] = 0.084*col[width*i+j] + 0.084*col[width*(i+1)+j] + 0.084*col[width*(i-1)+j];
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.084*col[width*i+(j+1)] + 0.084*col[width*i+(j-1)];
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i+1)+(j+1)] + 0.063*col[width*(i+1)+(j-1)];
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i-1)+(j+1)] + 0.063*col[width*(i-1)+(j-1)];
        }
   return;
}

//  Место для экспериментов
void color(unsigned char *blr_pic, unsigned char *res, int size)
{
  int i;
    for(i=1;i<size;i++)
    {
        res[i*4]=40+blr_pic[i]+0.35*blr_pic[i-1];
        res[i*4+1]=65+blr_pic[i];
        res[i*4+2]=170+blr_pic[i];
        res[i*4+3]=255;
    }
    return;
}
// Рабочие области
int regions[12][4] = {
                        {542,326,550,332},
                        {504,328,534,333},
                        {323,19,361,87},
                        {287,83,327,107},
                        {534,28,717,305},
                        {503,302,564,315},
                        {551,316,566,331},
                        {569,329,634,420},
                        {636,317,849,623},
                        {523,444,623,515},
                        {565,570,629,599},
                        {535,517,637,559}
};

void dfs(unsigned char *pic, int *visited, int width, int height, int x, int y) {
    // Проверка условий
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    if (visited[y * width + x]) return;
    if (pic[y * width + x] <10) return;

    // Помечаем точку как посещённую
    visited[y * width + x] = 1;

    // Рекурсивно вызываем для соседних точек
    dfs(pic, visited, width, height, x + 1, y);
    dfs(pic, visited, width, height, x - 1, y);
    dfs(pic, visited, width, height, x, y + 1);
    dfs(pic, visited, width, height, x, y - 1);
}
int main()
{
    const char* filename = "skull.png";
    unsigned int width, height;
    int size;
    int bw_size;

    // Прочитали картинку
    unsigned char* picture = load_png("skull.png", &width, &height);
    if (picture == NULL)
    {
        printf("Problem reading picture from the file %s. Error.\n", filename);
        return -1;
    }

    size = width * height * 4;
    bw_size = width * height;


    unsigned char* bw_pic = (unsigned char*)malloc(bw_size*sizeof(unsigned char));

    for (int i=0; i<bw_size; i++) {
        // находим среднее
        bw_pic[i] = (picture[4*i] + picture[4*i+1] + picture[4*i+2]) / 3;
    }

    unsigned char* blr_pic = (unsigned char*)malloc(bw_size*sizeof(unsigned char));
    unsigned char* finish = (unsigned char*)malloc(size*sizeof(unsigned char));

    // Например, поиграли с  контрастом
    contrast(bw_pic, bw_size);
    for(int i=0;i<bw_size;i++)
    {
        finish[4*i]=bw_pic[i];
        finish[4*i+1]=bw_pic[i];
        finish[4*i+2]=bw_pic[i];
        finish[4*i+3]=255;
    }
    // посмотрим на промежуточные картинки
    write_png("contrast.png", finish, width, height);

    //обрабатываем пиксели вне области
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int in_region = 0;
            for (int i = 0; i < 12; i++) {
                if (x >= regions[i][0] && x <= regions[i][2] && y >= regions[i][1] && y <= regions[i][3]) {
                    in_region = 1;
                    }
            }
            if (!in_region) {
                bw_pic[y * width + x] = 0;
            }
        }
    }

    int *visited = (int*)malloc(bw_size*sizeof(int));
    int count_ship=0;
    // Считаем корабли как компоненты связности
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            int idx = y*width + x;
            if (bw_pic[idx]>50 && !visited[idx]) {
                count_ship++;
                dfs(bw_pic, visited, width, height, x, y);
            }
        }
    }
    printf("Number of ships after contrast: %d\n", count_ship);

    // поиграли с Гауссом
    Gauss_blur(bw_pic, blr_pic, width, height);
    for(int i=0;i<bw_size;i++)
    {
        finish[4*i]=blr_pic[i];
        finish[4*i+1]=blr_pic[i];
        finish[4*i+2]=blr_pic[i];
        finish[4*i+3]=255;
    }
    count_ship=0;
    // Считаем корабли как компоненты связности
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            int idx = y*width + x;
            if (blr_pic[idx]>10 && !visited[idx]) {
                count_ship++;
                dfs(blr_pic, visited, width, height, x, y);
            }
        }
    }
    // посмотрим на промежуточные картинки
    write_png("gauss.png", finish, width, height);
    printf("Number of ships after contrast and gauss: %d\n", count_ship);

    color(blr_pic, finish, bw_size);
    write_png("intermediate_result.png", finish, width, height);

    // выписали результат
    write_png("picture_out.png", finish, width, height);


    // не забыли почистить память!
    free(visited);
    free(bw_pic);
    free(blr_pic);
    free(finish);
    free(picture);

    return 0;
}