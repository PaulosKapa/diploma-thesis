from fastapi import APIRouter, Depends
from pydantic import BaseModel
import numpy as np
import tensorflow as tf

#start a router
router = APIRouter()
#load the model
MODEL = tf.keras.models.load_model('model/afib_cnn_model.keras')
#declare a class that will include the ecg data. It is a list with the window samples. See the ml/saveCsv.py and ml/normalization.py for more.
class ECGData(BaseModel):
    signal: list[float]
#declare the api endpoint
@router.post('/predict')
async def predict_heart(input_data: ECGData):
    #get the signal and convert it to a numpy array
    raw_signal = np.array(input_data.signal)
    #calculate mean and standard deviation
    mean = np.mean(raw_signal)
    std = np.std(raw_signal)
    #if a value is zero set the standard deviation to 1, so it doesnt divide y zero afterwards
    if (std == 0):
        std = 1.0
    #get the z-score normalization
    normalized_signal = (raw_signal - mean) / std
    #reshape the signal that we received to much the list that the model expects. See the ml files for more.
    processed_signal = normalized_signal.reshape(1, 3600, 1)
    #get the model prediction
    prediction = MODEL.predict(processed_signal)
    #since the model gives a float from 0 to 1 (sinoid, see the ml/cnn_train.py for more).
    output = float(prediction[0][0])
    #if the model returns more than .5 return afib, else normal (see the afformentioned files for more).
    return {
        "is_afib": 1 if output > 0.5 else 0,
        "probability": output
    }