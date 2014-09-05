/*
 *  $Id: DmpLog.h, 2014-06-11 10:42:16 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 19/05/2014
 *    Andrii Tykhonov (Andrii.Tykhonov@cern.ch) 22/05/2014
*/

#ifndef DMP_LOG_H
#define DMP_LOG_H

#include <iostream>
#include <iomanip>

namespace DmpLog{
  enum{
    NONE    = 0, //
    ERROR   = 1, // ERROR
    WARNING = 2, // ERROR, WARNING
    INFO    = 3, // ERROR, WARNING, INFO
    DEBUG   = 4  // ERROR, WARNING, INFO, DEBUG
  };

//-------------------------------------------------------------------
  void SetLogLevel(const std::string&);
  void ShowLogHeader(bool show=false);

//-------------------------------------------------------------------
  extern short  logLevel;
  extern bool   logShowFunctionHeader;
}

#define DmpLogDebug   if(DmpLog::logLevel >= DmpLog::DEBUG) \
  std::cout<<"  DEBUG:   ["<<  (DmpLog::logShowFunctionHeader? __PRETTY_FUNCTION__:"") <<"] "
// *
// *  TODO: only need to show function header for DEBUG
  // use:       std::cout<<"  INFO:    "
// *
#define DmpLogInfo    if(DmpLog::logLevel >= DmpLog::INFO)\
  std::cout<<"  INFO:    ["<<  (DmpLog::logShowFunctionHeader? __PRETTY_FUNCTION__:"") <<"] "

#define DmpLogWarning if(DmpLog::logLevel >= DmpLog::WARNING)\
  std::cout<<"  WARNING: ["<<  (DmpLog::logShowFunctionHeader? __PRETTY_FUNCTION__:"") <<"] "

#define DmpLogError if(DmpLog::logLevel >= DmpLog::ERROR)\
  std::cout<<"  ERROR:   ["<<  (DmpLog::logShowFunctionHeader? __PRETTY_FUNCTION__:"") <<"] "

#define DmpLogEndl std::endl

#endif // DMP_LOG_H


