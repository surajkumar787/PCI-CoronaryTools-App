// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


/*************************************************************************************/
// MkxMarkerRefining.c  PL  
/*************************************************************************************/
#include <stdlib.h> 
#include <math.h> 

#include <MkxDev.h> 

#define MKX_DEBUG_PRINT_PROFILE 0
#define MY_BIG_FLOAT (float)3.4E+38

typedef struct{
  float g1, g2, e, a, x, y, c; //g for grey level, c for contrast
  int a1, a2;
  int Flag;
  int N;
}Profile;

typedef struct{
  float a, b, x, y, x_rotated, y_rotated, g1, g2, e1, e2;
  int n1, n2;
}RoiData;


//**************************************************************************
// MkxComputeProfileModeling2 = MkxComputeProfileModeling modified with nSlope
// to not taking into account the slope region
// Compute the model profile corresponding to the input profile and
// output caracteristics of this model profile. 
// size of F =  2*nStep+1
//**************************************************************************
static int   MkxComputeProfileModeling2(int nStep, float *F, int *p_a1, int *p_a2, float *p_g1, float *p_g2, float *p_e)
{
  float MinVal, g1, g2;
  float e; // normalised residual error
  int i, i1, i2, i1Min, i2Min, k, a1, a2;
  float **Val, **g1Tab, **g2Tab;
  int nPts = 2 * nStep + 1;
  int nC, nB, nSlope; // Nb of Margin steps near marker center (nC) and Border (nB)

  //	nC = (int)(0.1 * nStep); // 10% of half support
  nC = 1;
  nSlope = 1;
  nB = (int)(0.3 * nStep); // 30% of half support

  Val = (float **)malloc(nPts * sizeof(float*));
  g1Tab = (float **)malloc(nPts * sizeof(float*));
  g2Tab = (float **)malloc(nPts * sizeof(float*));
  for (k = 0; k <= 2 * nStep; k++)
  {
    Val[k] = (float *)calloc(nPts, sizeof(float));
    g1Tab[k] = (float *)calloc(nPts, sizeof(float));
    g2Tab[k] = (float *)calloc(nPts, sizeof(float));
  }

  for (i1 = nB; i1 <= nStep - nC; i1++)
  {
    for (i2 = nStep + nC; i2 <= 2 * nStep - nB; i2++)
    {
      float sum1, sum2;

#if 1 // use moments instead of direct normalized computations
      float sq1, sq2;

      sum1 = (float)0;
      sq1 = (float)0;
      for (i = i1 + nSlope; i <= i2 - nSlope; i++)
      {
        sum1 += F[i];
        sq1 += F[i] * F[i];
      }
      g1 = sum1 / (i2 - i1 + 1 - 2 * nSlope);

      sum2 = (float)0;
      sq2 = (float)0;
      for (i = 0; i <= i1 - nSlope; i++)
      {
        sum2 += F[i];
        sq2 += F[i] * F[i];
      }
      for (i = i2 + nSlope; i <= 2 * nStep; i++)
      {
        sum2 += F[i];
        sq2 += F[i] * F[i];
      }
      g2 = sum2 / (float)(2 * nStep - (i2 - i1) + 2 - 2 * nSlope);

      sum1 = (sq1 - sum1*sum1 / (float)(i2 - i1 + 1 - 2 * nSlope)) / (float)(i2 - i1 + 1 - 2 * nSlope);
      sum2 = (sq2 - sum2*sum2 / (float)(2 * nStep - (i2 - i1) + 2 - 2 * nSlope)) / (float)(2 * nStep - (i2 - i1) + 2 - 2 * nSlope);
#else
      float sum = (float)0;
      for(i = i1+nSlope; i <= i2-nSlope; i++) sum += F[i];
      g1 = sum /(float)(i2-i1+1-2*nSlope);

      sum = (float)0;
      for(i = 0; i <= i1-nSlope; i++) sum += F[i];
      for(i = i2+nSlope; i<= 2*nStep; i++) sum += F[i];
      g2 = sum /(float)(2*nStep-(i2-i1)+2-2*nSlope);

      sum1 = (float)0;
      for(i = i1+nSlope; i <= i2-nSlope; i++) sum1 += (F[i]-g1)*(F[i]-g1);
      sum1 /= (float)(i2-i1+1-2*nSlope);

      sum2 = (float)0;
      for(i = 0; i <= i1-nSlope; i++) sum2 += (F[i]-g2)*(F[i]-g2);
      for(i = i2+nSlope; i <= 2*nStep; i++) sum2 += (F[i]-g2)*(F[i]-g2);
      sum2 /= (float)(2*nStep-(i2-i1)+2-2*nSlope);
#endif

      Val[i1][i2] = sum1 + sum2;
      g1Tab[i1][i2] = g1;
      g2Tab[i1][i2] = g2;
    }
  }
  MinVal = Val[nB][nStep + nC];
  i1Min = nB;
  i2Min = nStep + nC;
  for (i1 = nB; i1 <= nStep - nC; i1++)
  for (i2 = nStep + nC; i2 <= 2 * nStep - nB; i2++)
  {
    if (Val[i1][i2] < MinVal)
    {
      MinVal = Val[i1][i2];
      i1Min = i1;
      i2Min = i2;
    }
  }
  a1 = i1Min;
  a2 = i2Min;
  g1 = g1Tab[i1Min][i2Min];
  g2 = g2Tab[i1Min][i2Min];

  for (k = 0; k <= 2 * nStep; k++)
  {
    free(Val[k]);
    free(g1Tab[k]);
    free(g2Tab[k]);
  }
  free(Val);
  free(g1Tab);
  free(g2Tab);

  *p_a1 = a1;
  *p_a2 = a2;
  *p_g1 = g1;
  *p_g2 = g2;

  if (g2 - g1 == (float)0)
  {
    *p_e = (float)0;
    return 1;
  }
  else
    e = MinVal / ((g2 - g1)*(g2 - g1));

  *p_e = e;

  return 0;
}

