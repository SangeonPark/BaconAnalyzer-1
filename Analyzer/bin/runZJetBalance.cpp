//================================================================================================
//
//
// Input arguments
//   argv[1] => lName = input bacon file name
//   argv[2] => lOption = dataset type: "mc", "data"
//   argv[3] => lJSON = JSON file for run-lumi filtering of data, specify "none" for MC or no filtering
//   argv[4] => lOutput = Output name
//   argv[5] => iSplit = Number of job
//   argv[6] => maxSplit = Number of jobs to split file into
//________________________________________________________________________________________________

#include "../include/GenLoader.hh"
#include "../include/EvtLoader.hh"
#include "../include/MuonLoader.hh"
#include "../include/ElectronLoader.hh"
#include "../include/PhotonLoader.hh"
#include "../include/JetLoader.hh"
#include "../include/VJetLoader.hh"
#include "../include/PerJetLoader.hh"
#include "../include/RunLumiRangeMap.h"

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include <TError.h>
#include <string>
#include <iostream>

// Object Processors
GenLoader       *fGen        = 0;
EvtLoader       *fEvt        = 0;
MuonLoader      *fMuon       = 0;
ElectronLoader  *fElectron   = 0;
PhotonLoader    *fPhoton     = 0;
PerJetLoader    *fVJet4      = 0;
RunLumiRangeMap *fRangeMap   = 0;

TH1F *fHist                  = 0;


const int NUM_PDF_WEIGHTS = 60;

// Load tree and return infile
TTree* load(std::string iName) {
  TFile *lFile = TFile::Open(iName.c_str());
  TTree *lTree = (TTree*) lFile->FindObjectAny("Events");
  fHist        = (TH1F* ) lFile->FindObjectAny("TotalEvents");
  lTree->SetDirectory(0);
  lFile->Close();
  return lTree;
}

// For Json
bool passEvent(unsigned int iRun,unsigned int iLumi) {
  RunLumiRangeMap::RunLumiPairType lRunLumi(iRun,iLumi);
  return fRangeMap->HasRunLumi(lRunLumi);
}

