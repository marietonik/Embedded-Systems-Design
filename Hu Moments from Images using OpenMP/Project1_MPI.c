#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

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

double Calculate_Hu_central_moment(unsigned int *sent_buffer, unsigned long height, unsigned long width, float centroid_x, float centroid_y, int p, int q){
  
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  unsigned int central_moment_pq=0, central_moment_00=0;
  unsigned long i,j;
  double central_moment_pq_norm;

  for (i = 0; i < height / size; i++) {
    for (j = 0; j < width; j++) {
        central_moment_00 += (j-centroid_x) * (i-centroid_y) * sent_buffer[i*width+j];
        central_moment_pq += pow((j-centroid_x),p) * pow((i-centroid_y),q) * sent_buffer[i*width+j];
    }
  }

  central_moment_pq_norm = (double)(central_moment_pq) / pow((double)(central_moment_00), 1+(p+q) / 2.0);
  return central_moment_pq_norm;
}

unsigned char** processImage(unsigned char **input_img, unsigned char **output_img, unsigned long height, unsigned long width) {
  
  int rank, size, *sendcounts, *displs, scatterResult;
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  printf("Debug Point 1 - Process %d of %d\n", rank, size);
  
  unsigned int *processImg, *grayscaleVector, *binaryVector, *sent_buffer;
  unsigned char *input_img_vector, *rec_buffer;
  unsigned long i, j;
  unsigned int sum_x=0, sum_y=0, total=0;
  float centroid_x, centroid_y;
  double eta00, eta20, eta02, eta11, eta12, eta21, eta03, eta30;
  double Hu1, Hu2, Hu3, Hu4, Hu5, Hu6, Hu7;
  double norm_hu1, norm_hu2, norm_hu3, norm_hu4, norm_hu5, norm_hu6, norm_hu7;
  double start_time_parallel, end_time_parallel, start_allocation_time, end_allocation_time, time_parallel;

  start_allocation_time = MPI_Wtime();

  if (rank == 0) {
  input_img_vector = (unsigned char *)malloc(height * width * sizeof(unsigned char));
  
  if (input_img_vector == NULL) {
    perror("Error allocating memory for input image");
    exit(-1);
  }

  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
        input_img_vector[i * width + j] = input_img[i][j];
    }
  }
  }

  // Δέσμευση μνήμης διανυσμάτων σε όλες τις διεργασίες
  processImg = (unsigned int *)malloc(height * width / size * sizeof(unsigned int));
  grayscaleVector = (unsigned int *)malloc(height * width / size * sizeof(unsigned int));
  binaryVector = (unsigned int *)malloc(height * width / size * sizeof(unsigned int));

  // Δέσμευση μνήμης rec_buffer σε όλες τις διεργασίες
  rec_buffer = (unsigned char *)malloc(width * height / size * sizeof(unsigned char));
  // Δέσμευση μνήμης sent_buffer σε όλες τις διεργασίες
  sent_buffer = (unsigned int *)malloc(width * height / size * sizeof(unsigned int));

  MPI_Barrier(MPI_COMM_WORLD);
  end_allocation_time = MPI_Wtime();
  start_time_parallel = MPI_Wtime();

  // Μέθοδος διαχείρισης υπολοίπου δεδομένων μεταξύ του διαμοιρασμού εικόνας στον αριθμό των πυρήνων
  // Ορισμός sendcounts and displs για MPI_Scatterv και MPI_Gatherv
  sendcounts = (int *)malloc(size * sizeof(int));
  displs = (int *)malloc(size * sizeof(int));

  // Μέτρηση αριθμού σειρών που θα αναλάβει κάθε διεργασία
  int remaining_rows = height % size;
  int base_rows = height / size;

  // Προσάρμοσε sendcounts και displs για τις σειρές που υπολοίπονται
  for (i = 0; i < size; i++) {
      sendcounts[i] = base_rows * width;
      if (i < remaining_rows) {
          sendcounts[i] += width;
      }

      displs[i] = i * base_rows * width;
      if (i < remaining_rows) {
          displs[i] += i * width;
      } else {
          displs[i] += remaining_rows * width;
      }
  }

  //MPI_Barrier(MPI_COMM_WORLD);
  MPI_Scatterv(input_img_vector, sendcounts, displs, MPI_UNSIGNED_CHAR, rec_buffer, sendcounts[rank], MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);

  // After MPI_Scatterv
  printf("Debug Point 3 - Process %d: After Scatter\n", rank);
  
  for (i = 0; i < height / size; i++) {
    for (j = 0; j < width; j++) {
        // ASCII σε Integers
        processImg[i*width+j] = (int)(rec_buffer[i*width+j]-'0');
        // Grayscale 8-bit μετατροπή
        grayscaleVector[i*width+j] = (processImg[i*width+j] * 255) / 255;
        // Binary μετατροπή
        sent_buffer[i*width+j] = (grayscaleVector[i*width+j] < 128) ? 0 : 255;
        // Υπολογισμός Centroids
        sum_x += j * sent_buffer[i*width+j];
        sum_y += i * sent_buffer[i*width+j];
        total += sent_buffer[i*width+j];
    }
  }

  centroid_x = sum_x / total;
  centroid_y = sum_y / total;

  // Υπολογισμός κεντρικών ροπών και κανονονικοποίηση τιμών
  eta00 = Calculate_Hu_central_moment(sent_buffer,height,width,centroid_x,centroid_y,0,0);
  eta20 = Calculate_Hu_central_moment(sent_buffer,height,width,centroid_x,centroid_y,2,0)/(eta00 * eta00);
  eta02 = Calculate_Hu_central_moment(sent_buffer,height,width,centroid_x,centroid_y,0,2)/(eta00 * eta00);
  eta11 = Calculate_Hu_central_moment(sent_buffer,height,width,centroid_x,centroid_y,1,1)/(eta00 * eta00);
  eta30 = Calculate_Hu_central_moment(sent_buffer,height,width,centroid_x,centroid_y,3,0)/(eta00 * eta00);
  eta12 = Calculate_Hu_central_moment(sent_buffer,height,width,centroid_x,centroid_y,1,2)/(eta00 * eta00);
  eta21 = Calculate_Hu_central_moment(sent_buffer,height,width,centroid_x,centroid_y,2,1)/(eta00 * eta00);
  eta03 = Calculate_Hu_central_moment(sent_buffer,height,width,centroid_x,centroid_y,0,3)/(eta00 * eta00);


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

  // Gather the results back to the root process
  MPI_Gatherv(sent_buffer, sendcounts[rank], MPI_UNSIGNED, binaryVector, sendcounts, displs, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

  //MPI_Barrier(MPI_COMM_WORLD);
  end_time_parallel = MPI_Wtime();

  if (rank == 0) {

  printf("\nCentroid X: %f\n", centroid_x);
  printf("Centroid Y: %f\n", centroid_y);
  printf("Total: %u\n", total);

  // Εκτύπωση Hu moments
  printf("Hu Moments:\n");
  printf("Hu1: %.20f\n", norm_hu1);
  printf("Hu2: %.20f\n", norm_hu2);
  printf("Hu3: %.20f\n", norm_hu3);
  printf("Hu4: %.20f\n", norm_hu4);
  printf("Hu5: %.20f\n", norm_hu5);
  printf("Hu6: %.20f\n", norm_hu6);
  printf("Hu7: %.20f\n", norm_hu7);

  time_parallel = end_time_parallel - start_time_parallel;
  printf("\n\nElapsed time for allocation only: %f seconds\n", end_allocation_time - start_allocation_time);
  printf("Elapsed parallel time: %f seconds\n\n", time_parallel);

  free(rec_buffer);
  free(sent_buffer);

  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
        // Integers σε ASCII
        output_img[i][j] = (char)(binaryVector[i*width+j]+'0');
    }
  }
  }

  free(input_img_vector);
  free(processImg);
  free(grayscaleVector);
  free(binaryVector);
  free(sendcounts);
  free(displs);

  return output_img;
}

void main(int argc, char *argv[]) {

char infname[50], outfname[50];
unsigned char **inimg, **outimg;
unsigned long height, width, i;
int rank, size;

MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);

// Ensure at least 2 processes are available
if (size < 2) {
    fprintf(stderr, "This program requires at least 2 processes.\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
}

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
MPI_Finalize();
}
