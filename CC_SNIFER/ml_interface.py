import cffi, sys, traceback, pickle, os, string
import pandas as pd
import numpy as np
from sklearn import preprocessing
from sklearn.feature_extraction.text import CountVectorizer, TfidfVectorizer, HashingVectorizer, TfidfTransformer
from sklearn_deltatfidf import DeltaTfidfVectorizer
from sklearn.svm import SVC
from sklearn.model_selection import GridSearchCV
from sklearn.pipeline import Pipeline

ffi = cffi.FFI()
ffi.cdef(open('ml_interface.h').read())
noGCDict = {}

@ffi.callback("char *(char*, int)")
def get_prediction(buffer, buffer_len):

    try:
        data = ffi.buffer(buffer, buffer_len)[:]
        result = ffi.new('char []', data)

        #TODO - IF TRAIN MODE ##########SUPERVISED MODE :        
        traffic_type = "NORMAL" ################### For Trained - Supervised Learning
        f_extension = "_training_data.csv"
        f_name = traffic_type + f_extension
        if not os.path.isfile(f_name):
            f = open(f_name, 'w')
            headers = 'Unique Id, TTL Value, Sum N-Gram Vectorizer, Average N-Gram-Feature, Tfidf Delta, Feature Count,Traffic Type\n'
            f.write(headers)
            f.close
            print("Generating new file",f_name)

        try:

            ml_bytes,unique_id,ttl,id = ml_processing(data,traffic_type)

            try:
                randomizator(ml_bytes,unique_id,ttl,id,f_name,traffic_type)
            except:
                print('randomizator Error!')
                traceback.print_exc()
                result = ffi.new('char []', b"Failed")  ### New message 
                noGCDict[ffi.addressof(result)] = result
                return result

                
        except:
            print('API C_Input ml_processing!')
            traceback.print_exc()
            result = ffi.new('char []', b"Failed")  ### New message 
            noGCDict[ffi.addressof(result)] = result
            return result

        ############ Result Alert               

        result = ffi.new('char []', b"Pass")  ### New message 
        noGCDict[ffi.addressof(result)] = result
        return result

    except:
        return ffi.NULL
    finally:
        
        ffi.release(result)
        
# Release object so that Python can reclaim the memory.
@ffi.callback("int (char*)")
def release_object(ptr):
    try:
        del noGCDict[ptr]
        return 0
    except:

        return 1

def fill_api(ptr):
    global api
    api = ffi.cast("struct API*", ptr)
    api.get_prediction = get_prediction
    api.release_object = release_object

def ml_processing(data,traffic_type):
    n=2
    ml_bytes =[]
    try:
        data = data.split(b',')
        ml_packet = [f.decode(encoding='utf-8', errors='strict') for f in data]

        if len(ml_packet) <= 6: # TODO Fix MMS on C side
            print("ML Packet must be minimum 6 len: ", len(ml_packet) )
            print("\ Python ML_bytes.......", ml_bytes) 
            return
   
        noxoo =str(ml_packet[8]).replace('\x00','')
        ml_packet[8]=noxoo
        unique_id = hash(''.join([ml_packet[0],ml_packet[1],ml_packet[2],ml_packet[3],ml_packet[4],ml_packet[5]]))
        unique_id =str(unique_id).replace('-','')

        for i in range(0,len(ml_packet[8]),n):
            ml_bytes.append(ml_packet[8][i:i+n])
        del ml_packet[8]
        del ml_bytes[-1]
        ml_bytes = " ".join(str(item) for item in ml_bytes)
        ml_bytes = [ml_bytes]
        ttl = str(ml_packet[6])
        id =  str(ml_packet[7])
          
    except:
        print('Def_Preprocessing Terrible Error!')
        traceback.print_exc()

    return ml_bytes,unique_id,ttl,id

def randomizator(ml_bytes,unique_id,ttl,id,f_name,traffic_type):
    
    if len(ml_bytes[0]) < 6:
        print("Packet must be minimum 6 byte len to meet N-Gram _ TODO")
        print("\ Python ML_bytes.......", ml_bytes) 
        return
    try:
        victor1 = CountVectorizer(ngram_range=(3,6))
        X = victor1.fit_transform(ml_bytes)
        features_names = victor1.get_feature_names()
        deltaV = X.toarray()
        deltaV.astype(float)
        deltaV = np.sum(deltaV)

        victor2 = HashingVectorizer()
        X = victor2.fit_transform(features_names)
        hashingVectorizer = str(X.shape).split(',')
        hashingVectorizer = str(hashingVectorizer[0]).replace('(','')

        victor3 = TfidfVectorizer(ngram_range=(3,6),norm=None) 
        X = victor3.fit_transform(ml_bytes)
        features = victor3.get_feature_names()
        tfidfVectorizer_array=X.toarray()
        features = ', '.join(str(item) for item in features)
        features = [features]

        victor4 = DeltaTfidfVectorizer(lowercase=False)
        X = victor4.fit_transform(ml_bytes, features)
        deltaT = X.toarray()
        deltaT.astype(float)
        deltaT = np.sum(deltaT)

        new_string = str(unique_id)+","+ttl+","+str(deltaV)+","+str(int(deltaV)/int(hashingVectorizer))+","+str(deltaT)+","+hashingVectorizer+","+traffic_type
        print(id,"- New Record", new_string)

        write_to_file(new_string,f_name)
        
    except:
        print('Even More Terrible Error!')
        traceback.print_exc()


def write_to_file(new_string,f_name):
    try:
        file = open(f_name, 'a')
        file.write(new_string+"\n")
        file.close()
    except:
        print('File -Terrible Error!')
        traceback.print_exc()


