#!/opt/gentoo/usr/bin/python3.6

import os,re,sys
import uproot
import numpy as np

JPDataDir = os.environ['JPDataDir']
date_search = re.compile(r"Jinping_1ton_Phy_(\d{8,8})_(\d{8,8})(_\d+)?.root")

def GetRunDate(runNo) :
    return int(date_search.match(os.listdir(JPDataDir+"/run{0:0>8d}".format(runNo))[0])[1])

def GetWaveform(runNo,FileNo,TriggerNo,ChannelId) :
    rundate = GetRunDate(runNo)
    if FileNo==0 :
        filename = JPDataDir + "/run{1:0>8d}/Jinping_1ton_Phy_{0}_{1:0>8d}.root".format(rundate,runNo)
    else : 
        filename = JPDataDir + "/run{1:0>8d}/Jinping_1ton_Phy_{0}_{1:0>8d}_{2}.root".format(rundate,runNo,FileNo)
    if not os.path.exists(filename) :
        raise FileExistsError(filename+" does not exist!")

    fip = uproot.open(filename)
    TriggerNos = fip["Readout"]["TriggerNo"].array()
    if TriggerNo<TriggerNos[0] or TriggerNo>TriggerNos[-1] :
        raise ValueError("This event is not in this file")
    entry = TriggerNo- TriggerNos[0]
    if not TriggerNo==TriggerNos[entry] :
        raise ValueError("TriggerNo not continuous!")
    ChannelID = fip["Readout"]["ChannelId"].array(entrystart=entry,entrystop=entry+1,flatten=True)
    Waveform = fip["Readout"]["Waveform"].array(entrystart=entry,entrystop=entry+1,flatten=True)
    WindowSize = int(len(Waveform)/len(ChannelID))
    Waveform=Waveform.reshape(len(ChannelID),WindowSize)
    instance = list(ChannelID).index(ChannelId)
    return Waveform[instance]


