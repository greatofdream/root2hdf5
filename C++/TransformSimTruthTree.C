/*================================================================
*   Filename   : TransformSimTruthTree.C
*   Author     : Yiyang Wu
*   Created on  Thu 29 Oct 2020 06:29:40 PM CST
*   Description:
*
================================================================*/


#include <H5Tpublic.h>
#include <bits/stdint-intn.h>
#include <cstdint>
#include <iostream>
#include "TTree.h"
#include <vector>
#include "H5PacketTable.h"
#include "TFile.h"
#include "JPSimOutput.hh"

using namespace std;

void Convert_SimTruth_Tree(TTree* SimTruthTree, hid_t outputfile, hid_t dsp, int chunksize)
{
	// Create SimTruth Group
	hid_t SimTruthGroup = H5Gcreate1(outputfile, "/SimTruth",100);

	// define truthlist data structure
	struct SimTruth_t 
	{
		int32_t RunId;		// The ID of Run
		int32_t SegmentId;	// The ID of Segment
		int32_t VertexId;
		int32_t VertexRadZ;
		int32_t VertexRadA;
		// int32_t nParticle;
		double x;
		double y;
		double z;
		int32_t Sec;
		int32_t NanoSec;
		double EkMerged;
		int32_t nFiredPMT;
		int32_t CPh;			// The number of C photons
		int32_t SPh;			// The number of S photons
		int32_t APh;			// The number of all photons
		int32_t CPE;			// The number of C p.e.
		int32_t SPE;
		int32_t APE;
	};
	SimTruth_t *SimTruth = (SimTruth_t*) malloc(sizeof(SimTruth_t));
	SimTruthTree->SetBranchAddress("RunId",&SimTruth->RunId);
	SimTruthTree->SetBranchAddress("SegmentId",&SimTruth->SegmentId);
	SimTruthTree->SetBranchAddress("VertexId",&SimTruth->VertexId);
	SimTruthTree->SetBranchAddress("VertexRadZ",&SimTruth->VertexRadZ);
	SimTruthTree->SetBranchAddress("VertexRadA",&SimTruth->VertexRadA);
	// SimTruthTree->SetBranchAddress("nParticle",&SimTruth->nParticle);
	SimTruthTree->SetBranchAddress("x",&SimTruth->x);
	SimTruthTree->SetBranchAddress("y",&SimTruth->y);
	SimTruthTree->SetBranchAddress("z",&SimTruth->z);
	SimTruthTree->SetBranchAddress("Sec",&SimTruth->Sec);
	SimTruthTree->SetBranchAddress("NanoSec",&SimTruth->NanoSec);
	SimTruthTree->SetBranchAddress("EkMerged",&SimTruth->EkMerged);
	SimTruthTree->SetBranchAddress("nFiredPMT",&SimTruth->nFiredPMT);
	SimTruthTree->SetBranchAddress("CPh",&SimTruth->CPh);
	SimTruthTree->SetBranchAddress("SPh",&SimTruth->SPh);
	SimTruthTree->SetBranchAddress("APh",&SimTruth->APh);
	SimTruthTree->SetBranchAddress("CPE",&SimTruth->CPE);
	SimTruthTree->SetBranchAddress("SPE",&SimTruth->SPE);
	SimTruthTree->SetBranchAddress("APE",&SimTruth->APE);

	// Create truthlist Table
	hid_t truthlisttable = H5Tcreate (H5T_COMPOUND, sizeof(SimTruth_t));
	H5Tinsert (truthlisttable, "RunId", HOFFSET(SimTruth_t, RunId), H5T_NATIVE_INT32); 
	H5Tinsert (truthlisttable, "SegmentId", HOFFSET(SimTruth_t, SegmentId), H5T_NATIVE_INT32); 
	H5Tinsert (truthlisttable, "VertexId", HOFFSET(SimTruth_t, VertexId), H5T_NATIVE_INT32); 
	H5Tinsert (truthlisttable, "VertexRadZ", HOFFSET(SimTruth_t, VertexRadZ), H5T_NATIVE_INT32); 
	H5Tinsert (truthlisttable, "VertexRadA", HOFFSET(SimTruth_t, VertexRadA), H5T_NATIVE_INT32); 
	// H5Tinsert (truthlisttable, "nParticle", HOFFSET(SimTruth_t, nParticle), H5T_NATIVE_INT32); 
	H5Tinsert (truthlisttable, "x", HOFFSET(SimTruth_t, x), H5T_NATIVE_DOUBLE); 
	H5Tinsert (truthlisttable, "y", HOFFSET(SimTruth_t, y), H5T_NATIVE_DOUBLE); 
	H5Tinsert (truthlisttable, "z", HOFFSET(SimTruth_t, z), H5T_NATIVE_DOUBLE); 
	H5Tinsert (truthlisttable, "Sec", HOFFSET(SimTruth_t, Sec), H5T_NATIVE_INT32); 
	H5Tinsert (truthlisttable, "NanoSec", HOFFSET(SimTruth_t, NanoSec), H5T_NATIVE_INT32); 
	H5Tinsert (truthlisttable, "EkMerged", HOFFSET(SimTruth_t, EkMerged), H5T_NATIVE_DOUBLE); 
	H5Tinsert (truthlisttable, "nFiredPMT", HOFFSET(SimTruth_t, nFiredPMT), H5T_NATIVE_INT32); 
	H5Tinsert (truthlisttable, "CPh", HOFFSET(SimTruth_t, CPh), H5T_NATIVE_INT32); 
	H5Tinsert (truthlisttable, "SPh", HOFFSET(SimTruth_t, SPh), H5T_NATIVE_INT32); 
	H5Tinsert (truthlisttable, "APh", HOFFSET(SimTruth_t, APh), H5T_NATIVE_INT32); 
	H5Tinsert (truthlisttable, "CPE", HOFFSET(SimTruth_t, CPE), H5T_NATIVE_INT32); 
	H5Tinsert (truthlisttable, "SPE", HOFFSET(SimTruth_t, SPE), H5T_NATIVE_INT32); 
	H5Tinsert (truthlisttable, "APE", HOFFSET(SimTruth_t, APE), H5T_NATIVE_INT32); 
	FL_PacketTable truthlist_d(SimTruthGroup, "SimTruth", truthlisttable, chunksize, dsp);
	if(! truthlist_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table SimTruth.");
		abort();
	}

	// Create Deposit Energy table.
	struct DepositEnergy_t 
	{
		int32_t RunId;		// The ID of Run
		int32_t SegmentId;	// The ID of Segment
		int32_t VertexId;
		double DepositEnergy;
	};
	DepositEnergy_t *DepositEnergy = (DepositEnergy_t*) malloc(sizeof(DepositEnergy_t));
	vector<double> *dEList = nullptr;
	SimTruthTree->SetBranchAddress("dEList",&dEList);
	hid_t dEtable = H5Tcreate (H5T_COMPOUND, sizeof(DepositEnergy_t));
	H5Tinsert (dEtable, "RunId", HOFFSET(DepositEnergy_t, RunId), H5T_NATIVE_INT32); 
	H5Tinsert (dEtable, "SegmentId", HOFFSET(DepositEnergy_t, SegmentId), H5T_NATIVE_INT32); 
	H5Tinsert (dEtable, "VertexId", HOFFSET(DepositEnergy_t, VertexId), H5T_NATIVE_INT32); 
	H5Tinsert (dEtable, "DepositEnergy", HOFFSET(DepositEnergy_t, DepositEnergy), H5T_NATIVE_DOUBLE); 
	FL_PacketTable dElist_d(SimTruthGroup, "DepositEnergy", dEtable, chunksize, dsp);
	if(! dElist_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table DepositEnergy.");
		abort();
	}

	// Create Track Table
	struct TrackHeader_t
	{
		int32_t RunId;
		int32_t VertexId;
		Int_t nSegmentId;
		Int_t nParentTrackId;
		Int_t nTrackId;
		Int_t nPrimaryId;
		Int_t nPdgId;
		Bool_t bDetectedPhoton;
	};
	vector<JPSimTrack_t> *Track_origin = nullptr;
	SimTruthTree->SetBranchAddress("trackList",&Track_origin);
	// size_t tracksize = sizeof(TrackHeader_t) + sizeof(JPSimTrack_t)
	// size_t tracksize = sizeof(TrackHeader_t) + HOFFSET(JPSimTrack_t, StepPoints);
	size_t tracksize = sizeof(TrackHeader_t);
	TrackHeader_t *Track = (TrackHeader_t*) malloc(tracksize);
	hid_t tracktable = H5Tcreate (H5T_COMPOUND, tracksize);
	H5Tinsert (tracktable, "RunId", HOFFSET(TrackHeader_t, RunId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "VertexId", HOFFSET(TrackHeader_t, VertexId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "nSegmentId", HOFFSET(TrackHeader_t, nSegmentId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "nParentTrackId", HOFFSET(TrackHeader_t, nParentTrackId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "nTrackId", HOFFSET(TrackHeader_t, nTrackId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "nPrimaryId", HOFFSET(TrackHeader_t, nPrimaryId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "nPdgId", HOFFSET(TrackHeader_t, nPdgId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "bDetectedPhoton", HOFFSET(TrackHeader_t, bDetectedPhoton), H5T_NATIVE_HBOOL);
	FL_PacketTable track_d(SimTruthGroup, "TrackList", tracktable, chunksize, dsp);
	if(! track_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table TrackList.");
		abort();
	}

	// Create StepPoint Table
	struct StepPointHeader_t 
	{
		int32_t RunId;		// The ID of Run
		int32_t SegmentId;	// The ID of Segment
		int32_t VertexId;	// The ID of Segment
		int32_t TrackId;	// The ID of Segment
	};
	size_t steppointsize = sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, nSecondaryPdgId);
	StepPointHeader_t* StepPointHeader = (StepPointHeader_t*) malloc(steppointsize);
	hid_t steppointtable = H5Tcreate (H5T_COMPOUND, steppointsize);
	H5Tinsert (steppointtable, "RunId", HOFFSET(StepPointHeader_t, RunId), H5T_NATIVE_INT32);
	H5Tinsert (steppointtable, "SegmentId", HOFFSET(StepPointHeader_t, SegmentId), H5T_NATIVE_INT32);
	H5Tinsert (steppointtable, "VertexId", HOFFSET(StepPointHeader_t, VertexId), H5T_NATIVE_INT32);
	H5Tinsert (steppointtable, "TrackId", HOFFSET(StepPointHeader_t, TrackId), H5T_NATIVE_INT32);
	H5Tinsert (steppointtable, "nProcessType", sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, nProcessType), H5T_NATIVE_INT32); 
	H5Tinsert (steppointtable, "nProcessSubType", sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, nProcessSubType), H5T_NATIVE_INT32); 
	H5Tinsert (steppointtable, "fX", sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, fX), H5T_NATIVE_DOUBLE); 
	H5Tinsert (steppointtable, "fY", sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, fY), H5T_NATIVE_DOUBLE); 
	H5Tinsert (steppointtable, "fZ", sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, fZ), H5T_NATIVE_DOUBLE); 
	H5Tinsert (steppointtable, "fPx", sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, fPx), H5T_NATIVE_DOUBLE); 
	H5Tinsert (steppointtable, "fPy", sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, fPy), H5T_NATIVE_DOUBLE); 
	H5Tinsert (steppointtable, "fPz", sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, fPz), H5T_NATIVE_DOUBLE); 
	H5Tinsert (steppointtable, "fEk", sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, fEk), H5T_NATIVE_DOUBLE); 
	H5Tinsert (steppointtable, "fdE", sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, fdE), H5T_NATIVE_DOUBLE); 
	H5Tinsert (steppointtable, "fTime", sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, fTime), H5T_NATIVE_DOUBLE); 
	H5Tinsert (steppointtable, "nTargetZ", sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, nTargetZ), H5T_NATIVE_INT32); 
	H5Tinsert (steppointtable, "nTargetA", sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, nTargetA), H5T_NATIVE_INT32); 
	FL_PacketTable steppoint_d(SimTruthGroup, "StepPoint", tracktable, chunksize, dsp);
	if(! steppoint_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table StepPoint.");
		abort();
	}

	// Create SecondaryParticle Table
	struct SecondaryParticle_t
	{
		int32_t RunId;		// The ID of Run
		int32_t SegmentId;	// The ID of Segment
		int32_t VertexId;
		int32_t TrackId;
		int32_t StepId;
		int32_t nSecondaryPdgId;
	};
	SecondaryParticle_t* secondaryparticle = (SecondaryParticle_t*) malloc(sizeof(SecondaryParticle_t));
	hid_t secondaryparticletable = H5Tcreate (H5T_COMPOUND, sizeof(SecondaryParticle_t));
	H5Tinsert (secondaryparticletable, "RunId", HOFFSET(SecondaryParticle_t, RunId), H5T_NATIVE_INT32);
	H5Tinsert (secondaryparticletable, "SegmentId", HOFFSET(SecondaryParticle_t, SegmentId), H5T_NATIVE_INT32);
	H5Tinsert (secondaryparticletable, "VertexId", HOFFSET(SecondaryParticle_t, VertexId), H5T_NATIVE_INT32);
	H5Tinsert (secondaryparticletable, "TrackId", HOFFSET(SecondaryParticle_t, TrackId), H5T_NATIVE_INT32);
	H5Tinsert (secondaryparticletable, "StepId", HOFFSET(SecondaryParticle_t, StepId), H5T_NATIVE_INT32);
	H5Tinsert (secondaryparticletable, "nSecondaryPdgId;", HOFFSET(SecondaryParticle_t, nSecondaryPdgId), H5T_NATIVE_INT32);
	FL_PacketTable secondaryparticle_d(SimTruthGroup, "SecondaryParticle", tracktable, chunksize, dsp);
	if(! secondaryparticle_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table SecondaryParticle.");
		abort();
	}

	// converting loop
	char outputfilename[100];
	H5Fget_name(outputfile,outputfilename,100);
	cout<<"Converting SimTruth Tree of "<<SimTruthTree->GetCurrentFile()->GetName()<<" to "<<outputfilename<<endl;
	uint64_t nevt= SimTruthTree->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;

	for (unsigned int ievt=0; ievt<nevt; ievt++) {
		SimTruthTree->GetEntry(ievt);
		// cout<<Track_origin->size()<<endl;
		truthlist_d.AppendPacket( SimTruth );
		secondaryparticle->RunId = StepPointHeader->RunId = Track->RunId = DepositEnergy->RunId = SimTruth->RunId;
		secondaryparticle->SegmentId = StepPointHeader->SegmentId = DepositEnergy->SegmentId = SimTruth->SegmentId;
		secondaryparticle->VertexId = StepPointHeader->VertexId = Track->VertexId = DepositEnergy->VertexId = SimTruth->VertexId;
		for(auto dE : *dEList) 
		{
			DepositEnergy->DepositEnergy = dE;
			dElist_d.AppendPacket( DepositEnergy );
		}
		for(auto track : *Track_origin) 
		{
			// *(int32_t*)(Track + sizeof(TrackHeader_t)) = 20;
			// memcpy(Track+sizeof(TrackHeader_t), &track, HOFFSET(JPSimTrack_t, StepPoints));
			// memcpy(&Track->nSegmentId, &track, HOFFSET(JPSimTrack_t, StepPoints));
			Track->nSegmentId = track.nSegmentId;
			Track->nParentTrackId = track.nParentTrackId;
			Track->nTrackId = track.nTrackId;
			Track->nPrimaryId = track.nPrimaryId;
			Track->nPdgId = track.nPdgId;
			Track->bDetectedPhoton = track.bDetectedPhoton;
			track_d.AppendPacket( Track );
			for(auto step : track.StepPoints ) 
			{
				// memcpy(StepPointHeader+sizeof(StepPointHeader_t), &step, HOFFSET(JPSimStepPoint_t, nSecondaryPdgId));
				steppoint_d.AppendPacket( StepPointHeader );
				for(auto secondarypdgid : step.nSecondaryPdgId ) 
				{
					secondaryparticle->nSecondaryPdgId = secondarypdgid;
					secondaryparticle_d.AppendPacket( secondaryparticle );
				}
			}
		}

		if (ievt==0) cout<<"start processing ..."<<endl;
		else if (ievt%1000==0) cout<<ievt<<" events converted"<<endl;
	}
	free(SimTruth);
	free(DepositEnergy);
	free(Track);
	free(StepPointHeader);
	free(secondaryparticle);
}
