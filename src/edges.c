/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Visionutils - example computer vision functions
 *  Copyright (c) 2011-2015, Bob Mottram
 *  bob@libreserver.org
 *
 *  This code is based on Visionutils by Bob Mottram which was
 *  originally distributed under BSD 2-clause license.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *********************************************************************/


#include "datamatrix.h"

#define SQUARE_MAG(a,b) (a*a + b*b)

/* You might need to experiment with this,
   depending upon your image size */
#define MAX_EDGES 1000000
/* max edges for each orientation histogram bucket */
#define MAX_ORIENTATION_EDGES 2048

struct Kernel {
  unsigned int Size;
  float * Data;
};


struct canny_params {
  float    GAUSSIAN_CUT_OFF;
  float    MAGNITUDE_SCALE;
  float    MAGNITUDE_LIMIT;
  float    MAGNITUDE_LIMIT_SQR;
  int      MAGNITUDE_MAX;

  float    gaussianKernelRadius;
  float    lowThreshold;
  float    highThreshold;
  unsigned int gaussianKernelWidth;

  struct Kernel   kernel;
  struct Kernel   diffKernel;
  unsigned int picSize;

  int    * data;
  int    * magnitude;

  float  * xConv;
  float  * yConv;
  float  * xGradient;
  float  * yGradient;

  int    * edge_pixel_index;
  float  * edge_magnitude;

  int     automaticThresholds;
  int    * edges;
  int      no_of_edges;
  unsigned char * edges_image;
  int     sampling_radius_percent;
  int     image_contrast;

  float   contrast_multiplier;
  float   lowThresholdOffset;
  float   lowThresholdMultiplier;
  float   highhresholdOffset;
  float   highhresholdMultiplier;
};


int params_initialised = 0;
struct canny_params cannyparams;

/**
 * \brief initialise object used to contain line segments
 * \param segments object containing line segments
 * \param width width of the image
 * \param height height of the image
 */
void init_line_segments(struct line_segments * segments,
                        int width, int height)
{
  int i;

  segments->max_members = 1000000;
  segments->max_segments = 2000;
  segments->image_border = 0;
  segments->no_of_segments = 0;
  segments->ignore_periphery = 0;
  segments->members =
    (int*)malloc(segments->max_members*2*sizeof(int));
  segments->no_of_members =
    (int*)malloc(segments->max_segments*sizeof(int));
  segments->joins =
    (unsigned char*)malloc(segments->max_segments*segments->max_segments*sizeof(unsigned char));
  segments->joins_sorted =
    (int*)malloc(segments->max_segments*sizeof(int));
  segments->selected =
    (unsigned char*)malloc(segments->max_segments*sizeof(unsigned char));
  segments->joined_length =
    (int*)malloc(segments->max_segments*sizeof(int));
  segments->minimum_segment_length = 20;

  segments->perimeter_left = (int*)malloc(height*sizeof(int));
  segments->perimeter_right = (int*)malloc(height*sizeof(int));
  segments->perimeter_top = (int*)malloc(width*sizeof(int));
  segments->perimeter_bottom = (int*)malloc(width*sizeof(int));
  segments->orientation_histogram = (int*)malloc(360*sizeof(int));

  segments->orientation_histogram_edges = (int**)malloc(360*sizeof(int*));
  for (i = 0; i < 360; i++) {
    segments->orientation_histogram_edges[i] =
      (int*)malloc(MAX_ORIENTATION_EDGES*4*sizeof(int));
  }

  segments->side_edges = (int**)malloc(4*sizeof(int*));
  for (i = 0; i < 4; i++) {
    segments->side_edges[i] =
      (int*)malloc(MAX_ORIENTATION_EDGES*2*sizeof(int));
  }
  segments->side_edges_count = (int*)malloc(4*sizeof(int));
  segments->linefit =
    (int*)malloc(MAX_ORIENTATION_EDGES*2*sizeof(int));
  segments->linefit2 =
    (int*)malloc(MAX_ORIENTATION_EDGES*2*sizeof(int));

  segments->edge_centre_hits = 0;
}

/**
 * \brief free memory for an object containing line segments
 * \param segments object containing line segments
 */
void free_line_segments(struct line_segments * segments)
{
  int i;

  for (i = 0; i < 360; i++) {
    free(segments->orientation_histogram_edges[i]);
  }
  free(segments->orientation_histogram_edges);

  for (i = 0; i < 4; i++) {
    free(segments->side_edges[i]);
  }
  free(segments->side_edges);
  free(segments->side_edges_count);

  free(segments->joins);
  free(segments->joins_sorted);
  free(segments->selected);
  free(segments->joined_length);
  free(segments->members);
  free(segments->no_of_members);
  free(segments->orientation_histogram);
  free(segments->perimeter_left);
  free(segments->perimeter_right);
  free(segments->perimeter_top);
  free(segments->perimeter_bottom);
  free(segments->linefit);
  free(segments->linefit2);
}

/**
 * \brief initialise arrays
 * \param params object used to store the results of edge detection
 */
static void canny_init_arrays(struct canny_params * params)
{
  unsigned int i;
  unsigned int picSize = params->picSize;

  params->edges = (int*)malloc(picSize*sizeof(int));
  params->edges_image =
    (unsigned char*)malloc(picSize*sizeof(unsigned char));
  params->data = (int*)malloc(picSize*sizeof(int));
  params->magnitude = (int*)malloc(picSize*sizeof(int));
  params->xConv = (float*)malloc(picSize*sizeof(float));
  params->yConv = (float*)malloc(picSize*sizeof(float));
  params->xGradient = (float*)malloc(picSize*sizeof(float));
  params->yGradient =
    (float*)malloc(picSize*sizeof(float));
  params->edge_pixel_index =
    (int*) malloc(picSize*sizeof(int));
  params->edge_magnitude =
    (float*) malloc(picSize*sizeof(float));

  for (i = 0; i < params->picSize; i++) {
    params->data[i] = 0;
    params->magnitude[i] = 0;
    params->xConv[i] = 0;
    params->yConv[i] = 0;
    params->xGradient[i] = 0;
    params->yGradient[i] = 0;
    params->edge_pixel_index[i] = 0;
    params->edge_magnitude[i] = 0;
    params->edges_image[i] = 255;
  }
}

/**
 * \brief initialise canny edges object
 * \param params object used to store the results of edge detection
 * \param edge_radius canny edge detection radius
 */
static void canny_init(struct canny_params * params, float edge_radius)
{
  params->GAUSSIAN_CUT_OFF = 0.005f;
  params->MAGNITUDE_SCALE = 100.0f;
  params->MAGNITUDE_LIMIT = 1000.0f;
  params->MAGNITUDE_LIMIT_SQR =
    (params->MAGNITUDE_LIMIT * params->MAGNITUDE_LIMIT);
  params->MAGNITUDE_MAX =
    ((int)(params->MAGNITUDE_SCALE * params->MAGNITUDE_LIMIT));

  params->sampling_radius_percent = edge_radius;
  params->lowThreshold = 2.5;
  params->highThreshold = 7.5;
  params->gaussianKernelRadius = 2.0;
  params->gaussianKernelWidth = 8;
  params->picSize = 0;
  params->automaticThresholds = 1;

  params->contrast_multiplier = 0.35f;
  params->lowThresholdOffset = 1.6f;
  params->lowThresholdMultiplier = 6.4f;
  params->highhresholdOffset = 2.0f;
  params->highhresholdMultiplier = 8.0f;

  params->data = NULL;
  params->magnitude = NULL;
  params->xConv = NULL;
  params->yConv = NULL;
  params->xGradient = NULL;
  params->yGradient = NULL;
  params->edge_pixel_index = NULL;
  params->edge_magnitude   = NULL;
  params->edges_image       = NULL;

  params->kernel.Size = 0;
  params->kernel.Data = NULL;
  params->diffKernel.Size = 0;
  params->diffKernel.Data = NULL;
}

/**
 * \brief free memory for a canny edges object
 * \param params object used to store the results of edge detection
 */
static void canny_free(struct canny_params * params)
{
  if (params->kernel.Data != NULL) {
    free(params->data);
    free(params->magnitude);
    free(params->xConv);
    free(params->yConv);
    free(params->xGradient);
    free(params->yGradient);
    free(params->kernel.Data);
    free(params->diffKernel.Data);
    free(params->edge_pixel_index);
    free(params->edge_magnitude);
    free(params->edges_image);
    free(params->edges);
  }
}

/**
 * \brief canny gausiian function
 */
static float canny_gaussian(float x, float sigma)
{
  return exp(-(x * x) / (2.0f * sigma * sigma));
}

/**
 * \brief non-recursive edge following
 */
static void canny_follow(unsigned int x1,
                         unsigned int y1,
                         unsigned int i1,
                         int * threshold,
                         int * followedEdges,
                         int * magnitude,
                         int * width,
                         int * height)
{
  int following = 1;
  unsigned int x0,x2,y0,y2,x,y,i2,i3;

  while (following!=0) {
    following = 0;

    x0 = x1 == 0 ? x1 : x1 - 1;
    x2 = x1 == (unsigned int)*width - 1 ? x1 : x1 + 1;
    y0 = y1 == 0 ? y1 : y1 - 1;
    y2 = y1 == (unsigned int)*height - 1 ? y1 : y1 + 1;

    followedEdges[i1] = magnitude[i1];

    y = y0;
    while ((y <= y2) && (following==0)) {
      i2 = y * *width;
      i3 = i2 + x0;

      x = x0;
      while ((x <= x2) && (following==0)) {
        if (followedEdges[i3] == 0) {
          if(magnitude[i3] >= *threshold) {
            i1 = i3;
            x1 = x;
            y1 = y;
            following = 1;
          }
        }
        x++;
        i3++;
      }
      y++;
    }
  }
}

/**
 * \brief canny edge detection hysteresis
 * \param width width of the image
 * \param height height of the image
 * \param followedEdges array containing edges followed
 * \param no_of_edges number of edges in the image
 * \param params object used to store the results of edge detection
 */
static void canny_perform_hysteresis(int width, int height,
                                     int low, int high,
                                     int followedEdges[],
                                     int no_of_edges,
                                     struct canny_params * params)
{
  int imageSize = width * height;
  int offset = imageSize - 1;
  int w,h,i,index,x,y;

