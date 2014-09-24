/*
 *  $Id: DmpIOSvc.cc, 2014-05-23 10:31:45 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 20/05/2014
 *    Andrii Tykhonov (Andrii.Tykhonov@cern.ch) 28/05/2014
*/

//#include "TTree.h"
#include "TFile.h"

#include "DmpIOSvc.h"
#include "DmpCore.h"
#include "TClonesArray.h"
#include "DmpCore.h"
#include "DmpRootIOSvc.h"

//-------------------------------------------------------------------
DmpIOSvc::DmpIOSvc()
 :DmpVSvc("DmpIOSvc"),
  fInDataName("NO"),
  fInRootFile(0),
  fOutFilePath("./"),
  fOutFileName("NO"),
  fOutRootFile(0),
  fUseTimestamp(true),
  fTag("Dmp"),
  fInFileTag(""),
  fUserOutFileName(""),
  fInEOF(false),
  fNoOutput(false)
{
  OptMap.insert(std::make_pair("InData/Read",   0));
  OptMap.insert(std::make_pair("InData/Update", 1));
  OptMap.insert(std::make_pair("OutData/Path",  2));
  OptMap.insert(std::make_pair("OutData/Tag",   3));
  OptMap.insert(std::make_pair("OutData/Timestamp", 4));
  OptMap.insert(std::make_pair("OutData/FileName", 5));
  OptMap.insert(std::make_pair("OutData/NoOutput", 6));
  /*
  fCollections    = new std::vector<TClonesArray*>();
  fContainers     = new std::vector<TObject*>();
  fContainerNames = new std::vector<std::string*>();
  */
}

//-------------------------------------------------------------------
DmpIOSvc::~DmpIOSvc(){
}

//-------------------------------------------------------------------
void DmpIOSvc::Set(const std::string &option,const std::string &argv){
  if(OptMap.find(option) == OptMap.end()){
    DmpLogError<<"No argument type: "<<option<<DmpLogEndl;
    throw;
  }
  switch (OptMap[option]){
    case 0:
    {// InData/Read
      fInDataName=argv;
      InFileTag(argv);
      break;
    }
    case 1:
    {// InData/Update
      if("./" != fOutFilePath){
        fOutFilePath = "WRONG_0";
        return;
      }
      fInDataName = argv;
      fOutFileName = "INPUT";
      break;
    }
    case 2:
    {// OutData/Path
      if("INPUT" == fOutFileName){
        fOutFilePath = "WRONG_0";
        return;
      }
      if(argv[argv.length()-1] == '/'){
        fOutFilePath = argv;
      }else{
        fOutFilePath = argv + "/";
      }
      break;
    }
    case 3:
    {// OutData/Tag 
      fTag += argv;
      break;
    }
    case 4:
    {// OutData/Timestamp
      if("off" == argv || "Off" == argv){
        fUseTimestamp = false;
      }else if("on" == argv || "On" == argv){
        fUseTimestamp = true;
      }else{
        DmpLogWarning<<"Wrong value("<<argv<<") for Timestamp received. Available options are: {On | Off}"<<DmpLogEndl;
        fUseTimestamp = false;
      }
      break;
    }
    case 5: // file name
    {
      if (TString(argv).Contains(TString(".root"))){
        fUserOutFileName = TString(argv);
      }
      else{
        fUserOutFileName = TString(argv)+TString(".root");
      }
      break;
    }
    case 6:
      if("True" == argv || "true" == argv || "TRUE" == argv){
        fNoOutput = true;
      }
      else if("False" == argv || "false" == argv || "FALSE" == argv){
        fNoOutput = false;
      }
      else{
        DmpLogError<<"[DmpIOSvc::Set] Unknown value for the argument: "<< option <<"  value: "<< argv<< "  ==> throwing exception!"<<DmpLogEndl;
        throw;
      }
      break;
  }
}

