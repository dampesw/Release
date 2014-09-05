/*
 *  Created on: Jan 21, 2014
 *      Author: Andrii Tykhonov
 */

#ifndef GEOMETRYMANAGER_H_
#define GEOMETRYMANAGER_H_

/*
#include "G4GDMLParser.hh"
#include "G4VPhysicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
*/

#include "DmpVSvc.h"

#include "TXMLEngine.h"
#include "TRotation.h"
#include "TVector3.h"



struct STKStripParameters{
  int isX;      // [0; 1]
  int plane;    // [0; 5]
  int block;    // [0; 3]
  int strip;    // [0; 384*4 -1]

  int iplane;      // [0; 11]
  int iblock;      // [0; 47]
  int ladder;      // [0; 192]
  int nl;          // [0; 3]
  int stripLadder; // [0; 383]
  int trb;         // [0; 7]
};

struct STKLadderParameters{
  int plane;   //   [0; 5]
  int iplane;  //   [0; 11]
  int iblock;  //   [0; 47]
  int block;   //   [0; 3]
  int isX;     //    0 / 1
  int trb;     //   [0; 7]
  int iladder; //   [0; 3]
};

struct STKTileCoordinates{
  std::string tilename;
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  double zmin;
  double zmax;
};

class RotationMatrix{
public:
  RotationMatrix( double the_xx,  double the_xy,  double the_xz, double the_yx, double the_yy,  double the_yz, double the_zx, double the_zy, double the_zz)
  {
   xx = the_xx;
   xy = the_xy;
   xz = the_xz;
   yx = the_yx;
   yy = the_yy;
   yz = the_yz;
   zx = the_zx;
   zy = the_zy;
   zz = the_zz;
  }
  ~RotationMatrix(){};
  TVector3 operator*(TVector3& vector){
    return TVector3(xx*vector.x() + xy*vector.y() + xz*vector.z(),
                    yx*vector.x() + yy*vector.y() + yz*vector.z(),
                    zx*vector.x() + zy*vector.y() + zz*vector.z());
  }
private:
  double xx;
  double xy;
  double xz;
  double yx;
  double yy;
  double yz;
  double zx;
  double zy;
  double zz;
};

class STLsiliconTile{
private:
  std::string volumename;
  std::string solidname;
};





class DmpGeometryManager : public DmpVSvc {
public:
    DmpGeometryManager();
    virtual ~DmpGeometryManager(){};
    bool Initialize();
    bool Finalize();
    void Set(const std::string&,const std::string&);


    /*
    static std::string GetGeometryMainGdmlName(){
      //static std::string gdmlGeometryMainName(DEFAULT_GEOMETRY_MAIN_GDML_NAME);
      return GdmlGeometryMainName;
    }
    */

    /*
    static void SetGeometryMainGdmlName(char* name){
      //static std::string gdmlGeometryMainName(DEFAULT_GEOMETRY_MAIN_GDML_NAME);
      GdmlGeometryMainName = std::string(name);
    }
    */


    // BGO parameters
    //-------------------------------------------------------------------------------------------
    int    getBGOCalLayerNb(){return m_CalLayerNb;};
    int    getBGOCalLayerBarNb(){return m_CalLayerBarNb;};
    double getBGOCalSizeXY(){ return m_CalSizeXY;};
    double getBGOCalSizeZ() { return m_CalSizeZ;};
    double getBGOCenterZ()  { return m_CalCenterZ;};
    double getBGOCalLayerSeparation(){return m_CalLayerSeparation;};
    double getBGOCalBarSeparation(){return m_CalBarSeparation;};
    double getBGOCalBarX()  {return m_CalBGOBarX;};
    double getBGOCalBarY()  {return m_CalBGOBarY;};
    double getBGOCalBarZ()  {return m_CalBGOBarZ;};
    std::vector<double>* getBGOXCalBarZ(){ return m_XCalBar_z;};
    std::vector<double>* getBGOYCalBarZ(){ return m_YCalBar_z;};
    double getBGOCalTopZ()   { return m_CalTopZ;};
    double getBGOCalBottomZ(){ return m_CalBottomZ;};
    double getBGOCalTopX()   { return m_CalTopX;};
    double getBGOCalTopY()   { return m_CalTopY;};


    // STK parameters
    //-------------------------------------------------------------------------------------------
    std::vector<double>* getSTKXStripsZ()         { return m_STKXstrips_z;};
    std::vector<double>* getSTKXStripsZbottom()   { return m_STKXstrips_z_b;};
    std::vector<double>* getSTKYstripsZ()         { return m_STKYstrips_z;};
    std::vector<double>* getSTKYstripsZbottom()   { return m_STKYstrips_z_b;};
    double getSTKConvertersZ       (int converterlayer)  { return 0.5* (m_STKConverter_z->at(converterlayer) + m_STKConverter_z_b->at(converterlayer)); }
    double getSTKConvertersZTop    (int converterlayer)  { return m_STKConverter_z->at(converterlayer);}
    double getSTKConvertersZBottom (int converterlayer)  { return m_STKConverter_z_b->at(converterlayer);}
    int    getSTKnStripsPerLadder() { return m_STKnStripsPerLadder;};
    double getSTKActiveTileSize()   { return m_STKactiveTileSize;};
    double getSTKSiliconGuardRIng() { return m_STKSiliconGuardRing;};
    double getSTKSiliconPitch()     { return m_STKsiliconPitch;};
    double getSTKSIliconROPitch()   { return m_STKsiliconROPitch;};
    //double getSTKTilesSeparation()  { return m_STKTilesSeparation;};
    int    getSTKnplanes()          { return m_STKnplanes;   };
    int    getSTKnlayers()          { return m_STKnplanes*2; };
    int    getSTKnladders()         { return m_STKnladders;  };
    int    getSTKnBlocksLayer()     { return m_STKnblocksPerLayer; };
    int    getSTKnLaddersLayer()    { return m_STKnLaddersLayer; };
    int    getSTKnTRB()             { return m_STKnTRB; };
    int    getSTKnLaddersTRB()      { return m_STKnLadTRB; }
    int    getSTKnROstripsLadder()  { return m_STKnROStripsPerLadder; }
    int    getSTKnLaddersBlock()    { return m_STKnLaddersBlock; }
    int    getSTKNConverters()      { return m_STKnConverterLayers;}
    double getSTKSilicoThickness()  { return m_STKsiliconThickness;}
    int    getSTKnVA()              { return m_STKnVA; }
    int    getSTKnChannelsVA()      { return m_STKnChannelsVA; }


    //* TODO: REPLACE WITH THE EXACT RADIATION LENGTH MAP
    double getSTKconverterMaterialX0()   {return m_STKConverterX0;}
    double getSTKSupportTrayAndSIliconNX0()  { return m_STKSupportTrayAndSiliconNX0;}
    double detSTKSupportTrayHCombNX0()       { return m_STKHCombNX0;}
    double getSTKHobeyCombWidth()            { return m_STKHobeyCombWidth;}
    //* TODO: REPLACE WITH THE EXACT RADIATION LENGTH MAP


    int    getSTKROstripID(int isX, int plane, int block, int nl, int stripLadder);
    STKStripParameters*  getSTKROstripParameters(int stripnm);
    STKLadderParameters* getSTKladderParameters (int laddernm);
    double getSTKROStripPositionOnLadder(int ladder, int strip);                   // ladder [0; 191], strip [0; 383]
    double getSTKClusterGlobalPosition(int laddernm, double positionOnLadder);     // ladder [0; 191]
    int    getSTKXlayerForPlane(int plane);   // plane = [0; 5]
    int    getSTKYlayerForPlane(int plane);   // plane = [0; 5]

    int    getSTKisLayerX(int layer);
    int    getSTKisLayerY(int layer);
    int    getSTKPlaneForLayer(int layer);        // layer [0; 11]
    double getSTKlayerZcoordinate(int layer);     // layer [0; 11]
    double getSTKXlayerZcoordinate(int plane);  // plane [0; 5]
    double getSTKYlayerZcoordinate(int plane);  // plane [0; 5]
    int    getSTKactivePlaneXforPoint(double point_x, double point_y, double point_z);
    int    getSTKactivePlaneYforPoint(double point_x, double point_y, double point_z);
    int    getSTKPlaneIndex(int countFromCalorimeter);
    int    getSTKNextPlaneIndex(int currentPlaneIndex, bool directionTowardsCalorimeter);
    bool   isSTKPlaneLowestZ(int plane);
    bool   isSTKPlaneHighestZ(int plane);
    //-------------------------------------------------------------------------------------------




private:
    int connectToGeometryXML();
    int readDampeGeometryParameters();
    double           __GetXMLParameter(const char* parameter, XMLNodePointer_t node, TXMLEngine* xml);
    //std::string      __getAuxGDMLParamter(G4GDMLAuxListType auxinfo, std::string parameterName);
    RotationMatrix*  __GetXMLRotation(const char* rotationname, XMLNodePointer_t node, TXMLEngine* xml);
    std::vector<STKTileCoordinates>*  __ParseXMLtesselatedVolumes(std::string volumenamemask, XMLNodePointer_t node, TXMLEngine* xml);
    //STKTileCoordinates* __getSTKtileCoordinates(G4VPhysicalVolume* stktile); // OBSOLETE (DO NOT DE); KEPT AS A WORKING EXMAPLE OF PARSING TESSELETED SOLIDS
    void                __convertSTKtileToGobalCoordinates(STKTileCoordinates* tile);

