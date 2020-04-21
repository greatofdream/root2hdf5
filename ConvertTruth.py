#!/opt/gentoo/usr/bin/python3.6

import argparse
psr = argparse.ArgumentParser()
psr.add_argument("--limit", type=int, default=999, help="limit of number of files to process")
psr.add_argument('ipt', type=str, help="input filename prefix")
psr.add_argument("opt", help="output")
args = psr.parse_args()
file_prefix = args.ipt
maxfiles = args.limit
opt = args.opt

from time import time
start = time()

import uproot
import tables
import numpy as np

Len = np.vectorize(len)

filename = file_prefix + ".root"
fip = uproot.open(filename)
WindowSize = int(len(fip["Readout"]["Waveform"].array(entrystart=0, entrystop=1)[0]) / len(fip["Readout"]["ChannelId"].array(entrystart=0, entrystop=1)[0]))


class WaveformData(tables.IsDescription):
    EventID = tables.Int64Col(pos=0)
    ChannelID = tables.Int16Col(pos=1)
    Waveform = tables.Col.from_type('int16', shape=WindowSize, pos=2)


class TriggerInfoData(tables.IsDescription):
    EventID = tables.Int64Col(pos=0)
    Sec = tables.Int32Col(pos=1)
    NanoSec = tables.Int32Col(pos=2)


class GroundTruthData(tables.IsDescription):
    EventID = tables.Int64Col(pos=0)
    ChannelID = tables.Int16Col(pos=1)
    RiseTime = tables.Int16Col(pos=2)
    Charge = tables.Float64Col(pos=3)


h5file = tables.open_file(opt, mode="w", title="OneTonDetector", filters=tables.Filters(complevel=9))
TriggerInfoTable = h5file.create_table("/", "TriggerInfo", TriggerInfoData, "Trigger info")
triggerinfo = TriggerInfoTable.row
WaveformTable = h5file.create_table("/", "Waveform", WaveformData, "Waveform")
waveform = WaveformTable.row
GroundTruthTable = h5file.create_table("/", "GroundTruth", GroundTruthData, "GroundTruth")
groundtruth = GroundTruthTable.row


def Loop_File(fip) :
    EventID = fip["Readout"]["TriggerNo"].array().astype(np.int64)
    Sec = fip["Readout"]["Sec"].array().astype(np.int32)
    NanoSec = fip["Readout"]["NanoSec"].array().astype(np.int32)

    nphotons = fip["SimTriggerInfo"]["PEList"].array()
    EventID3 = EventID.repeat(nphotons)
    ChannelID3 = fip["SimTriggerInfo"]["PEList.PMTId"].array(flatten=True)
    HitPos = fip["SimTriggerInfo"]["PEList.HitPosInWindow"].array(flatten=True)
    Charge = fip["SimTriggerInfo"]["PEList.Charge"].array(flatten=True)
    # TruthFrame = pd.DataFrame({"EventID":EventID3,"ChannelID":ChannelID3,"HitTime":HitTime,"PulseTime":PulseTime,"HitPos":HitPos})

    Waveform = fip["Readout"]["Waveform"].array(flatten=True).astype(np.int16)
    ChannelID = fip["Readout"]["ChannelId"].array()
    nChannels = Len(ChannelID)
    EventID2 = EventID.repeat(nChannels)
    ChannelID2 = ChannelID.flatten().astype(np.int16)
    WindowSize = int(len(Waveform) / len(ChannelID2))
    nWave = len(ChannelID2)
    Waveform = Waveform.reshape(nWave, WindowSize)
    # WaveFrame = pd.DataFrame({"EventID":EventID2,"ChannelID":ChannelID2,"Waveform":list(Waveform)}):

    for i in range(nWave) :
        waveform['EventID'] = EventID2[i]
        waveform['ChannelID'] = ChannelID2[i]
        waveform['Waveform'] = Waveform[i]
        waveform.append()

    for i in range(len(EventID)) :
        triggerinfo['EventID'] = EventID[i]
        triggerinfo['Sec'] = Sec[i]
        triggerinfo['NanoSec'] = NanoSec[i]
        triggerinfo.append()

    for i in range(len(HitPos)) :
        groundtruth["EventID"] = EventID3[i]
        groundtruth["ChannelID"] = ChannelID3[i]
        groundtruth["RiseTime"] = HitPos[i]
        groundtruth["Charge"] = Charge[i]
        groundtruth.append()

    TriggerInfoTable.flush()
    WaveformTable.flush()
    GroundTruthTable.flush()


print("Initialized, consuming {:.2f}s.".format(time() - start))
start = time()
print("Processing file {}".format(0))
Loop_File(fip)
for fileNo in range(1, maxfiles) :
    filename = file_prefix + "{}.root".format(fileNo)
    try : fip = uproot.open(filename)
    except FileNotFoundError :
        print("only {} files found, reading terminiated.".format(fileNo))
        break
    print("Processing file {}".format(fileNo))
    Loop_File(fileNo, fip)

h5file.close()
print("Finished, consuming {:.2f}s.".format(time() - start))
