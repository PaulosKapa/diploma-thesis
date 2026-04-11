import wfdb
import os
from dotenv import load_dotenv
import csv

load_dotenv()

data_dir = os.getenv("DATA_PATH")

#open the RECORDS file to know how many records there are
with open(os.path.join(data_dir, 'RECORDS'), 'r') as f:
    record_names = [line.strip() for line in f]

#the sampling rate is 360 Hz. We want a 10 secon sample
fs = 360 
window_samples = fs * 10

#open a csv file
with open("afib_dataset.csv", "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    #write a header and make a column for each sample in the sample window and the label for this sample
    header = ["Patient_ID"] + [str(i) for i in range(0, window_samples)] + ["Label"]
    writer.writerow(header)

    for name in record_names:
        #get the data from all the files
        record = wfdb.rdrecord((os.path.join(data_dir, name)), channels=[0])
    
        #only for the data that interest us (1 lead)
        if(record.sig_name == ['MLII']):
            #get the doctor's notes
            annotation =  wfdb.rdann((os.path.join(data_dir, name)), 'atr')
            #takes the ecg data and turns them to 1-array
            signal = record.p_signal.flatten()
            #get the sample and aux_note from the annotation files
            ann_samples = annotation.sample
            ann_notes = annotation.aux_note

            #to figure out where in the annotation sample window we are
            ann_idx = 0
            #to know if the sample window includes afib
            afib = False
            #to know which sample window we are in
            sample_ind = 0
            #looping through each patient, one sample window at a time
            for start in range(0, len(signal) - window_samples + 1, window_samples):
                end = start + window_samples

                notes_found = []
                while ann_idx < len(ann_samples) and ann_samples[ann_idx] < end:
                    #check if there is a doctor's note for this sample and strip any hidden chars
                    note = ann_notes[ann_idx].rstrip('\x00')
                    #if there is a note add it to the note_found list                    
                    if note != '':
                        notes_found.append(note)                            
                    ann_idx += 1
                #check if there is afib in the list
                if "(AFIB" in notes_found:
                    afib = True
                #if we find afib, then label as 1. Else label as 0
                if (afib == False):
                    label = 0
                else:
                    label = 1
                #if there are items on the list check for the last one. If it says anything else than afib, then flag it for the next sample window
                if len(notes_found) > 0:
                    if notes_found[-1] != "(AFIB":
                        afib = False
                    else:
                        afib = True

                #the data from the signal
                window_data = signal[start:end]
                #write the row (name of the patien, each sample to a row and if there is afib or not)
                row = [name] + window_data.tolist() + [label]
                writer.writerow(row)
                print(name+" "+str(sample_ind)+" end")
                sample_ind+=1
            print(name+ " end")
        print("end final")

