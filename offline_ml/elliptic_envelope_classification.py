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
n_features = 7

#===============================#
# Input connection string       #
#===============================#
#conn = sys.argv[1]
#feature_values = conn.split(",")
#conn = "1198897,9,51,13,22,0,10043209,7,7,16,14,32,850,459,17,8,59,5,3,1,3,1,2,0,2,1,1,1,1,4,4,2,1,1,1,1,1"
#conn = "0,0,0,0,0,0,0,4,0,0,1043,1373,0,0,0,0,0,0,0,0,0,0,1,1,0,0,18,18,0,0,0,0,1"
#conn = "0,0,0,0,0,0,0,2,0,0,1746,14789,0,0,0,0,0,0,0,0,0,0,1,1,0,0,18,18,0,0,0,0,1"
#conn = "6,39,0,0,0,0,0,3,0,0,4,31,0,0,22,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,1,0,1"
#conn = "0,0,799,2291,0,0,1006,16,-1"
#conn = "0,1007,1235,0,18,18,1"
conn = "0,1234,4649,0,3,3,1"
#if(sys.argv[1] == 0):
if(len(sys.argv) == 1):
    feature_values = conn.split(",")
    if(len(feature_values) == n_features):
        feature_values = [int(i) for i in feature_values]
        df = pd.DataFrame(np.array(feature_values).reshape(1,n_features))
        feature_values = df.values
        output = model.predict(feature_values)
        print(output)

else:
    leg_count = 0
    att_count = 0
    file_to_classify = sys.argv[1]
    with open(file_to_classify,'r') as fp:
        for line in fp:
            line = line.strip()
            if("class" in line):
                continue
            conn = line
            feature_values = conn.split(",")
            if(len(feature_values) == n_features):
                feature_values = [int(i) for i in feature_values]
                df = pd.DataFrame(np.array(feature_values).reshape(1,n_features))
                feature_values = df.values
                output = model.predict(feature_values)
                if(output[0] == 1):
                    leg_count = leg_count + 1
                else:
                    att_count = att_count + 1

                print("Classified connection " + line + " as " + str(output[0]))
    print("Leg Count = " + str(leg_count))
    print("Att Count = " + str(att_count))