  memset(followedEdges, 0, offset * sizeof(int));

  w = width;
  h = height;

  for (i = no_of_edges - 1; i >= 0; i--) {
    index = params->edge_pixel_index[i];

    if (followedEdges[index] == 0) {
      if (params->magnitude[index] >= high) {
        y = index % w;
        x = index - (y * w);
        canny_follow(x,y,index,&low,
                     followedEdges,
                     params->magnitude,&w,&h);
      }
    }
  }
}

/**
 * \brief gets mean dark and light thresholds for canny edge detection
 * \param reflectance histogram
 * \param meanDark returned mean dark threshold
 * \param meanLight returned mean light threshold
 */
static void canny_get_thresholds(unsigned int histogram[],
                                 float * meanDark,
                                 float * meanLight)
{
  float minVariance = 999999.0f;
  float currMeanDark = 0.0f;
  float currMeanLight = 0.0f;
  float varianceDark = 0.0f;
  float varianceLight = 0.0f;
  float darkHits = 0.0f;
  float lightHits = 0.0f;
  float histogramSquaredMagnitude[256] = {0};
  int h = 0;
  int bucket = 0;
  float magnitudeSqr = 0.0f;
  float variance = 0.0f;
  float divisor= 0.0f;
  unsigned int i;
  int greyLevel;
  *meanDark = 0;
  *meanLight = 0;

  /* Calculate squared magnitudes -
     avoids unneccessary multiplies later on */
  for(i = 0; i < 256; i++) {
    histogramSquaredMagnitude[i] =
      histogram[i] * histogram[i];
  }

  /* Evaluate all possible thresholds */
  for(greyLevel = 255; greyLevel >= 0; greyLevel--) {
    darkHits = 0;
    lightHits = 0;
    currMeanDark = 0;
    currMeanLight = 0;
    varianceDark = 0;
    varianceLight = 0;

    bucket = (int)greyLevel;

    for(h = 255; h >= 0; h--) {
      magnitudeSqr = histogramSquaredMagnitude[h];
      if (h < bucket) {
        currMeanDark += h * magnitudeSqr;
        varianceDark += (bucket - h) * magnitudeSqr;
        darkHits += magnitudeSqr;
      }
      else {
        currMeanLight += h * magnitudeSqr;
        varianceLight += (bucket - h) * magnitudeSqr;
        lightHits += magnitudeSqr;
      }
    }

    if (darkHits > 0) {
      /* Rescale into 0-255 range */
      divisor = darkHits * 256;
      currMeanDark = (currMeanDark * 255) / divisor;
      varianceDark = (varianceDark * 255) / divisor;
    }

    if (lightHits > 0) {
      /* Rescale into 0-255 range */
      divisor = lightHits * 256;
      currMeanLight = (currMeanLight * 255) / divisor;
      varianceLight = (varianceLight * 255) / divisor;
    }

    variance = varianceDark + varianceLight;
    if (variance < 0) {
      variance = -variance;
    }

    if (variance < minVariance) {
      minVariance = variance;
      *meanDark = currMeanDark;
      *meanLight = currMeanLight;
    }

    if ((int)(variance * 1000) == (int)(minVariance * 1000)) {
      *meanLight = currMeanLight;
    }
  }
}

/**
 * \brief canny edge detection with automatic threshold calculation
 * \param img mono image
 * \param width width of the image
 * \param height height of the image
 * \param samplingStepSize subsampling step size
 * \param params object used to store the results of edge detection
 */
static void canny_auto_threshold(unsigned char img[],
                                 int width, int height,
                                 int samplingStepSize,
                                 struct canny_params * params)
{
  unsigned int tx =
    (unsigned int)(width * params->sampling_radius_percent / 100);
  unsigned int ty =
    (unsigned int)(height * params->sampling_radius_percent / 100);
  unsigned int bx = (unsigned int)(width - 1 - tx);
  unsigned int by = (unsigned int)(height - 1 - ty);
  unsigned int histogram[256] = {0};
  float meanDark = 0;
  float meanLight = 0;
  float contrast = 0;
  float fraction = 0;
  unsigned int n = (ty * (unsigned int)width) + tx;
  unsigned int vertical_increment =
    (unsigned int)(width * samplingStepSize);
  unsigned int x,y,n2;
  float contrast_mult;

  for (y = ty; y <= by;
       y += samplingStepSize, n += vertical_increment) {
    n2 = n;

    for (x = tx; x <= bx; x += samplingStepSize, n2++)
      histogram[img[n2]]++;
  }

  canny_get_thresholds(histogram, &meanDark, &meanLight);
  meanDark /= 255.0f;
  meanLight /= 255.0f;
  contrast = meanLight - meanDark;

  params->image_contrast = (int)(contrast*255);

  contrast_mult =
    (1.0f - (contrast * params->contrast_multiplier));
  contrast *= contrast_mult;

  fraction = (contrast - 0.048f) / (0.42f - 0.048f);

  params->lowThreshold =
    params->lowThresholdOffset +
    (fraction * params->lowThresholdMultiplier);
  params->highThreshold =
    params->highhresholdOffset +
    (fraction * params->highhresholdMultiplier);
}

/**
 * \brief create masks for canny edges
 * \param kernelRadius radius of the edge kernel
 * \param kernelWidth width of the edge kernel
 * \param params object used to store the results of edge detection
 */
static void canny_create_masks(float kernelRadius,
                               unsigned int kernelWidth,
                               struct canny_params * params)
{
  int kw_length,kwidth;
  float g1,g2,g3;

  if ((params->kernel.Size == 0) ||
      (params->kernel.Size != kernelWidth)) {
    params->kernel.Data =
      (float*) realloc(params->kernel.Data,
                       kernelWidth*sizeof(float));
    params->kernel.Size = kernelWidth;
    params->diffKernel.Data =
      (float*) realloc(params->diffKernel.Data,
                       kernelWidth*sizeof(float));
    params->diffKernel.Size = kernelWidth;

    kw_length = kernelWidth * sizeof(unsigned int);
    memset(params->kernel.Data, 0, kw_length);
    memset(params->diffKernel.Data, 0, kw_length);

    for (kwidth = 0; kwidth < (int)kernelWidth; kwidth++) {
      g1 = canny_gaussian(kwidth, kernelRadius);
      if (g1 <= params->GAUSSIAN_CUT_OFF && kwidth >= 2)
        break;
      g2 = canny_gaussian(kwidth - 0.5, kernelRadius);
      g3 = canny_gaussian(kwidth + 0.5, kernelRadius);
      params->kernel.Data[kwidth] =
        ((g1 + g2 + g3) / 3.0) /
        (2.0 * PI * kernelRadius * kernelRadius);
      params->diffKernel.Data[kwidth] = g3 - g2;
    }
  }
}

/**
 * \brief compute gradients for canny edges
 * \param data
 * \param width width of the image
 * \param height height of the image
 * \param kernelRadius radius of the edge kernel
 * \param kernelWidth width of the edge kernel
 * \param params object used to store the results of edge detection
 * \return number of detected edges
 */
static int canny_compute_gradients(int data[],
                                   int width, int height,
                                   float kernelRadius,
                                   unsigned int kernelWidth,
                                   struct canny_params * params)
{
  int initX = 0;
  int maxX = 0;
  int initY = 0;
  int maxY = 0;
  int x,y,w, no_of_edges = 0;
  float sum,sumX,sumY,k;
  unsigned int xOffset,i;
  int yOffset,index,index1,index2,index3,index4;
  float * kern;
  float xGrad,yGrad,gradMag,tmp,xGrad_abs,yGrad_abs;
  int is_edge;
  int indexNE,indexE,indexSW,indexW;
  int indexN,indexS,indexSE,indexNW;
  float neMag,sumGrad,eMag,swMag,wMag;
  float nMag,sMag,seMag,nwMag;

  canny_create_masks(kernelRadius, kernelWidth, params);

  initX = kernelWidth - 1;
  maxX = width - (kernelWidth - 1);
  initY = width * (kernelWidth - 1);
  maxY = width * (height - (kernelWidth - 1));
  w = width;

  for (y = initY; y < maxY; y+= w) {
    for(x = initX; x < maxX; x++) {
      index = x + y;
      sumX = data[index] * params->kernel.Data[0];
      sumY = sumX;
      xOffset = 1;
      yOffset = w;
      index1 = index - yOffset;
      index2 = index + yOffset;
      index3 = index - xOffset;
      index4 = index + xOffset;
      k = 0.0f;
      for (xOffset = 1; xOffset < kernelWidth;
           xOffset++, index3--,index4++) {
        k = params->kernel.Data[xOffset];

        sumY += k * (data[index1] + data[index2]);
        sumX += k * (data[index3] + data[index4]);
        index1 -= w;
        index2 += w;
      }

      params->yConv[index] = sumY;
      params->xConv[index] = sumX;
    }
  }

  kern = params->diffKernel.Data;
  for(x = initX; x < maxX; x++) {
    for(y = initY; y < maxY; y += w) {
      sum = 0.0f;
      index = x + y;
      for (i = 1; i < kernelWidth; i++) {
        sum +=
          kern[i] *
          (params->yConv[index - i] -
           params->yConv[index + i]);
      }
      params->xGradient[index] = sum;
    }
  }

  for (x = (int)kernelWidth; x < w - (int)kernelWidth; x++) {
    for (y = initY; y < maxY; y += w) {
      sum = 0.0f;
      index = x + y;
      yOffset = w;
      for (i = 1; i < kernelWidth; i++) {
        sum +=
          kern[i] *
          (params->xConv[index - yOffset] -
           params->xConv[index + yOffset]);
        yOffset += w;
      }

      params->yGradient[index] = sum;
    }
  }

  initX = kernelWidth;
  maxX = width - kernelWidth;
  initY = width * kernelWidth;
  maxY = width * (height - kernelWidth);

