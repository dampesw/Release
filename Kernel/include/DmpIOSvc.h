/*
 *  $Id: DmpIOSvc.h, 2014-09-11 10:49:54 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 20/05/2014
 *    Andrii Tykhonov (Andrii.Tykhonov@cern.ch) 28/05/2014
*/

#ifndef DmpIOSvc_H
#define DmpIOSvc_H

//#include <vector>
//#include <fstream>

#include "boost/algorithm/string.hpp"

#include "TTree.h"

#include "DmpVSvc.h"

//-------------------------------------------------------------------
// ??
#define COLLECTION_TREE_NAME  "CollectionTree/"
#define RUNMETADATA_TREE_NAME "RunMetadata/"
//-------------------------------------------------------------------

class TObject;
class TFile;
//class TTree;

class DmpIOSvc : public DmpVSvc{
/* 
 *  DmpIOSvc
 *
 *      it's the first element in DmpServiceManager
 *      it's a singleton for eanch job
 *      only one input and one output root file
 */
public:
  static DmpIOSvc* GetInstance(){
    static DmpIOSvc instance;
    return &instance;
  }
  ~DmpIOSvc();
  void Set(const std::string&,const std::string&);
  bool Initialize();
  bool Finalize();
  bool GetEventLoopTerminateSignal() {return fInEOF;} 

public:
  template <typename T> bool GetContainer(const std::string &thePath,T *&dataPtr){
    //*
    //*  Returns container from input or output steram
    //*  If container doesn't ecist in cache, it tries to get it from input stream
    //*

    //* get full path to the container (inlcuding tree name)
    const std::string path = std::string(COLLECTION_TREE_NAME)+std::string(thePath);

    //* check if the container is already opened in the input stream
    for(int i=0; i<fInputContainerNames.size(); i++){
      if(fInputContainerNames[i]!=path) continue;
      dataPtr = (T*)(fInputContainers[i]);
      return true;
    }

    //* check if the container is already opened in the output stream
    for(int i=0; i<fOutputContainerNames.size(); i++){
      if(fOutputContainerNames[i]!=path) continue;
      dataPtr = (T*)(fOutputContainers[i]);
      return true;
    }

    //* if container is not yet opened -> open it from the input stream
    return ReadBranch(path,dataPtr);
  }
  template <typename T> bool ReadBranch(const std::string &path,T *&dataPtr){     // ReadBranch only used to read a input data.
  /*
   * setBranchAddress by the pointer of event class or TClonesArray.
   *
   *    path = TreeName/BranchName
   *
   */
    DmpLogDebug<<"[DmpIOSvc:ReadBranch]  path ="<<path<<DmpLogEndl;
    std::vector<std::string> pathLevel;
    boost::split(pathLevel,path,boost::is_any_of("/"));
    if(2 != pathLevel.size()){
      DmpLogError<<"path ("<<path<<") should be TreeName/BranchName"<<DmpLogEndl;
      return false;
    }
    TTree *tree = GetTree(pathLevel[0]);
    if(tree){
      tree->SetBranchAddress(pathLevel[1].c_str(),&dataPtr);
    }else{
      return false;
    }
    fInputContainerNames.push_back(path);
    fInputContainers.push_back((void*) dataPtr);
    return true;
  }
  template <typename T> bool AddOutputContainer(const std::string &thePath,T *&dataPtr){
    const std::string path = std::string(COLLECTION_TREE_NAME)+std::string(thePath);
    return AddBranch(path,dataPtr);
  }
  template <typename T> bool AddBranch(const std::string &path,T *&dataPtr){      // AddBranch only used to write data.
  /*
   * book a new branch into output root file by event class's name.
   *
   *    path = TreeName/BranchName/NameOfEventClass
   *
   * book a new branch into output root file by TClonesArray.
   *
   *    path = TreeName/BranchName
   *
   */
    //fCollections->push_back(collection);
    DmpLogInfo<<"add new branch: "<<path<<DmpLogEndl;
    std::vector<std::string> pathLevel;
    boost::split(pathLevel,path,boost::is_any_of("/"));
    if(2 == pathLevel.size()){    // book branch for TClonesArray
      BookTree(pathLevel[0])->Branch(pathLevel[1].c_str(),dataPtr,32000,2);
    }else if( 3 == pathLevel.size()){ // book branch for event class
      BookTree(pathLevel[0])->Branch(pathLevel[1].c_str(),pathLevel[2].c_str(),&dataPtr,32000,2);
    }else{
      DmpLogError<<"path ("<<path<<") should be TreeName/BranchName or TreeName/Branch/EventClassName"<<DmpLogEndl;
      return false;
    }
    fOutputContainerNames.push_back(path);
    fOutputContainers.push_back((void*) dataPtr);
    return true;
  }


  template <typename T> bool AddOutputPerJobContainer(const std::string &thePath,T *&dataPtr){
    const std::string path = std::string(RUNMETADATA_TREE_NAME)+std::string(thePath);
    DmpLogInfo<<"Add output  Per-Run container: "<<path<<DmpLogEndl;
    std::vector<std::string> pathLevel;
    boost::split(pathLevel,path,boost::is_any_of("/"));
    if(2 == pathLevel.size()){    // book branch for TClonesArray
      BookPerJobTree(pathLevel[0])->Branch(pathLevel[1].c_str(),dataPtr,32000,2);
    }else if( 3 == pathLevel.size()){ // book branch for event class
      BookPerJobTree(pathLevel[0])->Branch(pathLevel[1].c_str(),pathLevel[2].c_str(),&dataPtr,32000,2);
    }else{
      DmpLogError<<"path ("<<path<<") should be BranchName or Branch/EventClassName"<<DmpLogEndl;
      return false;
    }
    return true;
  }
  bool FillEvent();
  bool FillJob();
  bool ReadEvent();
  void InFileTag(const std::string&);

private:
  DmpIOSvc();
  std::string Timestamp();
  TTree* GetTree(const std::string &treeName);
  TTree* BookTree(const std::string &treeName);
  TTree* BookPerJobTree(const std::string &treeName);

private:
  std::string               fInDataName;
  TFile                    *fInRootFile;
  std::vector<TTree*>       fInTreeSet;
  long                      fInEntry;
  bool                      fInEOF;

private:
  std::string           fOutFilePath;       // file path. default is "./". if is input file, set "INPUT"
  std::string           fOutFileName;       // file name. if is input file, set "INPUT"
  std::string           fUserOutFileName;   // user can define it's own name for the output file
  TFile                 *fOutRootFile;
  std::vector<TTree*>   fOutTreeSet;
  std::vector<TTree*>   fOutPerJobTreeSet;
  bool                  fNoOutput;

  bool          fUseTimestamp;
  std::string   fTag;       // job tag, include notes
  std::string   fInFileTag;

private:
  std::vector<std::string>  fOutputContainerNames;
  std::vector<void*>        fOutputContainers;
  std::vector<std::string>  fInputContainerNames;
  std::vector<void*>        fInputContainers;
};

//-------------------------------------------------------------------
extern DmpIOSvc *gIOSvc;

#endif

