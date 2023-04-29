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

if __name__ == '__main__':
    socketio.run(app, host="0.0.0.0", port=80, debug=True)    
