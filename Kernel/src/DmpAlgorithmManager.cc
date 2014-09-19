/*
 *  $Id: DmpAlgorithmManager.cc, 2014-09-19 15:21:36 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 25/03/2014
*/

#include "DmpAlgorithmManager.h"

//-------------------------------------------------------------------
DmpAlgorithmManager::DmpAlgorithmManager()
 :DmpElementManager<DmpVAlg>("Algorithm manager")
{
  std::cout<<"===>  "<<Name()<<" created"<<std::endl;
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

