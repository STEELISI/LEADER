#==============================================================#
# This script classifies an instance of a connection           #
#==============================================================#
import sys
import math
import pickle
import numpy as np
import pandas as pd
from sklearn import svm
from sklearn import preprocessing
from sklearn.covariance import EllipticEnvelope
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report, confusion_matrix, accuracy_score

#================================#
# Load the model                 #
#================================#
model = pickle.load(open("./elliptic_envelope.mlmodel", 'rb'))
n_features = 37

#===============================#
# Input connection string       #
#===============================#
#conn = sys.argv[1]
#feature_values = conn.split(",")
conn = "1198897,9,51,13,22,0,10043209,7,7,16,14,32,850,459,17,8,59,5,3,1,3,1,2,0,2,1,1,1,1,4,4,2,1,1,1,1,1"
feature_values = conn.split(",")
if(len(feature_values) == n_features):
    feature_values = [int(i) for i in feature_values]
    df = pd.DataFrame(np.array(feature_values).reshape(1,n_features))
    feature_values = df.values
    output = model.predict(feature_values)
    print(output)