  for (y = initY; y < maxY; y += width) {
    for (x = initX; x < maxX; x++) {
      index = x + y;

      xGrad = params->xGradient[index];
      yGrad = params->yGradient[index];

      gradMag = SQUARE_MAG(xGrad, yGrad);

      /* perform non-maximal supression */
      tmp = 0;

      xGrad_abs = ABS(xGrad);

      yGrad_abs = ABS(yGrad);

      is_edge = 0;

      if (xGrad * yGrad <= 0.0f) {
        indexNE = index - w + 1;
        neMag = SQUARE_MAG(params->xGradient[indexNE],
                           params->yGradient[indexNE]);
        sumGrad = xGrad + yGrad;

        if (xGrad_abs >= yGrad_abs) {
          indexE = index + 1;
          eMag = SQUARE_MAG(params->xGradient[indexE],
                            params->yGradient[indexE]);
          if ((tmp = xGrad_abs * gradMag) >=
              ABS((yGrad * neMag) - (sumGrad * eMag))) {
            indexSW = index + w - 1;
            indexW = index - 1;
            swMag = SQUARE_MAG(params->xGradient[indexSW],
                               params->yGradient[indexSW]);
            wMag = SQUARE_MAG(params->xGradient[indexW],
                              params->yGradient[indexW]);

            if (tmp >
                ABS((yGrad * swMag) - (sumGrad * wMag)))
              is_edge = 1;
          }
        }
        else {
          indexN = index - w;
          nMag = SQUARE_MAG(params->xGradient[indexN],
                            params->yGradient[indexN]);
          if ((tmp = ABS(yGrad * gradMag)) >=
              ABS((xGrad * neMag) - (sumGrad * nMag))) {
            indexS = index + w;
            indexSW = indexS - 1;
            swMag = SQUARE_MAG(params->xGradient[indexSW],
                               params->yGradient[indexSW]);
            sMag = SQUARE_MAG(params->xGradient[indexS],
                              params->yGradient[indexS]);

            if (tmp > ABS((xGrad * swMag) - (sumGrad * sMag))) {
              is_edge = 1;
            }
          }
        }
      }
      else {
        indexSE = index + w + 1;

        seMag = SQUARE_MAG(params->xGradient[indexSE],
                           params->yGradient[indexSE]);
        if (xGrad_abs >= yGrad_abs) {
          indexE = index + 1;
          eMag = SQUARE_MAG(params->xGradient[indexE],
                            params->yGradient[indexE]);
          if ((tmp = xGrad_abs * gradMag) >=
              ABS((yGrad * seMag) + ((xGrad - yGrad) * eMag)))
            {
              indexNW = index - w - 1;
              indexW = index - 1;
              nwMag = SQUARE_MAG(params->xGradient[indexNW],
                                 params->yGradient[indexNW]);
              wMag = SQUARE_MAG(params->xGradient[indexW],
                                params->yGradient[indexW]);

              if (tmp >
                  ABS((yGrad * nwMag) +
                      ((xGrad - yGrad) * wMag))) {
                is_edge = 1;
              }
            }
        }
        else {
          indexS = index + w;
          sMag = SQUARE_MAG(params->xGradient[indexS],
                            params->yGradient[indexS]);
          if ((tmp = yGrad_abs * gradMag) >=
              ABS((xGrad * seMag) + ((yGrad - xGrad) * sMag))) {
            indexN = index - w;
            indexNW = indexN - 1;
            nwMag = SQUARE_MAG(params->xGradient[indexNW],
                               params->yGradient[indexNW]);
            nMag = SQUARE_MAG(params->xGradient[indexN],
                              params->yGradient[indexN]);
            if (tmp >
                ABS((xGrad * nwMag) +
                    ((yGrad - xGrad) * nMag)))
              is_edge = 1;
          }
        }
      }

      if (is_edge!=0) {
        if (gradMag >= params->MAGNITUDE_LIMIT_SQR)
          params->edge_magnitude[index] = -1;
        else
          params->edge_magnitude[index] = gradMag;

        params->edge_pixel_index[no_of_edges++] = index;

        if (gradMag < 0)
          params->magnitude[index] = params->MAGNITUDE_MAX;
        else
          params->magnitude[index] =
            (int)(params->MAGNITUDE_SCALE * sqrt(gradMag));
      }
    }
  }

  return(no_of_edges);
}

/**
 * \brief canny edge detection
 * \param width width of the image
 * \param height height of the image
 * \param data
 * \param edges array storing edges
 * \param no_of_edges returned number of edges
 */
static void canny_threshold_edges(int width, int height,
                                  int data[],
                                  int edges[],
                                  int * no_of_edges)
{
  int x,y,i;

  *no_of_edges = 0;
  x = width-1;
  y = height-1;
  for (i = (width * height) - 1; i >= 0; i--) {
    if (data[i] != 0) {
      data[i] = 0;
      if (*no_of_edges<MAX_EDGES) {
        edges[*no_of_edges * 2] = x;
        edges[*no_of_edges * 2 + 1] = y;
        *no_of_edges = *no_of_edges + 1;
      }
    }
    else {
      data[i] = 255;
    }
    x--;
    if (x < 0) {
      x += width;
      y--;
    }
  }
}

/**
 * \brief canny edge detection algorithm
 * \param img mono image array
 * \param width width of the image
 * \param height height of the image
 * \param params object used to store the results of edge detection
 */
static void canny_update(unsigned char img[],
                         int width, int height,
                         struct canny_params * params)
{
  int low = 0;
  int high = 0;
  int no_of_edges, pixels,x,y,i;

  if (params->automaticThresholds!=0) {
    canny_auto_threshold(img, width, height, 2, params);
  }

  params->picSize = width * height;


  canny_init_arrays(params);

  for (i=0;i<(int)params->picSize;i++) {
    params->data[i] = img[i];
  }

  no_of_edges =
    canny_compute_gradients(params->data,width,height,
                            params->gaussianKernelRadius,
                            params->gaussianKernelWidth,
                            params);

  low = (int)((params->lowThreshold *
               params->MAGNITUDE_SCALE) + 0.5f);
  high = (int)((params->highThreshold *
                params->MAGNITUDE_SCALE) + 0.5f);

  canny_perform_hysteresis(width,height,
                           low, high,
                           params->data, no_of_edges,
                           params);

  canny_threshold_edges(width,height,params->data,
                        params->edges,&(params->no_of_edges));

  pixels = width * height;

  for (i = pixels-1; i >= 0; i--) {
    params->edges_image[i] = 255;
  }

  for (i = 0; i < params->no_of_edges*2; i += 2) {
    x = params->edges[i];
    y = params->edges[i + 1];
    params->edges_image[(y * width) + x] = 0;
  }
}

/**
 * \brief Canny edge detection
 * \param img mono image array
 * \param width width of the image
 * \param height height of the image
 * \param threshold canny edge threshold
 * \param radius canny edge radius
 */
void detect_edges(unsigned char img[],
                  int width, int height,
                  float threshold, float radius)
{
  int i;

  /* initialise */
  canny_init(&cannyparams, radius);

  canny_update(img,width,height,&cannyparams);

  for (i = (width*height) - 1; i >= 0; i--) {
    img[i] = cannyparams.edges_image[i];
  }

  /* free memory */
  canny_free(&cannyparams);
}


/**
 * \brief traces along a set of edges and returns the edge members and the perimeter
 * \param edges_image image array containing edges
 * \param width width of the image
 * \param height height of the image
 * \param x starting x coordinate
 * \param y starting y coordinate
 * \param image_border
 * \param perimeter_tx returned top left x coordinate of the bounding box
 * \param perimeter_ty returned top left y coordinate of the bounding box
 * \param perimeter_bx returned bottom right x coordinate of the bounding box
 * \param perimeter_by returned bottom right y coordinate of the bounding box
 * \param ignore_periphery
 * \param max_members maximum segment length
 * \param members array containing line segment edges
 * \param no_of_members returned number of edges within the line segment
 */
static void trace_edge(unsigned char edges_image[],
                       int width, int height,
                       int x, int y,
                       int image_border,
                       int * perimeter_tx,
                       int * perimeter_ty,
                       int * perimeter_bx,
                       int * perimeter_by,
                       int ignore_periphery,
                       int max_members,
                       int members[],
                       int * no_of_members)
{
  int following,n,xx,yy;

  *no_of_members = 0;

  following = 1;
  while (following!=0) {
    following = 0;

    /* if we encounter the periphery of the
       image then abandon the trace */
    if ((ignore_periphery==0) ||
        ((ignore_periphery!=0) &&
         ((x >= image_border) &&
          (x <= width - 1 - image_border) &&
          (y >= image_border) &&
          (y <= height - 1 - image_border)))) {

      /* this edge has been followed */
      edges_image[(y * width) + x] = 255;

      /* add this point to the list */
      if (*no_of_members < max_members) {
        members[(*no_of_members)*2] = x;
        members[(*no_of_members)*2+1] = y;
        *no_of_members = (*no_of_members) + 1;
      }

      /* update the bounding box */
      if (x < *perimeter_tx) {
        *perimeter_tx = x;
      }
      else {
        if (x > *perimeter_bx) *perimeter_bx = x;
      }

      if (y < *perimeter_ty) {
        *perimeter_ty = y;
      }
      else {
        if (y > *perimeter_by) *perimeter_by = y;
      }

      /* unrolled loops */
      yy = y - 1;
      xx = x - 1;
      if (yy > -1) {
        n = yy * width;
        if (xx > -1) {
          if (edges_image[n + xx]==0) {
            x = xx;
            y = yy;
            following = 1;
          }
        }
        xx = x;
        if ((following==0) &&
            (edges_image[n + xx]==0)) {
          x = xx;
          y = yy;
          following = 1;
        }
        xx = x + 1;
        if ((following==0) && (xx < width)) {
          if (edges_image[n + xx]==0) {
            x = xx;
            y = yy;
            following = 1;
          }
        }
      }
      if (following == 0) {
        yy = y;
        n = yy * width;
        if (x > 0) {
          xx = x - 1;
          if (edges_image[n + xx]==0) {
            x = xx;
            y = yy;
            following = 1;
          }
        }
        if ((following==0) && (x < width - 1)) {
          xx = x + 1;
          if (edges_image[n + xx]==0) {
            x = xx;
            y = yy;
            following = 1;
          }
        }
      }
      if (following==0) {
        yy = y + 1;
        n = yy * width;
        if (yy < height) {
          xx = x - 1;
          if (xx > -1) {
            if (edges_image[n + xx] == 0) {
              x = xx;
              y = yy;
              following = 1;
            }
          }
          if (following == 0) {
            xx++;
            if (edges_image[n + xx] == 0) {
              x = xx;
              y = yy;
              following = 1;
            }
          }
          if (following == 0) {
            xx++;
            if (xx < width) {
              if (edges_image[n + xx] == 0) {
                x = xx;
                y = yy;
                following = 1;
              }
            }
          }
        }
      }
    }
  }
}


