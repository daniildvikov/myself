#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <locale.h>
#include <getopt.h>
#include <string.h>
#include <wchar.h>
#pragma pack (push,1)

typedef struct{
    uint16_t signature;
    uint32_t filesize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t pixelArrOffset;
} BitmapFileHeader;

typedef struct{
    uint32_t headerSize;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    uint32_t xPixelsPerMeter;
    uint32_t yPixelsPerMeter;
    uint32_t colorsInColorTable;
    uint32_t importantColorCount;
} BitmapInfoHeader;

typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
} Rgb;

typedef struct{
    BitmapFileHeader fileHead;
    BitmapInfoHeader infoHead;
    Rgb** pixels;
} BMP;

#pragma pack(pop)


typedef struct
{
    uint32_t x;
    uint32_t y;
    uint32_t I;
    uint32_t J;
    uint64_t sqr;
} SQR;

void HELP(){
    wprintf(L"\n\nДля запуска программы использовать следующий порядок аргументов:\n 'Program   Function    Readfile    Outfile  //   Flags\n\n");
    wprintf(L"-------------Доступные функции------------- \n");
    wprintf(L"triangle: рисование трегольника\n");
    wprintf(L"флаги:\n-A/--Apoint; -B/--Bpoint; -D/Cpoint: аргумент вида число;число -координаты вершин\n");
    wprintf(L"-с/--1color; -C/--2color : цвет 1 и 2; аргумент вида число.число.число\n");
    wprintf(L"-V/--lWidth: толщина линии; аргумент вида число\n");
    wprintf(L"-f/--Fill заливка\n\n" );
    wprintf(L"rectfind: поиск прямоуголька заданного цвета; смена цвета\n ");
    wprintf(L"флаги:\n ");
    wprintf(L"-с/--1color; -C/--2color : цвет 1 и 2; аргумент вида число.число.число\n\n");
    wprintf(L"collage: коллаж nxm\n ");
    wprintf(L"флаги:\n ");
    wprintf(L"-X/--xPic -Y/--yPic число\n ");
}

void displayinfo(BitmapInfoHeader header){
    printf("headerSize:\t%x (%u)\n", header.headerSize, header.headerSize);
    printf("width:     \t%x (%u)\n", header.width, header.width);
    printf("height:    \t%x (%u)\n", header.height, header.height);
    printf("planes:    \t%x (%hu)\n", header.planes, header.planes);
    printf("bitsPerPixel:\t%x (%hu)\n", header.bitsPerPixel, header.bitsPerPixel);
    printf("compression:\t%x (%u)\n", header.compression, header.compression);
    printf("imageSize:\t%x (%u)\n", header.imageSize, header.imageSize);
    printf("xPixelsPerMeter:\t%x (%u)\n", header.xPixelsPerMeter, header.xPixelsPerMeter);
    printf("yPixelsPerMeter:\t%x (%u)\n", header.yPixelsPerMeter, header.yPixelsPerMeter);
    printf("colorsInColorTable:\t%x (%u)\n", header.colorsInColorTable, header.colorsInColorTable);
    printf("importantColorCount:\t%x (%u)\n", header.importantColorCount, header.importantColorCount);
}


int BMPREAD(char* filename, BMP* picture){
    FILE* file=fopen(filename, "rb");
    if(!file){
        wprintf(L"Кажется, я не могу открыть этот файл: %hs", filename);
        return 0;
    }
    fread(&(picture->fileHead),1,sizeof(BitmapFileHeader), file);
    if(picture->fileHead.signature != 0x4d42){
        wprintf(L"Кажется, Ваш файл не поддерживается");
        return 0;
    }
    fread(&(picture->infoHead),1,sizeof(BitmapInfoHeader), file);
    if(picture->infoHead.headerSize != 40){wprintf(L"Версия BMP не поддерживается/Необхожимая версия-3"); return 0;}
    if(picture->infoHead.bitsPerPixel != 24){wprintf(L"Только 24 битные изображения!!!"); return 0;}
    if(picture->infoHead.compression !=0){wprintf(L"Кажется, Ваш файл использует сжатие."); return 0;}
    uint32_t H=picture->infoHead.height;
    uint32_t W=picture->infoHead.width;
    if(H>65535 || W>65535){wprintf(L"Кажется, изображение слишком большое"); return 0;}
    picture->pixels=malloc(H*sizeof(Rgb*));
    for(int i=0; i<H; i++){
        picture->pixels[i]=malloc(W * sizeof(Rgb)+(4-(W*sizeof(Rgb))%4)%4);
        fread(picture->pixels[i],1,W*sizeof(Rgb)+(4-(W*sizeof(Rgb))%4)%4,file);
    }
    fclose(file);
    return 1;
}

