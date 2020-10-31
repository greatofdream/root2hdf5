/*================================================================
*   Filename   : TransformTrack.C
*   Author     : Yiyang Wu
*   Created on  Sat 31 Oct 2020 09:18:26 PM CST
*   Description:
*
================================================================*/

#include <bits/stdint-intn.h>
#ifndef _TRANSFORMTRACK_H
#include "TransformTrack.h"
#endif

#include <cstdint>
#include "H5PacketTable.h"
#include "JPSimOutput.hh"

using namespace std;

TrackTransformer::TrackTransformer(hid_t outputfile, int chunksizes[], hid_t dsp)
{
	// Create Track Group
	hid_t TrackGroup = H5Gcreate1(outputfile, "/Track",100);

	// Create Track Table
	size_t tracksize = sizeof(TrackHeader_t) + HOFFSET(JPSimTrack_t, StepPoints);
	Track = (TrackHeader_t*) malloc(tracksize);
	hid_t tracktable = H5Tcreate (H5T_COMPOUND, tracksize);
	H5Tinsert (tracktable, "RunId", HOFFSET(TrackHeader_t, RunId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "VertexId", HOFFSET(TrackHeader_t, VertexId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "nSegmentId", sizeof(TrackHeader_t) + HOFFSET(JPSimTrack_t, nSegmentId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "nParentTrackId", sizeof(TrackHeader_t) + HOFFSET(JPSimTrack_t, nParentTrackId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "nTrackId", sizeof(TrackHeader_t) + HOFFSET(JPSimTrack_t, nTrackId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "nPrimaryId", sizeof(TrackHeader_t) + HOFFSET(JPSimTrack_t, nPrimaryId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "nPdgId", sizeof(TrackHeader_t) + HOFFSET(JPSimTrack_t, nPdgId), H5T_NATIVE_INT32);
	H5Tinsert (tracktable, "bDetectedPhoton", sizeof(TrackHeader_t) + HOFFSET(JPSimTrack_t, bDetectedPhoton), H5T_NATIVE_HBOOL);
	track_d = new FL_PacketTable(TrackGroup, "TrackList", tracktable, chunksizes[0], dsp);
	if(! track_d->IsValid()) {
		fprintf(stderr, "Unable to create packet table TrackList.");
		abort();
	}

	// Create StepPoint Table
	size_t steppointsize = sizeof(StepPointHeader_t) + HOFFSET(JPSimStepPoint_t, nSecondaryPdgId);
	StepPointHeader = (StepPointHeader_t*) malloc(steppointsize);
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
	steppoint_d = new FL_PacketTable(TrackGroup, "StepPoint", steppointtable, chunksizes[1], dsp);
	if(! steppoint_d->IsValid()) {
		fprintf(stderr, "Unable to create packet table StepPoint.");
		abort();
	}

	// Create SecondaryParticle Table
	secondaryparticle = (SecondaryParticle_t*) malloc(sizeof(SecondaryParticle_t));
	hid_t secondaryparticletable = H5Tcreate (H5T_COMPOUND, sizeof(SecondaryParticle_t));
	H5Tinsert (secondaryparticletable, "RunId", HOFFSET(SecondaryParticle_t, RunId), H5T_NATIVE_INT32);
	H5Tinsert (secondaryparticletable, "SegmentId", HOFFSET(SecondaryParticle_t, SegmentId), H5T_NATIVE_INT32);
	H5Tinsert (secondaryparticletable, "VertexId", HOFFSET(SecondaryParticle_t, VertexId), H5T_NATIVE_INT32);
	H5Tinsert (secondaryparticletable, "TrackId", HOFFSET(SecondaryParticle_t, TrackId), H5T_NATIVE_INT32);
	H5Tinsert (secondaryparticletable, "StepId", HOFFSET(SecondaryParticle_t, StepId), H5T_NATIVE_INT32);
	H5Tinsert (secondaryparticletable, "nSecondaryPdgId;", HOFFSET(SecondaryParticle_t, nSecondaryPdgId), H5T_NATIVE_INT32);
	secondaryparticle_d = new FL_PacketTable(TrackGroup, "SecondaryParticle", tracktable, chunksizes[2], dsp);
	if(! secondaryparticle_d->IsValid()) {
		fprintf(stderr, "Unable to create packet table SecondaryParticle.");
		abort();
	}
}

void TrackTransformer::LoopTrack(vector<JPSimTrack_t>* TrackList_origin, int32_t runid, int32_t segmentid, int32_t vertexid)
{
	secondaryparticle->RunId = StepPointHeader->RunId = Track->RunId = runid;
	secondaryparticle->SegmentId = StepPointHeader->SegmentId = segmentid;
	secondaryparticle->VertexId = StepPointHeader->VertexId = Track->VertexId = vertexid;
	for(auto track : *TrackList_origin) 
	{
		memcpy(Track->trackinfo, &track, HOFFSET(JPSimTrack_t, StepPoints));
		track_d->AppendPacket( Track );
		for(auto step : track.StepPoints ) 
		{
			memcpy(StepPointHeader->steppointinfo, &step, HOFFSET(JPSimStepPoint_t, nSecondaryPdgId));
			steppoint_d->AppendPacket( StepPointHeader );
			for(auto secondarypdgid : step.nSecondaryPdgId ) 
			{
				secondaryparticle->nSecondaryPdgId = secondarypdgid;
				secondaryparticle_d->AppendPacket( secondaryparticle );
			}
		}
	}
}

TrackTransformer::~TrackTransformer()
{
	free(Track);
	free(StepPointHeader);
	free(secondaryparticle);
	delete track_d;
	delete steppoint_d;
	delete secondaryparticle_d;
}
