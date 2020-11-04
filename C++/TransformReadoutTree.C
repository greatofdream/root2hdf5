/*================================================================
*   Filename   : TransformReadoutTree.C
*   Author     : Yiyang Wu
*   Created on  Thu 29 Oct 2020 06:29:40 PM CST
*   Description:
*
================================================================*/


#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <cstdint>
#include <iostream>
#include "TTree.h"
#include <vector>
#include "H5PacketTable.h"
#include "TFile.h"

const uint64_t buffersize = 1024*1024*1024*1 /*Bytes*/;

using namespace std;

void Convert_Readout_Tree(TTree* ReadoutTree, hid_t outputfile, hid_t dsp, vector<int> readout_chunksize)
{
	// Create Readout Group
	hid_t ReadoutGroup = H5Gcreate1(outputfile, "/Readout",100);
	uint64_t nevt= ReadoutTree->GetEntries();
	if(nevt==0) return;
	// Read Waveform and ChannelId
	vector<uint32_t>* Waveform_origin = nullptr;
	vector<uint32_t>* ChannelId = nullptr;
	ReadoutTree->SetBranchAddress("ChannelId",&ChannelId);
	ReadoutTree->SetBranchAddress("Waveform",&Waveform_origin);

	// determine WindowSize
	ReadoutTree->GetEntry(0);
	for(int i=1;ChannelId->size()==0 || i<nevt;i++) ReadoutTree->GetEntry(i);
	int WindowSize;
	if(ChannelId->size()==0) return;
	else WindowSize = Waveform_origin->size()/ChannelId->size();

	// define outputfile data structure
	struct TriggerInfo_t {
		int32_t runno;
		int32_t triggerno;
		int32_t detectorid;
		int32_t triggertype;
		int32_t sec;
		int32_t nanosec;
	};
	uint64_t TriggerInfoBuffSize = buffersize/sizeof(TriggerInfo_t);
	if(TriggerInfoBuffSize<1) TriggerInfoBuffSize=1;
	TriggerInfo_t* TriggerInfo_begin_pointer= (TriggerInfo_t*) malloc(sizeof(TriggerInfo_t) * TriggerInfoBuffSize);
	TriggerInfo_t* TriggerInfo_origin= (TriggerInfo_t*) malloc(sizeof(TriggerInfo_t));

	// directly read root data into struct.
	ReadoutTree->SetBranchAddress("RunNo",&TriggerInfo_origin->runno);
	ReadoutTree->SetBranchAddress("TriggerNo",&TriggerInfo_origin->triggerno);
	ReadoutTree->SetBranchAddress("TriggerType",&TriggerInfo_origin->triggertype);
	ReadoutTree->SetBranchAddress("DetectorID",&TriggerInfo_origin->detectorid);
	ReadoutTree->SetBranchAddress("Sec",&TriggerInfo_origin->sec);
	ReadoutTree->SetBranchAddress("NanoSec",&TriggerInfo_origin->nanosec);
	
	// define outputfile data structure
	// caution: struct alignment. sizeof(Readout_t) != HOFFSET(Readout_t, waveform)
	struct Waveform_t {
		int32_t triggerno;
		int32_t channelid;
		int16_t waveform[];
	};
	size_t waveform_size = sizeof(Waveform_t)+sizeof(*Waveform_t::waveform)*WindowSize;
	uint64_t WaveformBuffSize = buffersize/waveform_size;
	if(WaveformBuffSize<1) WaveformBuffSize=1;
	Waveform_t* Waveform_begin_pointer= (Waveform_t*) malloc(waveform_size * WaveformBuffSize);
	cout<<"WaveformBuffSize="<<WaveformBuffSize<<endl;

	// Create outputfile Table
	hid_t waveformtable = H5Tcreate (H5T_COMPOUND, waveform_size);
	H5Tinsert (waveformtable, "TriggerNo", HOFFSET(Waveform_t, triggerno), H5T_NATIVE_INT32);
	H5Tinsert (waveformtable, "ChannelID", HOFFSET(Waveform_t, channelid), H5T_NATIVE_INT32);
	hsize_t dim[1]; dim[0] = WindowSize;
	hid_t waveform_t = H5Tarray_create(H5T_NATIVE_INT16, 1, dim);
	H5Tinsert (waveformtable, "Waveform", HOFFSET(Waveform_t, waveform), waveform_t);
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
	cout<<nevt<<" events to be processed"<<endl;

	int TriggerInfo_count = 0, Waveform_count = 0;
	Waveform_t * Waveform = Waveform_begin_pointer;
	TriggerInfo_t* TriggerInfo = TriggerInfo_begin_pointer;
	for (unsigned int ievt=0; ievt<nevt; ievt++) {
		// filling TriggerInfo
		ReadoutTree->GetEntry(ievt);
		memcpy(TriggerInfo, TriggerInfo_origin, sizeof(TriggerInfo_t));
		TriggerInfo_count++;
		TriggerInfo++;
		if(TriggerInfo_count==TriggerInfoBuffSize) 
		{
			triggerinfo_d.AppendPackets(TriggerInfoBuffSize, TriggerInfo_begin_pointer);
			TriggerInfo = TriggerInfo_begin_pointer;
			TriggerInfo_count=0;
		}

	 	// filling Waveform
	 	if(ChannelId->size()!=0)
	 	for(int i=0;i<ChannelId->size();i++)
	 	{
	 		Waveform->triggerno = TriggerInfo_origin->triggerno;
	 		Waveform->channelid = (*ChannelId)[i];
	 		for(int j=0;j<WindowSize;j++) Waveform->waveform[j]=(*Waveform_origin)[i*WindowSize+j];
	 		Waveform_count++;
	 		Waveform = (Waveform_t*) ((uint64_t)Waveform + waveform_size);
	 		if(Waveform_count==WaveformBuffSize) 
	 		{
	 			waveform_d.AppendPackets(WaveformBuffSize, Waveform_begin_pointer);
	 			Waveform = Waveform_begin_pointer;
	 			Waveform_count=0;
	 		}
	 	}

		if (ievt==0) cout<<"start processing ..."<<endl;
		else if (ievt%1000==0) cout<<ievt<<" events converted"<<endl;
	}
	if(TriggerInfo_count!=0) triggerinfo_d.AppendPackets(TriggerInfo_count, TriggerInfo_begin_pointer);
	if(Waveform_count!=0) waveform_d.AppendPackets(Waveform_count, Waveform_begin_pointer);

	free(TriggerInfo_begin_pointer);
	free(TriggerInfo_origin);
	free(Waveform_begin_pointer);
}