    //G4GDMLParser* fParser;

    // XML thing
    TXMLEngine*   xmlEngine;
    XMLNodePointer_t nodeDAMPEparameter;
    XMLNodePointer_t nodeSTKparameter;
    XMLNodePointer_t nodeSTKsiliconcoordinates;
    XMLNodePointer_t nodeSTKtungstencoordinates;
    XMLNodePointer_t nodeBGOEparameter;

    // BGO parameters
    int m_CalLayerNb;
    int m_CalLayerBarNb;
    double m_CalSizeXY;
    double m_CalSizeZ;
    double m_CalCenterZ;
    std::vector<double>* m_XCalBar_z;
    std::vector<double>* m_YCalBar_z;
    double m_CalTopZ;
    double m_CalBottomZ;
    double m_CalTopX;
    double m_CalTopY;
    double m_CalLayerSeparation;
    double m_CalBarSeparation;
    double m_CalBGOBarX;
    double m_CalBGOBarY;
    double m_CalBGOBarZ;
    double m_CalTopLayerZ;

    // STK parameters
    //double m_STKadjustXposition;
    //double m_STKadjustYposition;
    //double m_STKadjustZposition;
    double m_STKzOffset;
    TVector3*          m_STKlocalTranslation;
    RotationMatrix*    m_STKlocalRotation;


    std::vector<double>* m_STKXstrips_z;
    std::vector<double>* m_STKXstrips_z_b;
    std::vector<double>* m_STKYstrips_z;
    std::vector<double>* m_STKYstrips_z_b;

    std::vector<double>* m_STKXtiles_x;
    std::vector<double>* m_STKXtiles_y;
    std::vector<double>* m_STKYtiles_x;
    std::vector<double>* m_STKYtiles_y;

    std::vector<double>*  m_STKConverter_z;
    std::vector<double>*  m_STKConverter_z_b;
    std::vector<double>*  m_STKConverter_x;
    std::vector<double>*  m_STKConverter_y;

    int m_STKnStripsPerLadder;
    double m_STKactiveTileSize;
    double m_STKSiliconGuardRing;
    double m_STKsiliconPitch;
    double m_STKsiliconROPitch;
    //double m_STKTilesSeparation;
    double m_STKsiliconThickness;
    int    m_STKnConverterLayers;
    int    m_STKnconvertersXYdirection;

    //* TODO: REPLACE WITH THE EXACT RADIATION LENGTH MAP
    double m_STKConverterX0;
    double m_STKSupportTrayAndSiliconNX0;
    double m_STKHCombNX0;
    double m_STKHobeyCombWidth;
    //* TODO: REPLACE WITH THE EXACT RADIATION LENGTH MAP


    int m_STKnplanes;            // 6
    int m_STKnladders;           // 192
    int m_STKnblocksPerLayer;    // 4
    int m_STKnBlockaPerAxis;     // 2 = sqrt(4)
    int m_STKnTRB;               // 8
    int m_STKnLadTRB;            // 24
    int m_STKnLaddersLayer;      // 4*4 = 16
    int m_STKnROStripsPerLadder; // 384
    int m_STKnLaddersBlock;      // 4;
    int m_STKnROStripsBlock;     // 1536;  //384 *4
    // *
    // * The following parameters are used to cross-check algorithms
    // * if mismatch is found -> an Exception is thrown
    int m_STKnLaddersXdirection; // 8;
    int m_STKnLaddersYdirection; // 8;

    int m_STKnVA;            // 6
    int m_STKnChannelsVA;    // 64

    //std::string* gdmlGeometryMainName;

};





#endif
