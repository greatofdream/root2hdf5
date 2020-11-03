/*================================================================
*   Filename   : TransformReadoutTree.C
*   Author     : Yiyang Wu
*   Created on  Thu 29 Oct 2020 06:29:40 PM CST
*   Description:
*
================================================================*/


#include <iostream>
#include "TTree.h"
#include <vector>
#include "H5PacketTable.h"
#include "TFile.h"

using namespace std;

void Convert_Readout_Tree(TTree* ReadoutTree, hid_t outputfile, hid_t dsp, vector<int> readout_chunksize)
{
	// Read Waveform and ChannelId
	vector<uint32_t>* Waveform = nullptr;
	vector<uint32_t>* ChannelId = nullptr;
	ReadoutTree->SetBranchAddress("ChannelId",&ChannelId);
	ReadoutTree->SetBranchAddress("Waveform",&Waveform);

	// determine WindowSize
	ReadoutTree->GetEntry(0);
	int WindowSize = Waveform->size()/ChannelId->size();

	// define outputfile data structure
	struct TriggerInfo_t {
		int32_t runno;
		int32_t triggerno;
		int32_t detectorid;
		int32_t triggertype;
		int32_t sec;
		int32_t nanosec;
	};
	TriggerInfo_t* TriggerInfo= (TriggerInfo_t*) malloc(sizeof(TriggerInfo_t));

	// directly read root data into struct.
	ReadoutTree->SetBranchAddress("RunNo",&TriggerInfo->runno);
	ReadoutTree->SetBranchAddress("TriggerNo",&TriggerInfo->triggerno);
	ReadoutTree->SetBranchAddress("TriggerType",&TriggerInfo->triggertype);
	ReadoutTree->SetBranchAddress("DetectorID",&TriggerInfo->triggerno);
	ReadoutTree->SetBranchAddress("Sec",&TriggerInfo->sec);
	ReadoutTree->SetBranchAddress("NanoSec",&TriggerInfo->nanosec);
	
	// define outputfile data structure
	// caution: struct alignment. sizeof(Readout_t) != HOFFSET(Readout_t, waveform)
	struct Readout_t {
		int32_t triggerno;
		int32_t channelid;
		int16_t waveform[];
	};
	size_t readout_size = sizeof(Readout_t)+sizeof(*Readout_t::waveform)*WindowSize;
	Readout_t* Readout= (Readout_t*) malloc(readout_size);

	// Create Readout Group
	hid_t ReadoutGroup = H5Gcreate1(outputfile, "/Readout",100);
	// Create outputfile Table
	hid_t waveformtable = H5Tcreate (H5T_COMPOUND, readout_size);
	H5Tinsert (waveformtable, "TriggerNo", HOFFSET(Readout_t, triggerno), H5T_NATIVE_INT32);
	H5Tinsert (waveformtable, "ChannelID", HOFFSET(Readout_t, channelid), H5T_NATIVE_INT32);
	hsize_t dim[1]; dim[0] = WindowSize;
	hid_t waveform_t = H5Tarray_create(H5T_NATIVE_INT16, 1, dim);
	H5Tinsert (waveformtable, "Waveform", HOFFSET(Readout_t, waveform), waveform_t);
	FL_PacketTable waveform_d(ReadoutGroup, "Waveform", waveformtable, readout_chunksize[1], dsp);
	if(! waveform_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table Waveform.");
		abort();
	}
	
	hid_t triggerinfotable = H5Tcreate (H5T_COMPOUND, sizeof(TriggerInfo_t));
	H5Tinsert (triggerinfotable, "RunNo", HOFFSET(TriggerInfo_t, runno), H5T_NATIVE_INT32);
	H5Tinsert (triggerinfotable, "TriggerNo", HOFFSET(TriggerInfo_t, triggerno), H5T_NATIVE_INT32);
	H5Tinsert (triggerinfotable, "TriggerType", HOFFSET(TriggerInfo_t, triggertype), H5T_NATIVE_INT32);
	H5Tinsert (triggerinfotable, "DetectorID", HOFFSET(TriggerInfo_t, detectorid), H5T_NATIVE_INT32);
	H5Tinsert (triggerinfotable, "Sec", HOFFSET(TriggerInfo_t, sec), H5T_NATIVE_INT32);
	H5Tinsert (triggerinfotable, "NanoSec", HOFFSET(TriggerInfo_t, nanosec), H5T_NATIVE_INT32);
	FL_PacketTable triggerinfo_d(ReadoutGroup, "TriggerInfo", triggerinfotable, readout_chunksize[0], dsp);
	if(! triggerinfo_d.IsValid()) {
		fprintf(stderr, "Unable to create packet table TriggerInfo.");
		abort();
	}

	// converting loop
	char outputfilename[100];
	H5Fget_name(outputfile,outputfilename,100);
	cout<<"Converting Readout Tree of "<<ReadoutTree->GetCurrentFile()->GetName()<<" to "<<outputfilename<<endl;
	uint64_t nevt= ReadoutTree->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;

	for (unsigned int ievt=0; ievt<nevt; ievt++) {
		ReadoutTree->GetEntry(ievt);
		Readout->triggerno = TriggerInfo->triggerno;
		for(int i=0;i<ChannelId->size();i++)
		{
			Readout->channelid = (*ChannelId)[i];
			for(int j=0;j<WindowSize;j++) Readout->waveform[j]=(*Waveform)[i*WindowSize+j];
			waveform_d.AppendPacket( Readout );
		}
		triggerinfo_d.AppendPacket( TriggerInfo );

		if (ievt==0) cout<<"start processing ..."<<endl;
		else if (ievt%1000==0) cout<<ievt<<" events converted"<<endl;
	}
	free(Readout);
}