/**
 * \brief traces along a set of edges and returns the edge members and the perimeter
 * \param edges_image image array containing edges
 * \param width width of the image
 * \param height height of the image
 * \param segments object containing line segments
 */
static void trace_edges(unsigned char edges_image[],
                        int width, int height,
                        struct line_segments * segments)
{
  int x,y,n,index=0,max;
  int tx = segments->image_border;
  int ty = segments->image_border;
  int bx = width - segments->image_border - 1;
  int by = height - segments->image_border - 1;
  int border_tx,border_ty,border_bx,border_by;
  int * memb,no_of_memb=0;

  segments->no_of_segments = 0;

  for (y=ty;y<=by;y++) {
    for (x=tx;x<=bx;x++) {
      n = y*width + x;
      if (edges_image[n]==0) {
        border_tx = width;
        border_ty = height;
        border_bx = 0;
        border_by = 0;

        memb = &(segments->members[index*2]);
        max = segments->max_members - index - 1;
        trace_edge(edges_image, width, height,
                   x, y, segments->image_border,
                   &border_tx, &border_ty,
                   &border_bx, &border_by,
                   segments->ignore_periphery,
                   max,memb,
                   &no_of_memb);
        segments->no_of_members[segments->no_of_segments] =
          no_of_memb;

        if (segments->no_of_members[segments->no_of_segments] >
            segments->minimum_segment_length) {
          index +=
            segments->no_of_members[segments->no_of_segments];
          segments->no_of_segments++;
          if (segments->no_of_segments >=
              segments->max_segments) return;
        }

      }
    }
  }
}

/**
 * /brief extracts line segments from the edges image
 * \param edges_image image array containing edges
 * \param width width of the image
 * \param height height of the image
 * \param segments object containing line segments
 * \param min_segment_length minimum length of a line segment
 */
void get_line_segments(unsigned char edges_image[],
                       int width, int height,
                       struct line_segments * segments,
                       int min_segment_length)
{
  init_line_segments(segments, width, height);
  segments->minimum_segment_length = min_segment_length;
  trace_edges(edges_image,width,height,segments);
}

/**
 * \brief shows line segments within the given image
 * \param segments object containing line segments
 * \param result image array
 * \param width width of the image
 * \param height height of the image
 * \param result_bitsperpixel Number of bits per pixel
 */
void show_line_segments(struct line_segments * segments,
                        unsigned char result[], int width, int height,
                        int result_bitsperpixel)
{
  int i,j,index=0,x,y,n;
  unsigned char r,g,b;
  int result_bytesperpixel = result_bitsperpixel/8;

  memset(result, 0, width*height*result_bytesperpixel);

  for (i = 0; i < segments->no_of_segments; i++) {
    srand(i);
    for (j = 0; j < i; j++) {
      if (segments->joins[(j*segments->max_segments)+i] > 0) {
        srand(j);
        break;
      }
    }
    r = (unsigned char)(rand()%255);
    g = (unsigned char)(rand()%255);
    b = (unsigned char)(rand()%255);
    for (j = 0; j < segments->no_of_members[i]; j++,index++) {
      x = segments->members[index*2];
      y = segments->members[index*2+1];
      n = (y*width*result_bytesperpixel)+(x*result_bytesperpixel);
      result[n] = r;
      result[n+1] = g;
      result[n+2] = b;
    }
  }
}

/**
 * \brief line segment edges within a region of interest
 * \param segments object containing line segments
 * \param width width of the image
 * \param height height of the image
 * \param roi_radius_percent region of interest radius around the centre
 *        of the image as a percentage of image width
 */
void segment_edges_within_roi(struct line_segments * segments,
                              int width, int height,
                              int roi_radius_percent)
{
  int i, j, x, y, dx, dy, index=0, index_adjusted;
  int cx = width/2;
  int cy = height/2;
  int roi_radius_pixels = roi_radius_percent * width / 100;
  int removed_members, removed_total=0;

  roi_radius_pixels *= roi_radius_pixels;
  for (i = 0; i < segments->no_of_segments; i++) {
    removed_members = 0;
    for (j = 0; j < segments->no_of_members[i]; j++,index++) {
      x = segments->members[index*2];
      y = segments->members[index*2+1];
      if (removed_total > 0) {
        index_adjusted = index - removed_total;
        segments->members[index_adjusted*2] = x;
        segments->members[index_adjusted*2+1] = y;
      }
      dx = x - cx;
      dy = y - cy;
      if ((dx*dx) + (dy*dy) > roi_radius_pixels) {
        /* outside of the roi */
        removed_members++;
        removed_total++;
      }
    }
    segments->no_of_members[i] -= removed_members;
  }
}

/**
 * \brief returns the index of the start of a sequence of segments
 * \param segments object containing line segments
 * \param index of the line segment
 * \return start line segment index
 */
static int get_joined_segment_start(struct line_segments * segments,
                                    int index)
{
  int j;

  for (j = 0; j < index; j++) {
    if (segments->joins[(j*segments->max_segments)+index] != 0) {
      return j;
    }
  }
  return -1;
}

/**
 * \brief returns the index of the end of a sequence of segments
 * \param segments object containing line segments
 * \param index of the line segment
 * \return end line segment index
 */
static int get_joined_segment_end(struct line_segments * segments,
                                  int index)
{
  int j;

  for (j = index+1; j < segments->no_of_segments; j++) {
    if (segments->joins[(j*segments->max_segments)+index] != 0) {
      return j;
    }
  }
  return -1;
}

/**
 * \brief gets the total joined length of a given segment
 * \param segments object containing line segments
 * \param index of the line segment
 * \return length of the joined line segment
 */
static int get_joined_segment_length(struct line_segments * segments,
                                     int index)
{
  int total_length=segments->no_of_members[index];
  int prev_index=index, start_index=index;
  int next_index=index, end_index=index;

  while (prev_index != -1) {
    start_index = prev_index;
    prev_index = get_joined_segment_start(segments, start_index);
    if ((prev_index != start_index) && (prev_index != -1)) {
      total_length += segments->no_of_members[prev_index];
    }
  }
  while (next_index != -1) {
    end_index = next_index;
    next_index = get_joined_segment_end(segments, end_index);
    if ((next_index != end_index) && (next_index != -1)) {
      total_length += segments->no_of_members[next_index];
    }
  }
  return total_length;
}

/**
 * \brief join line segments which are close together
 * \param segments object containing line segments
 * \param join_radius radius within which to join line segments together
 */
void join_line_segments(struct line_segments * segments,
                        int join_radius)
{
  int i, j, start_x, start_y, end_x, end_y, dx, dy;
  int index, index2, length;
  int x, y, join_index, radius, join_radius_sqr;
  int max_length, max_length_index, ctr;

  /* clear the joins matrix */
  memset(segments->joins, 0,
         segments->max_segments * segments->max_segments *
         sizeof(unsigned char));

  /* populate the joins matrix, starting from the closest
     distance up to join_radius */
  for (radius = 1; radius <= join_radius; radius++) {
    join_radius_sqr = radius*radius;
    index = 0;
    for (i = 0; i < segments->no_of_segments; i++) {
      if (segments->no_of_members[i] == 0) continue;
      start_x = segments->members[index*2];
      start_y = segments->members[index*2+1];
      length = segments->no_of_members[i]-1;
      index += length;
      end_x = segments->members[index*2];
      end_y = segments->members[index*2+1];
      index2 = index+1;
      index++;
      for (j = i+1; j < segments->no_of_segments-1; j++) {
        if (segments->no_of_members[j] == 0) continue;
        x = segments->members[index2*2];
        y = segments->members[index2*2+1];
        dx = x - start_x;
        dy = y - start_y;
        if (dx*dx + dy*dy <= join_radius_sqr) {
          /* start points are joined */
          join_index = (j*segments->max_segments) + i;
          if (segments->joins[join_index] == 0) {
            segments->joins[join_index] = (unsigned char)1;
            join_index = (i*segments->max_segments) + j;
            segments->joins[join_index] = (unsigned char)1;
          }
        }
        dx = x - end_x;
        dy = y - end_y;
        if (dx*dx + dy*dy <= join_radius_sqr) {
          /* end joined to start */
          join_index = (j*segments->max_segments) + i;
          if (segments->joins[join_index] == 0) {
            segments->joins[join_index] = (unsigned char)2;
            join_index = (i*segments->max_segments) + j;
            segments->joins[join_index] = (unsigned char)3;
          }
        }
        length = segments->no_of_members[j]-1;
        index2 += length;
        x = segments->members[index2*2];
        y = segments->members[index2*2+1];
        index2++;
        dx = x - start_x;
        dy = y - start_y;
        if (dx*dx + dy*dy <= join_radius_sqr) {
          /* start joined to end */
          join_index = (j*segments->max_segments) + i;
          if (segments->joins[join_index] == 0) {
            segments->joins[join_index] = (unsigned char)3;
            join_index = (i*segments->max_segments) + j;
            segments->joins[join_index] = (unsigned char)2;
          }
        }
        dx = x - end_x;
        dy = y - end_y;
        if (dx*dx + dy*dy <= join_radius_sqr) {
          /* ends joined */
          join_index = (j*segments->max_segments) + i;
          if (segments->joins[join_index] == 0) {
            segments->joins[join_index] = (unsigned char)4;
            join_index = (i*segments->max_segments) + j;
            segments->joins[join_index] = (unsigned char)4;
          }
        }
      }
    }
  }

  /* update joined segment lengths */
  for (i = 0; i < segments->no_of_segments; i++) {
    segments->joined_length[i] = get_joined_segment_length(segments, i);
  }

  /* clear the sorted joins matrix */
  memset(segments->joins_sorted, 0,
         segments->max_segments * sizeof(int));
  memset(segments->selected, 0,
         segments->max_segments * sizeof(unsigned char));

  /* sort joined segments into length order */
  ctr = 0;
  for (i = 0; i < segments->no_of_segments; i++) {
    max_length = 0;
    max_length_index = -1;
    for (j = 0; j < segments->no_of_segments; j++) {
      if (segments->selected[j] != 0) continue;
      if (segments->joined_length[j] >= max_length) {
        max_length = segments->joined_length[j];
        max_length_index = j;
      }
    }
    if (max_length_index == -1) {
      printf("No max length found\n");
      break;
    }
    segments->selected[max_length_index] = 1;
    segments->joins_sorted[ctr++] = max_length_index;
  }
}

