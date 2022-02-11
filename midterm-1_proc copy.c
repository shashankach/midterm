#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

typedef unsigned char BYTE;

typedef struct BITMAPFILEHEADER
{
    WORD bfType;  //specifies the file type
    DWORD bfSize;  //specifies the size in bytes of the bitmap file
    WORD bfReserved1;  //reserved; must be 0
    WORD bfReserved2;  //reserved; must be 0
    DWORD bfOffBits;  //specifies the offset in bytes from the bitmapfileheader to the bitmap bits
}BITMAPFILEHEADER;


typedef struct BITMAPINFOHEADER
{
    DWORD biSize;  //specifies the number of bytes required by the struct
    LONG biWidth;  //specifies width in pixels
    LONG biHeight;  //specifies height in pixels
    WORD biPlanes;  //specifies the number of color planes, must be 1
    WORD biBitCount;  //specifies the number of bits per pixel
    DWORD biCompression;  //specifies the type of compression
    DWORD biSizeImage;  //size of image in bytes
    LONG biXPelsPerMeter;  //number of pixels per meter in x axis
    LONG biYPelsPerMeter;  //number of pixels per meter in y axis
    DWORD biClrUsed;  //number of colors used by the bitmap
    DWORD biClrImportant;  //number of colors that are important
}BITMAPINFOHEADER;

typedef struct col
{ //below is the structure “col” described
    int r,g,b; //red, green and blue, in that order. The values will not exceed 255!
} col;


typedef struct compressedformat
{
    int width, height; //width and height of the image, with one byte for each color, blue, green and red

    int rowbyte_quarter[4];//for parallel algorithms! That’s the location in bytes which exactly splits the result image after decompression into 4 equal parts!
    int palettecolors; //how many colors does the picture have?
    col *colors; //all participating colors of the image. Further
} compressedformat;


typedef struct chunk
{
    BYTE color_index; //which of the color of the palette
    short count;//How many pixel of the same color from color_index are continuously appearing
} chunk;



void WriteRGB(BYTE* image, int x, int y, int width, BYTE r, BYTE g, BYTE b){
    image[3*x + 3*y*width + 2] = r;
    image[3*x + 3*y*width + 1] = g;
    image[3*x + 3*y*width + 0] = b;
}

BYTE getRed(BYTE *image, int x, int y, int width){
    return image[3*x + 3*y*width + 2];
}

BYTE getGreen(BYTE *image, int x, int y, int width){
    return image[3*x + 3*y*width + 1];
}

BYTE getBlue(BYTE *image, int x, int y, int width){
    return image[3*x + 3*y*width + 0];
}

int numcap(int num){
    if(num > 255){
        num = 255;
    }
    return num;
}





/*
fread width, height, rowbyte [size(int) * 4], pallete
colors = malloc(pallete * sizeof(col))
fread colors

*/

