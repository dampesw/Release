/*
 *  $Id: DmpAlgorithmManager.h, 2014-04-30 23:19:47 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 22/03/2014
*/

#ifndef DmpAlgorithmManager_H
#define DmpAlgorithmManager_H

#include "DmpVAlg.h"
#include "DmpElementManager.h"

class DmpAlgorithmManager : public DmpElementManager<DmpVAlg>{
/*
 * DmpAlgorithmManager
 *
 * manage all algorithmes of DAMPE software
 *
 */
public:
  static DmpAlgorithmManager* GetInstance(){
    static DmpAlgorithmManager instance;
    return &instance;
  }
  ~DmpAlgorithmManager();
  bool ProcessOneEvent();
  bool GetEventLoopTerminateSignal();
  /*
// *  TODO: 
   *    we'd better delete GetEventLoopTerminateSignal(), if a algorithm wants to terminate a run,
   *    1.  include DmpCore.h
   *    2.  gCore->TerminateRun(), in DmpVAlg::ProcessThisEvent()
   */

private:
  DmpAlgorithmManager();

};

#endif