/**
 * \brief returns the bounding box for the given segment
 * \param segments object containing line segments
 * \param index index of the line segment
 * \param min_x returned top left x coordinate for the bounding box
 * \param min_y returned top left y coordinate for the bounding box
 * \param max_x returned bottom right x coordinate for the bounding box
 * \param max_y returned bottom right y coordinate for the bounding box
 */
static void get_segment_bounding_box(struct line_segments * segments,
                                     int index,
                                     int * min_x, int * min_y,
                                     int * max_x, int * max_y)
{
  int i, j, x, y, idx=0;

  for (i = 0; i < index; i++) idx += segments->no_of_members[i];

  for (j = 0; j < segments->no_of_members[i]; j++,idx++) {
    x = segments->members[idx*2];
    y = segments->members[idx*2+1];
    if ((x < *min_x) || (*min_x == -1)) {
      *min_x = x;
    }
    if (x > *max_x) {
      *max_x = x;
    }
    if ((y < *min_y) || (*min_y == -1)) {
      *min_y = y;
    }
    if (y > *max_y) {
      *max_y = y;
    }
  }
}

/**
 * \brief returns the aspect ratio of the given segment
 * \param segments object containing line segments
 * \param index index of the line segment
 * \return integer aspect ratio (x100)
 */
int get_segment_aspect_ratio(struct line_segments * segments,
                             int index)
{
  int min_x=-1, min_y=-1;
  int max_x=-1, max_y=-1;
  int dx, dy;
  int prev_index=index, start_index=index;
  int next_index=index, end_index=index;

  get_segment_bounding_box(segments, index,
                           &min_x, &min_y, &max_x, &max_y);

  while (prev_index != -1) {
    start_index = prev_index;
    prev_index = get_joined_segment_start(segments, start_index);
    if ((prev_index != start_index) && (prev_index != -1)) {
      get_segment_bounding_box(segments, prev_index,
                               &min_x, &min_y, &max_x, &max_y);
    }
  }
  while (next_index != -1) {
    end_index = next_index;
    next_index = get_joined_segment_end(segments, end_index);
    if ((next_index != end_index) && (next_index != -1)) {
      get_segment_bounding_box(segments, next_index,
                               &min_x, &min_y, &max_x, &max_y);
    }
  }
  dx = max_x - min_x;
  dy = max_y - min_y;
  return dx * 100 / (1+dy);
}

/**
 * \brief shows square shaped line segments
 * \param segments object containing line segments
 * \param result image within which to show the line segments
 * \param width width of the image
 * \param height height of the image
 * \param result_bitsperpixel Number of bits per pixel
 */
void show_square_line_segments(struct line_segments * segments,
                               unsigned char result[], int width, int height,
                               int result_bitsperpixel)
{
  int i,j,index=0,x,y,n,aspect_ratio;
  unsigned char r,g,b;
  int result_bytesperpixel = result_bitsperpixel/8;

  memset(result, 0, width*height*result_bytesperpixel);

  for (i = 0; i < segments->no_of_segments; i++) {
    aspect_ratio = get_segment_aspect_ratio(segments, i);
    if ((aspect_ratio < 90) || (aspect_ratio > 110)) {
      index += segments->no_of_members[i];
      continue;
    }

    srand(i);
    r = (unsigned char)(rand()%255);
    g = (unsigned char)(rand()%255);
    b = (unsigned char)(rand()%255);
    for (j = 0; j < segments->no_of_members[i]; j++,index++) {
      x = segments->members[index*2];
      y = segments->members[index*2+1];
      n = (y*width*result_bytesperpixel)+(x*result_bytesperpixel);
      result[n] = r;
      result[n+1] = g;
      result[n+2] = b;
    }
  }
}

/**
 * \brief returns 1 if the given aspect ratio could correspond to
 *        a rectangular datamatrix
 * \param aspect_ratio aspect ratio (x100) of joined line segments
 * \return 1 if the aspect ratio is rectangular
 */
unsigned char rectangular_joined_line_segments(int aspect_ratio)
{
  int i, possible_aspect_ratio, min_aspect_ratio, max_aspect_ratio;
  const int aspect_tollerance = 10;
  const int no_of_valid_rectangles = 6;
  const int IEC16022_valid_rectangles[] = {
    8, 18,
    8, 32,
    12, 26,
    12, 36,
    16, 36,
    16, 48
  };

  for (i = 0; i < no_of_valid_rectangles; i++) {
    if (aspect_ratio < 100) {
      possible_aspect_ratio =
        IEC16022_valid_rectangles[i*2] * 100 / IEC16022_valid_rectangles[i*2+1];
    }
    else {
      possible_aspect_ratio =
        IEC16022_valid_rectangles[i*2+1] * 100 / IEC16022_valid_rectangles[i*2];
    }
    min_aspect_ratio = possible_aspect_ratio - aspect_tollerance;
    max_aspect_ratio = possible_aspect_ratio + aspect_tollerance;
    if ((aspect_ratio >= min_aspect_ratio) ||
        (aspect_ratio <= max_aspect_ratio)) {
      return (unsigned char)1;
    }
  }
  return (unsigned char)0;
}

/**
 * \brief shows square shaped line segments
 * \param segments object containing line segments
 * \param result image within which to show the line segments
 * \param width width of the image
 * \param height height of the image
 * \param result_bitsperpixel Number of bits per pixel
 */
void show_rectangular_line_segments(struct line_segments * segments,
                                    unsigned char result[], int width, int height,
                                    int result_bitsperpixel)
{
  int i,j,index=0,x,y,n,aspect_ratio;
  unsigned char r,g,b;
  int result_bytesperpixel = result_bitsperpixel/8;

  memset(result, 0, width*height*result_bytesperpixel);

  for (i = 0; i < segments->no_of_segments; i++) {
    aspect_ratio = get_segment_aspect_ratio(segments, i);

    if (rectangular_joined_line_segments(aspect_ratio) == 0) {
      index += segments->no_of_members[i];
      continue;
    }

    srand(i);
    r = (unsigned char)(rand()%255);
    g = (unsigned char)(rand()%255);
    b = (unsigned char)(rand()%255);
    for (j = 0; j < segments->no_of_members[i]; j++,index++) {
      x = segments->members[index*2];
      y = segments->members[index*2+1];
      n = (y*width*result_bytesperpixel)+(x*result_bytesperpixel);
      result[n] = r;
      result[n+1] = g;
      result[n+2] = b;
    }
  }
}

/**
 * \brief updates arrays used to find the periphery of a square, using
 *        the given segment
 * \param segments object containing line segments
 * \param index index of the line segment
 */
static void update_peripheral(struct line_segments * segments,
                              int index)
{
  int i, x, y, idx=0;

  for (i = 0; i < index; i++) idx += segments->no_of_members[i];

  for (i = 0; i < segments->no_of_members[index]; i++,idx++) {
    x = segments->members[idx*2];
    y = segments->members[idx*2+1];

    segments->edge_centre_x += x;
    segments->edge_centre_y += y;
    segments->edge_centre_hits++;

    /* left */
    if ((segments->perimeter_left[y] == 0) || (x < segments->perimeter_left[y]))
      segments->perimeter_left[y] = x;

    /* right */
    if ((segments->perimeter_right[y] == 0) || (x > segments->perimeter_right[y]))
      segments->perimeter_right[y] = x;

    /* top */
    if ((segments->perimeter_top[x] == 0) || (y < segments->perimeter_top[x]))
      segments->perimeter_top[x] = y;

    /* bottom */
    if ((segments->perimeter_bottom[x] == 0) || (y > segments->perimeter_bottom[x]))
      segments->perimeter_bottom[x] = y;
  }
}

/**
 * \brief for a given segment find the peripheral edges used to create a square
 * \param segments object containing line segments
 * \param index index of the line segment
 * \param width width of the image
 * \param height height of the image
 */
void get_peripheral_edges(struct line_segments * segments,
                          int index, int width, int height)
{
  int min_x=-1, min_y=-1;
  int max_x=-1, max_y=-1;
  int dx, dy;
  int prev_index=index, start_index=index;
  int next_index=index, end_index=index;

  /* clear perimeter arrays */
  memset(segments->perimeter_left, 0, height*sizeof(int));
  memset(segments->perimeter_right, 0, height*sizeof(int));
  memset(segments->perimeter_top, 0, width*sizeof(int));
  memset(segments->perimeter_bottom, 0, width*sizeof(int));

  get_segment_bounding_box(segments, index,
                           &min_x, &min_y, &max_x, &max_y);

  dx = max_x - min_x;
  dy = max_y - min_y;

  if ((dx < 1) || (dy < 1)) return;

  segments->edge_centre_x = 0;
  segments->edge_centre_y = 0;
  segments->edge_centre_hits = 0;

  /* perimeter edges for the current segment */
  update_peripheral(segments, index);

  /* perimeter edges for the previous joined segments */
  while (prev_index != -1) {
    start_index = prev_index;
    prev_index = get_joined_segment_start(segments, start_index);
    if ((prev_index != start_index) && (prev_index != -1)) {
      update_peripheral(segments, prev_index);
    }
  }

  /* perimeter edges for the next joined segments */
  while (next_index != -1) {
    end_index = next_index;
    next_index = get_joined_segment_end(segments, end_index);
    if ((next_index != end_index) && (next_index != -1)) {
      update_peripheral(segments, next_index);
    }
  }

  /* calculate the centre of edges */
  if (segments->edge_centre_hits > 0) {
    segments->edge_centre_x /= segments->edge_centre_hits;
    segments->edge_centre_y /= segments->edge_centre_hits;
  }
}

