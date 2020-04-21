#!/opt/gentoo/usr/bin/python3.6

import argparse
psr = argparse.ArgumentParser()
psr.add_argument("--limit", type=int, default=999, help="limit of number of files to process")
psr.add_argument('ipt', type=int, help="input run number")
psr.add_argument("opt", help="output")
args = psr.parse_args()
runNo = args.ipt
maxfiles = args.limit
opt = args.opt

from time import time
start = time()

import os
import re
import uproot
import tables
import numpy as np

Len = np.vectorize(len)
JPDataDir = os.environ["JPDataDir"]

filename_list = os.listdir(JPDataDir + "/run{:08d}".format(runNo))
regular_exp = re.compile(r"Jinping_1ton_Phy_([0-9]{8})_" + r"{:08d}".format(runNo) + r"_?([0-9]*)\.root")
filename_dict = dict([])
for filename in filename_list :
    fileNo = regular_exp.match(filename)[2]
    if fileNo == '' : fileNo = 0
    else : fileNo = int(fileNo)
    filename_dict[fileNo] = filename

fip = uproot.open(JPDataDir + "/run{:08d}/".format(runNo) + filename_list[0])
ReadoutTree = fip["Readout"]
Waveform = ReadoutTree["Waveform"].array(entrystart=0, entrystop=1, flatten=True)
ChannelID = ReadoutTree["ChannelId"].array(entrystart=0, entrystop=1, flatten=True)
WindowSize = int(len(Waveform) / len(ChannelID))


nWave = len(ChannelID)
nChannels = Len(np.array(ReadoutTree["ChannelId"].array()))


class WaveformData(tables.IsDescription):
    EventID = tables.Int64Col(pos=0)
    ChannelID = tables.Int16Col(pos=1)
    Waveform = tables.Col.from_type('int16', shape=WindowSize, pos=2)


class TriggerInfoData(tables.IsDescription):
    EventID = tables.Int64Col(pos=0)
    Sec = tables.Int32Col(pos=1)
    NanoSec = tables.Int32Col(pos=2)


h5file = tables.open_file(opt, mode="w", title="OneTonDetector", filters=tables.Filters(complevel=9))
# TriggerInfoTable = h5file.create_table("/", "TriggerInfo", TriggerInfoData, "Trigger info")
# triggerinfo = TriggerInfoTable.row
WaveformTable = h5file.create_table("/", "Waveform", WaveformData, "Waveform")
waveform = WaveformTable.row


def Loop_File(filename) :
    fip = uproot.open(filename)
    ReadoutTree = fip["Readout"]
    Waveform = np.array(ReadoutTree["Waveform"].array(flatten=True), dtype=np.int16)
    ChannelID = np.array(ReadoutTree["ChannelId"].array(flatten=True), dtype=np.int16)
    EventID = np.array(ReadoutTree["TriggerNo"].array(), dtype=np.int64)
    # Sec = ReadoutTree["Sec"].array()
    # NanoSec = ReadoutTree["NanoSec"].array()

    nWave = len(ChannelID)
    Waveform = Waveform.reshape(nWave, WindowSize)
    nChannels = Len(np.array(ReadoutTree["ChannelId"].array()))
    EventID_2 = EventID.repeat(nChannels)

    for i in range(nWave) :
        if(ChannelID[i] == 17) :
            waveform['EventID'] = EventID_2[i]
            waveform['ChannelID'] = ChannelID[i]
            waveform['Waveform'] = Waveform[i]
            waveform.append()

    # for i in range(len(EventID)) :
    #    triggerinfo['EventID'] = EventID[i]
    #    triggerinfo['Sec'] = Sec[i]
    #    triggerinfo['NanoSec'] = NanoSec[i]
    #    triggerinfo.append()

    # TriggerInfoTable.flush()
    WaveformTable.flush()


maxfiles = min(len(filename_dict.keys()), maxfiles)

print("Initialized, consuming {:.2f}s.".format(time() - start))
start = time()
for fileNo in np.sort(list(filename_dict.keys()))[0:maxfiles] :
    filename = JPDataDir + "/run{:08d}/".format(runNo) + filename_dict[fileNo]
    print("Processing file {}".format(fileNo))
    Loop_File(filename)

h5file.close()
print("Finished, consuming {:.2f}s.".format(time() - start))
