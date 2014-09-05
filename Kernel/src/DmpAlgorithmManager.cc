/*
 *  $Id: DmpAlgorithmManager.cc, 2014-06-16 10:49:31 DAMPE $
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
  std::cout<<"===>  "<<Name()<<" deleted"<<std::endl;
}

//-------------------------------------------------------------------
bool DmpAlgorithmManager::ProcessOneEvent(){
  for(std::list<DmpVAlg*>::iterator it = fElements.begin();it != fElements.end();++it){
    if(not (*it)->ProcessThisEvent())  return false;
  }
  return true;
}

//-------------------------------------------------------------------
  /*
// *  TODO: 
   *    we'd better delete GetEventLoopTerminateSignal(), if a algorithm wants to terminate a run,
   *    1.  include DmpCore.h
   *    2.  gCore->TerminateRun(), in DmpVAlg::ProcessThisEvent()
   */
bool DmpAlgorithmManager::GetEventLoopTerminateSignal(){
  //-------------------------------------------------------------------
  for(std::list<DmpVAlg*>::iterator it = fElements.begin();it != fElements.end();++it){
    if ((*it)->GetEventLoopTerminateSignal()) return true;
  }
  return false;
}

