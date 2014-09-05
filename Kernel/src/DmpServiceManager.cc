/*
 *  $Id: DmpServiceManager.cc, 2014-06-16 10:20:31 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 15/03/2014
*/

#include "DmpServiceManager.h"

//-------------------------------------------------------------------
DmpServiceManager::DmpServiceManager()
 :DmpElementManager<DmpVSvc>("Service manager")
{
  std::cout<<"===>  "<<Name()<<" created"<<std::endl;
}

//-------------------------------------------------------------------
DmpServiceManager::~DmpServiceManager(){
  std::cout<<"===>  "<<Name()<<" deleted"<<std::endl;
}

/*
bool DmpServiceManager::ProcessOneEvent(){
  for(std::list<DmpVSvc*>::iterator it = fElements.begin();it != fElements.end();++it){
    if(not (*it)->ProcessThisEvent())  return false;
  }
  return true;
}*/