// === MAIN =======================================================================================================
int main( int argc, char **argv ) {
  gROOT->ProcessLine("#include <vector>");
  const std::string lName        = argv[1];
  const std::string lOption      = argv[2];
  const std::string lLabel       = argv[3];
  const std::string lJSON        = argv[4];
  const std::string lOutput      = argv[5];
  const int         iSplit       = atoi(argv[6]);
  const int         maxSplit     = atoi(argv[7]);

  std::cout << "args " << argv[1] << " " << argv[2] << " "<< argv[3] << " " << argv[4] << " " << argv[5] << std::endl;
  std::string lJson="${CMSSW_BASE}/src/BaconAnalyzer-1/Analyzer/data/";
  lJson.append(lJSON);
  const std::string cmssw_base = getenv("CMSSW_BASE");
  std::string cmssw_base_env = "${CMSSW_BASE}";
  size_t start_pos = lJson.find(cmssw_base_env);
  if(start_pos != std::string::npos) {
    lJson.replace(start_pos, cmssw_base_env.length(), cmssw_base);
  }

  fRangeMap = new RunLumiRangeMap();
  if(lJSON.size() > 0) fRangeMap->AddJSONFile(lJson.c_str());

  bool isData;
  if(lOption.compare("data")!=0) isData = false;
  else isData = true;

  TFile *lInputFile = TFile::Open(lName.c_str());
  TTree *lTree = (TTree*) lInputFile->FindObjectAny("Events");

  // Declare Readers
  fEvt       = new EvtLoader     (lTree,lName);
  fMuon      = new MuonLoader    (lTree,lLabel);
  fElectron  = new ElectronLoader(lTree);                                                   // fElectrons and fElectronBr, fN = 2
  fPhoton    = new PhotonLoader  (lTree);                                                   // fPhotons and fPhotonBr, fN = 1
  fVJet4     = new PerJetLoader  (lTree,"AK4Puppi","AddAK4Puppi","AK4CHS","AddAK4CHS",3, isData, lLabel);

  if(lOption.compare("data")!=0) {
    if(lOption.compare("ps")==0) fGen      = new GenLoader     (lTree,true);
    else fGen      = new GenLoader     (lTree);
  }

  TFile *lFile = TFile::Open(lOutput.c_str(),"RECREATE");
  TTree *lOut  = new TTree("Events","Events");

  //Setup histograms containing total number of processed events (for normalization)
  TH1F *NEvents = new TH1F("NEvents", "NEvents", 1, 0.5, 1.5);
  TH1F *SumWeights = new TH1F("SumWeights", "SumWeights", 1, 0.5, 1.5);
  // And Pileup
  TH1F *Pu = new TH1F("Pu", "Pu", 100, 0, 100);

  // Setup Tree
  fEvt      ->setupTree      (lOut);
  fElectron ->setupTree      (lOut);
  fPhoton   ->setupTree      (lOut);
  fVJet4    ->setupTreeQbert      (lOut,"AK4Puppijet");
  fMuon     ->setupTree      (lOut);

  if(lOption.compare("data")!=0) {
    fGen ->setupTree (lOut);
    if(lOption.compare("ps")==0)   fGen->setPSWeights(lOut);
  }

  // Loop over events i0 = iEvent
  int neventstest = 0;
  int neventsTotal = int(lTree->GetEntriesFast());
  std::cout << maxSplit << std::endl;
  int minEventsPerJob = neventsTotal / maxSplit;
  std::cout << minEventsPerJob << " min events per job " << std::endl;
  int minEvent = iSplit * minEventsPerJob;
  int maxEvent = (iSplit+1) * minEventsPerJob;
  if (iSplit + 1 == maxSplit) maxEvent = neventsTotal;
  std::cout << neventsTotal << " total events, ";
  std::cout << iSplit << " iSplit, ";
  std::cout << maxSplit << " maxSplit, ";
  std::cout << minEvent << " min event ";
  std::cout << maxEvent << " max event " << std::endl;
  for(int i0 = minEvent; i0 < maxEvent; i0++) {
    if(neventstest==0) std::cout << "first iteration " << std::endl;
    if (i0%10000 == 0) std::cout << i0 << " events processed " << std::endl;

    fEvt->load(i0);
    float lWeight = 1;
    unsigned int passJson = 0;
    if(lOption.compare("data")!=0){
      fGen->load(i0);
      lWeight = fGen->fWeight;
      passJson = 1;
      Pu->Fill(fEvt->fPu);
      if(lOption.compare("ps")==0) {
	fGen->loadPSWeights(i0); fGen->fillPSWeights();
      }
    }
    else{
      if(passEvent(fEvt->fRun,fEvt->fLumi)) { passJson = 1;}
    }

    NEvents->SetBinContent(1, NEvents->GetBinContent(1)+lWeight);
    SumWeights->Fill(1.0, lWeight);

    // Primary vertex requirement
    if(!fEvt->PV()) continue;

    // Trigger
    // muon triggers
    fEvt ->addTrigger("HLT_Mu50_v*");
    fEvt ->addTrigger("HLT_TkMu50_v*");
    fEvt ->addTrigger("HLT_Mu100_v*");
    fEvt ->addTrigger("HLT_OldMu100_v*");

    fEvt      ->fillEvent(1,lWeight,passJson);

    // Objects
    gErrorIgnoreLevel=kError;
    std::vector<TLorentzVector> lVetoes;
    std::vector<TLorentzVector> cleaningMuons, cleaningElectrons, cleaningPhotons;
    fMuon     ->load(i0);
    fMuon     ->selectDiMuon(lVetoes);
    fMuon     ->load(i0);
    fMuon     ->selectMuons(cleaningMuons,fEvt->fMet,fEvt->fMetPhi);
    fElectron ->load(i0);
    fElectron ->selectElectrons(fEvt->fRho,fEvt->fMet,cleaningElectrons);
    fPhoton -> load(i0);
    fPhoton -> selectPhotons(fEvt->fRho,cleaningElectrons,cleaningPhotons);

    // CA15Puppi Jets
    //fVJet15   ->load(i0);
    //fVJet15   ->selectVJets(cleaningElectrons,cleaningMuons,cleaningPhotons,1.5,fEvt->fRho,fEvt->fRun);

    // AK8Puppi Jets
    //fVJet8    ->load(i0);
    //fVJet8    ->selectVJets(cleaningElectrons,cleaningMuons,cleaningPhotons,0.8,fEvt->fRho,fEvt->fRun);

    // AK4Puppi Jets
    fVJet4     ->load(i0);
    fVJet4     ->selectVJets(cleaningElectrons,cleaningMuons,cleaningPhotons,0.4,fEvt->fRho,fEvt->fRun);
    lOut->Fill();
    neventstest++;
  }
  std::cout << neventstest << " out of " << lTree->GetEntriesFast() << std::endl;
  lFile->cd();
  lOut->Write();
  NEvents->Write();
  SumWeights->Write();
  Pu->Write();
  lFile->Close();
  lInputFile->Close();
}