/**
 * \brief assigns edges to a single perimeter side
 * \param segments object containing line segments
 * \param side index of the side (0-3)
 * \param histogram_length length of the orientation histogram
 * \param orientation_quantized2
 * \param orthogonal
 * \param separator_x0 x coordinate of the start of the demarcation line
 * \param separator_y0 y coordinate of the start of the demarcation line
 * \param separator_x1 x coordinate of the end of the demarcation line
 * \param separator_y1 y coordinate of the end of the demarcation line
 */
static void assign_edges_to_side(struct line_segments * segments,
                                 int side, int histogram_length,
                                 int orientation_quantized2,
                                 int orthogonal,
                                 float separator_x0, float separator_y0,
                                 float separator_x1, float separator_y1)
{
  int i, size, side_index, edge_index, prev_x, prev_y, x, y;
  int index = orientation_quantized2;
  float dist_to_separator, ix, iy;

  /* are there any edges at this histogram orientation? */
  if (segments->orientation_histogram[index] == 0) return;

  size = segments->orientation_histogram[index]*4;
  for (i = 0; i < size; i += 4) {
    /* get the edge coordinates */
    prev_x = segments->orientation_histogram_edges[index][i];
    prev_y = segments->orientation_histogram_edges[index][i+1];
    x = segments->orientation_histogram_edges[index][i+2];
    y = segments->orientation_histogram_edges[index][i+3];

    /* squared distance of the edge from the separator line */
    dist_to_separator =
      point_dist_from_line(separator_x0, separator_y0,
                           separator_x1, separator_y1,
                           (float)x, (float)y, &ix, &iy);
    if (dist_to_separator == UNKNOWN_DISTANCE) continue;

    if (orthogonal == 0) {
      side_index = 0;
      if (dist_to_separator > 0)
        side_index = 1;

      if (segments->side_edges_count[side_index] < MAX_ORIENTATION_EDGES) {
        edge_index = segments->side_edges_count[side_index]*2;
        segments->side_edges[side_index][edge_index] = prev_x;
        segments->side_edges[side_index][edge_index+1] = prev_y;
        segments->side_edges_count[side_index]++;
      }

      if (segments->side_edges_count[side_index] < MAX_ORIENTATION_EDGES) {
        edge_index = segments->side_edges_count[side_index]*2;
        segments->side_edges[side_index][edge_index] = x;
        segments->side_edges[side_index][edge_index+1] = y;
        segments->side_edges_count[side_index]++;
      }
      continue;
    }

    side_index = 2;
    if (dist_to_separator > 0)
      side_index = 3;

    if (segments->side_edges_count[side_index] < MAX_ORIENTATION_EDGES) {
      edge_index = segments->side_edges_count[side_index]*2;
      segments->side_edges[side_index][edge_index] = prev_x;
      segments->side_edges[side_index][edge_index+1] = prev_y;
      segments->side_edges_count[side_index]++;
    }

    if (segments->side_edges_count[side_index] < MAX_ORIENTATION_EDGES) {
      edge_index = segments->side_edges_count[side_index]*2;
      segments->side_edges[side_index][edge_index] = x;
      segments->side_edges[side_index][edge_index+1] = y;
      segments->side_edges_count[side_index]++;
    }
  }
}

/**
 * \brief assigns edges to all sides of a perimeter using an orientation histogram
 * \param segments object containing line segments
 * \param orientation_quantized histogram index of the detected orientation
 * \param orientation_radians orientation of the shape in radians
 * \param quantization_degrees angular quantization used for orientation histogram
 */
static void assign_edges_to_sides(struct line_segments * segments,
                                  int orientation_quantized,
                                  float orientation_radians,
                                  int quantization_degrees)
{
  int orient, orthogonal, opposite, side;
  int no_of_buckets = (int)(360 / quantization_degrees);
  int histogram_length = no_of_buckets;
  int orientation_quantized2;
  int orientation_quantized_wiggle;
  float separator_x0 = segments->edge_centre_x;
  float separator_y0 = segments->edge_centre_y;
  float separator_x1, separator_y1;

  for (orthogonal = 0; orthogonal < 2; orthogonal++) {
    if (orthogonal == 0) {
      orient = orientation_quantized;
    }
    else {
      /* orientation plus 90 degrees (orthogonal direction) */
      orient = orientation_quantized + (no_of_buckets / 4);
    }

    /* calculate the separator line */
    separator_x1 =
      segments->edge_centre_x - (100 * (float)sin(orientation_radians + (orthogonal * (PI * 0.5f))));
    separator_y1 =
      segments->edge_centre_y - (100 * (float)cos(orientation_radians + (orthogonal * (PI * 0.5f))));

    /* two opposite sides */
    for (opposite = 0; opposite <= 1; opposite++, orient += (no_of_buckets / 2)) {
      /* try a few orientations in the vicinity */
      for (orientation_quantized_wiggle = orient - 1;
           orientation_quantized_wiggle <= orient + 1;
           orientation_quantized_wiggle++) {
        /* check that the orientation is in range */
        orientation_quantized2 = orientation_quantized_wiggle;
        if (orientation_quantized2 < 0) orientation_quantized2 += no_of_buckets;
        if (orientation_quantized2 >= no_of_buckets) orientation_quantized2 -= no_of_buckets;
        /* assign edges for each side of the square */
        for (side = 0; side < 4; side++) {
          assign_edges_to_side(segments, side, histogram_length,
                               orientation_quantized2,
                               orthogonal,
                               separator_x0, separator_y0,
                               separator_x1, separator_y1);
        }
      }
    }
  }
}

/**
 * \brief knowing the dominant orientation is useful because the
 *        square may be rotated such that left and right edge arrays
 *        are insufficient to get a good line fit.
 *        Once the orientation is known then we can figure out which
 *        edges correspond to which sides of the square
 * \param segments object storing line segments
 * \param width width of the image
 * \param height height of the image
 * \param quantization_degrees used to create orientation histogram buckets
 * \return orientation in radians
 */
float get_segments_orientation(struct line_segments * segments,
                               int width, int height,
                               int quantization_degrees)
{
  const int no_of_prev_coords = 8;
  const float twopi = (float)PI * 2;
  int * perimeter;
  int i, x, y, prev_coords[no_of_prev_coords*2], prev_coords_ctr;
  int ctr, prev_index, prev_x, prev_y, bucket, opposite_bucket, dx, dy;
  int hits, peak_hits=0, orientation_quantized=0;
  int half_histogram, side, leftright, topbottom, edge_index;
  float dist, angle, angle_degrees, orientation_radians;
  int no_of_buckets = (int)(360 / quantization_degrees);

  /* clear the orientation histogram */
  memset(segments->orientation_histogram, 0, 360*sizeof(int));

  /* clear the side edges count */
  for (side = 0; side < 4; side++) {
    segments->side_edges_count[side] = 0;
  }

  /* use the left and right edge arrays to populate the
     orientation histogram */
  for (leftright = 0; leftright < 2; leftright++) {
    if (leftright == 0) {
      perimeter = segments->perimeter_left;
    }
    else {
      perimeter = segments->perimeter_right;
    }
    /* add left/right side edges to the histogram */
    prev_coords_ctr = 0;
    ctr = 0;
    for (y = 0; y < height; y++) {
      /* skip unpopulated parts of the array */
      if (perimeter[y] == 0) continue;
      if ((perimeter[y] < 0) || (perimeter[y] >= width)) {
        continue;
      }
      /* if enough previous edge coordinates have been gathered */
      if (ctr > no_of_prev_coords) {
        prev_index = (ctr - no_of_prev_coords) % no_of_prev_coords;
        prev_x = prev_coords[prev_index*2];
        prev_y = prev_coords[prev_index*2+1];

        /* distance to the previous edge coordinate */
        dx = perimeter[y] - prev_x;
        dy = y - prev_y;
        dist = (dx * dx) + (dy * dy);
        angle = 0;
        if (dist > 0.001f) {
          dist = (float)sqrt(dist);
          /* angle to the previous edge coordinate */
          angle = (float)acos(dy / dist);
        }
        if (dx < 0) angle = twopi - angle;
        angle_degrees = angle / PI * 180.0f;
        /* update the histogram */
        bucket = (int)(angle_degrees / quantization_degrees);
        if ((bucket >= 0) && (bucket < no_of_buckets)) {
          if (segments->orientation_histogram[bucket] < MAX_ORIENTATION_EDGES-1) {
            /* store the edge */
            edge_index = segments->orientation_histogram[bucket]*4;
            segments->orientation_histogram_edges[bucket][edge_index] = prev_x;
            segments->orientation_histogram_edges[bucket][edge_index+1] = prev_y;
            segments->orientation_histogram_edges[bucket][edge_index+2] = perimeter[y];
            segments->orientation_histogram_edges[bucket][edge_index+3] = y;
            /* update the histogram */
            segments->orientation_histogram[bucket]++;

            /* opposite direction */
            opposite_bucket = (bucket + (no_of_buckets/2)) % no_of_buckets;
            /* store the edge */
            edge_index = segments->orientation_histogram[opposite_bucket]*4;
            segments->orientation_histogram_edges[opposite_bucket][edge_index] = perimeter[y];
            segments->orientation_histogram_edges[opposite_bucket][edge_index+1] = y;
            segments->orientation_histogram_edges[opposite_bucket][edge_index+2] = prev_x;
            segments->orientation_histogram_edges[opposite_bucket][edge_index+3] = prev_y;
            /* update the histogram */
            segments->orientation_histogram[opposite_bucket]++;
          }
        }
      }

      /* store previous coordinates */
      prev_coords[prev_coords_ctr*2] = perimeter[y];
      prev_coords[prev_coords_ctr*2+1] = y;
      prev_coords_ctr++;
      if (prev_coords_ctr >= no_of_prev_coords) prev_coords_ctr = 0;
      ctr++;
    }
  }

  /* use the top and bottom edge arrays to populate the
     orientation histogram */
  for (topbottom = 0; topbottom < 2; topbottom++) {
    if (topbottom == 0) {
      perimeter = segments->perimeter_top;
    }
    else {
      perimeter = segments->perimeter_bottom;
    }
    /* add top/bottom side edges to the histogram */
    prev_coords_ctr = 0;
    ctr = 0;
    for (x = 0; x < width; x++) {
      /* skip unpopulated parts of the array */
      if (perimeter[x] == 0) continue;
      if ((perimeter[x] < 0) || (perimeter[x] >= height)) {
        continue;
      }

      /* if enough previous edge coordinates have been gathered */
      if (ctr > no_of_prev_coords) {
        prev_index = (ctr - no_of_prev_coords) % no_of_prev_coords;
        prev_x = prev_coords[prev_index*2];
        prev_y = prev_coords[prev_index*2+1];

        /* distance to the previous edge coordinate */
        dx = x - prev_x;
        dy = perimeter[x] - prev_y;
        dist = (dx * dx) + (dy * dy);
        angle = 0;
        if (dist > 0.001f) {
          dist = (float)sqrt(dist);
          /* angle to the previous edge coordinate */
          angle = (float)acos(dy / dist);
        }
        if (dx < 0) angle = twopi - angle;
        angle_degrees = angle / PI * 180.0f;
        /* update the histogram */
        bucket = (int)(angle_degrees / quantization_degrees);
        if ((bucket >= 0) && (bucket < no_of_buckets)) {
          if (segments->orientation_histogram[bucket] < MAX_ORIENTATION_EDGES-1) {
            /* store the edge */
            edge_index = segments->orientation_histogram[bucket]*4;
            segments->orientation_histogram_edges[bucket][edge_index] = prev_x;
            segments->orientation_histogram_edges[bucket][edge_index+1] = prev_y;
            segments->orientation_histogram_edges[bucket][edge_index+2] = x;
            segments->orientation_histogram_edges[bucket][edge_index+3] = perimeter[x];
            /* update the histogram */
            segments->orientation_histogram[bucket]++;

            /* opposite direction */
            opposite_bucket = (bucket + (no_of_buckets/2)) % no_of_buckets;
            /* store the edge */
            edge_index = segments->orientation_histogram[opposite_bucket]*4;
            segments->orientation_histogram_edges[opposite_bucket][edge_index] = x;
            segments->orientation_histogram_edges[opposite_bucket][edge_index+1] = perimeter[x];
            segments->orientation_histogram_edges[opposite_bucket][edge_index+2] = prev_x;
            segments->orientation_histogram_edges[opposite_bucket][edge_index+3] = prev_y;
            /* update the histogram */
            segments->orientation_histogram[opposite_bucket]++;
          }
        }
      }

      /* store previous coordinates */
      prev_coords[prev_coords_ctr*2] = x;
      prev_coords[prev_coords_ctr*2+1] = perimeter[x];
      prev_coords_ctr++;
      if (prev_coords_ctr >= no_of_prev_coords) prev_coords_ctr = 0;
      ctr++;
    }
  }

  /* look for the orientation histogram peak */
  half_histogram = (int)(180 / quantization_degrees);
  for (i = half_histogram-1; i >= 0; i--) {
    hits = segments->orientation_histogram[i] +
      segments->orientation_histogram[i + half_histogram];
    if (hits > peak_hits) {
      peak_hits = hits;
      orientation_quantized = i;
    }
  }

  /* calculate the orientation */
  orientation_radians =
    ((float)orientation_quantized * quantization_degrees) * PI / 180.0f;

  /* using the orientation, assign edges to sides */
  assign_edges_to_sides(segments, orientation_quantized,
                        orientation_radians,
                        quantization_degrees);

  return orientation_radians;
}

