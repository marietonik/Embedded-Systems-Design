#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define N 372 /* frame dimension for QCIF format */
#define M 496 /* frame dimension for QCIF format */
#define filename "C://Users//Marietonik//Desktop//improved codes//cherry_496x372_444.yuv"
#define file_yuv "C://Users//Marietonik//Desktop//improved codes//cherry_yuv_output.yuv"

/* code for armulator*/
#pragma arm section zidata="ram"
int current_y[N][M];
int current_u[N][M];
int current_v[N][M];
#pragma arm section

int current_y_new[N][M],channel[N][M];
int hist[256],histogram[256];
int i,j,k,g,t1_int,t2_int;
double m,p1,p2,t1,t2;
double sum=0,n_low=0,m_high=0,weights=0,sum_weights=0;

void read()
{
  FILE *frame_c;
  if((frame_c=fopen(filename,"rb"))==NULL)
  {
    printf("current frame doesn't exist\n");
    exit(-1);
  }

  for(i=N-1;i>=0;i--)
  {
    for(j=M-1;j>=0;j--)
    {
      current_y[i][j]=fgetc(frame_c);
    }
  }
  for(i=N-1;i>=0;i--)
  {
    for(j=M-1;j>=0;j--)
    {
      current_u[i][j]=fgetc(frame_c);
    }
  }
  for(i=N-1;i>=0;i--)
  {
    for(j=M-1;j>=0;j--)
    {
      current_v[i][j]=fgetc(frame_c);
    }
  }

  fclose(frame_c);
}

void write()
{
  FILE *frame_yuv;
  frame_yuv=fopen(file_yuv,"wb");

  for(i=N-1;i>=0;i--)
  {
    for(j=M-1;j>=0;j--)
    {
      fputc(current_y_new[i][j],frame_yuv);
    }
  }

  for(i=N-1;i>=0;i--)
  {
    for(j=M-1;j>=0;j--)
    {
      fputc(current_u[i][j],frame_yuv);
    }
  }

  for(i=N-1;i>=0;i--)
  {
    for(j=M-1;j>=0;j--)
    {
      fputc(current_v[i][j],frame_yuv);
    }
  }
  fclose(frame_yuv);
}


void printhist_mean(int histogram[256],int channel[N][M])
{
  for(k=255;k>=0;k--)
  {
    for(i=N-1;i>=0;i--)
    {
      for(j=M-1;j>=0;j--)
      {
        if(channel[i][j]==(255-k))
        {
          hist[255-k]++;
        }
      }
    }

    weights=(255-k)*hist[255-k];
    sum_weights+=weights;
    sum+=hist[255-k];
  }
  m=sum_weights/sum;

  printf("___________________________________________\n\n");
  printf("  Number of index:      Occurrency: \n");
  printf("___________________________________________\n\n");

  for(k=255;k>=0;k--)
  {
    printf("         %d          |",(255-k));
    printf("         %d         ",hist[255-k]);
    printf("\n");
    printf("___________________________________________\n");
    printf("\n");
  }
}


void check(int channel[N][M])
{
  for(i=N-1;i>=0;i--)
  {
    for(j=M-1;j>=0;j--)
    {
      g=channel[i][j];
      
      if(g<=m)
      {       
        n_low++;
        current_y_new[i][j]=t1_int;
      }
      else if(g>m)
      {
        m_high++;
        current_y_new[i][j]=t2_int;
      }
    }
  }

  for(k=255;k>=0;k--){
    p1+=hist[255-k]/n_low;
    t1=m*p1;
    t1_int=t1;

    if((256-k)>=m+1){
        p2+=hist[255-k]/m_high;
        t2=(255-m)*p2;
        t2_int=t2;
    }
  }
}

int main()
{
  read();
  printhist_mean(hist,current_y);
  check(current_y);
  write();
}
