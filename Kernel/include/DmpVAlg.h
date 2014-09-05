/*
 *  $Id: DmpVAlg.h, 2014-07-09 22:03:45 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 22/03/2014
*/

#ifndef DmpVAlg_H
#define DmpVAlg_H

#include <map>
#include <string>
#include "DmpLog.h"

class DmpVAlg{
/*
 * DmpVAlg
 *
 *  drive yourself algorithm from this class, and then
 *  register your algorithm into Dmp Alorithm Manager, in python
 *
 */
public:
  DmpVAlg(const std::string &n):fName(n),fIniStatus(true){}
  virtual ~DmpVAlg(){}
  virtual bool Initialize()=0;
  virtual bool ProcessThisEvent()=0;
  virtual bool Finalize()=0;
  virtual void Set(const std::string&,const std::string&){}    // for options, binding me
  virtual bool GetEventLoopTerminateSignal(){return false;}
  /*
// *  TODO: 
   *    we'd better delete GetEventLoopTerminateSignal(), if a algorithm wants to terminate a run,
   *    1.  include DmpCore.h
   *    2.  gCore->TerminateRun(), in DmpVAlg::ProcessThisEvent()
   */

public:
  const std::string& Name() const {return fName;}

protected:
  std::map<std::string,short>   OptMap; // a map of set options
  bool          fIniStatus;     // status of initialization

private:
  std::string   fName;

};

#endif

