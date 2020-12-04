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
#include <vector>
#include "H5PacketTable.h"
#include "TFile.h"
#include "JPSimOutput.hh"
#include "TransformTrack.h"

using namespace std;

void Convert_SimTriggerInfo_Tree(TTree* SimTriggerInfoTree, hid_t outputfile, hid_t dsp, vector<int> simtriggerinfo_chunksize)
{
	SimTriggerInfoTree->SetBranchStatus("*",0);
	for(auto activeBranch : {"RunId", "SegmentId", "VertexId"}) SimTriggerInfoTree->SetBranchStatus(TString("truthList.")+TString(activeBranch), 1);
	// define pelist data structure
	struct PEList_t
	{
		int32_t triggerno;
		JPSimPE_t PEInfo[];
	};
	size_t pelistsize = sizeof(PEList_t) + sizeof(JPSimPE_t);
	PEList_t *PEList= (PEList_t*) malloc(pelistsize);
	vector<JPSimPE_t>* PEList_origin = nullptr;

	// directly read root data into struct.
	for(auto activeBranch : {"PEList","PEList.PMTId","PEList.HitPosInWindow","PEList.Charge","TriggerNo"})
		SimTriggerInfoTree->SetBranchStatus(activeBranch,1);
	SimTriggerInfoTree->SetBranchAddress("PEList",&PEList_origin);
	SimTriggerInfoTree->SetBranchAddress("TriggerNo",&PEList->triggerno);

	// Create SimTriggerInfo Group
	hid_t SimTriggerInfoGroup = H5Gcreate1(outputfile, "/SimTriggerInfo",100);

	// Create pelist Table
	hid_t pelisttable = H5Tcreate (H5T_COMPOUND, pelistsize);
	H5Tinsert (pelisttable, "EventID", HOFFSET(PEList_t, triggerno), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "ChannelID", sizeof(PEList_t) + HOFFSET(JPSimPE_t, PMTId), H5T_NATIVE_INT32);
	H5Tinsert (pelisttable, "RiseTime", sizeof(PEList_t) + HOFFSET(JPSimPE_t, HitPosInWindow), H5T_NATIVE_DOUBLE);
	H5Tinsert (pelisttable, "Charge", sizeof(PEList_t) + HOFFSET(JPSimPE_t, Charge), H5T_NATIVE_DOUBLE);
	FL_PacketTable pelist_d(SimTriggerInfoGroup, "PEList", pelisttable, simtriggerinfo_chunksize[1], dsp);
	if(! pelist_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table PEList.");
		abort();
	}

	// converting loop
	char outputfilename[100];
	H5Fget_name(outputfile,outputfilename,100);
	cout<<"Converting SimTriggerInfo Tree of "<<SimTriggerInfoTree->GetCurrentFile()->GetName()<<" to "<<outputfilename<<endl;
	uint64_t nevt= SimTriggerInfoTree->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;

	for (unsigned int ievt=0; ievt<nevt; ievt++) {
		SimTriggerInfoTree->GetEntry(ievt);
		for(auto pe : *PEList_origin) 
		{
			memcpy(PEList->PEInfo, &pe, sizeof(JPSimPE_t));
			pelist_d.AppendPacket( PEList );
		}

		if (ievt==0) cout<<"start processing ..."<<endl;
		else if (ievt%1000==0) cout<<ievt<<" events converted"<<endl;
	}
	free(PEList);
}
