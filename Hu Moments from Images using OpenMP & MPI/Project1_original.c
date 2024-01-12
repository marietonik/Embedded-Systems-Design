#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

double Calculate_Hu_central_moment(unsigned int *binaryVector, unsigned long height, unsigned long width, float centroid_x, float centroid_y, int p, int q){
  unsigned int central_moment_pq=0, central_moment_00=0;
  unsigned long i,j;
  double central_moment_pq_norm;

  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
        central_moment_00 += (j-centroid_x) * (i-centroid_y) * binaryVector[i*width+j];
        central_moment_pq += pow((j-centroid_x),p) * pow((i-centroid_y),q) * binaryVector[i*width+j];
    }
  }
  
  central_moment_pq_norm = (double)(central_moment_pq) / pow((double)(central_moment_00), 1+(p+q) / 2.0);
  return central_moment_pq_norm;
}

unsigned char** processImage(unsigned char **input_img, unsigned char **output_img, unsigned long height, unsigned long width) {
  unsigned int *processImg, *grayscaleVector, *binaryVector;
  unsigned long i, j;
  unsigned int sum_x=0, sum_y=0, total=0;
  float centroid_x, centroid_y;
  double eta00, eta20, eta02, eta11, eta12, eta21, eta03, eta30;
  double Hu1, Hu2, Hu3, Hu4, Hu5, Hu6, Hu7;
  double norm_hu1, norm_hu2, norm_hu3, norm_hu4, norm_hu5, norm_hu6, norm_hu7;

  processImg = (unsigned int *)malloc(height * width * sizeof(unsigned int));
  printf("\nHeight: %d\n", height);
  printf("Width: %d\n", width);

  if (processImg == NULL) {
    perror("Error allocating memory for processing image");
    exit(-1);
  }

  grayscaleVector = (unsigned int *)malloc(height * width * sizeof(unsigned int));
  binaryVector = (unsigned int *)malloc(height * width * sizeof(unsigned int));

  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
        // ASCII σε Integers
        processImg[i*width+j] = (int)(input_img[i][j]-'0');
        // Grayscale 8-bit μετατροπή
        grayscaleVector[i*width+j] = (processImg[i*width+j] * 255) / 255;
        // Binary μετατροπή
        binaryVector[i*width+j] = (grayscaleVector[i*width+j] < 128) ? 0 : 255;
        // Υπολογισμός Centroids
        sum_x += j * binaryVector[i*width+j];
        sum_y += i * binaryVector[i*width+j];
        total += binaryVector[i*width+j];
    }
  }
  
  centroid_x = sum_x / total;
  centroid_y = sum_y / total;

  printf("\nCentroid X: %f\n", centroid_x);
  printf("Centroid Y: %f\n", centroid_y);
  printf("Total: %u\n", total);

  // Υπολογισμός κεντρικών ροπών και κανονονικοποίηση τιμών
  eta00 = Calculate_Hu_central_moment(binaryVector,height,width,centroid_x,centroid_y,0,0);
  eta20 = Calculate_Hu_central_moment(binaryVector,height,width,centroid_x,centroid_y,2,0)/(eta00 * eta00);
  eta02 = Calculate_Hu_central_moment(binaryVector,height,width,centroid_x,centroid_y,0,2)/(eta00 * eta00);
  eta11 = Calculate_Hu_central_moment(binaryVector,height,width,centroid_x,centroid_y,1,1)/(eta00 * eta00);
  eta30 = Calculate_Hu_central_moment(binaryVector,height,width,centroid_x,centroid_y,3,0)/(eta00 * eta00);
  eta12 = Calculate_Hu_central_moment(binaryVector,height,width,centroid_x,centroid_y,1,2)/(eta00 * eta00);
  eta21 = Calculate_Hu_central_moment(binaryVector,height,width,centroid_x,centroid_y,2,1)/(eta00 * eta00);
  eta03 = Calculate_Hu_central_moment(binaryVector,height,width,centroid_x,centroid_y,0,3)/(eta00 * eta00);


  // Υπολογισμός Hu ροπών - Moments και κανονικοποίηση
  Hu1 = eta20 + eta02;
  Hu2 = (eta20 - eta02) * (eta20 - eta02) + 4 * eta11 * eta11;
  Hu3 = (eta30 - 3 * eta12) * (eta30 - 3 * eta12) + (3 * eta21 - eta03) * (3 * eta21 - eta03);
  Hu4 = (eta30 + eta12) * (eta30 + eta12) + (eta21 + eta03) * (eta21 + eta03);
  Hu5 = (eta30 - 3 * eta12) * (eta30 + eta12) * ((eta30 + eta12) * (eta30 + eta12) - 3 * (eta21 + eta03) * (eta21 + eta03))+ (3 * eta21 - eta03) * (eta21 + eta03) * (3 * (eta30 + eta12) * (eta30 + eta12) - (eta21 + eta03) * (eta21 + eta03));
  Hu6 = (eta20 - eta02) * ((eta30 + eta12) * (eta30 + eta12) - (eta21 + eta03) * (eta21 + eta03)) + 4 * eta11 * (eta30 + eta12) * (eta21 + eta03);
  Hu7 = (3 * eta21 - eta03) * (eta21 + eta03) * (3 * (eta30 + eta12) * (eta30 + eta12) - (eta21 + eta03) * (eta21 + eta03))- (eta30 - 3 * eta12) * (eta21 + eta03) * (3 * (eta30 + eta12) * (eta30 + eta12) - (eta21 + eta03) * (eta21 + eta03));

  norm_hu1 = -logf(fabsf(Hu1));
  norm_hu2 = -logf(fabsf(Hu2));
  norm_hu3 = -logf(fabsf(Hu3));
  norm_hu4 = -logf(fabsf(Hu4));
  norm_hu5 = -logf(fabsf(Hu5));
  norm_hu6 = -logf(fabsf(Hu6));
  norm_hu7 = -logf(fabsf(Hu7));

  // Εκτύπωση Hu moments
  printf("Hu Moments:\n");
  printf("Hu1: %.20f\n", norm_hu1);
  printf("Hu2: %.20f\n", norm_hu2);
  printf("Hu3: %.20f\n", norm_hu3);
  printf("Hu4: %.20f\n", norm_hu4);
  printf("Hu5: %.20f\n", norm_hu5);
  printf("Hu6: %.20f\n", norm_hu6);
  printf("Hu7: %.20f\n", norm_hu7);

  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
        // Integers σε ASCII
        output_img[i][j] = (char)(binaryVector[i*width+j]+'0');
    }
  }
  free(processImg);
  free(grayscaleVector);
  free(binaryVector);
  return output_img;
}

void main(int argc, char *argv[]) {
char infname[50], outfname[50];
unsigned char **inimg, **outimg;
unsigned long height, width, i;

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

outimg = processImage(inimg, outimg, height, width);

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
