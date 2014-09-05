// *
// * Prototype of Analysis class (ATLAS-style)
// * OBSOLETE!!! <- remove it later!
// * 
#ifndef DMPANALYSIS_H
#define DMPANALYSIS_H

#include "TROOT.h"

class DmpAnalysis{

public:

  DmpAnalysis(){

  }



  ~DmpAnalysis(){

  }



  int Initialize(){
    return 0;
  }



  int Process(Long64_t entry){
    return 0;;
  }



  int Finalize(){
    return 0;;
  }

};

#endif //DMPANALYSIS_H