//-------------------------------------------------------------------
bool DmpIOSvc::Initialize(){
  DmpLogDebug<<"[DmpIOSvc::Initialize] initializing ... "<<DmpLogEndl;

  if("WRONG_0" == fOutFilePath){
    DmpLogError<<"Can not set \'output file as a input file\' and \'output path\' at the same time"<<DmpLogEndl;
    return false;
  }

  if("NO" != fInDataName){
    fInRootFile = TFile::Open(fInDataName.c_str());
    if (!fInRootFile) {
      DmpLogError<<"Failed to open input ROOT file: "<<fInDataName<<DmpLogEndl;
      throw;
    }
    for(int i=0; i< fInRootFile->GetListOfKeys()->GetSize();i++){      
      if(std::string(fInRootFile->GetListOfKeys()->At(i)->GetName())+"/" != std::string(COLLECTION_TREE_NAME)) continue;  //SO FAR ONLY CollectionTree is read
      fInTreeSet.push_back(dynamic_cast<TTree*>(fInRootFile->Get(fInRootFile->GetListOfKeys()->At(i)->GetName())));
    }
    fInEntry = 0;
    fInEOF = false;
  }


  if(fNoOutput){
    DmpLogInfo<<"fNoOutput = true      ==> Output is switched off"<<DmpLogEndl;
  }
  else if("INPUT" == fOutFileName){
    DmpLogInfo<<"Result in the input file: "<<fOutFilePath<<DmpLogEndl;
    fOutRootFile = fInRootFile;
    fOutRootFile->cd();

  }
  /*
  else{
    fOutFileName = fTag + fInFileTag + Timestamp() +".root";
    if(fUserOutFileName!=std::string("")){
      fOutFileName = fUserOutFileName;
    }
    DmpLogInfo<<"Result in "<<fOutFilePath+fOutFileName<<DmpLogEndl;
    fOutRootFile = new TFile((TString)(fOutFilePath+fOutFileName),"RECREATE");
    fOutRootFile->cd();
  }
  */

  DmpLogDebug<<"[DmpIOSvc::Initialize] ... initialization done "<<DmpLogEndl;  
  return true;
}



//-------------------------------------------------------------------
//  Save output
#include <sys/stat.h>       // mkdir()
bool DmpIOSvc::Finalize(){
  if(not gCore->InitializeDone()){
    return true;
  }
  mkdir(fOutFilePath.c_str(),0755);
  if(!fOutTreeSet.size() && !fOutPerJobTreeSet.size()){
    DmpLogInfo<<"[DmpIOSvc::Finalize] No data to be stored..."<<DmpLogEndl;
    DmpLogInfo<<"[DmpIOSvc::Finalize] ... exiting without creating output ROOT file"<<DmpLogEndl;
    return true;
  }
  DmpLogInfo<<"[DmpIOSvc::Finalize] Fill Per Job information"<<DmpLogEndl;
  FillJob();
  /*
  // *
  // * Moved to the Inititalize method
  // * The reason is that TFile should be created before the TTree,
  // *   otherwise TTree may run out of memory (see: https://root.cern.ch/phpBB3/viewtopic.php?f=3&t=16908)
  // *
  if("INPUT" == fOutFileName){
    DmpLogInfo<<"Result in the input file: "<<fOutFilePath<<DmpLogEndl;
    fOutRootFile = fInRootFile;
    fOutRootFile->cd();
  }else{
    fOutFileName = fTag + fInFileTag + Timestamp() +".root";
    if(fUserOutFileName!=std::string("")){
      fOutFileName = fUserOutFileName;
    }
    DmpLogInfo<<"Result in "<<fOutFilePath+fOutFileName<<DmpLogEndl;
    fOutRootFile = new TFile((TString)(fOutFilePath+fOutFileName),"recreate");
  }
  fOutRootFile->cd();
  */

  gRootIOSvc->ActiveOutputRootFile();
  for(short i=0;i<fOutTreeSet.size();++i){
    DmpLogInfo<<"\tTree: "<<fOutTreeSet[i]->GetName()<<", entries = "<<fOutTreeSet[i]->GetEntries()<<DmpLogEndl;
    fOutTreeSet[i]->Write();
    delete fOutTreeSet[i];
  }
  for(short i=0;i<fOutPerJobTreeSet.size();++i){
    DmpLogInfo<<"\tTree: "<<fOutPerJobTreeSet[i]->GetName()<<", entries = "<<fOutPerJobTreeSet[i]->GetEntries()<<DmpLogEndl;
    fOutPerJobTreeSet[i]->Write();
    delete fOutPerJobTreeSet[i];
  }
  if(fInRootFile){
    fInRootFile->Close();
    delete fInRootFile;
  }
  /*
  if("INPUT" != fOutFileName && "NO" != fOutFileName){
    fOutRootFile->Close();
    delete fOutRootFile;
  }
  */
  DmpLogDebug<<DmpLogEndl;
  return true;
}

//-------------------------------------------------------------------
bool DmpIOSvc::ReadEvent(){
  //* Check if there are any input trees available
  if(!fInTreeSet.size()){
    DmpLogDebug<<"[DmpIOSvc::ReadEvent] No input ROOT trees defined. Continue without doing anything"<<DmpLogEndl;
    return true;
  }

  //* Read the trees
  DmpLogDebug<<"[DmpIOSvc::ReadEvent] Read event ... "<< fInEntry <<DmpLogEndl;
  for(short i=0;i<fInTreeSet.size();++i){
    if(std::string(fInTreeSet[i]->GetName()) + "/" != std::string(COLLECTION_TREE_NAME)) continue;  //SO FAR ONLY CollectionTree is read
    
    /*
    std::cout<<"\n\n\n\nfInTreeSet[i]->GetName() :"<<fInTreeSet[i]->GetName()<<std::endl;
    std::cout<<"fInEntry :"<<fInEntry<<std::endl;
    std::cout<<"fInTreeSet[i]]->GetEntries() :"<<fInTreeSet[i]->GetEntries()<<std::endl;
    std::cout<<"1"<<std::endl;
    */     
    

    fInTreeSet[i]->GetEntry(fInEntry);

  }
  fInEntry++;

  //* Check if reached end of file
  for(short i=0;i<fInTreeSet.size();++i){    
    if(std::string(fInTreeSet[i]->GetName()) + "/" != std::string(COLLECTION_TREE_NAME)) continue;   //SO FAR ONLY CollectionTree is read
    if(fInEntry<fInTreeSet[i]->GetEntries()) continue;
    //fInEOF = true;
    gCore->TerminateRun();
    break;
  }

  

  //* All ok ==> return true
  DmpLogDebug<<"[DmpIOSvc::ReadEvent] ... finished reading event"<<DmpLogEndl;
  return true;
}


