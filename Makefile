# Dependency: JSAP, python3, scipy, h5py.
# ConvertTruth.py is for simulations.
# Convert.py is for real data.

JSAPSYS:=/opt/gentoo/usr/share/JSAP

.PHONY: first zinc alpha mix
mul=$(shell seq 0 99)

ab: ab-0.h5 ab-7.h5 ab-8.h5 ab-9.h5
zinc: zincm-ans.h5 zincm-problem.h5 $(mul:%=ztraining-%.h5)
first: first-problem.h5 $(mul:%=ftraining-%.h5)
alpha: alpha-ans.h5 alpha-problem.h5 $(mul:%=atraining-%.h5)

ztrain : $(mul:%=h5/ztraining-%.h5)

$(mul:%=ftraining-%.mac): %: first.mac.in
	sed 's,@seeds@,$(shell apg -M n -a 1 -n 2),' $^ > $@
$(mul:%=mac/ztraining-%.mac): %: zinc.mac.in
	sed 's,@seeds@,$(shell apg -M n -a 1 -n 2),' $^ > $@
$(mul:%=alpha-%.mac): %: alpha.mac.in
	sed 's,@seeds@,$(shell apg -M n -a 1 -n 2),' $^ > $@
$(mul:%=beta-%.mac): %: beta.mac.in
	sed 's,@seeds@,$(shell apg -M n -a 1 -n 2),' $^ > $@

root/%.root: mac/%.mac
	JPSim -t OFF -g /opt/gentoo/usr/share/JSAP/DetectorStructure/1t -m $^ -o $@ > $@.log 2>&1
h5/%.h5: root/%.root
	python3 -u ConvertTruth.py root/$* $@ > $@.log 2>&1

ab-%.h5: alpha-%.root beta-%.root
	python3 -u ConvertTruth.py --alpha $< --beta $(word 2,$^) -o $@ > $@.log 2>&1

zreal.h5: /srv/JinpingData/Jinping_1ton_Data/01_RawData/run00000893/Jinping_1ton_Phy_20180722_00000893.root
	python3 1tPrototype/Converter.py 893 --limit 20 -o $@

zexample.h5: /srv/JinpingData/Jinping_1ton_Data/01_RawData/run00000896/Jinping_1ton_Phy_20180723_00000896.root
	python3 1tPrototype/Converter.py 896 --limit 40 -o $@

zincm.h5: zinc.h5 zreal.h5
	python3 1tPrototype/mix.py $< -r $(word 2,$^) -o $@

%-problem.h5: %.h5
	cp $^ $@
	python3 -c 'import h5py; del h5py.File("$@")["GroundTruth"]'
	h5repack -i $@ -o $@-tmp
	mv -f $@-tmp $@
%-ans.h5: %.h5
	cp $^ $@
	python3 -c 'import h5py; del h5py.File("$@")["Waveform"]'
	h5repack -i $@ -o $@-tmp
	mv -f $@-tmp $@
%-submission.h5: %.h5
	cp $^ $@
	python3 example-submission.py $@
%-submission-2.h5: %.h5
	cp $^ $@
	python3 example-submission.py $@
%-submission-3.h5: %.h5
	cp $^ $@
	python3 example-submission.py $@

.SECONDEXPANSION:
Jinping-%.h5: $(wildcard /home/jinping/JinpingData/Jinping_1ton_Data/01_RawData/run00000%/Jinping_1ton_Phy_*_00000%.root)
	python3 1tPrototype/Converter.py $* --limit 1 -o $@

# Delete partial files when the processes are killed.
.DELETE_ON_ERROR:
# Keep intermediate files around
.SECONDARY:
