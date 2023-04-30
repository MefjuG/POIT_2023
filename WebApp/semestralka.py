import serial.tools.list_ports
import json
from threading import Lock
from flask import Flask, render_template, session, request, jsonify, url_for
from flask_socketio import SocketIO, emit, disconnect    
import serial
import re
import mysql.connector

ports = serial.tools.list_ports.comports()

for port, desc, hwid in sorted(ports):
    print("{}: {} [{}]".format(port, desc, hwid))


async_mode = None
app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, async_mode=async_mode)
thread = None
thread_lock = Lock() 

mysql_config = {
  'user': 'root',
  'password': 'heslo',
  'host': '192.168.56.102',
  'db': 'mysql',
  'raise_on_warnings': True
}

def saveDB(data):
    try:
        cnx = mysql.connector.connect(**mysql_config)
        cursor = cnx.cursor()

        query = "INSERT INTO semestralka_tabel (value) VALUES (%s)"
        data_str = json.dumps(data)
        value = (data_str,)

        cursor.execute(query, value)
        cnx.commit()

        print(f"Successfully inserted {data} into semestralka_tabel")
    except mysql.connector.Error as error:
        print(f"Error inserting {data} into semestralka_tabel: {error}")
    finally:
        if cnx.is_connected():
            cursor.close()
            cnx.close()

def readDB():
    try:
        cnx = mysql.connector.connect(**mysql_config)
        cursor = cnx.cursor()

        query = "SELECT value FROM semestralka_tabel"
        cursor.execute(query)

        results = []
        for row in cursor.fetchall():
            results.append(row[0])

        return results
    except mysql.connector.Error as error:
        print(f"Error reading from semestralka_tabel: {error}")
    finally:
        if cnx.is_connected():
            cursor.close()
            cnx.close()

def saveFile(data, filename):
    try:
        with open(filename, 'a') as f:
            f.write(json.dumps(data) + '\n')

        print(f"Successfully saved data to {filename}")
    except IOError as e:
        print(f"Error saving data to {filename}: {e}")

def readFile(filename):
    try:
        with open(filename, 'r') as f:
            data = f.read().splitlines()
        print(f"Successfully read data from {filename}")
        return data
    except IOError as e:
        print(f"Error reading data from {filename}: {e}")
        return None

def background_thread(args, ms=''):
    last = ""
    data = ""
    count = 0
    arduino = serial.Serial('/dev/ttyS0', baudrate=115200)
    if ms != '':
        if(ms and ms != last):
            arduino.write(ms.encode())
            last = ms
            print('EMIT CALL: ' + ms) 
    else:    
        while True:
            received_message = arduino.readline().decode();
            data_list = []
            print(received_message)
            if(received_message):
                count = count + 1
                try:
                    data = json.loads(received_message)
                    data_list.append(data)
                    print(data)
                    saveDB(data)
                    saveFile(data, "output_data")
                    json_object = readFile("output_data")
                    socketio.emit('my_response', {'data': json_object[-1], 'count': count }, namespace='/test')
                except json.JSONDecodeError:
                    pass   
        #arduino.close()            

if __name__ == '__main__':
    socketio.run(app, host="0.0.0.0", port=80, debug=True)    
