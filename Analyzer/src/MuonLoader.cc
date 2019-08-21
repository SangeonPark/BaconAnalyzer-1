#include "../include/MuonLoader.hh"

#include "TFile.h"
#include <cmath>
#include <iostream>
#include <sstream>

using namespace baconhep;

MuonLoader::MuonLoader(TTree *iTree,std::string iLabel) {
  fMuons  = new TClonesArray("baconhep::TMuon");
  iTree->SetBranchAddress("Muon",       &fMuons);
  fMuonBr  = iTree->GetBranch("Muon");
  fN = 2;

  fYear = iLabel;
  for(int i0 = 0; i0 < fN*3.; i0++) {double pVar = 0; fVars.push_back(pVar);}
  for(int i0 = 0; i0 <     4; i0++) {double pVar = 0; fVars.push_back(pVar);}

  //Add Dimuon Variables for Z boson
  fDiMuon = new TLorentzVector(0.,0.,0.,0.);
  fMassMin = 71.2;
  fMassMax = 111.2;
}
MuonLoader::~MuonLoader() {
  delete fMuons;
  delete fMuonBr;
}
void MuonLoader::reset() {
  fNMuonsLoose = 0;
  fNMuonsTight = 0;
  fismu0Tight  = 0;
  fismu1Tight  = 0;
  fLooseMuons.clear();
  fMediumMuons.clear();
  fTightMuons.clear();
  fHighPtMuons.clear();
  looseMuons.clear();
  mediumMuons.clear();
  tightMuons.clear();
  highptMuons.clear();
  for(unsigned int i0 = 0; i0 < fVars.size(); i0++) fVars[i0] = 0;
}
//Resetting Dimuon Vector
void MuonLoader::resetDiMu() {
  fDiMuon->SetPtEtaPhiM(1e-9,0,0,0);
}
void MuonLoader::setupTree(TTree *iTree) {
  reset();
  fTree = iTree;
  fTree->Branch("nmuLoose"  ,&fNMuonsLoose ,"fNMuonsLoose/I");
  fTree->Branch("nmuMedium" ,&fNMuonsMedium,"fNMuonsMedium/I");
  fTree->Branch("nmuTight"  ,&fNMuonsTight ,"fNMuonsTight/I");
  fTree->Branch("nmuHighPt" ,&fNMuonsHighPt,"fNMuonsHighPt/I");
  setupNtuple("vmuoLoose"   ,iTree,fN,fVars);       // add leading 2 muons: pt,eta,phi,mass (2*4=8)
  fTree->Branch("dimu","TLorentzVector", &fDiMuon);
}
void MuonLoader::setupTreeQbert(TTree *iTree) {
  reset();
  fTree = iTree;
  setupNtuple  ("mu",iTree,fN,fVars);
}
void MuonLoader::load(int iEvent) {
  fMuons  ->Clear();
  fMuonBr ->GetEntry(iEvent);
}
void MuonLoader::selectMuons(std::vector<TLorentzVector> &iMuons, float met, float metPhi) {
  reset();
  int lCount = 0,lMCount = 0, lHPCount =0,lTCount =0;
  fvMetNoMu.SetMagPhi(met,metPhi);

  // Muon selection
  for  (int i0 = 0; i0 < fMuons->GetEntriesFast(); i0++) {
    TMuon *pMuon = (TMuon*)((*fMuons)[i0]);
    if(pMuon->pt        <=  10)                      continue;
    if(fabs(pMuon->eta) >=  2.4)                     continue;
    if(!passMuonLooseSel(pMuon,fYear))               continue;
    lCount++;

    if(!passMuonMediumSel(pMuon,fYear))             lMCount++;
    if(!passMuonHighPtSel(pMuon,fYear))             lHPCount++;

    TVector2 vMu; vMu.SetMagPhi(pMuon->pt, pMuon->phi);
    fvMetNoMu = fvMetNoMu + vMu;

    addMuon(pMuon,fLooseMuons);
    if(pMuon->pt>20 && fabs(pMuon->eta)< 2.4 && passMuonTightSel(pMuon,fYear)){
      if(lCount==1) fismu0Tight = 1;
      if(lCount==2) fismu1Tight = 1;
      lTCount++;
    }
    addMuon(pMuon,fTightMuons);

  }
  addVMuon(fLooseMuons,looseMuons,MUON_MASS);
  addVMuon(fMediumMuons,mediumMuons,MUON_MASS);
  addVMuon(fLooseMuons,tightMuons,MUON_MASS);
  addVMuon(fHighPtMuons,highptMuons,MUON_MASS);

  fNMuonsLoose = lCount;
  fNMuonsMedium = lMCount;
  fNMuonsTight = lTCount;
  fNMuonsHighPt = lHPCount;

  // Cleaning iMuons
  if(fTightMuons.size()>1){
    iMuons.push_back(tightMuons[0]); // save first tight muon
    iMuons.push_back(looseMuons[1]); // if leading lepton is tight, save the subleading loose one
  }

  if(fVars.size() > 0) fillMuon(fN,fLooseMuons,fVars);
}
void MuonLoader::selectDiMuon(std::vector<TLorentzVector> &iVetoes) {
  resetDiMu();
  for  (int i0 = 0; i0 < fMuons->GetEntriesFast(); i0++) {
    TMuon *pMuon1 = (TMuon*)((*fMuons)[i0]);
    if(!passJEC(pMuon1) || !passMuonTightSel(pMuon1,fYear)) continue;
    for(int i1 = 0; i1 < fMuons->GetEntriesFast(); i1++) {
      if(i0 == i1) continue;
      TMuon *pMuon2 = (TMuon*)((*fMuons)[i1]);
      if(!passJEC(pMuon2) || !passMuonTightSel(pMuon2,fYear)) continue;
      TLorentzVector pVec1;      pVec1.SetPtEtaPhiM(pMuon1->pt,pMuon1->eta,pMuon1->phi,0.105);
      TLorentzVector pVec2;      pVec2.SetPtEtaPhiM(pMuon2->pt,pMuon2->eta,pMuon2->phi,0.105);
      if(fMassMin > (pVec1+pVec2).M() || fMassMax < (pVec1+pVec2).M()) continue;
      iVetoes.push_back(pVec1);
      iVetoes.push_back(pVec2);
      break;
    }
    if(iVetoes.size() > 1) break;
  }
  TLorentzVector lDi;
  if(iVetoes.size() > 1) lDi =  (iVetoes[0] + iVetoes[1]);
  if(iVetoes.size() > 1) fDiMuon->SetPtEtaPhiM( lDi.Pt(),lDi.Eta(),lDi.Phi(),lDi.M());
}
bool MuonLoader::passJEC(TMuon *iMuon) {
  //if(iMuon->pt > 10) std::cout << "===> " << iMuon->pt << " -- " << iMuon->eta << " -- " << iMuon->nTkLayers << " -- " << iMuon->nPixHits << " -- " << iMuon->trkKink << " -- " << iMuon->dz << " -- " << iMuon->ptErr/iMuon->pt << " -- " << (iMuon->typeBits & kPFMuon) << std::endl;
  if(!(fabs(iMuon->eta)     < 2.3     ))                               return false;
  if(!(iMuon->pt            > 20      ))                               return false;
  return true;
}
