
#include <stdlib.h>

#include <MkxDev.h> 

#include <deque>
#include <utility>

/***************************************************************************/
// expand : expand the labelling procedure                                 */
// The iterative procedure begins at pixel (i0, j0). Lab[i0][j0]  equals   */
// "etiquette". The pixels which are sufficiently close to a pixel which   */
// value is "etiquette" are set to the value "etiquette".                  */
/***************************************************************************/

void  MkxLabellingExpand(int *Lab, int IwProc, CMkxRoiCoord roiCoord, int i0, int j0, int range, int etiquette)
{
  int i, j;
  int xmin, ymin, xmax, ymax;
  ymin = roiCoord.YMin + range;
  ymax = roiCoord.YMax - range;
  xmin = roiCoord.XMin + range;
  xmax = roiCoord.XMax - range;

  for (i = i0 - range; i <= i0 + range; i++)
  for (j = j0 - range; j <= j0 + range; j++)
    //if(i >= ymin && i < ymax && j >= xmin && j < xmax)
  if (i >= ymin && i <= ymax && j >= xmin && j <= xmax) //EA: Bugfix
  {
    if (Lab[i*IwProc+j] == 1)
    {
      Lab[i*IwProc+j] = etiquette;
      MkxLabellingExpand(Lab, IwProc, roiCoord, i, j, range, etiquette);
    }
  }
}

void  MkxLabelling(
  const UC  *Thr,
  int *Lab,
  int *NbLblBlobs,
  int Iw, int Ih,
  CMkxRoiCoord roiCoord
  )
{
  using namespace std;

  int i, j, etiquette = 2;
  int range = 1; // eight-connectivity
  int xmin, ymin, xmax, ymax;
  ymin = roiCoord.YMin + range;
  ymax = roiCoord.YMax - range;
  xmin = roiCoord.XMin + range;
  xmax = roiCoord.XMax - range;

  // EAttia version, takes some freedom like Label is not initialized where its not used
  // Also we dont clear Lab == 1, it is very consuming and rest of code has been change accordingly
  // (Some Lab != 0 replaced with Lab >= 2)
  // Plus some <= xmax/ymax bug as been fixed to < xmax/ymax

  deque<pair<int, int> > seedsCandidates;

  for (i = roiCoord.YMin; i < roiCoord.YMax; i++)
  {
    for (j = roiCoord.XMin; j < roiCoord.XMax; j++)
    {
#ifndef EXPECT
#define EXPECT(x,y) (x)
#endif
      if EXPECT(Thr[(i*Iw)+j], false)
      {
        Lab[(i*Iw)+j] = 1;
        if (j >= xmin && j < xmax && i >= ymin && i < ymax)
        {
          seedsCandidates.push_back(make_pair(j, i));
        }
      }
      else
      {
        Lab[(i*Iw)+j] = 0;
      }
    }
  }

  for (size_t t = 0; t < seedsCandidates.size(); t++)
  {
    int seedX = seedsCandidates[t].first;
    int seedY = seedsCandidates[t].second;

    if (Lab[(seedY*Iw)+seedX] == 1)
    {
      Lab[(seedY*Iw)+seedX] = etiquette;
      MkxLabellingExpand(Lab, Iw, roiCoord, seedY, seedX, range, etiquette);
      etiquette++;
    }
  }

  *NbLblBlobs = etiquette - 2;
}

