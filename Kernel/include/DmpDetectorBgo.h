/*
 *  $Id: DmpDetectorBgo.h, 2014-06-11 15:23:47 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 06/08/2013
*/

// *
// * Comment by A.Tykhonov: suggest to consider this class as a deprecated one, since all
// * geometry parameters are read directly from GDML geometry using GeometryManager class
// * of the Kernel package...
// *


#ifndef DmpDetectorBgo_H
#define DmpDetectorBgo_H

namespace DmpDetector{

  namespace Bgo{
    const short kPlaneNo = 7;
    const short kBarNo   = 22;
    const short kRefBarNo= 2;
    const short kSideNo  = 2;
    const short kDyNo    = 3;
  }
}

#endif

