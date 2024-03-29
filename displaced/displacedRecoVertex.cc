// Displaced DGS Project
// Written by Giovanna Cottin (gfc24@cam.ac.uk, gfcottin@uc.cl)
// ATLAS SUSY displaced vertex analysis in arXiv:1504.05162 and 
// https://atlas.web.cern.ch/Atlas/GROUPS/PHYSICS/PAPERS/SUSY-2014-02/
// DV+jets channel
// Cuts: 
// Event Selection :
//   *   4,5,6 jets, pT>90, 65, 55 GeV and |eta|>2.8
// DV Selection - Event pass if at least one displaced vertex with :
//   *   pT of tracks coming from DV  > 1 GeV
//   *   |d0| of tracks coming from DV  > 2 mm
//   *   Material Veto  : If neutralino (if the displaced vertex) is found in dense material region, veto event. 
//                        Note the analysis applies a 3D material map. Here just an upper limit for the fidutial volume.
//   This last two cuts define the signal region :
//   *   Number of tracks coming out of vertex > 5 
//   *   Invariant mass of vertex > 10 GeV 

#include "Pythia8/Pythia.h"
#include "ToyDetector-ATLAS.h"

#include <vector>
#include <string>
#include <iterator>
#include <algorithm>

using namespace Pythia8;

//tracking efficiency parametrization
double efficiency(double pT, double r, double z){

double eff= 0.5*(1.0-exp(-pT/4.0))*exp(-z/270.0)*std::max(-0.0022*r+0.8, 0.0);
return eff;
}

