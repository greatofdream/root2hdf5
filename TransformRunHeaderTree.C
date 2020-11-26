/*================================================================
*   Filename   : TransformRunHeaderTree.C
*   Author     : Yiyang Wu
*   Created on  Thu 29 Oct 2020 06:29:40 PM CST
*   Description:
*
================================================================*/


#include <H5Tpublic.h>
#include <iostream>
#include "TTree.h"
#include <vector>
#include "H5PacketTable.h"
#include "TFile.h"
#include "JPSimOutput.hh"

using namespace std;

void Convert_RunHeader_Tree(TTree* RunHeaderTree, hid_t outputfile, hid_t dsp, int chunksize)
{
	// runheader data structure
	JPSimRunHeader_t* RunHeader= (JPSimRunHeader_t*) malloc(sizeof(JPSimRunHeader_t));

	// directly read root data into struct.
	RunHeaderTree->SetBranchAddress("RunId",&RunHeader->RunId);
	RunHeaderTree->SetBranchAddress("WindowSize",&RunHeader->WindowSize);
	RunHeaderTree->SetBranchAddress("TriggerPosition",&RunHeader->TriggerPosition);
	RunHeaderTree->SetBranchAddress("PhotonFactor",&RunHeader->PhotonFactor);
	RunHeaderTree->SetBranchAddress("DynamicRange",&RunHeader->DynamicRange);
	RunHeaderTree->SetBranchAddress("Bit",&RunHeader->Bit);
	RunHeaderTree->SetBranchAddress("DCOffset",&RunHeader->DCOffset);

	// Create output Table
	hid_t runheadertable = H5Tcreate (H5T_COMPOUND, sizeof(JPSimRunHeader_t));
	H5Tinsert(runheadertable, "RunId", HOFFSET(JPSimRunHeader_t, RunId), H5T_NATIVE_INT32);
	H5Tinsert(runheadertable, "WindowSize", HOFFSET(JPSimRunHeader_t, WindowSize), H5T_NATIVE_INT32);
	H5Tinsert(runheadertable, "TriggerPosition", HOFFSET(JPSimRunHeader_t, TriggerPosition), H5T_NATIVE_INT32);
	H5Tinsert(runheadertable, "PhotonFactor", HOFFSET(JPSimRunHeader_t, PhotonFactor), H5T_NATIVE_DOUBLE);
	H5Tinsert(runheadertable, "DynamicRange", HOFFSET(JPSimRunHeader_t, DynamicRange), H5T_NATIVE_DOUBLE);
	H5Tinsert(runheadertable, "Bit", HOFFSET(JPSimRunHeader_t, Bit), H5T_NATIVE_INT32);
	H5Tinsert(runheadertable, "DCOffset", HOFFSET(JPSimRunHeader_t, DCOffset), H5T_NATIVE_DOUBLE);


	FL_PacketTable runheader_d(outputfile, "/RunHeader", runheadertable, chunksize, dsp);
	if(! runheader_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table RunHeader.");
		abort();
	}

	// converting loop
	char outputfilename[100];
	H5Fget_name(outputfile,outputfilename,100);
	cout<<"Converting RunHeader Tree of "<<RunHeaderTree->GetCurrentFile()->GetName()<<" to "<<outputfilename<<endl;
	uint64_t nevt= RunHeaderTree->GetEntries();

	for (unsigned int ievt=0; ievt<nevt; ievt++) {
		RunHeaderTree->GetEntry(ievt);
		runheader_d.AppendPacket( RunHeader );
		if (ievt==0) cout<<"start processing ..."<<endl;
		else if (ievt%1000==0) cout<<ievt<<" events converted"<<endl;
	}
	free(RunHeader);
}