/*************************************************************************************/
//ProfileAnalysis: select profiles and compute a rectangle
/*************************************************************************************/
static void  ProfileAnalysis(Profile *Prof, int HLength, Profile *BaryProf)
{
  float g1, g2, e;
  int i, iMax = 0, N, k;
  float Sum, c, a, x, ValMax, y;
  float Epsi = (float)0.1;
  int Flag = 0;
  float *X, Mean, Min, Max, Stdv;

  ValMax = 0;
  for (i = 0; i<1 + 2 * HLength; i++){
    if (Prof[i].Flag){
      if (Prof[i].c > ValMax){
        ValMax = Prof[i].c;
        iMax = i;
        Flag = 1;
      }
    }
  }
  if (!Flag){
    BaryProf->g1 = (float)-999;
    BaryProf->g2 = (float)-999;
    BaryProf->c = (float)-999;
    BaryProf->e = (float)-999;
    BaryProf->a = (float)-999;
    BaryProf->x = (float)-999;
    BaryProf->y = (float)-999;
    BaryProf->N = 0;
    return;
  }
  // selection on contrast
  for (i = 0; i < 1 + 2 * HLength; i++){
    if (Prof[i].Flag){
      if (Prof[i].c < 0.67 * ValMax) // empirical value: 0.67 = 2/3
        Prof[i].Flag = 0;
    }
  }
  //selection on connexity
  for (i = iMax + 1; i < 1 + 2 * HLength; i++){
    Prof[i].Flag *= Prof[i - 1].Flag;
  }
  for (i = iMax - 1; i >= 0; i--){
    Prof[i].Flag *= Prof[i + 1].Flag;
  }

  // compute barycentre of values
  Sum = 0;
  g1 = 0;
  g2 = 0;
  c = 0;
  a = 0;
  x = 0;
  y = 0;
  N = 0;
  for (i = 0; i < 1 + 2 * HLength; i++){
    if (Prof[i].Flag){
      g1 += Prof[i].c * Prof[i].g1;
      g2 += Prof[i].c * Prof[i].g2;
      c += Prof[i].c * Prof[i].c;
      a += Prof[i].c * Prof[i].a;
      x += Prof[i].c * Prof[i].x;
      y += Prof[i].c * (float)i;
      Sum += Prof[i].c;
      N++;
#if MKX_DEBUG_PRINT_PROFILE
      printf("i=%3d g1=%8.2f g2=%8.2f c=%8.2f e=%8.2f a=%8.2f x=%8.2f\n", i, Prof[i].g1, Prof[i].g2, Prof[i].c, Prof[i].e, Prof[i].a, Prof[i].x);
#endif
    }
  }
  if (N == 0 || Sum < Epsi){
    g1 = g2 = c = e = a = x = (float)-999;
    N = 0;																			//for Sum < Epsi to indicate unsuccess
  }
  else{
    g1 /= Sum;
    g2 /= Sum;
    c /= Sum;
    a /= Sum;
    x /= Sum;
    y /= Sum;

    //compute error by statistics on a
    X = (float *)malloc(N*sizeof(float));
    for (i = 0, k = 0; i < 1 + 2 * HLength; i++){
      if (Prof[i].Flag){
        X[k++] = Prof[i].a;
      }
    }
    MkxStatComput(N, X, &Mean, &Min, &Max, &Stdv);
    e = Stdv; //PL_070720
    free(X);
  }

  BaryProf->g1 = g1;
  BaryProf->g2 = g2;
  BaryProf->c = c;
  BaryProf->e = e;
  BaryProf->a = a;
  BaryProf->x = x;
  BaryProf->y = y;
  BaryProf->N = N;
}


