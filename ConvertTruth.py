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
global_start = time()

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
#TriggerInfoTable = h5file.create_table("/", "TriggerInfo", TriggerInfoData, "Trigger info")
#triggerinfo = TriggerInfoTable.row
WaveformTable = h5file.create_table("/", "Waveform", WaveformData, "Waveform")
waveform = WaveformTable.row
GroundTruthTable = h5file.create_table("/", "GroundTruth", GroundTruthData, "GroundTruth")
groundtruth = GroundTruthTable.row


def ReadFile(fip) :
    EventID = fip["Readout"]["TriggerNo"].array().astype(np.int64)
    # Sec = fip["Readout"]["Sec"].array().astype(np.int32)
    # NanoSec = fip["Readout"]["NanoSec"].array().astype(np.int32)

    Waveform = fip["Readout"]["Waveform"].array(flatten=True).astype(np.int16)
    ChannelID = fip["Readout"]["ChannelId"].array()
    nChannels = Len(ChannelID)
    EventID2 = EventID.repeat(nChannels)
    ChannelID2 = ChannelID.flatten().astype(np.int16)
    WindowSize = int(len(Waveform) / len(ChannelID2))
    nWave = len(ChannelID2)
    Waveform = Waveform.reshape(nWave, WindowSize)
    # WaveFrame = pd.DataFrame({"EventID":EventID2,"ChannelID":ChannelID2,"Waveform":list(Waveform)}):

    EventID = fip["SimTriggerInfo"]["TriggerNo"].array().astype(np.int64)
    nphotons = fip["SimTriggerInfo"]["PEList"].array(entrystop=len(EventID))
    EventID3 = EventID.repeat(nphotons)
    ChannelID3 = fip["SimTriggerInfo"]["PEList.PMTId"].array(flatten=True, entrystop=len(EventID))
    HitPos = fip["SimTriggerInfo"]["PEList.HitPosInWindow"].array(flatten=True, entrystop=len(EventID))
    Charge = fip["SimTriggerInfo"]["PEList.Charge"].array(flatten=True, entrystop=len(EventID))
    # TruthFrame = pd.DataFrame({"EventID":EventID3,"ChannelID":ChannelID3,"HitTime":HitTime,"PulseTime":PulseTime,"HitPos":HitPos})
    return {"EventID3": EventID3, "ChannelID3": ChannelID3, "Charge": Charge, "HitPos": HitPos, "EventID2": EventID2, "ChannelID2": ChannelID2, "Waveform": Waveform}


file_handles = [fip]
for fileNo in range(1, maxfiles) :
    filename = file_prefix + "_{}.root".format(fileNo)
    try : fip = uproot.open(filename)
    except FileNotFoundError :
        print("only {} files found.".format(fileNo))
        break
    file_handles.append(fip)
print("Initialized, consuming {:.2f}s.".format(time() - start))

start = time()
Results = [ReadFile(fip) for fip in file_handles]
keys = Results[-1].keys()
Result = {key: np.concatenate([result[key] for result in Results]) for key in keys}
print("root file read, consuming {:.2f}s.".format(time() - start))

start = time()
for i in range(len(Result["Waveform"])) :
    waveform['EventID'] = Result["EventID2"][i]
    waveform['ChannelID'] = Result["ChannelID2"][i]
    waveform['Waveform'] = Result["Waveform"][i]
    waveform.append()

# for i in range(len(Result["EventID"])) :
#    triggerinfo['EventID'] = Result["EventID"][i]
#    triggerinfo['Sec'] = Result["Sec"][i]
#    triggerinfo['NanoSec'] = Result["NanoSec"][i]
#    triggerinfo.append()

for i in range(len(Result["HitPos"])) :
    groundtruth["EventID"] = Result["EventID3"][i]
    groundtruth["ChannelID"] = Result["ChannelID3"][i]
    groundtruth["RiseTime"] = Result["HitPos"][i]
    groundtruth["Charge"] = Result["Charge"][i]
    groundtruth.append()

# TriggerInfoTable.flush()
WaveformTable.flush()
GroundTruthTable.flush()
print("h5 file wrote, consuming {:.2f}s.".format(time() - start))

h5file.close()
print("Finished, consuming {:.2f}s.".format(time() - global_start))