/**
 * \brief shows edges detected around a perimeter
 * \param segments object containing line segments
 * \param result image to be updated
 * \param width width of the image
 * \param height height of the image
 * \param result_bitsperpixel Number of bits per pixel
 */
void show_peripheral_edges(struct line_segments * segments,
                           unsigned char result[], int width, int height,
                           int result_bitsperpixel)
{
  int side, edge_index, x, y, n, no_of_edges;
  unsigned char r=0, g=0, b=0;
  int result_bytesperpixel = result_bitsperpixel/8;

  memset(result, 0, width*height*result_bytesperpixel);

  for (side = 0; side < 4; side++) {
    switch(side) {
    case 0: {
      r = 255; g = 255; b = 255;
      break;
    }
    case 1: {
      r = 255; g = 255; b = 0;
      break;
    }
    case 2: {
      r = 255; g = 0; b = 0;
      break;
    }
    case 3: {
      r = 0; g = 255; b = 0;
      break;
    }
    }
    no_of_edges = segments->side_edges_count[side]*2;
    for (edge_index = 0; edge_index < no_of_edges; edge_index+=2) {
      x = segments->side_edges[side][edge_index];
      y = segments->side_edges[side][edge_index+1];
      n = (y*width*result_bytesperpixel)+(x*result_bytesperpixel);
      result[n] = r;
      result[n+1] = g;
      result[n+2] = b;
    }
  }
}

/**
 * \brief shows a detected perimeter within an image
 * \param segments object containing line segments
 * \param result image to be updated
 * \param width width of the image
 * \param height height of the image
 * \param result_bitsperpixel Number of bits per pixel
 */
void show_perimeter(struct line_segments * segments,
                    unsigned char result[], int width, int height,
                    int result_bitsperpixel)
{
  int side, x, y, n;
  unsigned char r=0, g=0, b=0, vertical;
  int result_bytesperpixel = result_bitsperpixel/8;
  int * perimeter;

  memset(result, 0, width*height*result_bytesperpixel);

  for (side = 0; side < 4; side++) {
    switch(side) {
    case 0: {
      r = 255; g = 255; b = 255;
      perimeter = segments->perimeter_left;
      vertical = 0;
      break;
    }
    case 1: {
      r = 255; g = 255; b = 0;
      perimeter = segments->perimeter_right;
      vertical = 0;
      break;
    }
    case 2: {
      r = 255; g = 0; b = 0;
      perimeter = segments->perimeter_top;
      vertical = 1;
      break;
    }
    case 3: {
      r = 0; g = 255; b = 0;
      perimeter = segments->perimeter_bottom;
      vertical = 1;
      break;
    }
    }

    if (vertical == 0) {
      for (y = 0; y < height; y++) {
        x = perimeter[y];
        if ((x > 0) && (x < width)) {
          n = (y*width*result_bytesperpixel)+(x*result_bytesperpixel);
          result[n] = r;
          result[n+1] = g;
          result[n+2] = b;
        }
      }
    }
    else {
      for (x = 0; x < width; x++) {
        y = perimeter[x];
        if ((y > 0) && (y < height)) {
          n = (y*width*result_bytesperpixel)+(x*result_bytesperpixel);
          result[n] = r;
          result[n+1] = g;
          result[n+2] = b;
        }
      }
    }
  }
}

/**
 * \brief fits a perimeter to all four sides
 * \param segments object containing line segments
 * \param width width of the image
 * \param height height of the image
 * \param max_deviation
 * \param centre_x
 * \param centre_y
 * \param perimeter_x0 returned first perimeter x coord
 * \param perimeter_y0 returned first perimeter y coord
 * \param perimeter_x1 returned second perimeter x coord
 * \param perimeter_y1 returned second perimeter y coord
 * \param perimeter_x2 returned third perimeter x coord
 * \param perimeter_y2 returned third perimeter y coord
 * \param perimeter_x3 returned fourth perimeter x coord
 * \param perimeter_y3 returned fourth perimeter y coord
 * \return 0 on success, -1 otherwise
 */
static unsigned char fit_perimeter_to_all_sides(struct line_segments * segments,
                                                int width, int height,
                                                float max_deviation,
                                                int centre_x, int centre_y,
                                                float * perimeter_x0, float * perimeter_y0,
                                                float * perimeter_x1, float * perimeter_y1,
                                                float * perimeter_x2, float * perimeter_y2,
                                                float * perimeter_x3, float * perimeter_y3)
{
  int side, side2, no_of_samples, no_of_edge_samples, fit_edges;
  float x0=0, y0=0, x1=0, y1=0;
  float xi=0, yi=0;
  float line_x0[4], line_y0[4], line_x1[4], line_y1[4];


  /* fit lines to all sides */
  for (side = 0; side < 4; side++) {
    no_of_samples = segments->side_edges_count[side];
    no_of_edge_samples = segments->side_edges_count[side];
    fit_edges = ransac_fit(segments->side_edges[side],
                           segments->side_edges_count[side],
                           segments->linefit,
                           MAX_ORIENTATION_EDGES,
                           max_deviation,
                           no_of_samples,
                           no_of_edge_samples,
                           &x0, &y0, &x1, &y1);
    if (fit_edges == NO_LINE_FIT) {
      printf("All sides unable to fit line %d %d\n", side, fit_edges);
      return -1;
    }
    line_x0[side] = x0;
    line_y0[side] = y0;
    line_x1[side] = x1;
    line_y1[side] = y1;
  }

  *perimeter_x0 = -1;
  *perimeter_x1 = -1;
  *perimeter_x2 = -1;
  *perimeter_x3 = -1;

  /* calculate line intersections */
  for (side = 0; side < 3; side++) {
    for (side2 = side+1; side2 < 4; side2++) {
      if (!intersection(line_x0[side], line_y0[side],
                        line_x1[side], line_y1[side],
                        line_x0[side2], line_y0[side2],
                        line_x1[side2], line_y1[side2],
                        &xi, &yi)) continue;
      if ((xi < 0) || (xi >= width) || (yi < 0) || (yi >= height)) continue;
      if (xi < centre_x) {
        if (yi < centre_y) {
          *perimeter_x0 = xi;
          *perimeter_y0 = yi;
        }
        else {
          *perimeter_x3 = xi;
          *perimeter_y3 = yi;
        }
      }
      else {
        if (yi < centre_y) {
          *perimeter_x1 = xi;
          *perimeter_y1 = yi;
        }
        else {
          *perimeter_x2 = xi;
          *perimeter_y2 = yi;
        }
      }
    }
  }

  if ((*perimeter_x0 < 0) || (*perimeter_x1 < 0) ||
      (*perimeter_x2 < 0) || (*perimeter_x3 < 0)) {
    return -1;
  }

  return 0;
}

/**
 * \brief finds a RANSAC best fit to the side edges to generate the perimeter
 * \param perimeter_x0 returned first perimeter x coord
 * \param perimeter_y0 returned first perimeter y coord
 * \param perimeter_x1 returned second perimeter x coord
 * \param perimeter_y1 returned second perimeter y coord
 * \param perimeter_x2 returned third perimeter x coord
 * \param perimeter_y2 returned third perimeter y coord
 * \param perimeter_x3 returned fourth perimeter x coord
 * \param perimeter_y3 returned fourth perimeter y coord
 * \return 0 on success, -1 otherwise
 */