int main(int argc, const char* argv[]) {
  //argv[1]: lifetime
  //argv[2]: cmndfile
  //argv[3]: outputfilename

  Pythia pythia;
  Event& event = pythia.event;
  pythia.readFile(argv[2]);
  bool is8TeV = true;
  // string ctauValues[11] = {"1","1","2.1544","4.6415","10","21.544","46.4158","100","215.4434","464.1588","1000."};
  // string ctauValues[21] = {"1","1","1.43845", "2.06914", "2.97635", "4.28133","6.15848", "8.85867","12.7427","18.3298", "26.3665","37.9269","54.5559","78.476","112.884", "162.378", "233.572","335.982",  "483.293", "695.193","1000"};
  // string pseudoscalarMass[11] = {"20.0","20.0","25.0","35.0","45.0","55.0","60.0","75.0","80.0","95.0","99.0"};
  string pythiaCtau = "1000022:tau0 = ";
  string pythiaMass = "36:m0 = ";
  int nEvent   = pythia.mode("Main:numberOfEvents");
  int nAbort   = pythia.mode("Main:timesAllowErrors"); 

  std::string outputfilename(argv[3]);
  ofstream resultfile;
  resultfile.open(outputfilename);

  std::string lifetime(argv[1]);

  pythia.init();

  cout<<"###################"<<endl;
  cout<<"ctauValues[iTau] = "<< lifetime << endl;
  pythia.readString(pythiaCtau + lifetime);
  cout<<"pythia.particleData.tau0(1000022) = "<<pythia.particleData.tau0(1000022)<<endl;

  int iAbort = 0;
  int nJetCuts=0;
  int nPromptJets=0;
  int nPromptDphi3=0;
  int nPromptDphi=0;
  int nPromptMET=0;
  int nPromptMeff=0;
  int nPromptRatio=0;
  int nDVReco=0;
  int nMaterial=0;
  int nFidutial=0;
  int nTrk=0;
  int nMass=0;
  ToyDetector detector;

  //Start event loop
  for (int iEvent = 0; iEvent < nEvent; ++iEvent) {
    // Generate events. Quit if many failure.
    if (!pythia.next()) {
      event.list();
      if (++iAbort < nAbort) continue;
        cout << " Event generation aborted prematurely, owing to error!\n";
      break;
    }
    if(!detector.getObjects(event)) {
      cout << "No objects found" << endl;
    } 
    else {
    }
    bool passEvtJetCuts = false; 
    bool passEvtDVReco = false;
    bool passEvtPromptJets  =false;
    bool passEvtPromptDphi3  =false;
    bool passEvtPromptDphi  =false;
    bool passEvtPromptMET   =false;
    bool passEvtPromptMeff  =false;
    bool passEvtPromptRatio =false;

    /////////////////
    //Event Selection
    /////////////////
    std::vector<double> JetpT;
    // Cuts for jets 
    for (unsigned long j=0; j < detector.jets.size(); j++) {
      JetpT.push_back(detector.jets.at(j).pT());
      if(abs(detector.jets.at(j).eta())<2.8) continue; 
    }
    //Here I cut on the corresponding n jets and pT (ordered in pT)
    bool is4j80AccepCut = (JetpT.size() > 3) && (JetpT[3] > 90.);
    bool is5j55AccepCut = (JetpT.size() > 4) && (JetpT[4] > 65.);
    bool is6j45AccepCut = (JetpT.size() > 5) && (JetpT[5] > 55.);
    if(is4j80AccepCut || is5j55AccepCut || is6j45AccepCut) passEvtJetCuts=true;
     passEvtJetCuts=true;
    if(passEvtJetCuts) nJetCuts++;
      else continue;
    // /////////////////////////////////
    // //DV Selection
    // //Starts by selecting good tracks
    // /////////////////////////////////
    vector <Vec4> trackProduction;
    vector <Vec4> trackMomenta;
    std::vector<int> trackIndices;
    std::vector<double> trackeff;
    trackMomenta.clear();
    trackProduction.clear();
    trackIndices.clear();
    trackeff.clear();
    //Particle event loop
    for (int i= 0; i < event.size(); i++){
      //Look for good tracks in the particle loop
      //if(hasAmother && event[i].isCharged() && event[i].isFinal()){
      if(event[i].isCharged() && event[i].isFinal()){
        bool passTrackQuality  = true; // true unless probe otherwise
        double rTrk=sqrt(pow2((event[i].vProd()).px())+ pow2((event[i].vProd()).py()));
        double zTrk=abs((event[i].vProd()).pz());
        double phixy = event[i].vProd().phi();
        double deltaphi = phixy - event[i].phi();
        if (abs(deltaphi) > 3.1415) deltaphi = 2 * 3.1415 - abs(deltaphi);
        double d02 = rTrk*sin(deltaphi);        
        //select good tracks for DV reco
        if(abs(d02)<2.0 || event[i].pT()<1.0 || abs(event[i].eta())>2.5) passTrackQuality=false;
        //Reject the track before forming the vertex 
        double iRndNumber = ((double) rand() / (RAND_MAX));
        if(efficiency(event[i].pT(),rTrk,zTrk)<iRndNumber) passTrackQuality=false;
        // Fill the track vectors with good tracks 
        if(passTrackQuality){
          //cout<<"track with index "<<i<<" and pdgID "<<event[i].id()<<" came from a b "<<endl;
          trackIndices.push_back(i); 
          trackProduction.push_back(event[i].vProd());
          trackMomenta.push_back(event[i].p());
        }
      }
    } //end particle loop
    /////////////////////////
    // Vertex reconstruction
    ///////////////////////// 
    // Search through all good tracks and iteratively clusters their origins
    // This assumes that tracks separated 1mm will form a vertex.
    std::vector<std::vector<int> > ClusterIndices;
    std::vector<int> indexAux;

    if(trackProduction.size()==0) continue;
    for(unsigned int p1 =0;p1<trackProduction.size()-1;p1++){
      bool isorig=false;
      //track indices
      std::vector<int> trackidxs;
      trackidxs.push_back(p1);
      //Need not to associate the same track to other cluster
      //exitst will return the index where is p2
      bool exists = std::find(std::begin(indexAux), std::end(indexAux), p1) != std::end(indexAux);
      if(!exists){
        for(unsigned int p2=p1+1;p2<trackProduction.size();p2++ ){
          double distx = pow2(trackProduction[p1].px() - trackProduction[p2].px());
          double disty = pow2(trackProduction[p1].py() - trackProduction[p2].py());
          double distz = pow2(trackProduction[p1].pz() - trackProduction[p2].pz());  
          double rdist = sqrt(distx+disty+distz);  
          // Find tracks less than X mm apart to form DV. Based on vertex resolution information.
          // Worst resolution outside third pixel layer, so merge tracks further appart ?
          if(rdist<1){
            isorig=true;
            trackidxs.push_back(p2);
            indexAux.push_back(p2);
            //trackidxs.size() will be the number of tracks clustered in the vertex
          }
        }
      }
      if (isorig) {
        ClusterIndices.push_back(trackidxs);
      }
    }
    std::vector<double> xDVertex;
    std::vector<double> yDVertex;
    std::vector<double> zDVertex;
    //Calculate DV position from the average position of tracks in cluster
    for(unsigned int clus=0;clus<ClusterIndices.size();clus++){
      double xpos = 0.0;
      double ypos = 0.0;
      double zpos = 0.0;
      for(unsigned int clustrk=0;clustrk<ClusterIndices[clus].size();clustrk++) {
        int index = ClusterIndices[clus][clustrk];
        xpos += trackProduction[index].px();
        ypos += trackProduction[index].py();
        zpos += trackProduction[index].pz();
      }
      //Average position of DV
      double xDV = xpos/ClusterIndices[clus].size();   
      double yDV = ypos/ClusterIndices[clus].size();   
      double zDV = zpos/ClusterIndices[clus].size(); 
      xDVertex.push_back(xDV);
      yDVertex.push_back(yDV);
      zDVertex.push_back(zDV);
    }//close cluster (DV)  
    std::vector<int> clusDVIndices;
    //Merge DVs less than 1 mm apart
    //dv1 and dv2 are cluster indices    
    if(xDVertex.size()==0) continue;
    ///cout<<" Event "<<  iEvent << " has "<<xDVertex.size()<<" displaced vertices "<<endl;
    for(unsigned int dv1=0;dv1<xDVertex.size()-1;dv1++){
      for(unsigned int dv2=dv1+1;dv2<xDVertex.size();dv2++){
        double DVdistx=xDVertex[dv1]-xDVertex[dv2];
        double DVdisty=yDVertex[dv1]-yDVertex[dv2];
        double DVdistz=zDVertex[dv1]-zDVertex[dv2];
        double distDV=sqrt(pow2(DVdistx)+pow2(DVdisty)+pow2(DVdistz));
        if(distDV <= 1){
          cout<<" ----- Merging Vertices ----- "<<endl;
          clusDVIndices.push_back(dv2);
          xDVertex.erase(std::begin(xDVertex)+dv2);
          yDVertex.erase(std::begin(yDVertex)+dv2);
          zDVertex.erase(std::begin(zDVertex)+dv2);
          ClusterIndices.erase(std::begin(ClusterIndices)+dv2);
          break;
        }
      }
    }
    //Reconstructed vertex in the event
    passEvtDVReco = true;
    if(passEvtDVReco) nDVReco++;
      else continue;
    // Vertex cuts
    bool vtxPassFidutial=false;
    bool vtxPassesMaterial=false; 
    bool vtxPassesNtrk=false;
    bool vtxPassesMass=false;
    //save selected DV indices
    std::vector<std::vector<int> > DisplacedVertexIndices;
    for (unsigned int dv=0;dv<xDVertex.size();dv++){
      //cout<<"ClusterIndices[dv] = "<<ClusterIndices[dv]<<endl;
      //cout<<"DV has pdgID "<<event[ClusterIndices[dv]].id()<<endl;
      double rDisplacedVertex=sqrt(pow2(xDVertex[dv])+pow2(yDVertex[dv]));
      //cout<<" rDisplacedVertex = "<<rDisplacedVertex<<endl;
      double zDisplacedVertex=abs(zDVertex[dv]);
      //Vertex fidutial region 
      if(zDisplacedVertex>300.) continue;
      if(4.>rDisplacedVertex && rDisplacedVertex>300.) continue; 
        vtxPassFidutial=true;
      //Also, Vertex must also not be inside the pixel layers
      if(4.5<rDisplacedVertex && rDisplacedVertex<6.0) continue;
      if(8.9<rDisplacedVertex && rDisplacedVertex<9.5) continue;
      if(12.<rDisplacedVertex && rDisplacedVertex<13.) continue;
        vtxPassesMaterial=true;
      if(ClusterIndices[dv].size()<5) continue;
       vtxPassesNtrk=true;
      //DV invariant mass calculation
      Vec4 total4p;
      Vec4 total4pPion;
      double trum=0.0;
      double trumPion=0.0;
      int strack=0;
      //Find neutralino mother index of the track coming from the DV  
      std::vector<int> daughterIndices;
      std::vector<int> motherIndices;
      //int nMatchedTracks = 0;
      //Loop over tracks from the DV 
      for(unsigned trackIdx=0;trackIdx<ClusterIndices[dv].size();trackIdx++){
        strack++;
        double trackX=trackProduction[trackIdx].px();
        double trackY=trackProduction[trackIdx].py();
        double trackZ=trackProduction[trackIdx].pz();
        double trackpX=trackMomenta[trackIdx].px();
        double trackpY=trackMomenta[trackIdx].py();
        double trackpZ=trackMomenta[trackIdx].pz();
        double trackpMod=sqrt(pow2(trackpX)+pow2(trackpY)+pow2(trackpZ));
        double seedRq = trackX*trackpX/trackpMod+trackY*trackpY/trackpMod+trackZ*trackpZ/trackpMod;
        //At least 2 tracks in each DV with ssedReq
        if(strack<=2 && seedRq< -20.) continue;
        int evtRecordTrackIndex=trackIndices[ClusterIndices[dv][trackIdx]];
        //double rTrk=sqrt(pow2((event[evtRecordTrackIndex].vProd()).px())+ pow2((event[evtRecordTrackIndex].vProd()).py()));
        //double phixy = event[evtRecordTrackIndex].vProd().phi();
        //double deltaphi = phixy - event[evtRecordTrackIndex].phi();
        //if (abs(deltaphi) > 3.1415) deltaphi = 2 * 3.1415 - abs(deltaphi);
        //double d0 = rTrk*sin(deltaphi);    
        //cout<<"evtRecordTrackIndex is =  "<<evtRecordTrackIndex<<" has d0= "<<d0<<endl;
        //d0ofBs<<rDisplacedVertex<<" "<<d0<<endl;
        double TrackMass=event[evtRecordTrackIndex].mCalc();
        //This is scalar mass of tracks contained in the vertex
        trum+=TrackMass;
        //from four momenta
        double particlepx = event[evtRecordTrackIndex].px();
        double particlepy = event[evtRecordTrackIndex].py();
        double particlepz = event[evtRecordTrackIndex].pz();
        double particleE = event[evtRecordTrackIndex].e();
        double particleEPion = sqrt(pow2(0.1395700)+pow2(particlepx)+pow2(particlepy)+pow2(particlepz));
        double TrktrumDVPion=sqrt(abs(pow2(particleEPion)-(pow2(particlepx)+pow2(particlepy)+pow2(particlepz))));
        trumPion+=TrktrumDVPion;
        //Invariant mass of the vertex, also with pion mass hypothesis (analysis requirement)
        total4p+=Vec4(particlepx,particlepy,particlepz,particleE);
        total4pPion+=Vec4(particlepx,particlepy,particlepz,particleEPion);
      }
      double totalmPion=total4pPion.mCalc();
      //cout<<" totalmPion = "<<totalmPion<<endl;
      //mDV<< setprecision(9) <<fixed<<totalmPion<<" "<<ClusterIndices[dv].size()<<endl;
      //Cut on mDV 
      if(totalmPion<10.) continue;
       vtxPassesMass=true;
      //Selected DV indices  
      DisplacedVertexIndices.push_back(ClusterIndices[dv]);  
    }// close DV loop
    if(vtxPassFidutial) nFidutial++;
      else continue;
    if(vtxPassesMaterial) nMaterial++;
      else continue;
    if(vtxPassesNtrk) nTrk++;
     else continue;
    if(vtxPassesMass) nMass++;
     else continue;
  } //End of event loop.
  pythia.stat();
  double sigma = pythia.info.sigmaGen(); // in mb

  ////////////////////////
  // CutFlow default ATLAS
  ////////////////////////
  // Number of events - Relative Efficiency - Overall efficiency
  resultfile <<"{" << endl;
  resultfile <<"\"cutflow\": {" << endl;
  resultfile <<"\"All events\":["          << nEvent    << "," << nEvent*100./nEvent       << "," << nEvent*100./nEvent    << "]," << endl;
  resultfile <<"\"Jet Cuts\":["            << nJetCuts  << "," << nJetCuts*100./nEvent     << "," << nJetCuts*100./nEvent  << "]," << endl;
  resultfile <<"\"DV Reconstruction\":["   << nDVReco   << "," << nDVReco*100./nJetCuts    << "," << nDVReco*100./nEvent   << "]," << endl;
  resultfile <<"\"DV Fidutial\":["         << nFidutial << "," << nFidutial*100./nDVReco   << "," << nFidutial*100./nEvent << "]," << endl;
  resultfile <<"\"DV Material\":["         << nMaterial << "," << nMaterial*100./nFidutial << "," << nMaterial*100./nEvent << "]," << endl;
  resultfile <<"\"DV Ntrk\":["             << nTrk      << "," << nTrk*100./nMaterial      << "," << nTrk*100./nEvent      << "]," << endl;
  resultfile <<"\"DV Mass\":["             << nMass     << "," << nMass*100./nTrk          << "," << nMass*100./nEvent     << "]"  << endl;
  resultfile << "}," << endl;
 
  ////////////////////
  //Efficiency Vs Ctau
  ////////////////////
  resultfile << "\"lifetime\": " << pythia.particleData.tau0(1000022)<<", \"nTrk\": "<<setprecision(5)<< fixed << nTrk <<", \"eff\": "<<setprecision(5)<<nTrk*1.0/nEvent << "," << endl;
  resultfile << "\"xsec\": " << sigma * 1e12 << ", \"xsec unit\": " << "\"fb\"" << endl;
  resultfile << "}" << endl;

  //Final statistics and histogram output.
  ///////////////////////////////////////////////////////////////
  // Cross section (needs to be multiplied by overall efficiency)
  ///////////////////////////////////////////////////////////////
  cout << "####################################################################" << endl;
  cout << " Cross section after cuts is " << sigma * 1e12  << " [fb] "<<endl;
  cout << "####################################################################" << endl;

  resultfile.close();
  return 0;
}
