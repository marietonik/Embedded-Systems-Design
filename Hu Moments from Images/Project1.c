#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void read_rawimage(char *fname, unsigned long height, unsigned long width, unsigned char **image){
  unsigned long i;
  FILE *file;
  
  file=fopen(fname,"r");
  for (i=0; i<height; i++) fread(image[i], 1, width, file);
  fclose(file);
}

void write_rawimage(char *fname, unsigned long height, unsigned long width, unsigned char **image){
  unsigned long i;
  FILE *file;
  
  file=fopen(fname,"w");
  for (i=0; i<height; i++) fwrite(image[i], 1, width, file);
  fclose (file);
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
for (i = 0; i < height; i++)
    inimg[i] = malloc(width * sizeof(unsigned char));
read_rawimage(infname, height, width, inimg);

// Commented out code
// outimg = (unsigned char **)malloc(height * sizeof(unsigned char *));
// for (i = 0; i < height; i++) outimg[i] = malloc(width * sizeof(unsigned char));

// t1 -> κλήση συνάρτησης χρόνου

// Εδώ θα καλέσετε τη συνάρτηση που υλοποιεί τη ζητούμενη λειτουργία
// ή θα συμπεριλάβετε τη λειτουργία στη main(), π.χ. :
// for (i = 0; i < height; i++) for (j = 0; j < width; j++) outimg[i][j] = inimg[i][j];

// t2 -> κλήση συνάρτησης χρόνου

// Αν η έξοδος σας είναι εικόνα αποθηκεύστε την με τον παρακάτω κώδικα
// write_rawimage(outfname, height, width, outimg);
// free(outimg);

free(inimg);
}