void setPixel(uint16_t x, uint16_t y, Rgb*** mas, Rgb* color){
    (*mas)[x][y].b=color->b;
    (*mas)[x][y].g=color->g;
    (*mas)[x][y].r=color->r;
}



void findAndColorRectangle(BMP* bmp, Rgb targetColor, Rgb newColor) {
    int width = bmp->infoHead.width;
    int height = bmp->infoHead.height;

    int maxRectX = 0;
    int maxRectY = 0;
    int maxRectWidth = 0;
    int maxRectHeight = 0;

    // Находим самый большой прямоугольник заданного цвета
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Rgb pixelColor = bmp->pixels[y][x];

            if (pixelColor.r == targetColor.r && pixelColor.g == targetColor.g && pixelColor.b == targetColor.b) {
                // Ищем границы текущего прямоугольника
                int rectX = x;
                int rectY = y;
                int rectWidth = 1;
                int rectHeight = 1;

                // Ищем правую границу прямоугольника
                while (x + rectWidth < width) {
                    Rgb nextPixelColor = bmp->pixels[y][x + rectWidth];
                    if (nextPixelColor.r != targetColor.r || nextPixelColor.g != targetColor.g || nextPixelColor.b != targetColor.b)
                        break;

                    rectWidth++;
                }

                // Ищем нижнюю границу прямоугольника
                while (y + rectHeight < height) {
                    int lineOffset = (y + rectHeight) * width;
                    for (int i = 0; i < rectWidth; i++) {
                        Rgb nextPixelColor = bmp->pixels[y + rectHeight][x + i];
                        if (nextPixelColor.r != targetColor.r || nextPixelColor.g != targetColor.g || nextPixelColor.b != targetColor.b)
                            goto finish;
                    }

                    rectHeight++;
                }

            finish:
                // Проверяем, является ли текущий прямоугольник самым большим
                if (rectWidth * rectHeight > maxRectWidth * maxRectHeight) {
                    maxRectX = rectX;
                    maxRectY = rectY;
                    maxRectWidth = rectWidth;
                    maxRectHeight = rectHeight;
                }
            }
        }
    }
    for (int y = maxRectY; y < maxRectY + maxRectHeight; y++) {
        for (int x = maxRectX; x < maxRectX + maxRectWidth; x++) {
            bmp->pixels[y][x] = newColor;
        }
    }
}


void lineW(uint16_t x, uint16_t y, Rgb*** mas, uint8_t width, unsigned int H, unsigned int W, Rgb* color) {
    uint16_t i = 0;
    uint16_t x1 = x;

    while (i < width / 2 && x1 < H) {
        setPixel(x1, y, mas, color);
        i++;
        x1++;
    }

    i = 0;
    x1 = x;

    while (i < width / 2 && x1 >= 0) {
        setPixel(x1, y, mas, color);
        i++;
        x1--;
    }

    i = 0;
    uint16_t y1 = y;

    while (i < width / 2 && y1 < W) {
        setPixel(x, y1, mas, color);
        i++;
        y1++;
    }

    i = 0;
    y1 = y;

    while (i < width / 2 && y1 >= 0) {
        setPixel(x, y1, mas, color);
        i++;
        y1--;
    }
}

void Palit(uint16_t x1, uint16_t y1,uint16_t x2,uint16_t y2, uint16_t x3,uint16_t y3, Rgb*** mas, unsigned int H, unsigned int W, Rgb* color){
    for(int i=0; i<H; i++){
        for(int j=0; j<W; j++){
            if((((x1-i)*(y2-y1)-(x2-x1)*(y1-j)) >0 &&
                ((x2-i)*(y3-y2)-(x3-x2)*(y2-j))>0 &&
                ((x3-i)*(y1-y3)-(x1-x3)*(y3-j))>0)
               || (((x1-i)*(y2-y1)-(x2-x1)*(y1-j)) <0 &&
                   ((x2-i)*(y3-y2)-(x3-x2)*(y2-j))<0 &&
                   ((x3-i)*(y1-y3)-(x1-x3)*(y3-j))<0))
            {
                setPixel(i,j,mas, color);
            }
        }
    }

}



