/*
 *  Created on: Jan 21, 2014
 *      Author: Andrii Tykhonov
 */

#include "DmpGeometryManager.h"
//#include "G4Box.hh"
#include <sstream>
#include <iostream>

#include <stdlib.h>
#include <math.h>

#include "TMath.h"

#include "DmpLog.h"

//#include "Math/GenVector/Rotation3D.h"
//#include "Math/GenVector/AxisAngle.h"

//#include "TVector3.h"


/*
// XML parser libraries
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
*/

#define DEBUG_MESSAGES true
#define NOTANUMBER     1e33

#define BGO_DETECTOR_GDML_NAME      "BGO_detector_vol"
#define BGO_CENTER_Z_GDML_NAME      "bgo_z_offset"
#define BGO_BAR_GDML_NAME           "BGO_with_rubber_vol"
#define STK_SILICON_GDML_NAME       "stk_si"
#define STK_SILICONTILEX_GDML_NAME  "ActiveTileX"
#define STK_SILICONTILEY_GDML_NAME  "ActiveTileY"
#define STK_SILICONTILE_GDML_NAME   "ActiveTile"
#define STK_CONTAINER_GDML_NAME     "STK_detector_vol"
#define STK_CONVERTERTILE_GDML_NAME "Converter-"


#define GEOMETRY_PATH_ENVVARIABLE  "DMPSWGEOMETRY"

//#define STK_ADJUST_X_POSITION_NAME "stk_adjust_x_position"
//#define STK_ADJUST_Y_POSITION_NAME "stk_adjust_y_position"
//#define STK_ADJUST_Z_POSITION_NAME "stk_adjust_z_position"
#define STK_Z_OFFSET_NAME          "stk_z_offset"

#define STK_SI_TRANSLATION_X "stk_adjust_x_position"
#define STK_SI_TRANSLATION_Y "stk_adjust_y_position"
#define STK_SI_TRANSLATION_Z "stk_adjust_z_position"
#define STK_ROTATION_NAME    "old_to_new_coordinatesystem_rotation"


#define DAMPE_PARAMETERS_XML_FILE        "/Geometry/Parameters/DAMPE.xml"
#define STK_PARAMETERS_XML_FILE          "/Geometry/Parameters/STK.xml"
#define BGO_PARAMETERS_XML_FILE          "/Geometry/Parameters/BGO.xml"
#define STKSILICON_PARAMETERS_XML_FILE   "/Geometry/STK/silicon_coordinates.xml"
#define STKCONVERTER_PARAMETERS_XML_FILE "/Geometry/STK/converter_coordinates.xml"
//#define STKSILICON_PARAMETERS_XML_FILE   "/Geometry/STK/stk_si_advanced.gdml"
//#define STKCONVERTER_PARAMETERS_XML_FILE "/Geometry/STK/stk_tungsten_advanced.gdml"


#define DEFAULT_GEOMETRY_MAIN_GDML_NAME "Geometry/DAMPE.gdml"

#define GEOMETRY_PATH_ENVVARIABLE  "DMPSWGEOMETRY"

#define micrometer   0.001
#define mm           1.0
#define cm          10.0

//std::string GdmlGeometryMainName = std::string(DEFAULT_GEOMETRY_MAIN_GDML_NAME);


double rectify(double value){
  if (fabs(1-fabs(value)) > 1e-10 &&  fabs(value) > 1e-10 ){
    std::ostringstream error;
    error<<"[rectify] ERROR: improper argument [value] = "<< value<<" should be close either to +- 1 or to 0"<<DmpLogEndl;
    DmpLogError<<error.str()<<DmpLogEndl;
    throw error.str();
  }

  if (fabs(value) < 1e-10)     return  0;
  if (fabs(1-value)< 1e-10)    return  1;
  if (fabs(1+value)< 1e-10)    return -1;
  return value;
}

DmpGeometryManager::DmpGeometryManager():
    DmpVSvc("DmpGeometryManager")
{
    //init();
    //readDampeGeometryParameters();
}

void DmpGeometryManager::Set(const std::string& arg,const std::string& val){
  /*
  if(std::string("GeometryMainGdmlName") == arg){
    GdmlGeometryMainName = val;
    return;
  }
  */
  DmpLogError<<"[DmpGeometryManager::Set] Obtained unknown parameter: "<<arg<<" ==> throwing exception!"<<DmpLogEndl;
  throw;
}

bool DmpGeometryManager::Initialize(){
  connectToGeometryXML();
  readDampeGeometryParameters();
  return true;
}

bool DmpGeometryManager::Finalize(){
  return true;
}