static void  PointRotate(int HWidth, int HHeight, float Ang, float X, float Y, float *x, float *y)
{
  float Theta = (float)(Ang * MKX_PI / 180.);
  float CosTheta = (float)cos((double)Theta);
  float SinTheta = (float)sin((double)Theta);

  *x = HWidth + (X - HWidth) * CosTheta + (Y - HHeight) * SinTheta;
  *y = HHeight - (X - HWidth) * SinTheta + (Y - HHeight) * CosTheta;
}


/*************************************************************************************/
// Bilinear Interpolation                                                            */
/*************************************************************************************/
static float BilinearInterpolationShort(const short *Ori, int Iw, float x, float y)
{
  int   i, j;
  float dx, dy, Res;
  float a1, a2, a3, a4, b1, b2, b3, b4;

  j = (int)x;
  dx = x - (float)j;
  i = (int)y;
  dy = y - (float)i;

  a1 = (float)((1 - dx)*(1 - dy));
  a2 = (float)(dx*(1 - dy));
  a3 = (float)(dx*dy);
  a4 = (float)((1 - dx)*dy);

  b4 = (float)Ori[(i + 1)*Iw + j];
  b1 = (float)Ori[i*Iw + j];
  b2 = (float)Ori[i*Iw + j + 1];
  b3 = (float)Ori[(i + 1)*Iw + j + 1];


  Res = a1*b1 + a2*b2 + a3*b3 + a4*b4;

  return Res;
}


/*************************************************************************************/
//Get a Roi (1+2*RoiHWidth, 1+2*RoiHHeight) around X0, Y0, rotated by angle Ang
/*************************************************************************************/
static void  MkxRoiRotate(const short *Ori, float *RoiRotated, int Iw, int Ih, int HWidth, int HHeight, float X0, float Y0, float deg)
{
  int   i, j;
  float x, y, Res;

  float Theta = (float)(deg * MKX_PI / 180.);
  float CosTheta = (float)cos((double)Theta);
  float SinTheta = (float)sin((double)Theta);
  int width = 1 + 2 * HWidth/*,height=1+2*HHeight*/;

  for (i = -HHeight; i <= HHeight; i++)
  for (j = -HWidth; j <= HWidth; j++)
  {
    // for each pixel in the registered ROI, 
    // calculation of the corresponding float coordinates in the original image
    x = X0 + j * CosTheta + i * SinTheta;
    y = Y0 - j * SinTheta + i * CosTheta;

    int k = j + HWidth + width*(i + HHeight);

    // bilinear interpolation
    if (x >= 0 && x < Iw - 1 && y >= 0 && y < Ih - 1)
    {
      Res = BilinearInterpolationShort(Ori, Iw, x, y);
      RoiRotated[k] = Res;
    }
    else
      RoiRotated[k] = 0;
  }
}

