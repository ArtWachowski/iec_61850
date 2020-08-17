import cffi, sys, traceback, pickle, os, string
import pandas as pd
import numpy as np
#import nltk
#nltk.download('brown')
#nltk.download('punkt')
#from nltk.corpus import brown
from sklearn import preprocessing
from sklearn.feature_extraction.text import CountVectorizer, TfidfVectorizer, HashingVectorizer, TfidfTransformer
from sklearn_deltatfidf import DeltaTfidfVectorizer
from sklearn.svm import SVC
from sklearn.cluster import KMeans
from sklearn.pipeline import Pipeline
from io import StringIO

ffi = cffi.FFI()
ffi.cdef(open('ml_interface.h').read())
noGCDict = {}

@ffi.callback("char *(char*, int)")
def get_prediction(buffer, buffer_len):

   
    try:
        data = ffi.buffer(buffer, buffer_len)[:]
        result = ffi.new('char []', data)

        #TODO - IF TRAIN MODE ##########SUPERVISED MODE :        
        traffic_type = "LIVE" ################### For Trained - Supervised Learning
        f_extension = "_training_data.csv"
        f_name = traffic_type + f_extension
        if not os.path.isfile(f_name):
            f = open(f_name, 'w')
            headers = "Unique Id"+"\t"+"TTL Value"+"\t"+"Sum N-Gram Vectorizer"+"\t"+"Average N-Gram-Feature"+"\t"+"Tfidf Delta"+"\t"+"Feature Count"+"\t"+"N-GRAMS_3"+"\t"+"N-GRAMS_DELTA_3"+"\t"+"N-GRAMS_4"+"\t"+"N-GRAMS_DELTA_4"+"\t"+"N-GRAMS_5"+"\t"+"N-GRAMS_DELTA_5"+"\t"+"N-GRAMS_6"+"\t"+"N-GRAMS_DELTA_6"+"\t"+"Traffic Type"+"\n"
            f.write(headers)
            f.close
            print("Generating new file",f_name)

        try:

            ml_bytes,unique_id,ttl,id = ml_processing(data,traffic_type)

            try:
                aa = randomizator(ml_bytes,unique_id,ttl,id,f_name,traffic_type)
                #print("thissss ... aaaaaa",aa)
                sv = get_super(aa)
                print("SVM prediction result: ", sv)

            except:
                print('randomizator Error!')
                traceback.print_exc()
                result = ffi.new('char []', b"Failed")  ### New message 
                noGCDict[ffi.addressof(result)] = result
                return result


                
        except:
            print('API C_Input ml_processing!')
            #traceback.print_exc()
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

def get_super(data):

    TESTDATA = StringIO(data)
    df = pd.read_csv(TESTDATA, sep='\t',names=["Unique Id","TTL Value","Sum N-Gram Vectorizer","Average N-Gram-Feature","Tfidf Delta","Feature Count","N-GRAMS_3","N-GRAMS_DELTA_3","N-GRAMS_4","N-GRAMS_DELTA_4","N-GRAMS_5","N-GRAMS_DELTA_5","N-GRAMS_6","N-GRAMS_DELTA_6","Traffic Type"])

    df.drop('Unique Id', axis=1, inplace=True)
    df.drop('N-GRAMS_3', axis=1, inplace=True)
    df.drop('N-GRAMS_4', axis=1, inplace=True)
    df.drop('N-GRAMS_5', axis=1, inplace=True)
    df.drop('N-GRAMS_6', axis=1, inplace=True)
    X = df.drop('Traffic Type',axis=1)
    print(df.head(2))
    pickle_in = open('SVMlinearmodel','rb')
    svm = pickle.load(pickle_in)
    result = svm.predict(X)

    return result

def ml_processing(data,traffic_type):
    n=2
    ml_bytes =[]
    try:
        data = data.split(b',')
        ml_packet = [f.decode(encoding='utf-8', errors='strict') for f in data]

        if ml_packet[8] is None:
            return

        if len(ml_packet[8]) <= 6: # TODO Fix MMS on C side
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
        #traceback.print_exc()

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

        #NGRAMS ...............   
        ngram3 = CountVectorizer(ngram_range=(3,3))
        X3 = ngram3.fit_transform(ml_bytes)
        n3= ngram3.get_feature_names()
        n3= ''.join([str(elem) for elem in n3])
        
        #print("\...........", natural_language(n3))

        delta3 = X3.toarray()
        delta3.astype(float)
        d3 = np.sum(delta3)

        ngram4 = CountVectorizer(ngram_range=(4,4))
        X4 = ngram4.fit_transform(ml_bytes)
        n4= ngram4.get_feature_names()
        n4= ''.join([str(elem) for elem in n4])

        #print("\...........", natural_language(n4))


        delta4 = X4.toarray()
        delta4.astype(float)
        d4 = np.sum(delta4)

        ngram5 = CountVectorizer(ngram_range=(5,5))
        X5 = ngram5.fit_transform(ml_bytes)
        n5= ngram5.get_feature_names()
        n5= ''.join([str(elem) for elem in n5])

        #print("\...........", natural_language(n5))

        delta5 = X5.toarray()
        delta5.astype(float)
        d5 = np.sum(delta5)

        ngram6 = CountVectorizer(ngram_range=(6,6))
        X6 = ngram6.fit_transform(ml_bytes)
        n6= ngram6.get_feature_names()
        n6= ''.join([str(elem) for elem in n6])

        #print("\...........", natural_language(n6))


        delta6 = X6.toarray()
        delta6.astype(float)
        d6 = np.sum(delta6)

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

        global new_string
        ng=str(n3)+"\t"+str(d3)+"\t"+str(n4)+"\t"+str(d4)+"\t"+str(n5)+"\t"+str(d5)+"\t"+str(n6)+"\t"+str(d6)
        new_string=str(unique_id)+"\t"+ttl+"\t"+str(deltaV)+"\t"+str(int(deltaV)/int(hashingVectorizer))+"\t"+str(deltaT)+"\t"+hashingVectorizer+"\t"+ng+"\t"+traffic_type

        #print(id,"- New Record", new_string)

        write_to_file(new_string,f_name)
        
        
    except:
        print('Randomizer!') #Even More Terrible Error!')
        #traceback.print_exc()

    return new_string


def natural_language(raw):

    #ref: https://www.nltk.org/book/ch05.html#sec-dictionaries

    bs = brown.tagged_sents(categories='news') #### TODO Regex This 
    b = brown_sents = brown.sents(categories='news')
    tokens = nltk.word_tokenize(raw)
    #tagger = nltk.DefaultTagger('NN')
    tagger = nltk.UnigramTagger(bs)
    tagger.tag(tokens)

    ni=tagger.evaluate(bs)

    return ni ## Example Value 0.13089484257215028


def write_to_file(new_string,f_name):
    try:
        file = open(f_name, 'a')
        file.write(new_string+"\n")
        file.close()
    except:
        print('File -Terrible Error!')
        traceback.print_exc()


