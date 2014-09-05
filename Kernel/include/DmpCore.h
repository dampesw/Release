/*
 *  $Id: DmpCore.h, 2014-06-11 09:34:10 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 22/04/2014
*/

#ifndef DmpCore_H
#define DmpCore_H

#include "DmpAlgorithmManager.h"
#include "DmpServiceManager.h"

class DmpCore{
/*
 *  DmpCore
 *
 *      Core of DAMPE software. It's a singleton in one job.
 *
 */
public:
  static DmpCore* GetInstance(){
    static DmpCore instance;
    return &instance;
  }
  ~DmpCore();

public:     // binding functions
  bool Initialize();            // execute all elements' Initialize() in all *Mgr
// *
// *  TODO: we need to choose one as official version.
// *
  bool Run();                   // run one job
  bool run();                   // run one job
  bool Finalize();              // execute all elements' Finalize() in all *Mgr

public:
// *
// *  TODO: we'd better delete SetXXX(),
// *    we can use Set(const std::string &type,const std::string &value) in job option file
// *        because DmpVAlg and DmpVSvc use the same set method, Set(const std::string &type, const std::string &value)
// *
  void SetLogLevel(const std::string&, const short &s=0) const;
  void SetMaxEventNumber(const long &i) {fMaxEventNo = i;}
  //void SetTimeWindow(const std::string &type,const int &YMD,const int &HMS);

public:
  const long& GetMaxEventNumber() const {return fMaxEventNo;}
  bool EventInTimeWindow(const long &second) const;
  const bool& InitializeDone() const {return fInitializeDone;}

public:
  void Set(const std::string &type,const std::string &value);
  DmpAlgorithmManager*  AlgorithmManager() const {return fAlgMgr;}
  DmpServiceManager*    ServiceManager() const {return fSvcMgr;}

public:
  void TerminateRun()   {fTerminateRun = true;} // call me in algorithms

private:
  DmpCore();
  long DeltaTime(const std::string&)const;

private:
  DmpAlgorithmManager   *fAlgMgr;       // algorithm manager, singleton
  DmpServiceManager     *fSvcMgr;       // service manager, singleton
  std::string           fLaunchTime;    // lauch time, 20130101-0000
  long                  fMaxEventNo;    // run how many event
  long                  fStartTime;     // unit: second. start time of time window
  long                  fStopTime;      // unit: second. stop time of time window

private:
  bool      fInitializeDone;        // default is false
  bool      fTerminateRun;          // concrete algorithm could set this value
  std::map<std::string,short>    OptMap; // option map

};

//-------------------------------------------------------------------
extern DmpCore  *gCore;

#endif
