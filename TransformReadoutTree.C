/*================================================================
*   Filename   : TransformReadoutTree.C
*   Author     : Yiyang Wu
*   Created on  Thu 29 Oct 2020 06:29:40 PM CST
*   Description:
*
================================================================*/


#include <iostream>
#include "TTree.h"
#include "TChain.h"
#include <vector>
#include "H5PacketTable.h"
#include "TFile.h"

using namespace std;

void Convert_Readout_Tree(TChain* ReadoutTree, hid_t outputfile, hid_t dsp, vector<int> readout_chunksize)
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
	if(nevt>1) ReadoutTree->GetEntry(1); else ReadoutTree->GetEntry(0);
	for(int i=2;ChannelId->size()==0 && i < nevt - 1;i++) ReadoutTree->GetEntry(i);
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
	TriggerInfo_t* TriggerInfo= (TriggerInfo_t*) malloc(sizeof(TriggerInfo_t));

	// directly read root data into struct.
	ReadoutTree->SetBranchAddress("RunNo",&TriggerInfo->runno);
	ReadoutTree->SetBranchAddress("TriggerNo",&TriggerInfo->triggerno);
	ReadoutTree->SetBranchAddress("TriggerType",&TriggerInfo->triggertype);
	ReadoutTree->SetBranchAddress("DetectorID",&TriggerInfo->detectorid);
	ReadoutTree->SetBranchAddress("Sec",&TriggerInfo->sec);
	ReadoutTree->SetBranchAddress("NanoSec",&TriggerInfo->nanosec);
	
	// define outputfile data structure
	// caution: struct alignment. sizeof(Readout_t) != HOFFSET(Readout_t, waveform)
	struct Waveform_t {
		int32_t triggerno;
		int32_t channelid;
		int16_t waveform[];
	};
	size_t waveform_size = sizeof(Waveform_t)+sizeof(*Waveform_t::waveform)*WindowSize;
	Waveform_t* Waveform= (Waveform_t*) malloc(waveform_size);

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
	// cout<<"Converting Readout Tree of "<<ReadoutTree->GetCurrentFile()->GetName()<<" to "<<outputfilename<<endl;
	cout<<"Converting Readout Tree of "<<ReadoutTree->GetFile()->GetName()<<" to "<<outputfilename<<endl;

	cout<<nevt<<" events to be processed"<<endl;

	cout<<"start processing ..."<<endl;
	for (unsigned int ievt=0; ievt<nevt; ievt++) {
		ReadoutTree->GetEntry(ievt);
		if(Waveform_origin->size() == WindowSize * ChannelId->size())
		{
			Waveform->triggerno = TriggerInfo->triggerno;
			for(int i=0;i<ChannelId->size();i++)
			{
				Waveform->channelid = (*ChannelId)[i];
				for(int j=0;j<WindowSize;j++) Waveform->waveform[j]=(*Waveform_origin)[i*WindowSize+j];
				waveform_d.AppendPacket( Waveform );
			}
			triggerinfo_d.AppendPacket( TriggerInfo );
		}
		else cerr<<"Entry "<<ievt<<" has inconsistent WindowSize="<<Waveform_origin->size()/ChannelId->size()<<"!"<<endl;

		if (ievt!=0 && ievt%1000==0) cout<<ievt<<" events converted"<<endl;
	}
	free(Waveform);
}
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
	if(nevt>1) ReadoutTree->GetEntry(1); else ReadoutTree->GetEntry(0);
	for(int i=2;ChannelId->size()==0 && i < nevt - 1;i++) ReadoutTree->GetEntry(i);
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
	TriggerInfo_t* TriggerInfo= (TriggerInfo_t*) malloc(sizeof(TriggerInfo_t));

	// directly read root data into struct.
	ReadoutTree->SetBranchAddress("RunNo",&TriggerInfo->runno);
	ReadoutTree->SetBranchAddress("TriggerNo",&TriggerInfo->triggerno);
	ReadoutTree->SetBranchAddress("TriggerType",&TriggerInfo->triggertype);
	ReadoutTree->SetBranchAddress("DetectorID",&TriggerInfo->detectorid);
	ReadoutTree->SetBranchAddress("Sec",&TriggerInfo->sec);
	ReadoutTree->SetBranchAddress("NanoSec",&TriggerInfo->nanosec);
	
	// define outputfile data structure
	// caution: struct alignment. sizeof(Readout_t) != HOFFSET(Readout_t, waveform)
	struct Waveform_t {
		int32_t triggerno;
		int32_t channelid;
		int16_t waveform[];
	};
	size_t waveform_size = sizeof(Waveform_t)+sizeof(*Waveform_t::waveform)*WindowSize;
	Waveform_t* Waveform= (Waveform_t*) malloc(waveform_size);

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
	//cout<<"Converting Readout Tree of "<<ReadoutTree->GetFile()->GetName()<<" to "<<outputfilename<<endl;

	cout<<nevt<<" events to be processed"<<endl;

	cout<<"start processing ..."<<endl;
	for (unsigned int ievt=0; ievt<nevt; ievt++) {
		ReadoutTree->GetEntry(ievt);
		if(Waveform_origin->size() == WindowSize * ChannelId->size())
		{
			Waveform->triggerno = TriggerInfo->triggerno;
			for(int i=0;i<ChannelId->size();i++)
			{
				Waveform->channelid = (*ChannelId)[i];
				for(int j=0;j<WindowSize;j++) Waveform->waveform[j]=(*Waveform_origin)[i*WindowSize+j];
				waveform_d.AppendPacket( Waveform );
			}
			triggerinfo_d.AppendPacket( TriggerInfo );
		}
		else cerr<<"Entry "<<ievt<<" has inconsistent WindowSize="<<Waveform_origin->size()/ChannelId->size()<<"!"<<endl;

		if (ievt!=0 && ievt%1000==0) cout<<ievt<<" events converted"<<endl;
	}
	free(Waveform);
}

