import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.utils import class_weight
from keras import layers, models, Input
import numpy as np
from sklearn.metrics import classification_report, confusion_matrix

#open the csv file
df = pd.read_csv("afid_normalized.csv")

#find the patients
unique_patients = df['Patient_ID'].unique()

#seperating between train and test data, using the patient id
train_ids, test_ids = train_test_split(unique_patients, test_size=0.2, random_state = 10)

#create the training and testing datasets based on the data we seperated above
train_df = df[df['Patient_ID'].isin(train_ids)]
test_df = df[df['Patient_ID'].isin(test_ids)]

#seperate between the values (x) and the labels (y). The values don't include the patient id and label
X_train = train_df.drop(columns=['Patient_ID', 'Label']).values
y_train = train_df['Label'].values

#the same for the training dataset
X_test = test_df.drop(columns=['Patient_ID', 'Label']).values
y_test = test_df['Label'].values

#change from 2d to 3d array for 1D-CNN. We get the number of the sample and the number of the sample window.
#The new array will contain sample arrays that contain sample window arrays, each with 1 element.
X_train = X_train.reshape(X_train.shape[0], X_train.shape[1], 1)
X_test = X_test.reshape(X_test.shape[0], X_test.shape[1], 1)

#set the weights so the model prioritizes the minority class (the afib)
weights = class_weight.compute_class_weight(
    class_weight='balanced',
    classes=np.unique(y_train),
    y=y_train
)
class_weights_dict = dict(enumerate(weights))

#building the model
model = models.Sequential([
    Input(
        #what the algorithm should expect. 3600 samples, from 1 signal
        shape=(3600, 1)),    
        #first training block
        layers.Conv1D(16, 32, activation='relu'),
        layers.MaxPooling1D(4),
        #second training block
        layers.Conv1D(32, 16, activation='relu'),
        layers.MaxPooling1D(4),
        # #third training block
        # layers.Conv1D(64, 8, activation='relu'),
        # layers.MaxPooling1D(4),
        layers.GlobalAveragePooling1D(),   
        #validation block
        layers.Dense(32, activation='relu'),
        layers.Dropout(0.5),
        layers.Dense(1, activation='sigmoid')
])

#configure the options of the model for training
#adam: best for 1d-cnn,  binary_crossentropy: good for true false classification, accuracy: good for classification tasks
model.compile(optimizer='adam', 
              loss='binary_crossentropy', 
              metrics=['accuracy'])
#train the model for 10 epochs
history = model.fit(X_train,
                    y_train, 
                    epochs=10,
                    validation_data=(X_test, y_test),
                    class_weight=class_weights_dict)

#predicting the outcomes from the dataset. 
#used to classify the model's abilities
predictions = np.round(model.predict(X_test))
#confusion matrix
cm = confusion_matrix(y_test, predictions)
print("True Negatives: " + cm[0][0])
print("False Positives: " + cm[0][1])
print("False Negatives: " + cm[1][0])
print("True Positives: " + cm[1][1])
#medical metrics
print(classification_report(y_test, predictions, target_names=["Normal (0)", "AFib (1)"]))

#save the model
model.save("afib_cnn_model.keras")
print("END")