//-------------------------------------------------------------------
bool DmpIOSvc::FillEvent(){
  for(short i=0;i<fOutTreeSet.size();++i){
    fOutTreeSet[i]->Fill();
  }
  //DmpLogDebug<<"Fill event "<<fOutTreeSet[0]->GetEntries()<<DmpLogEndl;
  DmpLogDebug<<"Fill event "<<DmpLogEndl;
  return true;
}

//-------------------------------------------------------------------
bool DmpIOSvc::FillJob(){
  for(short i=0;i<fOutPerJobTreeSet.size();++i){
    fOutPerJobTreeSet[i]->Fill();
  }
  DmpLogDebug<<"Fill job "<<DmpLogEndl;
  return true;
}

//-------------------------------------------------------------------
#include <boost/filesystem/path.hpp>
void DmpIOSvc::InFileTag(const std::string &filename){
  boost::filesystem::path inpath(filename);
  fInFileTag += "_"+inpath.stem().string();
}

//-------------------------------------------------------------------
#include <time.h>
std::string DmpIOSvc::Timestamp(){
  if(not fUseTimestamp) return "";
  time_t now;
  struct tm *p;
  time(&now);
  p = localtime(&now);
  char timeStamp[20];
  strftime(timeStamp,99,"_%Y%m%d%H%M%S",p);
  return timeStamp;
}

//-------------------------------------------------------------------
TTree* DmpIOSvc::GetTree(const std::string &treeName){
  for(short i=0;i<fInTreeSet.size();++i){
    if(treeName != fInTreeSet[i]->GetName()) continue;
    return fInTreeSet[i];
  }
  DmpLogError<<"[DmpIOSvc::GetTree] Input tree not found: "<< treeName<< " ==> throwing exception!"<<DmpLogEndl;
  throw;
}

//-------------------------------------------------------------------
TTree* DmpIOSvc::BookTree(const std::string &treeName){
  short index = -1;
  for(short i=0;i<fOutTreeSet.size();++i){
    if(treeName == fOutTreeSet[i]->GetName()){
      index = i;
      break;
    }
  }
  if(-1 == index){
    index = fOutTreeSet.size();
    fOutTreeSet.push_back(new TTree(treeName.c_str(),treeName.c_str()));
  }
  return fOutTreeSet[index];
}


//-------------------------------------------------------------------
TTree* DmpIOSvc::BookPerJobTree(const std::string &treeName){
  short index = -1;
  for(short i=0;i<fOutPerJobTreeSet.size();++i){
    if(treeName == fOutPerJobTreeSet[i]->GetName()){
      index = i;
      break;
    }
  }
  if(-1 == index){
    index = fOutPerJobTreeSet.size();
    fOutPerJobTreeSet.push_back(new TTree(treeName.c_str(),treeName.c_str()));
  }
  return fOutPerJobTreeSet[index];
}


/*
//-------------------------------------------------------------------
TClonesArray* DmpIOSvc::NewOutCollection(char* collectrionName,char* className, int approxsize = 10000)
{
  TClonesArray* collection  = new TClonesArray(className, approxsize);
  collection->SetName(collectrionName);
  AddBranch(fName +  std::string("/") + std::string(collectionname)+std::string("/") +std::string(className) ,collection);
  return collection;
}


//-------------------------------------------------------------------
TClonesArray* DmpIOSvc::GetOutCollection(std::string collectrionName,char* className, int approxsize = 10000) const{
  for(int i=0; i<fCollections->size();i++){
    if(std::string(fCollections->at(i)->GetName()) != collectrionName ) continue;
    return fCollections->at(i);
  }
  TClonesArray* collection  = new TClonesArray(className, approxsize);
  collection->SetName(collectrionName);
  fCollections->push_back(collection);
  ReadBranch(fName +  std::string("/") + std::string(collectionname)+std::string("/") +std::string(className), collection);
  return collection;
}
*/

DmpIOSvc *gIOSvc = DmpIOSvc::GetInstance();
