import math
import pickle
import numpy as np
import pandas as pd
from sklearn import svm
from sklearn.svm import SVC
from sklearn import preprocessing
from sklearn.covariance import EllipticEnvelope
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report, confusion_matrix, accuracy_score
#======================================================#
#  Please provide the file training for training here  #
#======================================================#
data = pd.read_csv("data/final_data_6f.csv")
#=====================================================#
# Outlier Fraction needs to be specified here         #
#=====================================================#
outliers_fraction = 0.03

cols = data.columns
side_data = data.copy()
side_data.drop('class', 1, inplace=True)
 
side_data = pd.DataFrame(side_data, columns = cols[:-1])
side_data['class'] = data['class']
 
#data=np.array(side_data, dtype=float)
data = side_data

#train_data = data.as_matrix()
train_data = data.values
print(train_data.shape)
labels = np.asarray(data['class'])

# Feature names
features = data.columns.values[:-1].tolist()

for i in range(train_data.shape[0]):
    for j in range(train_data.shape[1]):
        print(train_data[i, j])
        if np.isinf(train_data[i, j]):
            train_data[i, j] = -1
        if (math.isnan(train_data[i, j])): 
            train_data[i, j] = -1
print (np.all(np.isfinite(train_data)))

labels[np.where(labels==0)] = -1
labels[np.where(labels==1)] = 1
np.unique(labels)
print(len(np.where(labels==1)[0]), len(np.where(labels==-1)[0]))
model = EllipticEnvelope(contamination=outliers_fraction)
model.fit(train_data)
print (accuracy_score(labels, model.predict(train_data)))
print (confusion_matrix(labels, model.predict(train_data)))
pickle.dump(model, open("elliptic_envelope.mlmodel", 'wb'))

#conn = "1987,9,51,13,22,0,10043209,7,7,16,14,32,850,459,17,8,59,5,3,1,3,1,2,0,2,1,1,1,1,4,4,2,1,1,1,1,1"
conn = "0,0,117,114,0,0,2,2,1,1"
#conn = sys.argv[1]
feature_values = conn.split(",")
if(len(feature_values) == 7):
    feature_values = [int(i) for i in feature_values]
    df = pd.DataFrame(np.array(feature_values).reshape(1,37))
    #feature_values = df.as_matrix()
    #feature_values = feature_values.as_matrix()
    #feature_values.reshape(1,36)
    #print(feature_values.shape)
    feature_values = df.values
    output = model.predict(feature_values)
    print("\n\n Prediction",output)