/*************************************************************************************/
// RoiProcess1: compute a fitting rectangle by computing, selecting and analysing
// profiles in horizontal and vertical
/*************************************************************************************/
static void  RoiProcess1(float *Roi2, int HWidth, int HHeight, float *p_x, float *p_y, float *p_a, float *p_b, float *p_g1, float *p_g2, float *p_e1, float *p_e2, int *p_n1, int *p_n2)
{
  float *H, *V, g1, g2, e;
  int i, j;
  Profile *ProfH, *ProfV, BaryProf;
  int N1, N2;
  int a1, a2;

  ProfH = (Profile *)malloc((1 + 2 * HHeight)*sizeof(Profile));

  //horizontal profiles
#if MKX_DEBUG_PRINT_PROFILE
  printf("horizontal profiles\n");
#endif
  H = (float *)malloc((1 + 2 * HWidth)*sizeof(float));
  for (i = 0; i < 1 + 2 * HHeight; i++){
    for (j = 0; j < 1 + 2 * HWidth; j++){
      H[j] = Roi2[i*(1 + 2 * HWidth) + j];
    }
    MkxComputeProfileModeling2(HWidth, H, &a1, &a2, &g1, &g2, &e);
    ProfH[i].a1 = a1;
    ProfH[i].a2 = a2;
    ProfH[i].g1 = g1;
    ProfH[i].g2 = g2;
    ProfH[i].c = g2 - g1;
    ProfH[i].a = (float)(a2 - a1);
    ProfH[i].x = (float)(a1 + a2) / 2;
    ProfH[i].e = e;
    ProfH[i].Flag = (e < (float) 0.2) ? 1 : 0;
#if MKX_DEBUG_PRINT_PROFILE
    printf("i=%3d g1=%8.2f g2=%8.2f c=%8.2f e=%8.2f a=%8.2f x=%8.2f \n", i, ProfH[i].g1, ProfH[i].g2, ProfH[i].c, ProfH[i].e, ProfH[i].a, ProfH[i].x);
#endif
  }
  free(H);
#if MKX_DEBUG_PRINT_PROFILE
  printf("\n");
#endif
  ProfileAnalysis(ProfH, HHeight, &BaryProf);
#if MKX_DEBUG_PRINT_PROFILE
  printf("\ng1=%8.2f g2=%8.2f c=%8.2f e=%8.2f a=%8.2f x=%8.2f y=%8.2f \n", BaryProf.g1, BaryProf.g2, BaryProf.c, BaryProf.e, BaryProf.a, BaryProf.x, BaryProf.y);
#endif
  N1 = BaryProf.N;
  if (N1){
    *p_g1 = BaryProf.g1;
    *p_g2 = BaryProf.g2;
    *p_a = BaryProf.a;
    *p_x = BaryProf.x;
    *p_e1 = BaryProf.e;
    *p_n1 = N1;
  }
  else{
    *p_g1 = *p_g2 = *p_a = *p_b = *p_x = *p_y = *p_e1 = *p_e2 = (float)-999;
    *p_n1 = *p_n2 = 0;
    free(ProfH);
    return;
  }

  ProfV = (Profile *)malloc((1 + 2 * HWidth)*sizeof(Profile));

  //vertical profiles
#if MKX_DEBUG_PRINT_PROFILE
  printf("vertical profiles\n");
#endif
  V = (float *)malloc((1 + 2 * HHeight)*sizeof(float));
  for (j = 0; j < 1 + 2 * HWidth; j++){
    for (i = 0; i < 1 + 2 * HHeight; i++){
      V[i] = Roi2[i*(1 + 2 * HWidth) + j];
    }
    MkxComputeProfileModeling2(HHeight, V, &a1, &a2, &g1, &g2, &e);
    ProfV[j].a1 = a1;
    ProfV[j].a2 = a2;
    ProfV[j].g1 = g1;
    ProfV[j].g2 = g2;
    ProfV[j].c = g2 - g1;
    ProfV[j].a = (float)(a2 - a1);
    ProfV[j].x = (float)(a1 + a2) / 2;
    ProfV[j].e = e;
    ProfV[j].Flag = (e < (float) 0.2) ? 1 : 0;
#if MKX_DEBUG_PRINT_PROFILE
    printf("j=%3d g1=%8.2f g2=%8.2f c=%8.2f e=%8.2f a=%8.2f x=%8.2f \n", j, ProfV[j].g1, ProfV[j].g2, ProfV[j].c, ProfV[j].e, ProfV[j].a, ProfV[j].x);
#endif
  }
  free(V);
#if MKX_DEBUG_PRINT_PROFILE
  printf("\n");
#endif
  ProfileAnalysis(ProfV, HWidth, &BaryProf);

#if MKX_DEBUG_PRINT_PROFILE
  printf("\ng1=%8.2f g2=%8.2f c=%8.2f e=%8.2f a=%8.2f x=%8.2f y=%8.2f \n", BaryProf.g1, BaryProf.g2, BaryProf.c, BaryProf.e, BaryProf.a, BaryProf.y, BaryProf.x);
#endif
  N2 = BaryProf.N;
  if (N2){
    *p_g1 += BaryProf.g1;
    *p_g1 /= 2;                //mean on horizontal and vertical
    *p_g2 += BaryProf.g2;
    *p_g2 /= 2;                //mean on horizontal and vertical
    *p_b = BaryProf.a;
    *p_y = BaryProf.x;
    *p_e2 = BaryProf.e;
    *p_n2 = N2;
  }
  else{
    *p_g1 = *p_g2 = *p_a = *p_b = *p_x = *p_y = *p_e1 = *p_e2 = (float)-999;
    *p_n1 = *p_n2 = 0;
    free(ProfH);
    free(ProfV);
    return;
  }
  free(ProfH);
  free(ProfV);
}