unsigned char fit_perimeter_to_sides(struct line_segments * segments,
                                     int width, int height,
                                     float * perimeter_x0, float * perimeter_y0,
                                     float * perimeter_x1, float * perimeter_y1,
                                     float * perimeter_x2, float * perimeter_y2,
                                     float * perimeter_x3, float * perimeter_y3)
{
  int side, no_of_edges, max_edges=0, max_edges2=0, max_side1=-1, max_side2=-1;
  int first_fit_edges, second_fit_edges, edge_index, x, y, dx, dy, dist, max_dist;
  int no_of_samples, no_of_edge_samples, edge_idx;
  unsigned char enough_edges = 1;
  float max_deviation=5;
  float x0, y0, x1, y1, cx, cy, dx2, dy2;
  float x2, y2, x3, y3, x4=0, y4=0, x5=0, y5=0;
  float xi=0, yi=0, xi_outer=0, yi_outer=0;

  /* get the side with the highest number of edges */
  for (side = 0; side < 4; side++) {
    no_of_edges = segments->side_edges_count[side];
    if (no_of_edges > max_edges) {
      max_edges = no_of_edges;
      max_side1 = side;
    }
  }
  if (max_side1 == -1) return -1;

  /* get the side with the second highest number of edges */
  for (side = 0; side < 4; side++) {
    if (side == max_side1) continue;
    no_of_edges = segments->side_edges_count[side];
    if (no_of_edges > max_edges2) {
      max_edges2 = no_of_edges;
      max_side2 = side;
    }
  }
  if (max_side2 == -1) return -1;

  /* do the other two sides have enough edges to fit lines to them? */
  for (side = 0; side < 4; side++) {
    if ((side == max_side1) || (side == max_side2)) continue;
    no_of_edges = segments->side_edges_count[side];
    if (no_of_edges < max_edges/8) {
      enough_edges = 0;
      break;
    }
  }
  if (enough_edges == 1) {
    return fit_perimeter_to_all_sides(segments, width, height, max_deviation,
                                      segments->edge_centre_x, segments->edge_centre_y,
                                      perimeter_x0, perimeter_y0,
                                      perimeter_x1, perimeter_y1,
                                      perimeter_x2, perimeter_y2,
                                      perimeter_x3, perimeter_y3);
  }

  /* if lines cannot be fitted to all sides then try fitting two lines and
     then interpolating the rest. This is not as reliable, but better than
     nothing. */

  /* line fit the first side */
  no_of_samples = segments->side_edges_count[max_side1];
  no_of_edge_samples = segments->side_edges_count[max_side1];
  first_fit_edges = ransac_fit(segments->side_edges[max_side1],
                               segments->side_edges_count[max_side1],
                               segments->linefit,
                               MAX_ORIENTATION_EDGES,
                               max_deviation,
                               no_of_samples,
                               no_of_edge_samples,
                               &x0, &y0, &x1, &y1);
  if (first_fit_edges == NO_LINE_FIT) {
    printf("Unable to fit first line %d\n", first_fit_edges);
    return -1;
  }

  /* line fit the second side */
  no_of_samples = segments->side_edges_count[max_side2];
  no_of_edge_samples = segments->side_edges_count[max_side2];
  second_fit_edges = ransac_fit(segments->side_edges[max_side2],
                                segments->side_edges_count[max_side2],
                                segments->linefit2,
                                MAX_ORIENTATION_EDGES,
                                max_deviation,
                                no_of_samples,
                                no_of_edge_samples,
                                &x2, &y2, &x3, &y3);
  if (second_fit_edges == NO_LINE_FIT) {
    printf("Unable to fit second line %d\n", second_fit_edges);
    return -1;
  }

  /* do the sides intersect? */
  if (!intersection(x0, y0, x1, y1, x2, y2, x3, y3,
                    &xi, &yi)) return -1;
  if ((xi < 0) || (xi >= width) || (yi < 0) || (yi >= height)) return -1;

  *perimeter_x0 = xi;
  *perimeter_y0 = yi;

  /* find the outermost edge on the first side */
  max_dist = 0;
  for (edge_index = 0; edge_index < first_fit_edges; edge_index++) {
    edge_idx = segments->linefit[edge_index];
    x = segments->side_edges[max_side1][edge_idx];
    y = segments->side_edges[max_side1][edge_idx+1];
    dx = x - (int)xi;
    dy = y - (int)yi;
    dist = dx*dx + dy*dy;
    if (dist > max_dist) {
      max_dist = dist;
      x4 = (float)x;
      y4 = (float)y;
    }
  }
  /* orthogonal line from outermost edge */
  x5 = x4 + (y1 - y0);
  y5 = y4 + (x1 - x0);
  if (!intersection(x0, y0, x1, y1, x4, y4, x5, y5,
                    &xi_outer, &yi_outer)) return -1;
  *perimeter_x1 = xi_outer;
  *perimeter_y1 = yi_outer;

  /* find the outermost edge on the second side */
  max_dist = 0;
  for (edge_index = 0; edge_index < second_fit_edges; edge_index++) {
    edge_idx = segments->linefit2[edge_index];
    x = segments->side_edges[max_side2][edge_idx];
    y = segments->side_edges[max_side2][edge_idx+1];
    dx = x - (int)xi;
    dy = y - (int)yi;
    dist = dx*dx + dy*dy;
    if (dist > max_dist) {
      max_dist = dist;
      x4 = (float)x;
      y4 = (float)y;
    }
  }
  /* orthogonal line from outermost edge */
  x5 = x4 + (y3 - y2);
  y5 = y4 + (x3 - x2);
  if (!intersection(x2, y2, x3, y3, x4, y4, x5, y5,
                    &xi_outer, &yi_outer)) return -1;
  *perimeter_x3 = xi_outer;
  *perimeter_y3 = yi_outer;

  /* find the centre of the square */
  cx = (*perimeter_x1) + (((*perimeter_x3) - (*perimeter_x1))/2.0f);
  cy = (*perimeter_y1) + (((*perimeter_y3) - (*perimeter_y1))/2.0f);

  /* interpolate the final vertex */
  dx2 = cx - xi;
  dy2 = cy - yi;
  *perimeter_x2 = cx + dx2;
  *perimeter_y2 = cy + dy2;
  if ((*perimeter_x2 < 0) || (*perimeter_y2 < 0) ||
      (*perimeter_x2 >= width) || (*perimeter_y2 >= height)) {
    return -1;
  }
  return 0;
}

/**
 * \brief shows a perimeter within an image
 * \param segments object containing line segments
 * \param result image to be updated
 * \param width width of the image
 * \param height height of the image
 * \param result_bitsperpixel Number of bits per pixel
 * \param perimeter_x0 first perimeter x coord
 * \param perimeter_y0 first perimeter y coord
 * \param perimeter_x1 second perimeter x coord
 * \param perimeter_y1 second perimeter y coord
 * \param perimeter_x2 third perimeter x coord
 * \param perimeter_y2 third perimeter y coord
 * \param perimeter_x3 fourth perimeter x coord
 * \param perimeter_y3 fourth perimeter y coord
 */
void show_shape_perimeter(struct line_segments * segments,
                          unsigned char result[], int width, int height,
                          int result_bitsperpixel,
                          float perimeter_x0, float perimeter_y0,
                          float perimeter_x1, float perimeter_y1,
                          float perimeter_x2, float perimeter_y2,
                          float perimeter_x3, float perimeter_y3)
{
  unsigned char r=0, g=255, b=0;

  draw_line(result, width, height, result_bitsperpixel,
            (int)perimeter_x0, (int)perimeter_y0,
            (int)perimeter_x1, (int)perimeter_y1,
            1, r, g, b);
  draw_line(result, width, height, result_bitsperpixel,
            (int)perimeter_x1, (int)perimeter_y1,
            (int)perimeter_x2, (int)perimeter_y2,
            1, r, g, b);
  draw_line(result, width, height, result_bitsperpixel,
            (int)perimeter_x2, (int)perimeter_y2,
            (int)perimeter_x3, (int)perimeter_y3,
            1, r, g, b);
  draw_line(result, width, height, result_bitsperpixel,
            (int)perimeter_x3, (int)perimeter_y3,
            (int)perimeter_x0, (int)perimeter_y0,
            1, r, g, b);
}

/**
 * \brief returns the aspect ratio (x100) for the perimeter
 * \param perimeter_x0 first perimeter x coord
 * \param perimeter_y0 first perimeter y coord
 * \param perimeter_x1 second perimeter x coord
 * \param perimeter_y1 second perimeter y coord
 * \param perimeter_x2 third perimeter x coord
 * \param perimeter_y2 third perimeter y coord
 * \param perimeter_x3 fourth perimeter x coord
 * \param perimeter_y3 fourth perimeter y coord
 * \return integer aspect ratio (x100)
 */
int get_shape_aspect_ratio(float perimeter_x0, float perimeter_y0,
                           float perimeter_x1, float perimeter_y1,
                           float perimeter_x2, float perimeter_y2,
                           float perimeter_x3, float perimeter_y3)
{
  int dx, dy;
  float dist0, dist1, dist2, max_dist, min_dist;

  dx = perimeter_x1 - perimeter_x0;
  dy = perimeter_y1 - perimeter_y0;
  dist0 = (float)sqrt(dx*dx + dy*dy);
  max_dist = dist0;
  min_dist = dist0;

  dx = perimeter_x2 - perimeter_x1;
  dy = perimeter_y2 - perimeter_y1;
  dist1 = (float)sqrt(dx*dx + dy*dy);
  if (dist1 > max_dist) max_dist = dist1;
  if (dist1 < min_dist) min_dist = dist1;

  dx = perimeter_x3 - perimeter_x2;
  dy = perimeter_y3 - perimeter_y2;
  dist2 = (float)sqrt(dx*dx + dy*dy);
  if (dist2 > max_dist) max_dist = dist2;
  if (dist2 < min_dist) min_dist = dist2;

  if (max_dist == min_dist) return 100;
  if (min_dist > 0) {
    return (int)((max_dist * 100) / min_dist);
  }
  return -1;
}