void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Rgb*** mas, uint8_t width, unsigned int H, unsigned int W, Rgb* color) {
    const int16_t deltaX = abs(x2 - x1);
    const int16_t deltaY = abs(y2 - y1);
    const int16_t signX = x1 < x2 ? 1 : -1;
    const int16_t signY = y1 < y2 ? 1 : -1;
    int16_t error = deltaX - deltaY;
    setPixel(x2, y2, mas, color);
    lineW(x2, y2, mas, width, H, W, color); // Pass additional arguments to lineW
    while (x1 != x2 || y1 != y2) {
        setPixel(x1, y1, mas, color);
        lineW(x1, y1, mas, width, H, W, color); // Pass additional arguments to lineW
        int16_t error2 = error * 2;
        if (error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }
        if (error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
}


void BMPWRITE(FILE* file, BMP* bmp) {
fwrite(&bmp->fileHead, sizeof(BitmapFileHeader), 1, file);
fwrite(&bmp->infoHead, sizeof(BitmapInfoHeader), 1, file);
int width = bmp->infoHead.width;
int height = bmp->infoHead.height;
int padding = (4 - (width * 3) % 4) % 4;
uint8_t paddingBytes[3] = { 0 };
for (int y = 0; y < height; y++) {
    fwrite(bmp->pixels[y], sizeof(Rgb), width, file);
    fwrite(paddingBytes, sizeof(uint8_t), padding, file); // Записываем выравнивающие байты
    }
}
Rgb creatergb(uint8_t blue, uint8_t green, uint8_t red){
    Rgb bgr = {blue, green, red};
    return bgr;
}

void trgb(char* str, Rgb* color) {
    int r, g, b;
    sscanf(str, "%d.%d.%d", &r, &g, &b);
    color->r = (uint8_t)r;
    color->g = (uint8_t)g;
    color->b = (uint8_t)b;
}

void collage(BMP* picture, uint8_t n, uint8_t m){

    BMP widepic;

    uint32_t W=picture->infoHead.width;
    uint32_t H=picture->infoHead.height;

    widepic.infoHead=picture->infoHead;
    widepic.fileHead=picture->fileHead;

    widepic.infoHead.height=n*H;
    widepic.infoHead.width=m*W;

    if(widepic.infoHead.height>32767 || widepic.infoHead.width> 32767){
        wprintf(L"Большой размер изображения");
        return;
    }

    widepic.pixels=malloc(n*H*sizeof(Rgb*));

    for(int i=0; i<H*n; i++){
        widepic.pixels[i]=malloc(m*W*sizeof(Rgb)+(4-(W*sizeof(Rgb))%4)%4);
    }

    for(int i=0; i<m*W; i++){
        for(int j=0; j<n*H;j++){
            widepic.pixels[j][i]=picture->pixels[j%H][i%W];
        }
    }


    picture->infoHead=widepic.infoHead;
    picture->fileHead=widepic.fileHead;
    picture->pixels=widepic.pixels;

}


int main(int argc, char* argv[]) {

    setlocale(LC_ALL, "");

    const struct option lineWidth = {"lWidth", required_argument, NULL, 'V'};
    const struct option firstColor = {"1color", required_argument, NULL, 'c'};
    const struct option secondColor = {"2color", required_argument, NULL, 'C'};
    const struct option xPic = {"xPic", required_argument, NULL, 'X'};
    const struct option yPic = {"yPic", required_argument, NULL, 'Y'};
    const struct option APoint = {"Apoint", required_argument, NULL, 'A'};
    const struct option BPoint = {"Bpoint", required_argument, NULL, 'B'};
    const struct option CPoint = {"Cpoint", required_argument, NULL, 'D'};
    const struct option Fill = {"Fill", no_argument, NULL, 'f'};

    BMP picture;
    int key;
    Rgb color1 = {0, 0, 0};
    Rgb color2 = {0, 0, 0};
    int zalivka = 0;
    int i;
    uint16_t Ax=0;
    uint16_t Ay=0;
    uint16_t Bx=0;
    uint16_t By=0;
    uint16_t Cx=0;
    uint16_t Cy=0;
    char* end;

    FILE* infile = fopen(argv[2], "rb");
    FILE* outfile = fopen(argv[3], "wb");
    int w=0;
    int x=1;
    int y=1;

    if (argc == 1) {
        HELP();
        return 0;
    }
    else if (!strcmp(argv[1], "help")) {
        HELP();
        return 0;
    }
    else if (!BMPREAD(argv[2], &picture)) {
        return -1;
    }
    else if (!strcmp(argv[1], "info")) {
        displayinfo(picture.infoHead);
    }
    else if (!strcmp(argv[1], "triangle")) {
        struct option opts[] = {firstColor, secondColor, APoint, BPoint, CPoint, Fill, lineWidth};
        key = getopt_long(argc, argv, "c:C:A:B:D:fV:", opts, &i);
        while (key != -1) {
            switch (key) {
                case 'c':
                    trgb(optarg, &color1);
                    break;
                case 'C':
                    trgb(optarg, &color2);
                    break;
                case 'A':
                    Ay=atoi(strtok(optarg,";"));
                    Ax=atoi(strtok(NULL,";"));
                    if((Ay >= picture.infoHead.width) || (Ax >= picture.infoHead.height)){
                        wprintf(L"слишком большие координаты точки А");
                        return 1;
                    }

                    break;
                case 'B':
                    By=atoi(strtok(optarg,";"));
                    Bx=atoi(strtok(NULL,";"));
                    if((By >= picture.infoHead.width) || (Bx >= picture.infoHead.height)){
                        wprintf(L"слишком большие координаты точки B");
                        return 1;
                    }

                    break;
                case 'D':
                    Cy=atoi(strtok(optarg,";"));
                    Cx=atoi(strtok(NULL,";"));
                    if((Cy >= picture.infoHead.width) || (Cx >= picture.infoHead.height)){
                        wprintf(L"слишком большие координаты точки C");
                        return 1;
                    }

                    break;
                case 'f':
                    zalivka = 1;
                    break;
                case 'V':
                    w=atoi(optarg);
                    break;
                case '?':
                    wprintf(L" нечитаемый ключ ");
                    break;
            }
            key = getopt_long(argc, argv, "c:C:A:B:D:fV:", opts, &i);
        }
        if (zalivka == 1) {
            Palit(Ax, Ay, Bx, By, Cx, Cy, &(picture.pixels), picture.infoHead.height, picture.infoHead.width,
                  &color2);
            drawLine(Ax, Ay, Cx, Cy, &(picture.pixels), w, picture.infoHead.height, picture.infoHead.width,
                     &color1);
            drawLine(Bx, By, Cx, Cy, &(picture.pixels), w, picture.infoHead.height, picture.infoHead.width,
                     &color1);
            drawLine(Ax, Ay, Bx, By, &(picture.pixels), w, picture.infoHead.height, picture.infoHead.width,
                     &color1);
            BMPWRITE(outfile, &picture);
            return 0;
        }
        else{
            drawLine(Ax, Ay, Cx, Cy, &(picture.pixels), w, picture.infoHead.height, picture.infoHead.width,
                     &color1);
            drawLine(Bx, By, Cx, Cy, &(picture.pixels), w, picture.infoHead.height, picture.infoHead.width,
                     &color1);
            drawLine(Ax, Ay, Bx, By, &(picture.pixels), w, picture.infoHead.height, picture.infoHead.width,
                     &color1);
            BMPWRITE(outfile, &picture);
            return 0;
        }
    }
    else if (!strcmp(argv[1], "rectfind")) {
        struct option opts[] = {{"color", required_argument, NULL, 'c'},
                                {"new-color", required_argument, NULL, 'C'},
                                {NULL, 0, NULL, 0}};
        key = getopt_long(argc, argv, "c:C:", opts, &i);
        while (key != -1) {
            switch (key) {
                case 'c':
                    trgb(optarg, &color1);
                    break;
                case 'C':
                    trgb(optarg, &color2);
                    break;
                case '?':
                    printf("Нераспознанный ключ\n");
                    break;
            }
            key = getopt_long(argc, argv, "c:C:", opts, &i);
        }
        findAndColorRectangle(&picture, color1, color2);
        BMPWRITE(outfile, &picture);
    }
    else if (!strcmp(argv[1], "collage")) {
        struct option opts[] = {xPic, yPic};
        key = getopt_long(argc, argv, "X:Y:", opts, &i);
        while (key != -1) {
            switch (key) {
                case 'X':
                    x=atoi(optarg);
                    break;
                case 'Y':
                    y=atoi(optarg);
                    break;
                case '?':
                    wprintf(L" нечитаемый ключ ");
                    break;
            }
            key = getopt_long(argc, argv, "X:Y:", opts, &i);
        }
        collage(&picture,x,y);
        BMPWRITE(outfile, &picture);
        return 0;
    }
    else if (!strcmp(argv[1], "write")) {
        BMPWRITE(outfile, &picture);
    }
    else {
        wprintf(L"Команды %hs не существует\n", argv[1]);
    }

    return 0;
}
