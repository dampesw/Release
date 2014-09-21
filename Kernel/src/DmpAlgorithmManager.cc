/*
 *  $Id: DmpAlgorithmManager.cc, 2014-09-21 13:22:11 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 25/03/2014
*/

#include "DmpAlgorithmManager.h"

//-------------------------------------------------------------------
DmpAlgorithmManager::DmpAlgorithmManager()
 :DmpElementManager<DmpVAlg>("Algorithm manager")
{
}

//-------------------------------------------------------------------
DmpAlgorithmManager::~DmpAlgorithmManager(){
}

//-------------------------------------------------------------------
bool DmpAlgorithmManager::ProcessOneEvent(){
  for(std::list<DmpVAlg*>::iterator it = fElements.begin();it != fElements.end();++it){
    if(not (*it)->ProcessThisEvent()){
      return false;
    }
  }
  return true;
}

