import pandas as pd
import numpy as np

#open the datasheet
df = pd.read_csv("afib_dataset.csv")
patient_ids = df['Patient_ID']
labels = df['Label']
#exclude the patient id and label colums
signals = df.drop(columns=['Patient_ID', 'Label']).values
#calculate mean and standard deviation
mean = np.mean(signals, axis=1, keepdims=True)
std = np.std(signals, axis=1, keepdims=True)
#if a value is zero set the standard deviation to 1, so it doesnt divide y zero afterwards
std[std == 0] = 1.0
#get the z-score normalization
normalized_signals = (signals - mean) / std
#make the columns again, with first being the patient number and last if there is afid or not
norm_df = pd.DataFrame(normalized_signals, columns=df.columns[1:-1])
norm_df.insert(0, 'Patient_ID', patient_ids)
norm_df['Label'] = labels
#export to csv
norm_df.to_csv("afid_normalized.csv", index=False)
