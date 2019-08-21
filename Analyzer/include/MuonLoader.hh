#ifndef MuonLoader_H
#define MuonLoader_H
#include "TH2D.h"
#include "TVector2.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include "TBranch.h"
#include "TClonesArray.h"
#include "BaconAna/DataFormats/interface/TMuon.hh"
#include "Utils.hh"

using namespace baconhep;

class MuonLoader {
public:
  MuonLoader(TTree *iTree,std::string iLabel="2017");
  ~MuonLoader();
  void reset();
  void setupTree(TTree *iTree);
  void setupTreeQbert(TTree *iTree);
  void load(int iEvent);
  void selectMuons(std::vector<TLorentzVector> &iMuons, float met, float metPhi);
  //dimuon functions
  void resetDiMu();
  void selectDiMuon(std::vector<TLorentzVector> &iVetoes);
  bool passJEC(TMuon *iMuon);
  std::vector<TMuon*> fLooseMuons, fMediumMuons, fTightMuons, fHighPtMuons;
  std::vector<TLorentzVector> looseMuons, mediumMuons, tightMuons, highptMuons;
  int           fNMuonsLoose;
  int           fNMuonsMedium;
  int           fNMuonsTight;
  int           fNMuonsHighPt;
  int           fismu0Tight;
  int           fismu1Tight;

  TVector2      fvMetNoMu;

  const double MUON_MASS = 0.105658369;

  float fMassMin;
  float fMassMax;

protected:
  TClonesArray *fMuons;
  TBranch      *fMuonBr;
  TTree        *fTree;
  std::vector<double> fVars;
  int           fN;
  std::string   fYear;

  TLorentzVector *fDiMuon;
};
#endif
