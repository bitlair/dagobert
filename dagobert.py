#!/usr/bin/python3

from serial.tools import list_ports
import serial
import io
import time
import subprocess
import csv
import getopt
import sys
import os
import math
import paramiko
from pathlib import Path
import configparser
import re
import threading
import syslog

users = {}

def read_configuration(configdir):
    if (configdir == ''):
        print("Missing configdir.")
        sys.exit(2)

    if not os.path.exists(configdir):
        print("Directory ",  configdir, " does not exist");
        sys.exit(2)

    configfile = Path(os.path.join(configdir, 'settings'))
    if not configfile.is_file():
        print(configfile, " does not exist")
        sys.exit(2)

    config = configparser.ConfigParser()
    config.readfp(configfile.open())

    expected_config_options = { 'mqtt': [ 'temperature.subject', 'water.subject', 'server', 'laseractive.subject', 'mqtt-simple' ],
                                'serial': [ 'device' ],
                                'revbank': [ 'host', 'username', 'target' ],
                                'git': [ 'git' ] }

    for section, options in expected_config_options.items():
        for option in options:
            if not config.has_option(section, option):
                print("Missing config option ", option, "in section", section)
                sys.exit(2)

    return config

def mqtt_thread(config, subject, value, persistent):
    if persistent:
        subprocess.call([config.get('mqtt', 'mqtt-simple'), "-h", config.get('mqtt', 'server'), "-r", "-p", subject, "-m", value])
    else:
        subprocess.call([config.get('mqtt', 'mqtt-simple'), "-h", config.get('mqtt', 'server'), "-p", subject, "-m", value])

def mqtt(config, subject, value, persistent=False):
    threading.Thread(target = mqtt_thread, args = (config, subject, value, persistent)).start()

def log(message):
    print("LOG " + message)
    syslog.syslog(message)

def read_userfile(configdir):
    users = {}

    userfile = Path(os.path.join(configdir, 'users.csv'))
    if not userfile.is_file():
        log(str(userfile) + " does not exist")
        return False

    try:
        with userfile.open() as csvfile:
            spamreader = csv.reader(csvfile)
            for row in spamreader:
                if len(row) == 4:
                    users[row[0]] = { 'maintenance': row[1] == "true", 'price': row[2], 'revbank': row[3] }
                else:
                    log("Wrong number of columns in row")

        return users
    except IndexError:
        log("Malformed user file")
        return False

def git_update(git_binary, config_dir):
    global users

    log("Updating git")
    subprocess.call([git_binary, "--work-tree=" + config_dir, "pull"])

    new_users = read_userfile(config_dir)
    if not new_users:
        log("Unable to update users after git update")
    else:
        users = new_users

def payment_thread(config, user, time):
    minutes = math.ceil(time / 60000.0)
    cents = minutes * int(user['price'])
    price = "{:.2f}".format(cents / 100.0)

    ssh = paramiko.SSHClient()
    ssh.load_system_host_keys()
    ssh.connect(config.get('revbank', 'host'), username=config.get('revbank', 'username'))

    stdin, stdout, stderr = ssh.exec_command("give " + config.get('revbank', 'target')
        + " " + price + " " + str(minutes) + "_minuten " + user['revbank'])

    result = stdout.read().decode("utf-8")

    match = re.search('\(transaction ID = (.+?)\)', result)
    if match:
        log("Deducted " + price + " from " + user['revbank'] + " (transaction " + match.group(1) + ")")
    else:
        log("Failed to deduct " + price + " from " + user['revbank'])

def payment(config, user, time):
    threading.Thread(target = payment_thread, args = (config, user, time)).start()

def main(argv):
    global users

    configdir = ''

    syslog.openlog('dagobert')

    try:
        opts, args = getopt.getopt(argv,"c:",["config="])
    except getopt.GetoptError:
        print('dagobert.py -c <configdir>')
        sys.exit(2)

    for opt, arg in opts:
        if opt == "-c" or opt == "--config":
            configdir = arg

    config = read_configuration(configdir)

    users = read_userfile(configdir)
    if not users:
        print("Error reading user file")
        sys.exit(2)

    while True:
        try:
            cdc = next(list_ports.grep(config.get('serial', 'device')))
            ser = serial.Serial(cdc[0])

            time.sleep(2);
            ser.write(b"R\n");

            while True:
                data = ser.readline()
                action = chr(data[0])
                value = data[1:-2].decode("iso-8859-1")

                if action == 'B':
                    print("Arduino ready")
                    ser.write(b"R\n")
                elif action == 'A':
                    print("Authenticating", value)
                    try:
                        if users[value]:
                            ser.write(b"U");
                            if users[value]['maintenance']:
                                ser.write(b'T')
                            else:
                                ser.write(b'F')

                            ser.write(users[value]['price'].encode('utf-8'))
                            ser.write(users[value]['revbank'].encode('utf-8'))
                            ser.write(b"\n")

                            log("Laser unlock by " + users[value]['revbank'] + " (" + value + ")")

                            active_user = users[value]
                    except KeyError:
                        log("Laser unlock attempt by unknown iButton " + value)
                        ser.write(b"FNiet gevonden\n");
                        threading.Thread(target = git_update, args = (config.get('git', 'git'), configdir)).start()

                elif action == 'W':
                    mqtt(config, config.get('mqtt','water.subject'), value, False);
                elif action == 'T':
                    mqtt(config, config.get('mqtt', 'temperature.subject'), value, False);
                elif action == 'S':
                    log("Laser job started for " + active_user['revbank'])
                    mqtt(config, config.get('mqtt', 'laseractive.subject'), '1', True)
                elif action == 'E':
                    log("Laser job finished for " + active_user['revbank'] + " in " + value + "ms")
                    mqtt(config, config.get('mqtt', 'laseractive.subject'), '0', True)
                    payment(config, active_user, float(value))
                elif action == 'M':
                    log("Maintenance state entered by " + active_user['revbank'])
                elif action == 'L':
                    log("Left maintenance state by " + active_user['revbank'])

        except serial.SerialException:
            print("Serial connection error")
            time.sleep(2)
        except StopIteration:
            print("No device found")
            time.sleep(2)

if __name__ == "__main__":
    main(sys.argv[1:])