int DmpGeometryManager::connectToGeometryXML() {

    // * Obtain the location of DAMPE geometry
    char* geometryPath = NULL;
    geometryPath = getenv (GEOMETRY_PATH_ENVVARIABLE);
    if (geometryPath==NULL){
      std::ostringstream errorstr;
      errorstr<<"[GeometryManager::init] ERROR: problem geometry path, "<<GEOMETRY_PATH_ENVVARIABLE<<" is not defined!";
      std::string error(errorstr.str());
      DmpLogError<<error<<DmpLogEndl;
      throw error;
    }


    // * reading xml geometry parameter files
    DmpLogInfo<< "[GeometryManager::init]  Reading XML parameter files ... " <<DmpLogEndl;
    xmlEngine = new TXMLEngine;
    XMLDocPointer_t xmldocDAMPEparameter         = xmlEngine->ParseFile(TString(geometryPath).Append(DAMPE_PARAMETERS_XML_FILE));
    XMLDocPointer_t xmldocSTKparameter           = xmlEngine->ParseFile(TString(geometryPath).Append(STK_PARAMETERS_XML_FILE));
    XMLDocPointer_t xmldocBGOparameter           = xmlEngine->ParseFile(TString(geometryPath).Append(BGO_PARAMETERS_XML_FILE));
    XMLDocPointer_t xmldocSTKsiliconcoordinates  = xmlEngine->ParseFile(TString(geometryPath).Append(STKSILICON_PARAMETERS_XML_FILE));
    XMLDocPointer_t xmldocSTKtungstencoordinates = xmlEngine->ParseFile(TString(geometryPath).Append(STKCONVERTER_PARAMETERS_XML_FILE));
    nodeDAMPEparameter         = xmlEngine->DocGetRootElement(xmldocDAMPEparameter);
    nodeBGOEparameter          = xmlEngine->DocGetRootElement(xmldocBGOparameter);
    nodeSTKparameter           = xmlEngine->DocGetRootElement(xmldocSTKparameter);
    nodeSTKsiliconcoordinates  = xmlEngine->DocGetRootElement(xmldocSTKsiliconcoordinates);
    nodeSTKtungstencoordinates = xmlEngine->DocGetRootElement(xmldocSTKtungstencoordinates);
    DmpLogInfo<< "[GeometryManager::init]  ... finished reading parameter files" <<DmpLogEndl;


    //*   Return success code
    return 0;
}
int DmpGeometryManager::readDampeGeometryParameters(){

    // *
    // *   Read BGO geometry parameters  (by Yifeng)
    // *
    m_CalCenterZ = __GetXMLParameter("bgo_z_offset",nodeDAMPEparameter,xmlEngine);
    m_CalLayerNb = __GetXMLParameter("BGO_NbOfCALLayers",             nodeBGOEparameter,xmlEngine);
    m_CalLayerBarNb = __GetXMLParameter("BGO_NbOfCALLayerBars",       nodeBGOEparameter,xmlEngine);
    m_CalSizeXY  = std::max(__GetXMLParameter("BGOdetectorX",         nodeBGOEparameter,xmlEngine), __GetXMLParameter("BGOdetectorY",nodeBGOEparameter,xmlEngine));
    m_CalSizeZ   = __GetXMLParameter("BGOdetectorZ",                  nodeBGOEparameter,xmlEngine);
    m_CalLayerSeparation = __GetXMLParameter("BGO_CALLayerSeparation",nodeBGOEparameter,xmlEngine);
    m_CalBarSeparation = __GetXMLParameter("BGO_CALBarsSeparation",   nodeBGOEparameter,xmlEngine);
    m_CalBGOBarX = __GetXMLParameter("BGObarX",nodeBGOEparameter,xmlEngine);
    m_CalBGOBarY = __GetXMLParameter("BGObarY",nodeBGOEparameter,xmlEngine);
    m_CalBGOBarZ = __GetXMLParameter("BGObarZ",nodeBGOEparameter,xmlEngine);
    m_CalTopLayerZ = __GetXMLParameter("BGO_CALTopLayerZ",nodeBGOEparameter,xmlEngine);
    m_XCalBar_z  = new std::vector<double>();
    m_YCalBar_z  = new std::vector<double>();
    for (int i=0;i<m_CalLayerNb/2;i++){
      double XBarCenterZ = 0;
      XBarCenterZ = m_CalTopLayerZ+2*i*(m_CalBGOBarZ+m_CalLayerSeparation)+m_CalCenterZ;
      m_XCalBar_z->push_back(XBarCenterZ);
    }
    for (int i=0;i<m_CalLayerNb/2;i++){
      double YBarCenterZ = 0;
      YBarCenterZ = m_CalTopLayerZ+(2*i+1)*(m_CalBGOBarZ+m_CalLayerSeparation)+m_CalCenterZ;
      m_YCalBar_z->push_back(YBarCenterZ);
    }
    m_CalTopZ = m_XCalBar_z->front()-0.5*m_CalBGOBarZ;
    m_CalBottomZ = m_YCalBar_z->back()+0.5*m_CalBGOBarZ;
    m_CalTopX = m_CalBGOBarY;
    m_CalTopY = m_CalLayerBarNb*m_CalBGOBarX+(m_CalLayerBarNb-1)*m_CalBarSeparation; //602.5mm

    /*
    m_CalTopZ    = -34.4;
    m_CalBottomZ  = -444.4;
    m_CalTopX     = 303.75;
    m_CalTopY     = 303.75;
    */


    DmpLogDebug<< "[GeometryManager::init] BGO parameters:"<<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_CalSizeXY  = " << m_CalSizeXY <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_CalSizeZ   = "  << m_CalSizeZ  <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_CalCenterZ = "<< m_CalCenterZ <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_XCalBar_z  = {"<<DmpLogEndl;
    for(int i=0; i<m_XCalBar_z->size();i++) DmpLogDebug<< "  "<<m_XCalBar_z->at(i)<<DmpLogEndl;
    DmpLogDebug<<"}"<<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_YCalBar_z  = {"<<DmpLogEndl;
    for(int i=0; i<m_YCalBar_z->size();i++) DmpLogDebug<< "  "<<m_YCalBar_z->at(i)<<DmpLogEndl;
    DmpLogDebug<<"}"<<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_CalTopZ    = "  << m_CalTopZ <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_CalBottomZ = " << m_CalBottomZ  <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_CalTopX    = " << m_CalTopX <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_CalTopY    = " << m_CalTopY <<DmpLogEndl;


    // *
    // *   Read STK geometry parameters
    // *
    m_STKnplanes            = __GetXMLParameter("NbOfTKRLayers",nodeSTKparameter,xmlEngine);     // 6
    m_STKnblocksPerLayer    = __GetXMLParameter("NbOfTKRBlocks",nodeSTKparameter,xmlEngine);     // 4;
    m_STKnLaddersBlock      = __GetXMLParameter("STKnLadddersBlock",nodeSTKparameter,xmlEngine); // 4;
    m_STKnTRB               = __GetXMLParameter("STKnTRB",nodeSTKparameter,xmlEngine);           // 8;
    m_STKnLadTRB            = __GetXMLParameter("STKnLaddersTRB",nodeSTKparameter,xmlEngine);    // 24
    m_STKnROStripsPerLadder = __GetXMLParameter("NbOfTKRROStrips",nodeSTKparameter,xmlEngine);   // 384;
    m_STKnVA                = __GetXMLParameter("STKnVA",nodeSTKparameter,xmlEngine);            // 6;
    m_STKnChannelsVA        = __GetXMLParameter("STKnChannelsVA",nodeSTKparameter,xmlEngine);    // 64;


    // * The following two parameters are used to cross-check algorithms
    // *  if mismatch is found -> an Exception is thrown
    m_STKnLaddersXdirection  = __GetXMLParameter("STKnladdersXdirection",nodeSTKparameter,xmlEngine);   //8;
    m_STKnLaddersYdirection  = __GetXMLParameter("STKnladdersYdirection",nodeSTKparameter,xmlEngine);   //8;

    m_STKnStripsPerLadder = __GetXMLParameter("NbOfTKRStrips",nodeSTKparameter,xmlEngine);
    m_STKSiliconGuardRing = __GetXMLParameter("SiliconGuardRing",nodeSTKparameter,xmlEngine)*mm;
    m_STKactiveTileSize   = __GetXMLParameter("TKRSiliconTileXY",nodeSTKparameter,xmlEngine)*cm;
    m_STKsiliconThickness = __GetXMLParameter("TKRSiliconThickness",nodeSTKparameter,xmlEngine)*micrometer;
    m_STKactiveTileSize  -= m_STKSiliconGuardRing;

    m_STKsiliconPitch    = __GetXMLParameter("TKRSiliconPitch",nodeSTKparameter,xmlEngine)*micrometer;
    m_STKsiliconROPitch  = __GetXMLParameter("TKRSiliconROPitch",nodeSTKparameter,xmlEngine)*micrometer;
    //m_STKTilesSeparation = __GetXMLParameter("TilesSeparation",nodeSTKparameter,xmlEngine)*micrometer;

    m_STKnConverterLayers         = __GetXMLParameter("NConverterLayers",            nodeSTKparameter,xmlEngine);
    m_STKnconvertersXYdirection   = __GetXMLParameter("STKnconvertersXYdirection",   nodeSTKparameter,xmlEngine);






    //* TODO: REPLACE WITH THE EXACT RADIATION LENGTH MAP
    m_STKConverterX0              = __GetXMLParameter("ConverterX0",                 nodeSTKparameter,xmlEngine);
    m_STKSupportTrayAndSiliconNX0 = __GetXMLParameter("TraySupportAllThicknessX0",   nodeSTKparameter,xmlEngine);
    m_STKHCombNX0                 = __GetXMLParameter("TraySupportHCombThicknessX0", nodeSTKparameter,xmlEngine);
    m_STKHobeyCombWidth           = __GetXMLParameter("STKHobeyCombWidth",           nodeSTKparameter,xmlEngine) * mm;
    //* TODO: REPLACE WITH THE EXACT RADIATION LENGTH MAP



    m_STKnLaddersLayer      = m_STKnLaddersBlock*m_STKnblocksPerLayer;                 //   16;  // 4*4
    m_STKnROStripsBlock     = m_STKnROStripsPerLadder * m_STKnLaddersBlock;            // 1536;  //384 *4
    m_STKnladders           = m_STKnblocksPerLayer*m_STKnLaddersBlock*m_STKnplanes *2; //  192;  (*2 - bcause we have X and Y planes)
    m_STKnBlockaPerAxis     = std::sqrt(m_STKnblocksPerLayer);
    if((int)std::pow((double)m_STKnBlockaPerAxis,2) != m_STKnblocksPerLayer){
      std::string error("[GeometryManager::readDampeGeometryParameters] ERROR: consistency chack failed for paramweter m_STKnBlockaPerAxis");
      DmpLogError<< error << DmpLogEndl;
      throw error;
    }

    m_STKXstrips_z   = new std::vector<double>();
    m_STKXstrips_z_b = new std::vector<double>();
    m_STKYstrips_z   = new std::vector<double>();
    m_STKYstrips_z_b = new std::vector<double>();

    m_STKXtiles_x    = new std::vector<double>();
    m_STKXtiles_y    = new std::vector<double>();
    m_STKYtiles_x    = new std::vector<double>();
    m_STKYtiles_y    = new std::vector<double>();


    //G4LogicalVolume* stkdetectorvol = fParser->GetVolume(STK_CONTAINER_GDML_NAME);
    //G4LogicalVolume* stksilicon     = fParser->GetVolume(STK_SILICON_GDML_NAME);


    m_STKzOffset          = __GetXMLParameter(STK_Z_OFFSET_NAME, nodeDAMPEparameter,xmlEngine);
    m_STKlocalRotation    = __GetXMLRotation(STK_ROTATION_NAME,nodeSTKparameter,xmlEngine);
    m_STKlocalTranslation = new  TVector3( __GetXMLParameter(STK_SI_TRANSLATION_X, nodeSTKparameter,xmlEngine),
                                           __GetXMLParameter(STK_SI_TRANSLATION_Y, nodeSTKparameter,xmlEngine),
                                           __GetXMLParameter(STK_SI_TRANSLATION_Z, nodeSTKparameter,xmlEngine));


    /*
    std::cout<<DmpLogEndl;
    std::cout<<DmpLogEndl;
    std::cout<<DmpLogEndl;
    std::cout<<"referenceSTKvolume->GetRotation().xx(): "<<m_STKlocalRotation1.xx <<DmpLogEndl;
    std::cout<<"referenceSTKvolume->GetRotation().xy(): "<<m_STKlocalRotation1.xy <<DmpLogEndl;
    std::cout<<"referenceSTKvolume->GetRotation().xz(): "<<m_STKlocalRotation1.xz <<DmpLogEndl;
    std::cout<<"referenceSTKvolume->GetRotation().yx(): "<<m_STKlocalRotation1.yx <<DmpLogEndl;
    std::cout<<"referenceSTKvolume->GetRotation().yy(): "<<m_STKlocalRotation1.yy <<DmpLogEndl;
    std::cout<<"referenceSTKvolume->GetRotation().yz(): "<<m_STKlocalRotation1.yz <<DmpLogEndl;
    std::cout<<"referenceSTKvolume->GetRotation().zx(): "<<m_STKlocalRotation1.zx <<DmpLogEndl;
    std::cout<<"referenceSTKvolume->GetRotation().zy(): "<<m_STKlocalRotation1.zy <<DmpLogEndl;
    std::cout<<"referenceSTKvolume->GetRotation().zz(): "<<m_STKlocalRotation1.zz <<DmpLogEndl;
    std::cout<<"referenceSTKvolume->GetFrameTranslation().x(): "<<referenceSTKvolume->GetTranslation().x() <<DmpLogEndl;
    std::cout<<"referenceSTKvolume->GetFrameTranslation().y(): "<<referenceSTKvolume->GetTranslation().y() <<DmpLogEndl;
    std::cout<<"referenceSTKvolume->GetFrameTranslation().z(): "<<referenceSTKvolume->GetTranslation().z() <<DmpLogEndl;
    std::cout<<DmpLogEndl;
    std::cout<<DmpLogEndl;
    std::cout<<DmpLogEndl;
    */



    std::vector<STKTileCoordinates>* tiles = __ParseXMLtesselatedVolumes(std::string(STK_SILICONTILE_GDML_NAME), nodeSTKsiliconcoordinates, xmlEngine);
    for(int i=0; i<tiles->size();i++){


        STKTileCoordinates tilecoord = tiles->at(i);
        double Zmax = tilecoord.zmax;
        double Zmin = tilecoord.zmin;
        double Xmax = tilecoord.xmax;
        double Xmin = tilecoord.xmin;
        double Ymax = tilecoord.ymax;
        double Ymin = tilecoord.ymin;
        TString tilename = TString(tilecoord.tilename);

        if(fabs(fabs(Zmax - Zmin) - fabs(m_STKsiliconThickness))>0.1*micrometer){
          std::string error("[GeometryManager::readDampeGeometryParameters] ERROR: consistency check failed: m_STKsiliconThickness differes from the actual silicon thickness!");
          DmpLogError<<error<<DmpLogEndl;
          throw error;
        }


        if(tilename.Contains(TString(STK_SILICONTILEX_GDML_NAME))){
            if(std::find(m_STKXstrips_z->begin(),   m_STKXstrips_z->end(),   Zmax) == m_STKXstrips_z->end())   m_STKXstrips_z->push_back(Zmax);
            if(std::find(m_STKXstrips_z_b->begin(), m_STKXstrips_z_b->end(), Zmin) == m_STKXstrips_z_b->end()) m_STKXstrips_z_b->push_back(Zmin);

            if(std::find(m_STKXtiles_x->begin(),   m_STKXtiles_x->end(),   Xmin) == m_STKXtiles_x->end())   m_STKXtiles_x->push_back(Xmin);
            if(std::find(m_STKXtiles_x->begin(),   m_STKXtiles_x->end(),   Xmax) == m_STKXtiles_x->end())   m_STKXtiles_x->push_back(Xmax);
            if(std::find(m_STKXtiles_y->begin(),   m_STKXtiles_y->end(),   Ymin) == m_STKXtiles_y->end())   m_STKXtiles_y->push_back(Ymin);
            if(std::find(m_STKXtiles_y->begin(),   m_STKXtiles_y->end(),   Ymax) == m_STKXtiles_y->end())   m_STKXtiles_y->push_back(Ymax);
        }

        if(tilename.Contains(TString(STK_SILICONTILEY_GDML_NAME))){
            if(std::find(m_STKYstrips_z->begin(),   m_STKYstrips_z->end(),   Zmax) == m_STKYstrips_z->end())   m_STKYstrips_z->push_back(Zmax);
            if(std::find(m_STKYstrips_z_b->begin(), m_STKYstrips_z_b->end(), Zmin) == m_STKYstrips_z_b->end()) m_STKYstrips_z_b->push_back(Zmin);

            if(std::find(m_STKYtiles_x->begin(),   m_STKYtiles_x->end(),   Xmin) == m_STKYtiles_x->end())   m_STKYtiles_x->push_back(Xmin);
            if(std::find(m_STKYtiles_x->begin(),   m_STKYtiles_x->end(),   Xmax) == m_STKYtiles_x->end())   m_STKYtiles_x->push_back(Xmax);
            if(std::find(m_STKYtiles_y->begin(),   m_STKYtiles_y->end(),   Ymin) == m_STKYtiles_y->end())   m_STKYtiles_y->push_back(Ymin);
            if(std::find(m_STKYtiles_y->begin(),   m_STKYtiles_y->end(),   Ymax) == m_STKYtiles_y->end())   m_STKYtiles_y->push_back(Ymax);
        }
    }
    std::sort(m_STKXstrips_z->begin(),   m_STKXstrips_z->begin()   + m_STKXstrips_z->size());
    std::sort(m_STKXstrips_z_b->begin(), m_STKXstrips_z_b->begin() + m_STKXstrips_z_b->size());
    std::sort(m_STKYstrips_z->begin(),   m_STKYstrips_z->begin()   + m_STKYstrips_z->size());
    std::sort(m_STKYstrips_z_b->begin(), m_STKYstrips_z_b->begin() + m_STKYstrips_z_b->size());

    std::sort(m_STKXtiles_x->begin(),   m_STKXtiles_x->begin()   + m_STKXtiles_x->size());
    std::sort(m_STKXtiles_y->begin(),   m_STKXtiles_y->begin()   + m_STKXtiles_y->size());
    std::sort(m_STKYtiles_x->begin(),   m_STKYtiles_x->begin()   + m_STKYtiles_x->size());
    std::sort(m_STKYtiles_y->begin(),   m_STKYtiles_y->begin()   + m_STKYtiles_y->size());


    // * below is the debigging cross-ckeck
    if(m_STKXstrips_z->size()   != m_STKnplanes){
      std::string error("[GeometryManager::readDampeGeometryParameters] ERROR: mismatch in the number of STK planes : m_STKXstrips_z->size()");
      DmpLogError<<error<<DmpLogEndl;
      throw error;
    }
    if(m_STKXstrips_z_b->size() != m_STKnplanes){
      std::string error("[GeometryManager::readDampeGeometryParameters] ERROR: mismatch in the number of STK planes : m_STKXstrips_z_b->size()");
      DmpLogError<<error<<DmpLogEndl;
      throw error;
    }
    if(m_STKYstrips_z->size()   != m_STKnplanes){
      std::string error("[GeometryManager::readDampeGeometryParameters] ERROR: mismatch in the number of STK planes : m_STKYstrips_z->size()");
      DmpLogError<<error<<DmpLogEndl;
      throw error;
    }
    if(m_STKYstrips_z_b->size() != m_STKnplanes){
      std::string error("[GeometryManager::readDampeGeometryParameters] ERROR: mismatch in the number of STK planes : m_STKYstrips_z_b->size()");
      DmpLogError<<error<<DmpLogEndl;
      throw error;

    }
    if(m_STKXtiles_x->size()!=m_STKnLaddersXdirection*2){
      std::string error("[GeometryManager::readDampeGeometryParameters] ERROR: mismatch in the number of STK ladders per X direction!");
      DmpLogError<<error<<DmpLogEndl;
      throw error;
    }
    if(m_STKXtiles_y->size()!=m_STKnLaddersXdirection*2){
      std::string error("[GeometryManager::readDampeGeometryParameters] ERROR: mismatch in the number of STK X-tiles per Y direction!");
      DmpLogError<<error<<DmpLogEndl;
      throw error;
    }
    if(m_STKYtiles_x->size()!=m_STKnLaddersYdirection*2){
      std::string error("[GeometryManager::readDampeGeometryParameters] ERROR: mismatch in the number of STK Y-tiles per X direction!");
      DmpLogError<<error<<DmpLogEndl;
      throw error;
    }
    if(m_STKYtiles_y->size()!=m_STKnLaddersYdirection*2){
      std::string error("[GeometryManager::readDampeGeometryParameters] ERROR: mismatch in the number of STK ladders per Y direction!");
      DmpLogError<<error<<DmpLogEndl;
      throw error;
    }


    m_STKConverter_z   = new std::vector<double>();
    m_STKConverter_z_b = new std::vector<double>();
    m_STKConverter_x   = new std::vector<double>();
    m_STKConverter_y   = new std::vector<double>();

    std::vector<STKTileCoordinates>* converters = __ParseXMLtesselatedVolumes(std::string(STK_CONVERTERTILE_GDML_NAME), nodeSTKtungstencoordinates, xmlEngine);
    for(int i=0; i<converters->size();i++){
      STKTileCoordinates tilecoord = converters->at(i);
      double Zmax = tilecoord.zmax;
      double Zmin = tilecoord.zmin;
      double Xmax = tilecoord.xmax;
      double Xmin = tilecoord.xmin;
      double Ymax = tilecoord.ymax;
      double Ymin = tilecoord.ymin;
      TString tilename = TString(tilecoord.tilename);

      if(std::find(m_STKConverter_z->begin(),   m_STKConverter_z->end(),   Zmax) == m_STKConverter_z->end())   m_STKConverter_z->push_back(Zmax);
      if(std::find(m_STKConverter_z_b->begin(), m_STKConverter_z_b->end(), Zmin) == m_STKConverter_z_b->end()) m_STKConverter_z_b->push_back(Zmin);
      if(std::find(m_STKConverter_x->begin(),   m_STKConverter_x->end(),   Xmin) == m_STKConverter_x->end())   m_STKConverter_x->push_back(Xmin);
      if(std::find(m_STKConverter_x->begin(),   m_STKConverter_x->end(),   Xmax) == m_STKConverter_x->end())   m_STKConverter_x->push_back(Xmax);
      if(std::find(m_STKConverter_y->begin(),   m_STKConverter_y->end(),   Ymin) == m_STKConverter_y->end())   m_STKConverter_y->push_back(Ymin);
      if(std::find(m_STKConverter_y->begin(),   m_STKConverter_y->end(),   Ymax) == m_STKConverter_y->end())   m_STKConverter_y->push_back(Ymax);
    }

    std::sort(m_STKConverter_z->begin(),   m_STKConverter_z->begin()   + m_STKConverter_z->size());
    std::sort(m_STKConverter_z_b->begin(), m_STKConverter_z_b->begin() + m_STKConverter_z_b->size());
    std::sort(m_STKConverter_x->begin(),   m_STKConverter_x->begin()   + m_STKConverter_x->size());
    std::sort(m_STKConverter_y->begin(),   m_STKConverter_y->begin()   + m_STKConverter_y->size());

    if(m_STKConverter_z->size() != m_STKnConverterLayers || m_STKConverter_z_b->size() != m_STKnConverterLayers || m_STKConverter_x->size()  != m_STKnconvertersXYdirection *2 || m_STKConverter_y->size() != m_STKnconvertersXYdirection * 2){
      DmpLogDebug<<"[GeometryManager::readDampeGeometryParameters]            m_STKnConverterLayers:      "<<m_STKnConverterLayers<<DmpLogEndl;
      DmpLogDebug<<"[GeometryManager::readDampeGeometryParameters]            m_STKConverter_z->size():   "<<m_STKConverter_z->size()<<DmpLogEndl;
      DmpLogDebug<<"[GeometryManager::readDampeGeometryParameters]            m_STKConverter_z_b->size(): "<<m_STKConverter_z_b->size()<<DmpLogEndl;
      DmpLogDebug<<"[GeometryManager::readDampeGeometryParameters]            m_STKConverter_x->size():   "<<m_STKConverter_x->size()<<DmpLogEndl;
      DmpLogDebug<<"[GeometryManager::readDampeGeometryParameters]            m_STKConverter_y->size():   "<<m_STKConverter_y->size()<<DmpLogEndl;
      std::string error("[GeometryManager::readDampeGeometryParameters] ERROR: consistency check failed: tungsten coordinates do not correspond to the claimed number of tungsten converters!");
      DmpLogError<<error<<DmpLogEndl;
      throw error;
    }





    DmpLogDebug<< "[GeometryManager::init]"<<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init] STK parameters:"<<DmpLogEndl;

    DmpLogDebug<< "[GeometryManager::init]  m_STKXstrips_z  = {"<<DmpLogEndl;
    for(int i=0; i<m_STKXstrips_z->size();i++) DmpLogDebug<< "  "<<m_STKXstrips_z->at(i)<<DmpLogEndl;
    DmpLogDebug<<"}"<<DmpLogEndl;

    DmpLogDebug<< "[GeometryManager::init]  m_STKXstrips_z_b  = {"<<DmpLogEndl;
    for(int i=0; i<m_STKXstrips_z_b->size();i++) DmpLogDebug<< "  "<<m_STKXstrips_z_b->at(i)<<DmpLogEndl;
    DmpLogDebug<<"}"<<DmpLogEndl;

    DmpLogDebug<< "[GeometryManager::init]  m_STKYstrips_z  = {"<<DmpLogEndl;
    for(int i=0; i<m_STKYstrips_z->size();i++) DmpLogDebug<< "  "<<m_STKYstrips_z->at(i)<<DmpLogEndl;
    DmpLogDebug<<"}"<<DmpLogEndl;

    DmpLogDebug<< "[GeometryManager::init]  m_STKYstrips_z_b  = {"<<DmpLogEndl;
    for(int i=0; i<m_STKYstrips_z_b->size();i++) DmpLogDebug<< "  "<<m_STKYstrips_z_b->at(i)<<DmpLogEndl;
    DmpLogDebug<<"}"<<DmpLogEndl;

    DmpLogDebug<< "[GeometryManager::init]  m_STKXtiles_x  = {"<<DmpLogEndl;
    for(int i=0; i<m_STKXtiles_x->size(); i++) DmpLogDebug<< "  "<<m_STKXtiles_x->at(i)<<DmpLogEndl;
    DmpLogDebug<<"}"<<DmpLogEndl;

    DmpLogDebug<< "[GeometryManager::init]  m_STKYtiles_y  = {"<<DmpLogEndl;
    for(int i=0; i<m_STKYtiles_y->size(); i++) DmpLogDebug<< "  "<<m_STKYtiles_y->at(i)<<DmpLogEndl;
    DmpLogDebug<<"}"<<DmpLogEndl;

    DmpLogDebug<< "[GeometryManager::init]  m_STKConverter_z  = {"<<DmpLogEndl;
    for(int i=0; i<m_STKConverter_z->size(); i++) DmpLogDebug<< "  "<<m_STKConverter_z->at(i)<<DmpLogEndl;
    DmpLogDebug<<"}"<<DmpLogEndl;

    DmpLogDebug<< "[GeometryManager::init]  m_STKConverter_z_b  = {"<<DmpLogEndl;
    for(int i=0; i<m_STKConverter_z_b->size(); i++) DmpLogDebug<< "  "<<m_STKConverter_z_b->at(i)<<DmpLogEndl;
    DmpLogDebug<<"}"<<DmpLogEndl;

    DmpLogDebug<< "[GeometryManager::init]  m_STKConverter_x  = {"<<DmpLogEndl;
    for(int i=0; i<m_STKConverter_x->size(); i++) DmpLogDebug<< "  "<<m_STKConverter_x->at(i)<<DmpLogEndl;
    DmpLogDebug<<"}"<<DmpLogEndl;

    DmpLogDebug<< "[GeometryManager::init]  m_STKConverter_y  = {"<<DmpLogEndl;
    for(int i=0; i<m_STKConverter_y->size(); i++) DmpLogDebug<< "  "<<m_STKConverter_y->at(i)<<DmpLogEndl;
    DmpLogDebug<<"}"<<DmpLogEndl;

    DmpLogDebug<< "[GeometryManager::init]  m_STKnStripsPerLadder   =  " << m_STKnStripsPerLadder <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_STKactiveTileSize     =  " << m_STKactiveTileSize <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_STKSiliconGuardRing   =  " << m_STKSiliconGuardRing <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_STKsiliconPitch       =  " << m_STKsiliconPitch <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_STKsiliconROPitch     =  " << m_STKsiliconROPitch <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_STKsiliconThickness   =  " << m_STKsiliconThickness <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_STKnConverterLayers   =  " << m_STKnConverterLayers <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_nconvertersXYdirection = " << m_STKnconvertersXYdirection<<DmpLogEndl;

    //* TODO: REPLACE WITH THE EXACT RADIATION LENGTH MAP
    DmpLogDebug<< "[GeometryManager::init]  m_STKConverterX0               =  " << m_STKConverterX0 <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_STKSupportTrayAndSiliconNX0  =  " << m_STKSupportTrayAndSiliconNX0 <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_STKHCombNX0                  =  " << m_STKHCombNX0 <<DmpLogEndl;
    DmpLogDebug<< "[GeometryManager::init]  m_STKHobeyCombWidth            =  " << m_STKHobeyCombWidth <<DmpLogEndl;
    //* TODO: REPLACE WITH THE EXACT RADIATION LENGTH MAP
    //std::cout<< "[GeometryManager::init]  m_STKTilesSeparation    = " << m_STKTilesSeparation <<DmpLogEndl;






    /*
     *   Read PSD geometry parameters TODO ...
     */




    return 0;
}

