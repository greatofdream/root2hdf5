/*================================================================
*   Filename   : TransformSimTriggerInfoTree.C
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
#include "TChain.h"
#include <vector>
#include "H5PacketTable.h"
#include "TFile.h"
#include "JPSimOutput.hh"
#include "TransformTrack.h"

using namespace std;

void Convert_SimTriggerInfo_Tree(TChain* SimTriggerInfoTree, hid_t outputfile, hid_t dsp, vector<int> simtriggerinfo_chunksize, bool SaveTrack)
{
	// define truthlist data structure
	struct TruthList_t
	{
		int32_t runno;
		int32_t segmentid;
		int32_t vertexid;
		int32_t triggerno;
	};
	TruthList_t *TruthList= (TruthList_t*) malloc(sizeof(TruthList_t));
	vector<JPSimTruthTree_t> *TruthList_origin = nullptr;
	SimTriggerInfoTree->SetBranchStatus("*",0);
	for(auto activeBranch : {"RunId", "SegmentId", "VertexId"}) SimTriggerInfoTree->SetBranchStatus(TString("truthList.")+TString(activeBranch), 1);
	if(SaveTrack) SimTriggerInfoTree->SetBranchStatus("truthList.trackList", 1);
	SimTriggerInfoTree->SetBranchAddress("truthList",&TruthList_origin);
	// define pelist data structure
	struct PEList_t
	{
		int32_t runno;
		int32_t triggerno;
		JPSimPE_t PEInfo[];
	};
	size_t pelistsize = sizeof(PEList_t) + sizeof(JPSimPE_t);
	PEList_t *PEList= (PEList_t*) malloc(pelistsize);
	vector<JPSimPE_t>* PEList_origin = nullptr;

	// directly read root data into struct.
	SimTriggerInfoTree->SetBranchStatus("PEList.*",1);
	SimTriggerInfoTree->SetBranchStatus("RunNo",1);
	SimTriggerInfoTree->SetBranchStatus("TriggerNo",1);
	SimTriggerInfoTree->SetBranchAddress("RunNo",&PEList->runno);
	SimTriggerInfoTree->SetBranchAddress("TriggerNo",&PEList->triggerno);
	SimTriggerInfoTree->SetBranchAddress("PEList",&PEList_origin);

	// Create SimTriggerInfo Group
	hid_t SimTriggerInfoGroup = H5Gcreate1(outputfile, "/SimTriggerInfo",100);

	// Create truthlist Table
	hid_t truthlisttable = H5Tcreate (H5T_COMPOUND, sizeof(TruthList_t));
	H5Tinsert (truthlisttable, "RunNo", HOFFSET(TruthList_t, runno), H5T_NATIVE_INT32);
	H5Tinsert (truthlisttable, "SegmentId", HOFFSET(TruthList_t, segmentid), H5T_NATIVE_INT32);
	H5Tinsert (truthlisttable, "VertexId", HOFFSET(TruthList_t, vertexid), H5T_NATIVE_INT32);
	H5Tinsert (truthlisttable, "TriggerNo", HOFFSET(TruthList_t, triggerno), H5T_NATIVE_INT32);
	FL_PacketTable truthlist_d(SimTriggerInfoGroup, "TruthList", truthlisttable, simtriggerinfo_chunksize[0], dsp);
	if(! truthlist_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table TruthList.");
		abort();
	}

	// Create pelist Table
	hid_t pelisttable = H5Tcreate (H5T_COMPOUND, pelistsize);
	H5Tinsert (pelisttable, "RunNo", HOFFSET(PEList_t, runno), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "TriggerNo", HOFFSET(PEList_t, triggerno), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "PMTId", sizeof(PEList_t) + HOFFSET(JPSimPE_t, PMTId), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "SegmentId", sizeof(PEList_t) + HOFFSET(JPSimPE_t, segmentId), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "PrimaryParticleId", sizeof(PEList_t) + HOFFSET(JPSimPE_t, primaryParticleId), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "photonX", sizeof(PEList_t) + HOFFSET(JPSimPE_t, photonX), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "photonY", sizeof(PEList_t) + HOFFSET(JPSimPE_t, photonY), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "photonZ", sizeof(PEList_t) + HOFFSET(JPSimPE_t, photonZ), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "dETime", sizeof(PEList_t) + HOFFSET(JPSimPE_t, dETime), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "photonTime", sizeof(PEList_t) + HOFFSET(JPSimPE_t, photonTime), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "HitTime", sizeof(PEList_t) + HOFFSET(JPSimPE_t, HitTime), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "PulseTime", sizeof(PEList_t) + HOFFSET(JPSimPE_t, PulseTime), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "PESec", sizeof(PEList_t) + HOFFSET(JPSimPE_t, PESec), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "PENanoSec", sizeof(PEList_t) + HOFFSET(JPSimPE_t, PENanoSec), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "PESubNanoSec", sizeof(PEList_t) + HOFFSET(JPSimPE_t, PESubNanoSec), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "HitPosInWindow", sizeof(PEList_t) + HOFFSET(JPSimPE_t, HitPosInWindow), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "Wavelength", sizeof(PEList_t) + HOFFSET(JPSimPE_t, Wavelength), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "PEType", sizeof(PEList_t) + HOFFSET(JPSimPE_t, PEType), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "Charge", sizeof(PEList_t) + HOFFSET(JPSimPE_t, Charge), H5T_NATIVE_DOUBLE);
	FL_PacketTable pelist_d(SimTriggerInfoGroup, "PEList", pelisttable, simtriggerinfo_chunksize[1], dsp);
	if(! pelist_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table PEList.");
		abort();
	}

	TrackTransformer *tracktransformer=nullptr;
	if(SaveTrack) tracktransformer = new TrackTransformer(SimTriggerInfoGroup, &simtriggerinfo_chunksize[2], dsp);

	// converting loop
	char outputfilename[100];
	H5Fget_name(outputfile,outputfilename,100);
	// cout<<"Converting SimTriggerInfo Tree of "<<SimTriggerInfoTree->GetCurrentFile()->GetName()<<" to "<<outputfilename<<endl;
	cout<<"Converting SimTriggerInfo Tree of "<<SimTriggerInfoTree->GetFile()->GetName()<<" to "<<outputfilename<<endl;

	uint64_t nevt= SimTriggerInfoTree->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;

	for (unsigned int ievt=0; ievt<nevt; ievt++) {
		SimTriggerInfoTree->GetEntry(ievt);
		TruthList->triggerno = PEList->triggerno;
		for(auto truth : *TruthList_origin) 
		{
			memcpy(TruthList, &truth, HOFFSET(TruthList_t, triggerno));
			truthlist_d.AppendPacket( TruthList );
			if(SaveTrack) tracktransformer->LoopTrack(&truth.trackList, truth.RunId, truth.SegmentId, truth.VertexId);
		}
		for(auto pe : *PEList_origin) 
		{
			memcpy(PEList->PEInfo, &pe, sizeof(JPSimPE_t));
			pelist_d.AppendPacket( PEList );
		}

		if (ievt==0) cout<<"start processing ..."<<endl;
		else if (ievt%1000==0) cout<<ievt<<" events converted"<<endl;
	}
	free(PEList);
	free(TruthList);
	delete tracktransformer;
}
void Convert_SimTriggerInfo_Tree(TTree* SimTriggerInfoTree, hid_t outputfile, hid_t dsp, vector<int> simtriggerinfo_chunksize, bool SaveTrack)
{
	// define truthlist data structure
	struct TruthList_t
	{
		int32_t runno;
		int32_t segmentid;
		int32_t vertexid;
		int32_t triggerno;
	};
	TruthList_t *TruthList= (TruthList_t*) malloc(sizeof(TruthList_t));
	vector<JPSimTruthTree_t> *TruthList_origin = nullptr;
	SimTriggerInfoTree->SetBranchStatus("*",0);
	for(auto activeBranch : {"RunId", "SegmentId", "VertexId"}) SimTriggerInfoTree->SetBranchStatus(TString("truthList.")+TString(activeBranch), 1);
	if(SaveTrack) SimTriggerInfoTree->SetBranchStatus("truthList.trackList", 1);
	SimTriggerInfoTree->SetBranchAddress("truthList",&TruthList_origin);
	// define pelist data structure
	struct PEList_t
	{
		int32_t runno;
		int32_t triggerno;
		JPSimPE_t PEInfo[];
	};
	size_t pelistsize = sizeof(PEList_t) + sizeof(JPSimPE_t);
	PEList_t *PEList= (PEList_t*) malloc(pelistsize);
	vector<JPSimPE_t>* PEList_origin = nullptr;

	// directly read root data into struct.
	SimTriggerInfoTree->SetBranchStatus("PEList.*",1);
	SimTriggerInfoTree->SetBranchStatus("RunNo",1);
	SimTriggerInfoTree->SetBranchStatus("TriggerNo",1);
	SimTriggerInfoTree->SetBranchAddress("RunNo",&PEList->runno);
	SimTriggerInfoTree->SetBranchAddress("TriggerNo",&PEList->triggerno);
	SimTriggerInfoTree->SetBranchAddress("PEList",&PEList_origin);

	// Create SimTriggerInfo Group
	hid_t SimTriggerInfoGroup = H5Gcreate1(outputfile, "/SimTriggerInfo",100);

	// Create truthlist Table
	hid_t truthlisttable = H5Tcreate (H5T_COMPOUND, sizeof(TruthList_t));
	H5Tinsert (truthlisttable, "RunNo", HOFFSET(TruthList_t, runno), H5T_NATIVE_INT32);
	H5Tinsert (truthlisttable, "SegmentId", HOFFSET(TruthList_t, segmentid), H5T_NATIVE_INT32);
	H5Tinsert (truthlisttable, "VertexId", HOFFSET(TruthList_t, vertexid), H5T_NATIVE_INT32);
	H5Tinsert (truthlisttable, "TriggerNo", HOFFSET(TruthList_t, triggerno), H5T_NATIVE_INT32);
	FL_PacketTable truthlist_d(SimTriggerInfoGroup, "TruthList", truthlisttable, simtriggerinfo_chunksize[0], dsp);
	if(! truthlist_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table TruthList.");
		abort();
	}

	// Create pelist Table
	hid_t pelisttable = H5Tcreate (H5T_COMPOUND, pelistsize);
	H5Tinsert (pelisttable, "RunNo", HOFFSET(PEList_t, runno), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "TriggerNo", HOFFSET(PEList_t, triggerno), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "PMTId", sizeof(PEList_t) + HOFFSET(JPSimPE_t, PMTId), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "SegmentId", sizeof(PEList_t) + HOFFSET(JPSimPE_t, segmentId), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "PrimaryParticleId", sizeof(PEList_t) + HOFFSET(JPSimPE_t, primaryParticleId), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "photonX", sizeof(PEList_t) + HOFFSET(JPSimPE_t, photonX), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "photonY", sizeof(PEList_t) + HOFFSET(JPSimPE_t, photonY), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "photonZ", sizeof(PEList_t) + HOFFSET(JPSimPE_t, photonZ), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "dETime", sizeof(PEList_t) + HOFFSET(JPSimPE_t, dETime), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "photonTime", sizeof(PEList_t) + HOFFSET(JPSimPE_t, photonTime), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "HitTime", sizeof(PEList_t) + HOFFSET(JPSimPE_t, HitTime), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "PulseTime", sizeof(PEList_t) + HOFFSET(JPSimPE_t, PulseTime), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "PESec", sizeof(PEList_t) + HOFFSET(JPSimPE_t, PESec), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "PENanoSec", sizeof(PEList_t) + HOFFSET(JPSimPE_t, PENanoSec), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "PESubNanoSec", sizeof(PEList_t) + HOFFSET(JPSimPE_t, PESubNanoSec), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "HitPosInWindow", sizeof(PEList_t) + HOFFSET(JPSimPE_t, HitPosInWindow), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "Wavelength", sizeof(PEList_t) + HOFFSET(JPSimPE_t, Wavelength), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "PEType", sizeof(PEList_t) + HOFFSET(JPSimPE_t, PEType), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "Charge", sizeof(PEList_t) + HOFFSET(JPSimPE_t, Charge), H5T_NATIVE_DOUBLE);
	FL_PacketTable pelist_d(SimTriggerInfoGroup, "PEList", pelisttable, simtriggerinfo_chunksize[1], dsp);
	if(! pelist_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table PEList.");
		abort();
	}

	TrackTransformer *tracktransformer=nullptr;
	if(SaveTrack) tracktransformer = new TrackTransformer(SimTriggerInfoGroup, &simtriggerinfo_chunksize[2], dsp);

	// converting loop
	char outputfilename[100];
	H5Fget_name(outputfile,outputfilename,100);
	cout<<"Converting SimTriggerInfo Tree of "<<SimTriggerInfoTree->GetCurrentFile()->GetName()<<" to "<<outputfilename<<endl;
	//cout<<"Converting SimTriggerInfo Tree of "<<SimTriggerInfoTree->GetFile()->GetName()<<" to "<<outputfilename<<endl;

	uint64_t nevt= SimTriggerInfoTree->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;

	for (unsigned int ievt=0; ievt<nevt; ievt++) {
		SimTriggerInfoTree->GetEntry(ievt);
		TruthList->triggerno = PEList->triggerno;
		for(auto truth : *TruthList_origin) 
		{
			memcpy(TruthList, &truth, HOFFSET(TruthList_t, triggerno));
			truthlist_d.AppendPacket( TruthList );
			if(SaveTrack) tracktransformer->LoopTrack(&truth.trackList, truth.RunId, truth.SegmentId, truth.VertexId);
		}
		for(auto pe : *PEList_origin) 
		{
			memcpy(PEList->PEInfo, &pe, sizeof(JPSimPE_t));
			pelist_d.AppendPacket( PEList );
		}

		if (ievt==0) cout<<"start processing ..."<<endl;
		else if (ievt%1000==0) cout<<ievt<<" events converted"<<endl;
	}
	free(PEList);
	free(TruthList);
	delete tracktransformer;
}
