from flask import * #Flask, render_template, request, flash, redirect
import sqlite3

import csv
import requests


import warnings
warnings.filterwarnings('ignore')

import pickle
model = pickle.load(open('new.pkl', 'rb'))

from twilio.rest import Client



app = Flask(__name__)
app.secret_key ="1234567890"

@app.route('/')
def home():
    return render_template('home.html')

@app.route('/index')
def index():
    return render_template('index.html')
@app.route('/aboutus')
def aboutus():
    return render_template('aboutus.html')

@app.route('/userlog', methods=['GET', 'POST'])
def userlog():
    if request.method == 'POST':

        connection = sqlite3.connect('user_data.db')
        cursor = connection.cursor()

        name = request.form['name']
        password = request.form['password']
        phone=request.form['mobile']

        query = "SELECT * FROM user WHERE name = '"+name+"' AND password= '"+password+"'"
        cursor.execute(query)

        result = cursor.fetchall()

        if result:
            data = requests.get("https://api.thingspeak.com/channels/2798296/feeds.json?results=2")
            data_json = data.json()
            hb = data_json['feeds'][-1]['field1']
            temp = data_json['feeds'][-1]['field2']
            oxy = data_json['feeds'][-1]['field3']
            bp = data_json['feeds'][-1]['field4']
            session['phone']=phone
            session['name']=name
            return render_template('logged.html',hb=hb,temp=temp,oxy=oxy,bp=bp)
        else:
            return render_template('index.html', msg='Sorry , Incorrect Credentials Provided,  Try Again')
    return render_template('index.html')


@app.route('/userreg', methods=['GET', 'POST'])
def userreg():
    if request.method == 'POST':

        connection = sqlite3.connect('user_data.db')
        cursor = connection.cursor()

        name = request.form['name']
        password = request.form['password']
        age = request.form['age']
        email = request.form['email']
        
        print(name, age, email, password)

        command = """CREATE TABLE IF NOT EXISTS user(name TEXT, password TEXT, age TEXT, email TEXT)"""
        cursor.execute(command)

        cursor.execute("INSERT INTO user VALUES ('"+name+"', '"+password+"', '"+age+"', '"+email+"')")
        connection.commit()

        return render_template('index.html', msg='Successfully Registered')
    
    return render_template('index.html')

@app.route('/logout')
def logout():
    return render_template('index.html')


@app.route("/kidneyPage")
def kidneyPage():
    data = requests.get("https://api.thingspeak.com/channels/2798296/feeds.json?results=2")
    data_json = data.json()
    hb = data_json['feeds'][-1]['field1']
    temp = data_json['feeds'][-1]['field2']
    oxy = data_json['feeds'][-1]['field3']
    bp = data_json['feeds'][-1]['field4']
    return render_template('logged.html',hb=hb,temp=temp,oxy=oxy,bp=bp)


@app.route("/predictPage", methods = ['POST', 'GET'])
def predictPage():
    bp = request.form['bp']
    oxy = request.form['oxy']
    hb = request.form['heart']
    Temperature = request.form['Temperature']
 
    print(bp,oxy,hb,Temperature)
    result = model.predict([[hb,oxy,bp]])[0]
    name=request.form['name'] 
    Age=request.form['age'] 
    
    number =  request.form['phone']

    msg="Women Name  :  "+str(name)+ "\n Age  :  "+str(Age)+  "\n Status  :  "+str(result)+" \n LOCATION:  https://maps.app.goo.gl/9nmGCjNPdjMo2pU66"
    link="https://maps.app.goo.gl/9nmGCjNPdjMo2pU66"
    print(number)
    if result =="Fright":
        account_sid ="ACbbfc5571014a2b87180aedaafbf36fe8"
        auth_token = "803ff93aab3595228701ff4ef390328b"
        client = Client(account_sid, auth_token)
        client.api.account.messages.create(
            to="+919035599155",
            from_="+13253089089" ,
            body=msg)
        print('sms1')

##        account_sid = "ACb924579b5b4cd39254940ba44f9ea35b"
##        auth_token = "e07b7d6df46056642b392fd98610d910"
##        client = Client(account_sid, auth_token)
##        client.api.account.messages.create(
##            to="+918147233691",
##            from_="+15077055792" ,
##            body=msg)
##        print('sms2')
##
##        account_sid = "AC68cf7853df11e8893c445a8ff4a5120d"
##        auth_token = "012281b5c525addfc8426c6f8ad3db40"
##        client = Client(account_sid, auth_token)
##        client.api.account.messages.create(
##            to="+918762440638",
##            from_="+15677575410" ,
##            body=msg)
##        print('sms3')
##
##        account_sid = "AC9cf84dbb862b7d5d9333a64e90dad489"
##        auth_token = "d510eaa637e4e2ac9db46faab06ff0e3"
##        client = Client(account_sid, auth_token)
##        client.api.account.messages.create(
##            to="+919036132094",
##            from_="+15672352593" ,
##            body=msg)
##        print('sms4')

    s_data=[name,Age,bp,oxy,hb,Temperature,result]
    csv_file = 'Log.csv'
    import os
    file_exists = os.path.isfile(csv_file)

    columns = ['Name','Age','BP', 'SPO2', 'HB', 'Temperature', 'result']

    with open(csv_file, mode='a', newline='') as file:
        writer = csv.writer(file)
        if not file_exists:
            writer.writerow(columns)
        writer.writerow(s_data)

    print("Data stored successfully in CSV file.")

    
    # out=output
    print(result)
    return render_template('predict.html', result = ["Patient  :  "+str(name), "Age  :  "+str(Age), "Status  :  "+str(result)], name =name,link=link)  #,out=out)

    # return render_template('logged.html')




if __name__ == '__main__':
	app.run(debug = True, use_reloader=False)