/*
std::string DmpGeometryManager::__getAuxGDMLParamter(G4GDMLAuxListType auxinfo, std::string parameterName){
    for(G4int i=0; i<auxinfo.size(); i++){
        std::string type = auxinfo.at(i).type;
        if (type == parameterName){
            return (std::string)(auxinfo.at(i).value);
        }
    }
    std::cout << "[GeometryManager::__getAuxGDMLParamter] ERROR: parameter "<< parameterName << " mot found in GDML file --> throwing Exception"<<DmpLogEndl;
    std::cerr << "[GeometryManager::__getAuxGDMLParamter] ERROR: parameter "<< parameterName << " mot found in GDML file --> throwing Exception"<<DmpLogEndl;
    throw;
}
*/


double DmpGeometryManager::__GetXMLParameter(const char* parameter, XMLNodePointer_t node, TXMLEngine* xml)
{
  // *
  // * Coded by Yifeng
  // *
  XMLAttrPointer_t attr;
  XMLAttrPointer_t NextAttr;
  double result = -1;

  XMLNodePointer_t child = xml->GetChild(node);
  while(child!=0){
    attr = xml->GetFirstAttr(child);
    while(attr!=0){
      //char* value = xml->GetAttrValue(attr);
      if(strcmp(xml->GetAttrValue(attr),parameter)==0){
        NextAttr = xml->GetNextAttr(attr);
        result = atof(xml->GetAttrValue(NextAttr));
        return result;
      }
      attr = xml->GetNextAttr(attr);
    }
    child = xml->GetNext(child);
  }
  if (result==-1){
    DmpLogError << "[GeometryManager::__GetXMLParameter] ERROR: parameter "<< parameter << " is not found" <<DmpLogEndl;
    throw;
  }
}



