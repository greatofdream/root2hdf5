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
#include "TransformTrack.h"

using namespace std;

void Convert_SimTruth_Tree(TTree* SimTruthTree, hid_t outputfile, hid_t dsp, vector<int> simtruth_chunksize)
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
	FL_PacketTable truthlist_d(SimTruthGroup, "SimTruth", truthlisttable, simtruth_chunksize[0], dsp);
	if(! truthlist_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table SimTruth.");
		abort();
	}

	// Create Primary Partiicle table.
	vector<JPSimPrimaryParticle_t> *PrimaryParticleList = nullptr;
	SimTruthTree->SetBranchAddress("PrimaryParticleList",&PrimaryParticleList);
	hid_t primaryparticletable = H5Tcreate (H5T_COMPOUND, sizeof(JPSimPrimaryParticle_t));
	H5Tinsert (primaryparticletable, "TrackId", HOFFSET(JPSimPrimaryParticle_t, TrackId), H5T_NATIVE_INT32); 
	H5Tinsert (primaryparticletable, "PdgId", HOFFSET(JPSimPrimaryParticle_t, PdgId), H5T_NATIVE_INT32); 
	H5Tinsert (primaryparticletable, "px", HOFFSET(JPSimPrimaryParticle_t, px), H5T_NATIVE_DOUBLE); 
	H5Tinsert (primaryparticletable, "py", HOFFSET(JPSimPrimaryParticle_t, py), H5T_NATIVE_DOUBLE); 
	H5Tinsert (primaryparticletable, "pz", HOFFSET(JPSimPrimaryParticle_t, pz), H5T_NATIVE_DOUBLE); 
	H5Tinsert (primaryparticletable, "Ek", HOFFSET(JPSimPrimaryParticle_t, Ek), H5T_NATIVE_DOUBLE); 
	FL_PacketTable PrimaryParticle_d(SimTruthGroup, "PrimaryParticle", primaryparticletable, simtruth_chunksize[1], dsp);
	if(! PrimaryParticle_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table DepositEnergy.");
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
	FL_PacketTable dElist_d(SimTruthGroup, "DepositEnergy", dEtable, simtruth_chunksize[2], dsp);
	if(! dElist_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table DepositEnergy.");
		abort();
	}

	vector<JPSimTrack_t> *Track_origin = nullptr;
	SimTruthTree->SetBranchAddress("trackList",&Track_origin);
	TrackTransformer tracktransformer(outputfile, &simtruth_chunksize[3], dsp);

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
		DepositEnergy->RunId = SimTruth->RunId;
		DepositEnergy->SegmentId = SimTruth->SegmentId;
		DepositEnergy->VertexId = SimTruth->VertexId;
		for(auto primaryparticle : *PrimaryParticleList) 
			PrimaryParticle_d.AppendPacket( &primaryparticle );
		for(auto dE : *dEList) 
		{
			DepositEnergy->DepositEnergy = dE;
			dElist_d.AppendPacket( DepositEnergy );
		}
		tracktransformer.LoopTrack(Track_origin, SimTruth->RunId, SimTruth->SegmentId, SimTruth->VertexId);

		if (ievt==0) cout<<"start processing ..."<<endl;
		else if (ievt%1000==0) cout<<ievt<<" events converted"<<endl;
	}
	free(SimTruth);
	free(DepositEnergy);
	free(PrimaryParticleList);
}
