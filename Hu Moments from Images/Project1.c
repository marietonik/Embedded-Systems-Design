#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void read_rawimage(char *fname, unsigned long height, unsigned long width, unsigned char **image){
  unsigned long i;
  FILE *file;
  
  file=fopen(fname,"rb");
  if (file == NULL) {
    perror("Error opening input file");
    exit(-1);
  }

  for (i=0; i<height; i++) {
    fread(image[i], 1, width, file);
  }

  fclose(file);
}

void write_rawimage(char *fname, unsigned long height, unsigned long width, unsigned char **image){
  unsigned long i;
  FILE *file;
  
  file=fopen(fname,"wb");

  if (file == NULL) {
    perror("Error opening output file");
    exit(-1);
  }

  for (i=0; i<height; i++) {
    fwrite(image[i], 1, width, file);
  }
  fclose (file);
}

unsigned char** processImage(unsigned char **input_img, unsigned char **output_img, unsigned long height, unsigned long width) {
  unsigned char *processImg;
  unsigned long i, j;

  processImg = (unsigned char *)malloc(height * width * sizeof(unsigned char));
  if (processImg == NULL) {
    perror("Error allocating memory for processing image");
    exit(-1);
  }

  for (i = 0; i < height; i++)
    for (j = 0; j < width; j++)
        processImg[i*width+j] = input_img[i][j];

  for (i = 0; i < height; i++)
    for (j = 0; j < width; j++)
        output_img[i][j] = processImg[i*width+j];

  free(processImg);
  return output_img;
}

void main(int argc, char *argv[]) {
char infname[50], outfname[50];
unsigned char **inimg, **outimg;
unsigned long height, width, i, j;

if (argc < 4) {
    printf("usage is: %s inimg height width [outimg]\n", argv[0]);
    exit(-1);
}

strcpy(infname, argv[1]);
height = (unsigned long)atoi(argv[2]);
width = (unsigned long)atoi(argv[3]);

inimg = (unsigned char **)malloc(height * sizeof(unsigned char *));

if (inimg == NULL) {
  perror("Error allocating memory for input image rows");
  exit(-1);
}

for (i = 0; i < height; i++) {
    inimg[i] = malloc(width * sizeof(unsigned char));
    if (inimg[i] == NULL) {
      perror("Error allocating memory for input image row");
      exit(-1);
    }
}

read_rawimage(infname, height, width, inimg);

// Commented out code
outimg = (unsigned char **)malloc(height * sizeof(unsigned char *));
if (outimg == NULL) {
    perror("Error allocating memory for output image rows");
    exit(-1);
}

for (i = 0; i < height; i++) {
  outimg[i] = malloc(width * sizeof(unsigned char));
  if (outimg[i] == NULL) {
      perror("Error allocating memory for output image row");
      exit(-1);
  }
}

// t1 -> κλήση συνάρτησης χρόνου

// Εδώ θα καλέσετε τη συνάρτηση που υλοποιεί τη ζητούμενη λειτουργία
// Allocate memory for the flattened version (for processing)
// Εκκίνηση της επεξεργασίας εικόνας

outimg = processImage(inimg, outimg, height, width);

// t2 -> κλήση συνάρτησης χρόνου

// Αν η έξοδος σας είναι εικόνα αποθηκεύστε την με τον παρακάτω κώδικα
strcpy(outfname, "output_image.raw");
write_rawimage(outfname, height, width, outimg);

// Free allocated memory
for (i = 0; i < height; i++) {
    free(inimg[i]);
    free(outimg[i]);
}
free(inimg);
free(outimg);
}