RotationMatrix* DmpGeometryManager::__GetXMLRotation(const char* rotationname, XMLNodePointer_t node, TXMLEngine* xml){
    XMLNodePointer_t child = xml->GetChild(node);
    while(child!=0){


      // Check if it is a rotation tag
      if(strcmp(xml->GetNodeName(child),"rotation") && strcmp(xml->GetNodeName(child),"Rotation")){
        child = xml->GetNext(child);
        continue;
      }



      // Get attributes
      const char*  name =      "";
      double x          = -9999.9;
      double y          = -9999.9;
      double z          = -9999.9;
      const char*  unit = 0;
      XMLAttrPointer_t attr = xml->GetFirstAttr(child);
      while(attr!=0){
        if      (strcmp(xml->GetAttrName(attr),"name")==0 || strcmp(xml->GetAttrName(attr),"Name")==0) name = xml->GetAttrValue(attr);
        else if (strcmp(xml->GetAttrName(attr),"unit")==0 || strcmp(xml->GetAttrName(attr),"Unit")==0) unit = xml->GetAttrValue(attr);
        else if (strcmp(xml->GetAttrName(attr),"x")==0    || strcmp(xml->GetAttrName(attr),"X")==0)    x    = atof(xml->GetAttrValue(attr));
        else if (strcmp(xml->GetAttrName(attr),"y")==0    || strcmp(xml->GetAttrName(attr),"Y")==0)    y    = atof(xml->GetAttrValue(attr));
        else if (strcmp(xml->GetAttrName(attr),"z")==0    || strcmp(xml->GetAttrName(attr),"Z")==0)    z    = atof(xml->GetAttrValue(attr));
        attr = xml->GetNextAttr(attr);
      }

      // Check name
      if(strcmp(name,rotationname)){
        child = xml->GetNext(child);
        continue;
      }

      // Create rotation matrix
      double rotationunit;
      if(!unit || strcmp(unit,"rad")==0 || strcmp(unit,"Rad")==0) rotationunit = 1.0;                // radians
      if(strcmp(unit,"deg")==0 || strcmp(unit,"Deg")==0)          rotationunit = TMath::Pi() / 180.; // degrees
      if( (x!=0 && y!=0 ) || (x!=0 && z!=0 ) || (y!=0 && z!=0 )){
        std::ostringstream error_s;
        error_s<<"[GeometryManager::__GetXMLRotation] Unsupported rotation type:\n";
        error_s<<"   only simple rotations around either x, y, or z axes are supported. Consider code revising.";
        std::string error(error_s.str());
        DmpLogError<<error<<DmpLogEndl;
        throw error;
      }

      TRotation* matrix = new TRotation();
      matrix->RotateX(x*rotationunit);
      matrix->RotateY(y*rotationunit);
      matrix->RotateZ(z*rotationunit);
      RotationMatrix* result = new RotationMatrix(rectify(matrix->XX()),
                                                  rectify(matrix->XY()),
                                                  rectify(matrix->XZ()),
                                                  rectify(matrix->YX()),
                                                  rectify(matrix->YY()),
                                                  rectify(matrix->YZ()),
                                                  rectify(matrix->ZX()),
                                                  rectify(matrix->ZY()),
                                                  rectify(matrix->ZZ())
                                                  );
      return result;

    }

    std::ostringstream error_s;
    error_s << "[GeometryManager::__GetXMLRotation] ERROR: rotationname "<< rotationname << " is not found";
    std::string error(error_s.str());
    DmpLogError<<error<<DmpLogEndl;
    throw error;

}


