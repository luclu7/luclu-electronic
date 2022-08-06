from serial import *
import raildriver
import argparse
import os
import configparser
import json
import threading

parser = argparse.ArgumentParser()
parser.add_argument("comport", help="selects which COM port to use to speak to the Arduino", default="COM46")
args = parser.parse_args()

rd = raildriver.RailDriver()
rd.set_rail_driver_connected(True)  # start data exchange
loco_name = rd.get_loco_name()
print(loco_name)

config = configparser.ConfigParser()
config.read('config.ini')
try:
    keys = config[loco_name[1]]
except TypeError as e:
    print(e)
    print("Is train Simulator running correctly?")
    input('Press Enter to exit')
    os._exit(1)


class Object:
    def toJSON(self):
        return json.dumps(self, default=lambda o: o.__dict__)

# debug
if False:
    while True:
        print("Force_control: " + str(
            rd.get_current_controller_value("Force_control") + " BrakePipePressureBAR: " + str(
            rd.get_current_controller_value("BrakePipePressureBAR")) + " TrainBrakeCylinderPressureBAR: " + str(
            rd.get_current_controller_value("TrainBrakeCylinderPressureBAR"))))
        time.sleep(1 / 2)

port_serie = Serial(port=args.comport, baudrate=115200, timeout=1, writeTimeout=1)


def readInput():
    if port_serie.isOpen():
        oldline = ""
        while True:
            ligne = port_serie.readline().rstrip()
            if ligne != oldline:
                try:
                    args = json.loads(ligne)
                    print(args)
                    textToPrint = ""
                    # textToPrint += "Regulator: " + args[0].decode("utf-8")
                    # textToPrint += "Reverser: " + args[1].decode("utf-8")
                    # textToPrint += " VACMA: " + args[2].decode("utf-8")

                    # print(textToPrint)
                    # rd.set_controller_value(keys["Regulator"], float(args[0]))
                    # rd.set_controller_value(keys["Reverser"], float(args[1]))
                    # rd.set_controller_value(keys["VACMA"], float(args[2].decode("utf-8")))

                    rd.set_controller_value(keys["BPSF"], float(args["SF"]))
                    rd.set_controller_value(keys["TEST"], float(args["TEST"]))
                    rd.set_controller_value(keys["BPFC"], float(args["FC"]))
                    # rd.set_controller_value(keys["BPMV"], float(args[6].decode("utf-8")))
                    rd.set_controller_value(keys["BPVAL"], float(args["VAL"]))
                    rd.set_controller_value(keys["BPSF"], float(args["SF"]))
                    #rd.set_controller_value(keys["FA"], float(args["FA"]))
                    #rd.set_controller_value(keys["GD"], float(args["BPGD"]))
                except ValueError as e:
                    print("ValueError", str(e), ligne)
                except IndexError as e:
                    print("IndexError")
                    print(e)
                oldline = ligne


def writeOutput():
    lastDataSent = ""
    if port_serie.isOpen():
        while True:
            dataToBeSent = Object()
            dataToBeSent.visu = str(int(rd.get_current_controller_value("KVB_visu_control")))
            dataToBeSent.autotest = str(int(rd.get_current_controller_value("Autotest_KVB_control")))
            dataToBeSent.LSFU = str(int(rd.get_current_controller_value("KVB_LS_FU_control")))
            dataToBeSent.LSV = str(int(rd.get_current_controller_value("KVB_LS_V_control")))
            dataToBeSent.LSV = str(int(rd.get_current_controller_value("KVB_LS_V_control")))
            dataToBeSent.FC = str(int(rd.get_current_controller_value("KVB_BP_CAR_lumiere_control")))
            dataToBeSent.VAL = str(int(rd.get_current_controller_value("KVB_BP_VAL_lumiere_control")))
            dataToBeSent.LSSF = str(int(rd.get_current_controller_value("KVB_LS_SF_control")))


            data = dataToBeSent.toJSON()
            # {"visu":7, "autotest":4,"LSFU":1,"ENGIN":1, "LSV": 1, "SOL": 1}
            if lastDataSent != data:
                print(data)
                port_serie.write(str.encode(data + "\n"))
                lastDataSent = data


if loco_name is None:
    print("Can't find any loco")
    os._exit(1)

threading.Thread(target=readInput).start()
threading.Thread(target=writeOutput).start()
