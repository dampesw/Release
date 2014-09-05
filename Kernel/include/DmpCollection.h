#ifndef DMP_COLLECTION
#define DMP_COLLECTION

#include "TClonesArray.h"
#include "TObject.h"

class DmpCollection: public TClonesArray{
public:
  //DmpCollection(char* object, long size):TClonesArray(object,size){};
  TObject* At(long i){return ConstructedAt(i);}
  int Size() {return GetLast()+1;}
  TObject* New() {return ConstructedAt(GetLast()+1);}
};



#endif

