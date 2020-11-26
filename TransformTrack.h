/*================================================================
*   Filename   : TransformTrack.h
*   Author     : Yiyang Wu
*   Created on  Sat 31 Oct 2020 09:05:42 PM CST
*   Description:
*
================================================================*/

#include <cstdint>
#include "H5PacketTable.h"
#include "H5PacketTable.h"
#include "JPSimOutput.hh"

#ifndef _TRANSFORMTRACK_H
#define _TRANSFORMTRACK_H

using namespace std;

struct TrackHeader_t
{
	int32_t RunId;
	int32_t VertexId;
	int trackinfo[];
};

struct StepPointHeader_t 
{
	int32_t RunId;		// The ID of Run
	int32_t SegmentId;	// The ID of Segment
	int32_t VertexId;	// The ID of Segment
	int32_t TrackId;	// The ID of Segment
	int steppointinfo[];
};

struct SecondaryParticle_t
{
	int32_t RunId;		// The ID of Run
	int32_t SegmentId;	// The ID of Segment
	int32_t VertexId;
	int32_t TrackId;
	int32_t StepId;
	int32_t nSecondaryPdgId;
};

class TrackTransformer
{
	TrackHeader_t *Track;
	StepPointHeader_t* StepPointHeader;
	SecondaryParticle_t* secondaryparticle;
	hid_t Group;
	int chunksizes[3];
	FL_PacketTable *track_d=nullptr, *steppoint_d=nullptr, *secondaryparticle_d=nullptr;
public:
	TrackTransformer(hid_t group, int chunksizes[], hid_t dsp);
	void LoopTrack(vector<JPSimTrack_t>* TrackList_origin, int32_t runid, int32_t segmentid, int32_t vertexid);
	~TrackTransformer();
};

#endif