/*
vector<STLsiliconTile>* DmpGeometryManager::__GetXMLVolumes(const char* namemask, XMLNodePointer_t node, TXMLEngine* xml){

}
*/

std::vector<STKTileCoordinates>* DmpGeometryManager::__ParseXMLtesselatedVolumes(std::string volumenamemask, XMLNodePointer_t node, TXMLEngine* xml){
  DmpLogDebug<<"[DmpGeometryManager::__ParseXMLtesselatedVolumes] Parsing STK tiles from XML file:"<<DmpLogEndl;

  std::vector<std::string>*                volumenames      = new std::vector<std::string>();
  std::vector<std::string>*                solidnames       = new std::vector<std::string>();
  std::vector<std::vector<std::string>*>*  solidvertexnames = new std::vector<std::vector<std::string>*>();
  std::vector<std::vector<double>*>*       solidvertex_x    = new std::vector<std::vector<double>*>();
  std::vector<std::vector<double>*>*       solidvertex_y    = new std::vector<std::vector<double>*>();
  std::vector<std::vector<double>*>*       solidvertex_z    = new std::vector<std::vector<double>*>();

  //*
  //* Volumes
  //*
  XMLNodePointer_t structurenode = 0;
  for(structurenode = node; structurenode!=0; structurenode = xml->GetNext(structurenode)){
    if(strcmp(xml->GetNodeName(structurenode),"structure") == 0) break;
  }
  if(!structurenode){
    std::string error = "[DmpGeometryManager::__ParseXMLtesselatedVolumes] ERROR: no 'structure' tag found!";
    DmpLogError<<error<<DmpLogEndl;
    throw error;
  }
  for(XMLNodePointer_t volume = xml->GetChild(structurenode); volume!=0 ; volume = xml->GetNext(volume)){
    if(strcmp(xml->GetNodeName(volume),"volume")) continue;
    XMLAttrPointer_t   attr = xml->GetFirstAttr(volume);
    std::string volume_name = std::string(xml->GetAttrValue(attr));
    if(TString(volume_name).Contains(volumenamemask) == false) continue;
    //std::string solidref = __GetXMLSolidrefForVolume(volume, xml);
    std::string solidref;
    for(XMLNodePointer_t solidref_node = xml->GetChild(volume); solidref_node!=0; solidref_node = xml->GetNext(solidref_node)){
      if(strcmp(xml->GetNodeName(solidref_node),"solidref")) continue;
      XMLAttrPointer_t attr = xml->GetFirstAttr(solidref_node);
      solidref = std::string(xml->GetAttrValue(attr));
    }

    volumenames->push_back(volume_name);
    solidnames ->push_back(solidref);
  }


  //*
  //* Solids
  //*
  XMLNodePointer_t solidnode = 0;
  for(solidnode = node; solidnode!=0;  solidnode = xml->GetNext(solidnode)){
    if(strcmp(xml->GetNodeName(solidnode),"solids") == 0) break;
  }
  if(!solidnode){
    std::string error = "[DmpGeometryManager::__ParseXMLtesselatedVolumes] ERROR: no 'solids' tag found!";
    DmpLogError<<error<<DmpLogEndl;
    throw error;
  }
  for(XMLNodePointer_t solid = xml->GetChild(solidnode); solid!=0; solid = xml->GetNext(solid)){
    if(solidvertexnames->size()== solidnames->size()) break;
    if(strcmp(xml->GetNodeName(solid),"tessellated")) continue;
    std::vector<std::string>* vertexnames = new std::vector<std::string>(); // __GetXMLVertexnamesForSolid(solidnode,xml);
    for(XMLNodePointer_t triangular = xml->GetChild(solid); triangular!=0; triangular = xml->GetNext(triangular)){
      if(strcmp(xml->GetNodeName(triangular),"triangular")) continue;
      for(XMLAttrPointer_t attr = xml->GetFirstAttr(triangular); attr!=0; attr = xml->GetNextAttr(attr)){
        std::string vertexname = std::string(xml->GetAttrValue(attr));
        if(std::find(vertexnames->begin(),vertexnames->end(),vertexname)!=vertexnames->end()) continue;
        vertexnames->push_back(vertexname);
      }
    }
    solidvertexnames->push_back(vertexnames);
    solidvertex_x   ->push_back(new std::vector<double>(vertexnames->size(),NOTANUMBER));
    solidvertex_y   ->push_back(new std::vector<double>(vertexnames->size(),NOTANUMBER));
    solidvertex_z   ->push_back(new std::vector<double>(vertexnames->size(),NOTANUMBER));
    int current_index = solidvertexnames->size() - 1;
    //std::string solidname = __GetXMLSolidName(solid, TXMLEngine);
    std::string solidname;
    for(XMLAttrPointer_t attr = xml->GetFirstAttr(solid); attr!=0; attr = xml->GetNextAttr(attr)){
      if(strcmp(xml->GetAttrName(attr),"name")) continue;
      solidname = std::string(xml->GetAttrValue(attr));
      break;
    }
    /*
    std::cout<<"solidname: "<<solidname<<DmpLogEndl;
    std::cout<<"solidvertexnames->size(): "<<solidname<<DmpLogEndl;
    */
    if(solidname != solidnames->at(current_index)){
      std::ostringstream error_s;
      error_s << "[DmpGeometryManager::__ParseXMLtesselatedVolumes] ERROR: consistency check failed\n";
      error_s << "[DmpGeometryManager::__ParseXMLtesselatedVolumes]        please ensure that tesseleted solids and volumes follow the same order in the .xml file.";
      std::string error(error_s.str());
      DmpLogError<<error<<DmpLogEndl;
      throw error;
    }
  }
  if(solidvertexnames->size()!= solidnames->size()){
    std::ostringstream error_s;
    error_s << "[DmpGeometryManager::__ParseXMLtesselatedVolumes] ERROR: consistency check failed\n";
    error_s << "[DmpGeometryManager::__ParseXMLtesselatedVolumes]        number of tesseleted volumes does not equal the number of corresponding solids in the .xml file.";
    std::string error(error_s.str());
    DmpLogError<<error<<DmpLogEndl;
    throw error;
  }


  //*
  //* Verices
  //*
  XMLNodePointer_t constantsnode = 0;
  for(constantsnode = node; constantsnode!=0; constantsnode = xml->GetNext(constantsnode)){
    if(strcmp(xml->GetNodeName(constantsnode),"define") == 0) break;
  }
  if(!constantsnode){
    std::string error = "[DmpGeometryManager::__ParseXMLtesselatedVolumes] ERROR: no 'define' tag found!";
    DmpLogError<<error<<DmpLogEndl;
    throw error;
  }
  int solid_for_position = 0;
  for(XMLNodePointer_t position = xml->GetChild(constantsnode); position!=0; position = xml->GetNext(position)){
    if(strcmp(xml->GetNodeName(position),"position")) continue;

    std::string vertexname = "";
    double x               = NOTANUMBER;
    double y               = NOTANUMBER;
    double z               = NOTANUMBER;
    const char*       unit = 0;
    for(XMLAttrPointer_t attr = xml->GetFirstAttr(position); attr!=0; attr = xml->GetNextAttr(attr)){
      if      (strcmp(xml->GetAttrName(attr),"name")==0 || strcmp(xml->GetAttrName(attr),"Name")==0) vertexname = std::string(xml->GetAttrValue(attr));
      else if (strcmp(xml->GetAttrName(attr),"unit")==0 || strcmp(xml->GetAttrName(attr),"Unit")==0) unit       = xml->GetAttrValue(attr);
      else if (strcmp(xml->GetAttrName(attr),"x")==0    || strcmp(xml->GetAttrName(attr),"X")==0)    x          = atof(xml->GetAttrValue(attr));
      else if (strcmp(xml->GetAttrName(attr),"y")==0    || strcmp(xml->GetAttrName(attr),"Y")==0)    y          = atof(xml->GetAttrValue(attr));
      else if (strcmp(xml->GetAttrName(attr),"z")==0    || strcmp(xml->GetAttrName(attr),"Z")==0)    z          = atof(xml->GetAttrValue(attr));
    }
    if( std::find(solidvertexnames->at(solid_for_position)->begin(),   solidvertexnames->at(solid_for_position)->end(),  vertexname) ==  solidvertexnames->at(solid_for_position)->end()){
      solid_for_position++;
    }

    int vertex_i;
    for(vertex_i=0; vertex_i<solidvertexnames->at(solid_for_position)->size(); vertex_i++){
      if(solidvertexnames->at(solid_for_position)->at(vertex_i) != vertexname) continue;
      solidvertex_x->at(solid_for_position)->at(vertex_i) = x;
      solidvertex_y->at(solid_for_position)->at(vertex_i) = y;
      solidvertex_z->at(solid_for_position)->at(vertex_i) = z;
      break;
    }
    if(vertex_i>=solidvertexnames->at(solid_for_position)->size()){
      std::ostringstream error_s;
      error_s << "[DmpGeometryManager::__ParseXMLtesselatedVolumes] ERROR: consistency check failed!"<<DmpLogEndl;
      error_s << "[DmpGeometryManager::__ParseXMLtesselatedVolumes]        no solid found for the vertex: "<<vertexname<<DmpLogEndl;;
      error_s << "[DmpGeometryManager::__ParseXMLtesselatedVolumes]        Please ensure that volumes, vertices, and solids follow the same order in the .xml file.";
      std::string error(error_s.str());
      DmpLogError<<error<<DmpLogEndl;
      throw error;
    }
  }


  // Final consistency check
  int nvertices = 0;
  int nvertex_x = 0;
  int nvertex_y = 0;
  int nvertex_z = 0;
  for(int i=0; i<volumenames->size(); i++){
    nvertices+=solidvertexnames->at(i)->size();
    nvertex_x+=solidvertex_x->at(i)->size();
    nvertex_y+=solidvertex_y->at(i)->size();
    nvertex_z+=solidvertex_z->at(i)->size();
  }
  DmpLogDebug<<"[DmpGeometryManager::__ParseXMLtesselatedVolumes] Consistency check:"<<DmpLogEndl;
  DmpLogDebug<<"[DmpGeometryManager::__ParseXMLtesselatedVolumes]   Number of tile volumes : " <<volumenames->size()<<DmpLogEndl;
  DmpLogDebug<<"[DmpGeometryManager::__ParseXMLtesselatedVolumes]   Number of tile solids  : " <<solidnames->size()<<DmpLogEndl;
  DmpLogDebug<<"[DmpGeometryManager::__ParseXMLtesselatedVolumes]   Number of tile vertices: " <<nvertices<<DmpLogEndl;
  DmpLogDebug<<"[DmpGeometryManager::__ParseXMLtesselatedVolumes]   Number of tile vertices (x coordinates):    " <<nvertex_x<<DmpLogEndl;
  DmpLogDebug<<"[DmpGeometryManager::__ParseXMLtesselatedVolumes]   Number of tile vertices (y coordinates):    " <<nvertex_y<<DmpLogEndl;
  DmpLogDebug<<"[DmpGeometryManager::__ParseXMLtesselatedVolumes]   Number of tile vertices (z coordinates):    " <<nvertex_z<<DmpLogEndl;
  bool consistency_check_passed = true;
  if(volumenames->size()!=solidnames->size()) consistency_check_passed = false;
  if(8*volumenames->size()!=nvertices)        consistency_check_passed = false;
  if(8*volumenames->size()!=nvertex_x)        consistency_check_passed = false;
  if(8*volumenames->size()!=nvertex_y)        consistency_check_passed = false;
  if(8*volumenames->size()!=nvertex_z)        consistency_check_passed = false;
  if(!consistency_check_passed){
    std::string error = "[DmpGeometryManager::__ParseXMLtesselatedVolumes] ERROR: consistency check failed! throwing exception!";
    DmpLogError<<error<<DmpLogEndl;
    throw error;
  }
  DmpLogDebug<<"[DmpGeometryManager::__ParseXMLtesselatedVolumes] consistency check success; continue"<<DmpLogEndl;
  DmpLogDebug<<"[DmpGeometryManager::__ParseXMLtesselatedVolumes]"<<DmpLogEndl;


  // Create STK tile objects
  std::vector<STKTileCoordinates>* tiles = new std::vector<STKTileCoordinates>();
  for(int tile_i = 0; tile_i<volumenames->size(); tile_i++){
    double min_x =  NOTANUMBER;
    double max_x = -NOTANUMBER;
    double min_y =  NOTANUMBER;
    double max_y = -NOTANUMBER;
    double min_z =  NOTANUMBER;
    double max_z = -NOTANUMBER;
    int n_vertices = solidvertex_x->at(tile_i)->size();
    for(int vertex_i=0; vertex_i< n_vertices; vertex_i++){
      if(solidvertex_x->at(tile_i)->at(vertex_i) > max_x ) max_x = solidvertex_x->at(tile_i)->at(vertex_i);
      if(solidvertex_y->at(tile_i)->at(vertex_i) > max_y ) max_y = solidvertex_y->at(tile_i)->at(vertex_i);
      if(solidvertex_z->at(tile_i)->at(vertex_i) > max_z ) max_z = solidvertex_z->at(tile_i)->at(vertex_i);
      if(solidvertex_x->at(tile_i)->at(vertex_i) < min_x ) min_x = solidvertex_x->at(tile_i)->at(vertex_i);
      if(solidvertex_y->at(tile_i)->at(vertex_i) < min_y ) min_y = solidvertex_y->at(tile_i)->at(vertex_i);
      if(solidvertex_z->at(tile_i)->at(vertex_i) < min_z ) min_z = solidvertex_z->at(tile_i)->at(vertex_i);
    }
    STKTileCoordinates tile;
    tile.tilename = volumenames->at(tile_i);
    tile.xmin = min_x;
    tile.xmax = max_x;
    tile.ymin = min_y;
    tile.ymax = max_y;
    tile.zmin = min_z;
    tile.zmax = max_z;
    tiles->push_back(tile);
  }


  // Apply translation/rotation to STK tiles
  for(int tile_i=0; tile_i<tiles->size(); tile_i++){
    __convertSTKtileToGobalCoordinates(&(tiles->at(tile_i)));
  }



  // Return
  DmpLogDebug<<"[DmpGeometryManager::__ParseXMLtesselatedVolumes] ... done parsing STK tiles"<<DmpLogEndl;
  return tiles;
}