/*************************************************************************************/
// RoiProcess2: compute a fitting rectangle by computing and analysing
// profiles in horizontal and vertical (no selection) (simpler but worse than RoiProcess1)
/*************************************************************************************/
static void  RoiProcess2(float *Roi2, int HWidth, int HHeight, float *p_x, float *p_y,
  float *p_a, float *p_b, float *p_g1, float *p_g2, float *p_e1, float *p_e2)
{
  int i, j;
  float *SumX, *SumY;
  float g11, g21, e1;
  float g12, g22, e2;
  int a11, a21;
  int a12, a22;

  SumX = (float *)malloc((1 + 2 * HHeight) *sizeof(float));
  SumY = (float *)malloc((1 + 2 * HWidth)  *sizeof(float));

  // horizontal projections
  for (i = 0; i < 1 + 2 * HHeight; i++){
    SumX[i] = 0;
    for (j = 0; j < 1 + 2 * HWidth; j++)
      SumX[i] += Roi2[i*(1 + 2 * HWidth) + j];
  }
  MkxComputeProfileModeling2(HHeight, SumX, &a11, &a21, &g11, &g21, &e1);

  for (j = 0; j < 1 + 2 * HWidth; j++){
    SumY[j] = 0;
    for (i = 0; i < 1 + 2 * HHeight; i++)
      SumY[j] += Roi2[i*(1 + 2 * HWidth) + j];
  }
  MkxComputeProfileModeling2(HWidth, SumY, &a12, &a22, &g12, &g22, &e2);

  free(SumX);
  free(SumY);

  *p_a = (float)(a22 - a12);
  *p_x = (float)(a12 + a22) / 2;
  *p_b = (float)(a21 - a11);
  *p_y = (float)(a11 + a21) / 2;
  *p_g1 = (g11 + g12) / 2;
  *p_g2 = (g21 + g22) / 2;
  *p_e1 = e1;
  *p_e2 = e2;
}

/*************************************************************************************/
// MkxRoiAnalysis: rotate  a roi and compute marker size and position by one of two methods                                                        */
/*************************************************************************************/
static void 	MkxRoiAnalysis(int Method, const short *Ori, int IwOri, int IhOri, int RoiHWidth, int RoiHHeight,
  float X, float Y, float Ang, RoiData *Data)
{

  float* roi1 = (float*)malloc((1 + 2 * RoiHWidth)*(1 + 2 * RoiHHeight)*sizeof(float));
  MkxRoiRotate(Ori, roi1, IwOri, IhOri, RoiHWidth, RoiHHeight, X, Y, -Ang); //-Ang to go to horizontal

  if (Method == 1)
    RoiProcess1(roi1, RoiHWidth, RoiHHeight, &Data->x_rotated, &Data->y_rotated, &Data->a, &Data->b,
    &Data->g1, &Data->g2, &Data->e1, &Data->e2, &Data->n1, &Data->n2);
  else if (Method == 2)
    RoiProcess2(roi1, RoiHWidth, RoiHHeight, &Data->x_rotated, &Data->y_rotated, &Data->a, &Data->b,
    &Data->g1, &Data->g2, &Data->e1, &Data->e2);

  if (Data->n1 != 0 && Data->n2 != 0)
    PointRotate(RoiHWidth, RoiHHeight, -Ang, Data->x_rotated, Data->y_rotated, &Data->x, &Data->y);

  free(roi1);
}

