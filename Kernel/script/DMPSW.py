#! /usr/bin/python
#@
#@ Loading the libraries of DAMPE SOFTWARE
#@ 
import os
import sys
sys.setdlopenflags(0x100|0x2)
import libDmpKernel as DmpKernel

#@ Instantiate DAMPE objects
#@---------------------------------------------------------------------------------------------------------------
Core = DmpKernel.DmpCore.GetInstance()   #@ Get instance of DAMPE core               (MANDATORY!)
SvcMgr  = Core.ServiceManager()          #@ Get instance of DAMPE service manager    (MANDATORY!) 
AlgMgr  = Core.AlgorithmManager()        #@ Get instance of DAMPE algorithm manager  (MANDATORY!)
DmpSysPath = os.getenv('DMPSWSYS')
SysPath = os.getenv('DMPSWSYS')
WorkPath = os.getenv('DMPSWWORK')
IOSvc  = SvcMgr.Get("DmpIOSvc")
GeoMgr = DmpKernel.DmpGeoMgr()






