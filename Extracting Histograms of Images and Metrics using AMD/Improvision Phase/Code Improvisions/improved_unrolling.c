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

  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j=j+4)
    {
      current_y[i][j]=fgetc(frame_c);
      current_y[i][j+1]=fgetc(frame_c);
      current_y[i][j+2]=fgetc(frame_c);
      current_y[i][j+3]=fgetc(frame_c);
    }
  }
  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j=j+4)
    {
      current_u[i][j]=fgetc(frame_c);
      current_u[i][j+1]=fgetc(frame_c);
      current_u[i][j+2]=fgetc(frame_c);
      current_u[i][j+3]=fgetc(frame_c);
    }
  }
  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j=j+4)
    {
      current_v[i][j]=fgetc(frame_c);
      current_v[i][j+1]=fgetc(frame_c);
      current_v[i][j+2]=fgetc(frame_c);
      current_v[i][j+3]=fgetc(frame_c);
    }
  }

  fclose(frame_c);
}

void write()
{
  FILE *frame_yuv;
  frame_yuv=fopen(file_yuv,"wb");

  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j=j+4)
    {
      fputc(current_y_new[i][j],frame_yuv);
      fputc(current_y_new[i][j+1],frame_yuv);
      fputc(current_y_new[i][j+2],frame_yuv);
      fputc(current_y_new[i][j+3],frame_yuv);
    }
  }

  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j=j+4)
    {
      fputc(current_u[i][j],frame_yuv);
      fputc(current_u[i][j+1],frame_yuv);
      fputc(current_u[i][j+2],frame_yuv);
      fputc(current_u[i][j+3],frame_yuv);
    }
  }

  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j=j+4)
    {
      fputc(current_v[i][j],frame_yuv);
      fputc(current_v[i][j+1],frame_yuv);
      fputc(current_v[i][j+2],frame_yuv);
      fputc(current_v[i][j+3],frame_yuv);
    }
  }
  fclose(frame_yuv);
}

void h(int channel[N][M])
{
  for(k=0;k<256;k++)
  {
    for(i=0;i<N;i++)
    {
      for(j=0;j<M;j=j+4)
      {
        if(channel[i][j]==k){
          hist[k]++;
        }
        if(channel[i][j+1]==k){
          hist[k]++;
        }
        if(channel[i][j+2]==k){
          hist[k]++;
        }
        if(channel[i][j+3]==k){
          hist[k]++;
        }
      }
    }
  }
}

void printhist(int histogram[256])
{
  printf("___________________________________________\n\n");
  printf("  Number of index:      Occurrency: \n");
  printf("___________________________________________\n\n");

  for(k=0;k<256;k=k+4)
  {
    printf("         %d          |",k);
    printf("         %d         ",hist[k]);
    printf("\n");
    printf("___________________________________________\n");
    printf("\n");

    printf("         %d          |",(k+1));
    printf("         %d         ",hist[k+1]);
    printf("\n");
    printf("___________________________________________\n");
    printf("\n");

    printf("         %d          |",(k+2));
    printf("         %d         ",hist[k+2]);
    printf("\n");
    printf("___________________________________________\n");
    printf("\n");

    printf("         %d          |",(k+3));
    printf("         %d         ",hist[k+3]);
    printf("\n");
    printf("___________________________________________\n");
    printf("\n");
  }
}

double mean(int histogram[256])
{
  for(k=0;k<256;k=k+4){
    weights=k*hist[k];
    sum_weights+=weights;
    sum+=hist[k];

    weights=(k+1)*hist[k+1];
    sum_weights+=weights;
    sum+=hist[k+1];

    weights=(k+2)*hist[k+2];
    sum_weights+=weights;
    sum+=hist[k+2];

    weights=(k+3)*hist[k+3];
    sum_weights+=weights;
    sum+=hist[k+3];
  }
  m=sum_weights/sum;
  return m;
}

void check(int channel[N][M])
{
  for(i=0;i<N;i++)
  {
    for(j=0;j<M;j=j+4)
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

      g=channel[i][j+1];

      if(g<=m)
      {       
        n_low++;
        current_y_new[i][j+1]=t1_int;
      }
      else if(g>m)
      {
        m_high++;
        current_y_new[i][j+1]=t2_int;
      }

      g=channel[i][j+2];

      if(g<=m)
      {       
        n_low++;
        current_y_new[i][j+2]=t1_int;
      }
      else if(g>m)
      {
        m_high++;
        current_y_new[i][j+2]=t2_int;
      }

      g=channel[i][j+3];

      if(g<=m)
      {       
        n_low++;
        current_y_new[i][j+3]=t1_int;
      }
      else if(g>m)
      {
        m_high++;
        current_y_new[i][j+3]=t2_int;
      }
    }
  }

  for(k=0;k<256;k=k+4){
    p1+=hist[k]/n_low;
    p1+=hist[k+1]/n_low;
    p1+=hist[k+2]/n_low;
    p1+=hist[k+3]/n_low;
    t1=m*p1;
    t1_int=t1;

    if((k>=m+1)||((k+1)>=m+1)||((k+2)>=m+1)||((k+3)>=m+1)){
        p2+=hist[k]/m_high;
        p2+=hist[k+1]/m_high;
        p2+=hist[k+2]/m_high;
        p2+=hist[k+3]/m_high;
        t2=(255-m)*p2;
        t2_int=t2;
    }
  }
}

int main()
{
  read();
  h(current_y);
  printhist(hist);
  mean(hist);
  check(current_y);
  write();
   printf("%f",m);
}