int main(){
    int numcolors;
    BYTE color_ind;
    short count;
    BITMAPFILEHEADER fh;
    BITMAPINFOHEADER fih;

    chunk c;
    int chunkno = 0;
    compressedformat f;
    FILE *file = fopen("compressed.bin", "rb");

    fread(&f.width, sizeof(int), 1, file);
    fread(&f.height, sizeof(int), 1, file);
    fread(&f.rowbyte_quarter, sizeof(int) * 4, 1, file);
    fread(&f.palettecolors, sizeof(int), 1, file);
    f.colors = malloc(f.palettecolors * sizeof(col));
    for (int i=0; i<f.palettecolors; i++){
        int a;
        fread(&a, sizeof(int), 1, file);
        f.colors[i].r = a;
        fread(&a, sizeof(int), 1, file);
        f.colors[i].g = a;
        fread(&a, sizeof(int), 1, file);
        f.colors[i].b = a;
    }

    // for (int i=0; i<f.palettecolors; i++){
    //     col c = f.colors[i];
    //     printf("%d-%d-%d\n", c.r, c.g, c.b);
    // }
    
    //printf("1\n");
    //fread(&f.rowbyte_quarter,sizeof(int)*4,1,file);
    int chunknum = f.rowbyte_quarter[3]/3;
    int csize = sizeof(BYTE)*chunknum;
    int *chunksizelist = (int*) malloc(sizeof(int)*chunknum);
    BYTE *chunkcolorlist = (BYTE*) malloc(sizeof(BYTE)*chunknum);
    int pixelcount = 0;
    csize = 0;
    //printf("EOF\n");
    //printf("2\n");
    for(int i=0;i<f.rowbyte_quarter[0];i++){

        fread(&c.color_index, sizeof(BYTE), 1, file);
        fread(&c.count, sizeof(short), 1, file);
        pixelcount += c.count;
        chunksizelist[i] = c.count;
        chunkcolorlist[i] = c.color_index;
        csize++;
    }
    for(int i=f.rowbyte_quarter[0];i<f.rowbyte_quarter[1];i++){

        fread(&c.color_index, sizeof(BYTE), 1, file);
        fread(&c.count, sizeof(short), 1, file);
        pixelcount += c.count;
        chunksizelist[i] = c.count;
        chunkcolorlist[i] = c.color_index;
        csize++;
    }
    for(int i=f.rowbyte_quarter[1];i<f.rowbyte_quarter[2];i++){

        fread(&c.color_index, sizeof(BYTE), 1, file);
        fread(&c.count, sizeof(short), 1, file);
        pixelcount += c.count;
        chunksizelist[i] = c.count;
        chunkcolorlist[i] = c.color_index;
        csize++;
    }

    for(int i=f.rowbyte_quarter[2];i<f.rowbyte_quarter[3];i++){

        fread(&c.color_index, sizeof(BYTE), 1, file);
        fread(&c.count, sizeof(short), 1, file);
        pixelcount += c.count;
        chunksizelist[i] = c.count;
        chunkcolorlist[i] = c.color_index;
        csize++;
        }






    fclose(file);
    pixelcount *= 3;
    BYTE *img = (BYTE *) mmap(NULL, pixelcount, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    int n=0;
    int x=1;
    int y=1;


    for(int i=0; i<f.rowbyte_quarter[0]; i++){
        for(int j=0; j<chunksizelist[i]; j++){
           
        
            BYTE currcolr = (BYTE) f.colors[chunkcolorlist[i]].r;
            BYTE currcolg = (BYTE) f.colors[chunkcolorlist[i]].g;
            BYTE currcolb = (BYTE) f.colors[chunkcolorlist[i]].b;
            img[x] = currcolb;
            img[x+1] = currcolg;
            img[x+2] = currcolr;
            x+=3;
           
        }
    }
      for(int i=f.rowbyte_quarter[0]; i<f.rowbyte_quarter[1]; i++){
        for(int j=0; j<chunksizelist[i]; j++){
           
        
            BYTE currcolr = (BYTE) f.colors[chunkcolorlist[i]].r;
            BYTE currcolg = (BYTE) f.colors[chunkcolorlist[i]].g;
            BYTE currcolb = (BYTE) f.colors[chunkcolorlist[i]].b;
            img[x] = currcolb;
            img[x+1] = currcolg;
            img[x+2] = currcolr;
            x+=3;
           
        }
    }
      for(int i=f.rowbyte_quarter[1]; i<f.rowbyte_quarter[2]; i++){
        for(int j=0; j<chunksizelist[i]; j++){
           
        
            BYTE currcolr = (BYTE) f.colors[chunkcolorlist[i]].r;
            BYTE currcolg = (BYTE) f.colors[chunkcolorlist[i]].g;
            BYTE currcolb = (BYTE) f.colors[chunkcolorlist[i]].b;
            img[x] = currcolb;
            img[x+1] = currcolg;
            img[x+2] = currcolr;
            x+=3;
           
        }
    }
      for(int i=f.rowbyte_quarter[2]; i<f.rowbyte_quarter[3]; i++){
        for(int j=0; j<chunksizelist[i]; j++){
           
        
            BYTE currcolr = (BYTE) f.colors[chunkcolorlist[i]].r;
            BYTE currcolg = (BYTE) f.colors[chunkcolorlist[i]].g;
            BYTE currcolb = (BYTE) f.colors[chunkcolorlist[i]].b;
            img[x] = currcolb;
            img[x+1] = currcolg;
            img[x+2] = currcolr;
            x+=3;
           
        }
    }
    
    fh.bfType = 19778;
    fh.bfSize = 4320054;
    fh.bfReserved1 =0;
    fh.bfReserved2=0;
    fh.bfOffBits=54;

    fih.biSizeImage = 4320000;
    fih.biSize = 40;
    fih.biWidth = 1200;
    fih.biHeight = 1200;
    fih.biPlanes=1;
    fih.biBitCount=24;
    fih.biCompression=0;
    fih.biXPelsPerMeter=3780;
    fih.biYPelsPerMeter=3780;
    fih.biClrUsed=0;
    fih.biClrImportant=0;

    file = fopen("new.bmp","wb");
    fwrite(&fh.bfType,2,1,file);
    fwrite(&fh.bfSize,sizeof(int),1,file);
    fwrite(&fh.bfReserved1,sizeof(short),1,file);
    fwrite(&fh.bfReserved2,sizeof(short),1,file);
    fwrite(&fh.bfOffBits,sizeof(int),1,file);
    fwrite(&fih,sizeof(BITMAPINFOHEADER),1,file);
    fwrite(img,fih.biSizeImage,1,file);
    fclose(file);
 
}