/*************************************************************************************/
// fitting Tab by a 2nd order polynomial and compute the minimum 
// calculations come from mathematica5                                                           */
/*************************************************************************************/
static void  MkxComputeCenterByCurveFitting(int N, int *Tab, float *X)
{
  //fitting Tab by a 2nd order polynomial and compute the minimum
  //calculations come from mathematica5
  int i;
  double s1, s2, s3, s4, b1, b2, b3, num, denom;

  s1 = s2 = s3 = s4 = b1 = b2 = b3 = 0;

  for (i = 0; i < N; i++)
  {
    s1 += i;
    s2 += i*i;
    s3 += i*i*i;
    s4 += i*i*i*i;
    b1 += Tab[i] * i*i;
    b2 += Tab[i] * i;
    b3 += Tab[i];
  }
  num = (double)(b1*s1*s2 - b2*s2*s2 - b1*N*s3 + b3*s2*s3 + b2*N*s4 - b3*s1*s4);
  denom = (double)(b1*s1*s1 - b1*N*s2 - b2*s1*s2 + b3*s2*s2 + b2*N*s3 - b3*s1*s3);
  *X = (float)(0.5*num / denom);
}

/*************************************************************************************/
// Compute one marker position by horizontal and vertical projections                                                           */
/*************************************************************************************/
static void  MkxGetOneMarkerMethod0(const short *Ori, int IwOri, int IhOri, int HalfSize, int X1, int Y1, float *X1f, float *Y1f)
{
  int i, j, i0, j0, i1, j1, SizeX, SizeY, *Sum, ii, jj;
  int Size = 2 * HalfSize + 1;
  short *Roi;
  float x;

  Roi = (short *)malloc(Size*Size*sizeof(short));

  j0 = MKX_MAX(0, X1 - HalfSize);
  j1 = MKX_MIN(IwOri - 1, X1 + HalfSize);
  i0 = MKX_MAX(0, Y1 - HalfSize);
  i1 = MKX_MIN(IhOri - 1, Y1 + HalfSize);
  SizeX = j1 - j0 + 1;
  SizeY = i1 - i0 + 1;

  for (i = 0; i < SizeY; i++)
  {
    ii = i0 + i;
    for (j = 0; j < SizeX; j++)
    {
      jj = j0 + j;
      Roi[j + i*SizeX] = Ori[jj + ii*IwOri];
    }
  }
  //compute horizontal projections
  Sum = (int*)calloc(SizeY, sizeof(int));
  for (i = 0; i < SizeY; i++)
  for (j = 0; j<SizeX; j++)
    Sum[i] += Roi[j + i*SizeX];
  //compute cdg by 2nd order curve fitting
  MkxComputeCenterByCurveFitting(SizeY, Sum, &x);
  if (x > 0 && x < SizeY)   *Y1f = (float)i0 + x;
  else                     *Y1f = (float)Y1;
  free(Sum);

  //compute vertical projections
  Sum = (int*)calloc(SizeX, sizeof(int));
  for (j = 0; j < SizeX; j++)
  for (i = 0; i<SizeY; i++)
    Sum[j] += Roi[j + i*SizeX];
  //compute cdg by 2nd order curve fitting
  MkxComputeCenterByCurveFitting(SizeX, Sum, &x);
  if (x > 0 && x < SizeX)   *X1f = j0 + x;
  else                     *X1f = (float)X1;
  free(Sum);
  free(Roi);
}

/*************************************************************************************/
// Compute marker position by horizontal and vertical projections                                                           */
/*************************************************************************************/
static void  MkxMarkerRefiningRough(
  const short *Ori, int IwOri, int IhOri,
  int X1, int Y1, int X2, int Y2,
  float* X1f, float* Y1f, float* X2f, float* Y2f,
  float BlobEnhRingRadiusInPix, int SSFactor
  )
{
  int HalfSize = (int)(BlobEnhRingRadiusInPix)-1; //empirical value (typ. HalfSize = 3)
  if (SSFactor > 1)HalfSize *= SSFactor;
  HalfSize = MKX_MAX(1, HalfSize); //PL_070427

  MkxGetOneMarkerMethod0(Ori, IwOri, IhOri, HalfSize, X1, Y1, X1f, Y1f);
  MkxGetOneMarkerMethod0(Ori, IwOri, IhOri, HalfSize, X2, Y2, X2f, Y2f);
}

