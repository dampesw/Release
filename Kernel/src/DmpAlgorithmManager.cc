/*
 *  $Id: DmpAlgorithmManager.cc, 2014-09-06 11:07:27 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 25/03/2014
*/

#include "DmpAlgorithmManager.h"
#include "DmpCore.h"

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
      DmpLogError<<"Event ID = "<<gCore->GetCurrentEventID()<<",\talgorithm failed: "<<(*it)->Name()<<DmpLogEndl;
      return false;
    }
  }
  return true;
}

