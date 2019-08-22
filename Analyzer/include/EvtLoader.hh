#include "BaconAna/DataFormats/interface/TEventInfo.hh"
#include "BaconAna/DataFormats/interface/TVertex.hh"
#include "BaconAna/Utils/interface/TTrigger.hh"
#include "Utils.hh"

#include "TH1F.h"
#include "TH2D.h"
#include "TTree.h"
#include "TBranch.h"
#include "TClonesArray.h"
#include "TLorentzVector.h"

#include <string>
#include <vector>

using namespace baconhep;

class EvtLoader {
public:
  EvtLoader(TTree *iTree,std::string iName,
	    std::string iHLTFile="${CMSSW_BASE}/src/BaconAnalyzer-1/Analyzer/data/HLTFile_25ns",
	    std::string iPUWeight="${CMSSW_BASE}/src/BaconAnalyzer-1/Analyzer/data/puWeights_8X.root");
  ~EvtLoader();
  void reset();
  void setupTree  (TTree *iTree);
  void setupTreeQbert(TTree *iTree);
  void load (int iEvent);
  //Fillers
  void fillEvent(unsigned int trigBit, float lWeight, unsigned int passJson);
  bool passSkim();
  //Trigger
  bool passFilter();
  void addTrigger(std::string iName);
  bool passTrigger();
  bool passTrigger(std::string iTrigger);
  ULong64_t triggerBit();
  //PU
  float        puWeight(float iPU);
  int          nVtx();
  bool         PV();
  //EWK and kFactor
  void computeCorr(float iPt,
		   std::string iHist0,
		   std::string iHist1,
		   std::string iHist2,
		   std::string iNLO,
                   std::string ikfactor="${CMSSW_BASE}/src/BaconAnalyzer-1/Analyzer/data/kfactors.root");

  //Vars
  float fRho;
  float fMetPhi;
  float fMet;
  float fPuppEtPhi;
  float fPuppEt;

  unsigned int fRun;
  unsigned int fEvtV;
  unsigned int fLumi;
  unsigned int fPassJson;

  float fPu;

  TEventInfo   *fEvt;

  float fevtWeight;
  float fScale;

  float fkfactorQCD,fkfactorEWK;

  int fNVtx;
protected:
  TBranch      *fEvtBr;

  TClonesArray *fVertices;
  TBranch      *fVertexBr;

  TTree        *fTree;
  TTrigger     *fTrigger;
  TH1F         *fPUWeightHist;

  TH1F         *fHist0;
  TH1F         *fHist1;
  TH1F         *fHist2;

  std::vector<std::string> fTrigString;

  char*  fSample;
  ULong64_t  fITrigger;
  unsigned int fMetFilters;
  unsigned int fNPU;

  float fPUWeight;
};