/*************************************************************************************/
// Compute marker position by first rotating a roi according to the marker direction                                                            */
/*************************************************************************************/
static void  MkxMarkerRefiningFine(
  CMkxMarkerCoupleRefined *mkcplRefined,
  const short *Ori, int IwOri, int IhOri,
  int X1, int Y1, int X2, int Y2, float dir1, float dir2,
  float BlobEnhRingRadiusInPix, int SSFactor, int Method
  )
{
  int HalfSize, RoiHWidth, RoiHHeight;
  float Dir1, Dir2;
  RoiData Data1,Data2;
  
  HalfSize = (int)BlobEnhRingRadiusInPix + 2; //empirical value (typ. HalfSize = 6)

  if (SSFactor > 1)HalfSize *= SSFactor;
  HalfSize = MKX_MAX(1, HalfSize); //PL_070427

  RoiHWidth = RoiHHeight = HalfSize;

  Dir1 = (float)(((int)dir1) % 180);
  Dir1 = (Dir1 > 90) ? (Dir1 - 180) : Dir1;

  Dir2 = (float)(((int)dir2) % 180);
  Dir2 = (Dir2 > 90) ? (Dir2 - 180) : Dir2;

  MkxRoiAnalysis(Method, Ori, IwOri, IhOri, RoiHWidth, RoiHHeight, ((float)X1), ((float)Y1), Dir1, &Data1);
  MkxRoiAnalysis(Method, Ori, IwOri, IhOri, RoiHWidth, RoiHHeight, ((float)X2), ((float)Y2), Dir2, &Data2);

  mkcplRefined->CplFine.V1f.Xf=Data1.x + X1 - RoiHWidth;
  mkcplRefined->CplFine.V1f.Yf=Data1.y + Y1 - RoiHHeight;
  mkcplRefined->CplFine.V2f.Xf=Data2.x + X2 - RoiHWidth;
  mkcplRefined->CplFine.V2f.Yf=Data2.y + Y2 - RoiHHeight;

  mkcplRefined->Advanced1.a = Data1.a;
  mkcplRefined->Advanced1.b = Data1.b;
  mkcplRefined->Advanced1.level_in = Data1.g1;
  mkcplRefined->Advanced1.level_out = Data1.g2;
  mkcplRefined->Advanced1.err = MKX_MAX(Data1.e1, Data1.e2);
  mkcplRefined->Advanced1.n = MKX_MIN(Data1.n1, Data1.n2);

  mkcplRefined->Advanced2.a = Data2.a;
  mkcplRefined->Advanced2.b = Data2.b;
  mkcplRefined->Advanced2.level_in = Data2.g1;
  mkcplRefined->Advanced2.level_out = Data2.g2;
  mkcplRefined->Advanced2.err = MKX_MAX(Data2.e1, Data2.e2);
  mkcplRefined->Advanced2.n = MKX_MIN(Data2.n1, Data2.n2);
}


void  MkxRefining(
  CMkxMarkerCoupleRefined *mkcplRefined,
  CMkxCoupleInt cpl,
  const short* InIma, int IwOri, int IhOri,
  float BlobEnhRingRadiusInPix, int SSFactor
  )
{
  float dir1, dir1Conf, dir2, dir2Conf;

  MkxMarkerDirection(
    InIma, IwOri, IhOri,
    cpl.V1.X, cpl.V1.Y, cpl.V2.X, cpl.V2.Y,
    &dir1, &dir2, &dir1Conf, &dir2Conf,
    BlobEnhRingRadiusInPix, SSFactor
    );
  MkxMarkerRefiningFine(
    mkcplRefined,
    InIma, IwOri, IhOri,
    cpl.V1.X, cpl.V1.Y, cpl.V2.X, cpl.V2.Y,
    dir1, dir2,
    BlobEnhRingRadiusInPix, SSFactor, 1
    );
}


/************************************************************************************/
// End of file
/*************************************************************************************/

