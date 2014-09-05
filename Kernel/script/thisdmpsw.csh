#! /bin/tcsh
#+
#+  $Id: thisdmpsw.csh, 2014-05-09 15:19:20 DAMPE $
#+  Author(s):
#+    Chi WANG (chiwang@mail.ustc.edu.cn) 01/10/2013
#+-----------------------------------------------------------------------
#+   source me(anywhere is Okay) to setup envirenment of DAMPE software
#+-----------------------------------------------------------------------
#+
#+------------------------------------------------------------------
#+   Fabio Gargano (fabio.gargano@ba.infn.it) 06/06/2014
#+     I have removed the if statement at the beginig to reset the 
#+     DMPSWSYS variable every time
#+     I have corrected a minor bug
#+
#+-------------------------------------------------------------------

#+  Set installation environment of DAMPE offline software 
#if ( ! ($?DMPSWSYS) ) then
  #set thisDmpSW="`dirname $0`/.."
  #setenv DMPSWSYS "`cd ${thisDmpSW};pwd`"
  setenv DMPSWSYS "`pwd`"
  #unset thisDmpSW
  setenv DMPSWWORK "$HOME/dmpwork"
  setenv PATH ${DMPSWSYS}/bin:${PATH}
  setenv PYTHONPATH ${DMPSWSYS}/lib:${DMPSWWORK}/lib:${PYTHONPATH}
  setenv DMPSWGEOMETRY ${DMPSWSYS}/share
  setenv DMPSWCONFIGPATH ${DMPSWSYS}/share/Configuration
  if ( ($?LD_LIBRARY_PATH) ) then
    setenv LD_LIBRARY_PATH ${DMPSWSYS}/lib:${DMPSWWORK}/lib:${LD_LIBRARY_PATH}
  endif
  if ( ($?DYLD_LIBRARY_PATH) ) then        # Mac OS
    setenv DYLD_LIBRARY_PATH ${DMPSWSYS}/lib:${DMPSWWORK}/lib:${DYLD_LIBRARY_PATH}
  endif
#endif