/*    ____________________________________________
 *   |                                            |
 *   |    STK geometry interface functions        |
 *   |____________________________________________|
 */
STKStripParameters* DmpGeometryManager::getSTKROstripParameters(int stripnm)
{
  // *
  // * For the STK geometry documentation, see:
  // *     http://dpnc.unige.ch/SVNDAMPE/DAMPE/Documents/SimulationSTKLaytout.pdf
  // *

  STKStripParameters* result = new STKStripParameters();


  //---------------------------------------------------------------------------
  result->isX         =  stripnm / 1000000;                                                                          // isX   = [0; 1]
  result->plane       = (stripnm - result->isX*1000000)/100000;                                                     // plane = [0; 5]
  result->block       = (stripnm - result->isX*1000000 - result->plane*100000)/m_STKnROStripsBlock;                 // block = [0; 3]
  result->strip       =  stripnm - result->isX*1000000 - result->plane*100000 - result->block*m_STKnROStripsBlock;  // strip = [0; 384*4 -1]
  result->nl          = result->strip/m_STKnROStripsPerLadder;                                                      // nl    = [0; 3]
  result->stripLadder = result->strip- result->nl*m_STKnROStripsPerLadder;                                          // stripLadder = [0; 383]
  //---------------------------------------------------------------------------


  //result->iplane      = result->plane  * 2 + result->isX;                                                                             // iplane      = [0; 11]
  result->iplane      = result->plane  * 2 + !(result->isX);                                                                          // iplane      = [0; 11]
  result->iblock      = result->iplane * m_STKnblocksPerLayer + result->block;                                                        // iblock      = [0; 47]
  result->ladder      = result->strip/m_STKnROStripsPerLadder + result->block*m_STKnLaddersBlock + result->iplane*m_STKnLaddersLayer; // ladder      = [0; 192]
  result->trb         = result->block*2+!(result->isX);                                                                                  // trb         = [0; 7]

  return result;
}

