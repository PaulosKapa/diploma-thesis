import wfdb
import os
import csv
from dotenv import load_dotenv
from scipy.signal import resample_poly

load_dotenv()
data_dir = os.getenv("DATA_PATH")
#the sampling rate is 300 Hz and we want to change it to 360Hz. We want a 10 second sample
target_fs = 360 
window_samples = target_fs * 10 

# Load the CinC 2017 labels
labels_dict = {}

#open the REFERENCE file to know how many records there are
reference_path = os.path.join(data_dir, 'REFERENCE.csv')
with open(reference_path, 'r') as ref_file:
    #create a lookup table saved in the memory
    reader = csv.reader(ref_file)
    for row in reader:
        if len(row) == 2:
            labels_dict[row[0]] = row[1]

record_names = list(labels_dict.keys())


#open a csv file
with open("afib_dataset.csv", "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    #write a header and make a column for each sample in the sample window and the label for this sample
    header = ["Patient_ID"] + [str(i) for i in range(0, window_samples)] + ["Label"]
    writer.writerow(header)

    for name in record_names:
        try:
            #get the data from all the files
            record = wfdb.rdrecord(os.path.join(data_dir, name), channels=[0])
        except Exception as e:
            print(f"Skipped {name}: {e}")
            continue
        #takes the ecg data and turns them to 1-array
        signal = record.p_signal.flatten()
        overall_label = labels_dict[name]
        #if we find afib, then label as 1. Else label as 0
        if overall_label == 'A':
            label = 1
        elif overall_label in ['N', 'O']:
            label = 0
        else:
            continue

        #unsample the signal from 300Hz to 360 Hz
        signal_360 = resample_poly(signal, up=6, down=5)

        #slice the signal every 10s, so we get a dataset with 3600 values
        #this is done for each patient seperately. If a patient has a signal that is less than 10s, they don't get included.If they have a signal that isn\t
        #divided by 10, the leftover data aren't getting included
        sample_ind = 0
        for start in range(0, len(signal_360) - window_samples + 1, window_samples):
            end = start + window_samples
            window_data = signal_360[start:end]            
            row = [name] + window_data.tolist() + [label]
            writer.writerow(row)
            sample_ind += 1
            
        print(name + " processed")

print("Completion")