int DmpGeometryManager::getSTKROstripID(int isX /* 0,1 */, int plane /* 0 - 5 */, int block /* 0 - 3 */, int nl /* 0 - 3 */, int stripLadder /* 0 - 384 */)
{
  // *
  // * This function is "inverse" to getSTKROstripParameters one
  // *    see: http://dpnc.unige.ch/SVNDAMPE/DAMPE/Documents/SimulationSTKLaytout.pdf
  // *
  // * TODO: If this function is changed, one has to change also the strip ID assignment in the simulation/digitization !!!
  // *

  return isX*1000000 + plane*100000 + block * m_STKnROStripsBlock + nl * m_STKnROStripsPerLadder + stripLadder;
}





STKLadderParameters* DmpGeometryManager::getSTKladderParameters(int laddernm)  // 0- 192
{

  STKLadderParameters* result = new STKLadderParameters();
  result->iplane  =  laddernm / m_STKnLaddersLayer;                        // [0; 11]
  result->plane   =  result->iplane / 2;                                   // [0; 5]
  result->iblock  =  laddernm / m_STKnLaddersBlock;                        // [0; 47]
  result->block   =  result->iblock - result->iplane*m_STKnblocksPerLayer; // [0; 3]
  //result->isX     =  result->iplane - 2*((int)(result->iplane/2));       //  0 / 1
  result->isX     =  getSTKisLayerX(result->iplane);                       //  0 / 1
  result->trb     =  result->block*2 + !(result->isX);                     // [0; 7]
  result->iladder =  laddernm - result->iplane*m_STKnLaddersLayer - result->block*m_STKnLaddersBlock; // [0; 3]
  return result;
}

int DmpGeometryManager::getSTKisLayerX(int layer){
  return !(layer%2);
}



int DmpGeometryManager::getSTKisLayerY(int layer){
  return layer%2;
}



int DmpGeometryManager::getSTKXlayerForPlane(int plane){       // [0; 5]
  return 2*plane;
}



int DmpGeometryManager::getSTKYlayerForPlane(int plane){       // [0; 5]
  return 2*plane + 1;
}



int DmpGeometryManager::getSTKPlaneForLayer(int layer){       // layer [0; 11]
  return layer /2 ;
}






double DmpGeometryManager::getSTKlayerZcoordinate(int layer){  // layer [0; 11]
  /*
   *  Returns the middle z position of silicon in the layer [mm]
   */
  int plane = getSTKPlaneForLayer(layer);
  if (getSTKisLayerX(layer))       return (m_STKXstrips_z->at(plane) + m_STKXstrips_z_b->at(plane)) * 0.5;
  else if  (getSTKisLayerY(layer)) return (m_STKYstrips_z->at(plane) + m_STKYstrips_z_b->at(plane)) * 0.5;
  std::string error ("[GeometryManager::getSTKlayerZcoordinate] ERROR: internal cross-check failed: illegal 'layer' parameter");
  DmpLogError<<error<<DmpLogEndl;
  throw error;
}


double DmpGeometryManager::getSTKXlayerZcoordinate(int plane){  // plane [0;5]
  /*
   *  Returns the middle z position of silicon in the layer [mm]
   */
  return (m_STKXstrips_z->at(plane) + m_STKXstrips_z_b->at(plane)) * 0.5;
}


double DmpGeometryManager::getSTKYlayerZcoordinate(int plane){  // plane [0;5]
  /*
   *  Returns the middle z position of silicon in the layer [mm]
   */
  return (m_STKYstrips_z->at(plane) + m_STKYstrips_z_b->at(plane)) * 0.5;
}




double DmpGeometryManager::getSTKClusterGlobalPosition(int laddernm, double positionOnladder){
  // *
  // *   Returns the global position of the point, which holds the position on the ladder equal to [positionOnladder]
  // *   The resulting position is with respect to the plane (X and Y coordinate for X- and Y- ladders respectively)
  // *   see Refs: http://dpnc.unige.ch/SVNDAMPE/DAMPE/Documents/SimulationSTKLaytout.pdf
  // *             http://dpnc.unige.ch/dampe/dokuwiki/lib/exe/fetch.php?media=stk-wiki:strip-channel-numbering_modi7.pdf
  // *
  STKLadderParameters* ladder = getSTKladderParameters(laddernm);

  int isX = ladder->isX;
  int isY = !isX;
  int block = ladder->block;
  int isFirstFloat = 0;
  if( (isX &&  block == 1) ||
      (isX &&  block == 3) ||
      (isY &&  block == 2) ||
      (isY &&  block == 3)){
    isFirstFloat = 1;
  }

  if(ladder->isX == 1){
    int index = ladder->block / m_STKnBlockaPerAxis * m_STKnLaddersBlock + ladder->iladder;
    return m_STKXtiles_x->at(index*2) + m_STKSiliconGuardRing + m_STKsiliconPitch/2 + m_STKsiliconPitch *isFirstFloat +  positionOnladder;
  }
  if(ladder->isX == 0){
    int index = ladder->block % m_STKnBlockaPerAxis * m_STKnLaddersBlock + ladder->iladder;
    return m_STKYtiles_y->at(m_STKYtiles_y->size()-1-index*2) - m_STKSiliconGuardRing - m_STKsiliconPitch/2 -m_STKsiliconPitch *isFirstFloat - positionOnladder;
  }
  std::string error("[GeometryManager::getSTKLadderTopPosition] ERROR: the ladder is neither X nor Y - something went wrong!");
  DmpLogError<<error<<DmpLogEndl;
  throw error;
}




double DmpGeometryManager::getSTKROStripPositionOnLadder(int ladder, int strip){
  return strip * m_STKsiliconROPitch;
}





void DmpGeometryManager::__convertSTKtileToGobalCoordinates(STKTileCoordinates* tile){
  // *
  // *  Returns coordinates of STK silicon tile in the global reference frame
  // *  NOTE: geant4 GetTranslation method provides the obeject's translation
  // *  before applying rotation, while getFrameTranslation returns the
  // *  translation coordinates after apllying rotation
  // *

  TVector3  Min(tile->xmin,tile->ymin,tile->zmin);
  TVector3  Max(tile->xmax,tile->ymax,tile->zmax);

  // Apply local STK rotation
  Min = (*m_STKlocalRotation)*Min;
  Max = (*m_STKlocalRotation)*Max;

  // Apply local STK translation
  Min+= *(m_STKlocalTranslation);
  Max+= *(m_STKlocalTranslation);

  // Apply global STK translation wrt world coordinate system
  tile->xmin = std::min(Min.x(),Max.x());
  tile->xmax = std::max(Min.x(),Max.x());
  tile->ymin = std::min(Min.y(),Max.y());
  tile->ymax = std::max(Min.y(),Max.y());
  tile->zmin = std::min(Min.z(),Max.z()) + m_STKzOffset;
  tile->zmax = std::max(Min.z(),Max.z()) + m_STKzOffset;
}





/*
STKTileCoordinates* DmpGeometryManager::__getSTKtileCoordinates(G4VPhysicalVolume* stktile){
  // *
  // *  Returns coordinates of STK silicon tile in the global reference frame
  // *  NOTE: geant4 GetTranslation method provides the obeject's translation
  // *  before applying rotation, while getFrameTranslation returns the
  // *  translation coordinates after apllying rotation
  // *


  //G4ThreeVector
  TVector3      Min( ((G4TessellatedSolid*)(stktile->GetLogicalVolume()->GetSolid()))->GetMinXExtent(),
                     ((G4TessellatedSolid*)(stktile->GetLogicalVolume()->GetSolid()))->GetMinYExtent(),
                     ((G4TessellatedSolid*)(stktile->GetLogicalVolume()->GetSolid()))->GetMinZExtent());

  //G4ThreeVector
  TVector3      Max( ((G4TessellatedSolid*)(stktile->GetLogicalVolume()->GetSolid()))->GetMaxXExtent(),
                     ((G4TessellatedSolid*)(stktile->GetLogicalVolume()->GetSolid()))->GetMaxYExtent(),
                     ((G4TessellatedSolid*)(stktile->GetLogicalVolume()->GetSolid()))->GetMaxZExtent());



  //std::cout<<" \n\n\n\n stklocalrotation->xx(): "<<stklocalrotation->xx()<<"\n\n\n\n";

  //std::cout<<" \n\n\n\n one \n";
  //std::cout<<" Min.x() : "<<Min.x()<<DmpLogEndl;
  //std::cout<<" Min.y() : "<<Min.y()<<DmpLogEndl;
  //std::cout<<" Min.z() : "<<Min.z()<<DmpLogEndl;
  //std::cout<<" Max.x() : "<<Min.x()<<DmpLogEndl;
  //std::cout<<" Max.y() : "<<Min.y()<<DmpLogEndl;
  //std::cout<<" Max.z() : "<<Min.z()<<DmpLogEndl;
  //std::cout<<" \n\n\n\n";





  // Apply local STK rotation
  Min = (*m_STKlocalRotation)*Min;
  Max = (*m_STKlocalRotation)*Max;
  //Min = ((G4RotationMatrix)(*stklocalrotation))*Min;
  //Max = ((G4RotationMatrix)(*stklocalrotation))*Min;


  //std::cout<<" \n\n\n\n two \n";
  //  std::cout<<" Min.x() : "<<Min.x()<<DmpLogEndl;
  //  std::cout<<" Min.y() : "<<Min.y()<<DmpLogEndl;
  //  std::cout<<" Min.z() : "<<Min.z()<<DmpLogEndl;
  //  std::cout<<" Max.x() : "<<Min.x()<<DmpLogEndl;
  //  std::cout<<" Max.y() : "<<Min.y()<<DmpLogEndl;
  //  std::cout<<" Max.z() : "<<Min.z()<<DmpLogEndl;
  //  std::cout<<" \n\n\n\n";





  // Apply local STK translation
  Min+= *(m_STKlocalTranslation);
  Max+= *(m_STKlocalTranslation);


  //std::cout<<" \n\n\n\n three \n";
  //  std::cout<<" Min.x() : "<<Min.x()<<DmpLogEndl;
  //  std::cout<<" Min.y() : "<<Min.y()<<DmpLogEndl;
  //  std::cout<<" Min.z() : "<<Min.z()<<DmpLogEndl;
  //  std::cout<<" Max.x() : "<<Min.x()<<DmpLogEndl;
  //  std::cout<<" Max.y() : "<<Min.y()<<DmpLogEndl;
  //  std::cout<<" Max.z() : "<<Min.z()<<DmpLogEndl;
  //  std::cout<<" \n\n\n\n";




  // Apply global STK translation wrt world coordinate system
  STKTileCoordinates* tilecoordinates = new STKTileCoordinates;
  tilecoordinates->xmin = std::min(Min.x(),Max.x());
  tilecoordinates->xmax = std::max(Min.x(),Max.x());
  tilecoordinates->ymin = std::min(Min.y(),Max.y());
  tilecoordinates->ymax = std::max(Min.y(),Max.y());
  tilecoordinates->zmin = std::min(Min.z(),Max.z()) + m_STKzOffset;
  tilecoordinates->zmax = std::max(Min.z(),Max.z()) + m_STKzOffset;

  //
  return tilecoordinates;
}
*/


int DmpGeometryManager::getSTKPlaneIndex(int countFromCalorimeter){
  //*
  //* Returns an index of the STK plane:
  //*   countFromCalorimeter = 0 corresponds to the plane closest to the calorimeter
  //*
  if(countFromCalorimeter == -1) return 0;
  return m_STKnplanes - 1  - countFromCalorimeter;
}

int DmpGeometryManager::getSTKNextPlaneIndex(int currentPlaneIndex, bool directionTowardsCalorimeter){
  //*
  //* Returns an index of next STK plane:
  //*   returns -1 if reached end of calorimeter
  //*
  int nextPlaneIndex;
  if(directionTowardsCalorimeter)  nextPlaneIndex =  currentPlaneIndex + 1;
  else                             nextPlaneIndex =  currentPlaneIndex - 1;

  if(nextPlaneIndex<0 || nextPlaneIndex> m_STKnplanes-1) return -1;
  return nextPlaneIndex;
}

bool DmpGeometryManager::isSTKPlaneLowestZ(int plane){
  // *
  // * Returns true if plane has the lowest Z coordinate
  // *
  if(plane == 0) return true;
  return false;
}

bool DmpGeometryManager::isSTKPlaneHighestZ(int plane){
  // *
  // * Returns true if plane has the highest Z coordinate
  // *
  if(plane == m_STKnplanes - 1) return true;
  return false;
}


int DmpGeometryManager::getSTKactivePlaneXforPoint(double point_x, double point_y, double point_z){
  // *
  // * - Returns the index of X plane containing the point
  // * - Returns -1 if point pertains to neither of STK active X-planes
  // * - [point_x], [point_y], [point_z] - coordinates of point in
  // *   the global reference frame [mm]
  // *

  bool matchx = false;
  for(int tile_i=0; tile_i < m_STKnLaddersXdirection; tile_i++){
    if(m_STKXtiles_x->at(2*tile_i)    > point_x) continue;
    if(m_STKXtiles_x->at(2*tile_i+1)  < point_x) continue;
    matchx = true;
    break;
  }
  bool matchy = false;
  for(int tile_i=0; tile_i < m_STKnLaddersYdirection; tile_i++){
    if(m_STKXtiles_y->at(2*tile_i)     > point_y) continue;
    if(m_STKXtiles_y->at(2*tile_i+1)   < point_y) continue;
    matchy = true;
    break;
  }
  if( !matchx  ||  !matchy) return -1;

  for(int plane_i=0; plane_i < m_STKnplanes; plane_i++){
    double margin = (m_STKXstrips_z->at(plane_i) - m_STKXstrips_z_b->at(plane_i))*0.01;
    if(m_STKXstrips_z_b->at(plane_i) - margin > point_z ) continue;
    if(m_STKXstrips_z->at(plane_i)   + margin < point_z ) continue;
    return plane_i;
  }

  return -1;
}




int DmpGeometryManager::getSTKactivePlaneYforPoint(double point_x, double point_y, double point_z){
  // *
  // * - Returns the index of Y plane containing the point
  // * - Returns -1 if point pertains to neither of STK active X-planes
  // * - [point_x], [point_y], [point_z] - coordinates of point in
  // *   the global reference frame [mm]
  // *

  bool matchx = false;
  for(int tile_i=0; tile_i < m_STKnLaddersXdirection; tile_i++){
    if(m_STKYtiles_x->at(2*tile_i)   > point_x) continue;
    if(m_STKYtiles_x->at(2*tile_i+1) < point_x) continue;
    matchx = true;
    break;
  }
  bool matchy = false;
  for(int tile_i=0; tile_i < m_STKnLaddersYdirection; tile_i++){
    if(m_STKYtiles_y->at(2*tile_i)    > point_y) continue;
    if(m_STKYtiles_y->at(2*tile_i+1)  < point_y) continue;
    matchy = true;
    break;
  }
  if( !matchx  ||  !matchy) return -1;

  for(int plane_i=0; plane_i < m_STKnplanes; plane_i++){
    double margin = (m_STKYstrips_z->at(plane_i) - m_STKYstrips_z_b->at(plane_i))*0.01;
    if(m_STKYstrips_z_b->at(plane_i) - margin > point_z ) continue;
    if(m_STKYstrips_z->at(plane_i)   + margin < point_z ) continue;
    return plane_i;
  }

  return -1;
}









