#!/usr/bin/python

"""
Scales Version: 1.0

Procedure to Run the simulator:
1. Install python 2.x or 3.x
2. Install the 'Serial' package/library (pyserial)
3. Edit the config file for the appropriate serial port and input CSV file
4. Use the desired mode: 'loop' mode will continuously play the data from the CSV file in a loop
5. Edit the input CSV file to have a correct mass
6. Execute the python file:
        Linux   : sudo ./pacific_scales_simulator.py
        Windows : Open the file in Python GUI and run execute (F5)


Known Issues:
1.  The mass info can be negative if the scales is not calibrated. The behavior of the
    serial data output from the pacific scales is different, depending on the number of output digits.
    example: "-12345" --> for a 5 digit number
             "- 1234" --> for a 4 digit number. Notice the space between '-' and the numbers
             "-  123" --> for a 3 digit number. Notice the space between '-' and the numbers
             "-   12" --> for a 2 digit number. Notice the space between '-' and the numbers

    This behavior is addressed in this simulator.

    The application in 4000 would detect the negative number and report it as ZERO the scales
    has not been calibrated.
"""

import configparser, csv, time, serial


def form_scales_pkt(scale_type, per_ch, mass):
    if(scale_type == 4):
        output_stream = "/\r\n"
        output_stream += ("A    : %6d Kg\r\n" % per_ch[0])
        output_stream += ("B    : %6d Kg\r\n" % per_ch[1])
        output_stream += ("C    : %6d Kg\r\n" % per_ch[2])
        output_stream += ("D    : %6d Kg\r\n" % per_ch[3])
        output_stream += ("TOTAL: %6d Kg\r\n" % mass)
        output_stream += "\\\r\n"
    elif(scale_type == 6):
        output_stream = "/\r\n"
        output_stream += ("A    : %6d Kg\r\n" % per_ch[0])
        output_stream += ("B    : %6d Kg\r\n" % per_ch[1])
        output_stream += ("C    : %6d Kg\r\n" % per_ch[2])
        output_stream += ("D    : %6d Kg\r\n" % per_ch[3])
        output_stream += ("E    : %6d Kg\r\n" % per_ch[4])
        output_stream += ("F    : %6d Kg\r\n" % per_ch[5])
        output_stream += ("TOTAL: %6d Kg\r\n" % mass)
        output_stream += "\\\r\n"

    return output_stream;

def serial_data_write(port, baud, timeout, buf):
    ser = serial.Serial()
    ser.port     = port
    ser.baudrate = baud
    ser.bytesize = serial.EIGHTBITS #number of bits per bytes
    ser.parity   = serial.PARITY_NONE #set parity check: no parity
    ser.stopbits = serial.STOPBITS_ONE #number of stop bits
    ser.timeout  = 1            #non-block read
    ser.xonxoff  = False     #disable software flow control

    try:
        ser.open()
    #except Exception(err): #serial.SerialException:
    except serial.SerialException as err:
        print("Error sending data through serial port: " + str(err))
        exit()

    if ser.isOpen():
        print("Serial port open for comms")
        try:
            #ser.flushInput()
            #ser.flushOutput()
            bytes_out = ser.write(buf)
            ser.flush()
            print("Write data:%i" % bytes_out)
            print("%s" % buf)

        #except Exception(e):
        except Exception as err2:
            print("Error communicating to serial: " + str(err2))
    else:
        print("Cannot open serial port")


    return;

def serialize_data_from_file(input_file, device, baud, periodicity, num_of_ch):
    with open(('./' + input_file.strip('\'\"'))) as inputfile:
       inputData = csv.reader(inputfile)
       inputData.__next__() # this is supposed to skip a row [.next() not exist]
       for row in inputData:
           total_mass = 0
           mass_per_ch = list()
           for ch_num in range(0, num_of_ch):
               mass_per_ch.append(int(row[ch_num]))
               total_mass += int(row[ch_num])

           # Create the packet to be transmitted over serial line
           #pkt_buf = form_scales_pkt(num_of_ch, mass_per_ch, total_mass)
           txt_buf = form_scales_pkt(num_of_ch, mass_per_ch, total_mass)
           txt_buf = "Dump a long line of crap"
           # send the packet over serial port, but this must be a byte array
           #NOTE: the CR/LF is sent as ascii \r\n instead of binary 0x0a, 0x0b
           pkt_buf = txt_buf.encode('ascii', errors='ignore')
           serial_data_write(device.strip('\'\"'), baud, 1, pkt_buf)
           #serial_data_write(device.strip('\'\"'), baud, periodicity, pkt_buf)

           time.sleep(periodicity)

    return;

def generate_packet(ser, input_file, periodicity, num_of_ch):
    with open(('./' + input_file.strip('\'\"'))) as inputfile:
       inputData = csv.reader(inputfile)
       inputData.__next__() # this is supposed to skip a row [.next() not exist]
       for row in inputData:
           total_mass = 0
           mass_per_ch = list()
           for ch_num in range(0, num_of_ch):
               mass_per_ch.append(int(row[ch_num]))
               total_mass += int(row[ch_num])

           # Create the packet to be transmitted over serial line
           #pkt_buf = form_scales_pkt(num_of_ch, mass_per_ch, total_mass)
           txt_buf = form_scales_pkt(num_of_ch, mass_per_ch, total_mass)
           #txt_buf = "Dump a long line of crap"
           # send the packet over serial port, but this must be a byte array
           #NOTE: the CR/LF is sent as ascii \r\n instead of binary 0x0a, 0x0b
           pkt_buf = txt_buf.encode('ascii', errors='ignore')

           serialise_packet(ser, pkt_buf)
           time.sleep(periodicity)
    return pkt_buf;

def serialise_packet(ser, pkt_buf):
    if ser.isOpen():
        try:
            #ser.flushInput()
            #ser.flushOutput()
            bytes_out = ser.write(pkt_buf)
            ser.flush()
            #print("Write data:%i" % bytes_out)
            print("%s" % pkt_buf)

        #except Exception(e):
        except Exception as err2:
            print("Error communicating to serial: " + str(err2))
    return;

def pacific_scale_sim():
    configParser = configparser.ConfigParser(inline_comment_prefixes = (";",))

    configFilePath = r'scales_simulator_config.cfg'
    configParser.read(configFilePath)

    serial_dev = configParser.get('global-config', 'dev')
    serial_baud = int(configParser.get('global-config', 'baud'))
    sim_mode = configParser.get('global-config', 'mode')
    sim_period = int(configParser.get('global-config', 'periodicity'))
    sim_type  = int(configParser.get('global-config', 'num-channels'))
    inFile    = configParser.get('global-config', 'mass-file')

    print("Serial Port          : %s" % serial_dev)
    print("Baud rate            : %d" % serial_baud)
    print("Simulator Mode       : %s" % sim_mode)
    print("Simulator Periodicity: %d" % sim_period)
    print("Number of channels   : %d" % sim_type)
    print("Input CSV File       : %s" % inFile)

    condition = True
    #open serial port
    ser = serial.Serial()
    ser.port     = serial_dev
    ser.baudrate = serial_baud
    ser.bytesize = serial.EIGHTBITS #number of bits per bytes
    ser.parity   = serial.PARITY_NONE #set parity check: no parity
    ser.stopbits = serial.STOPBITS_ONE #number of stop bits
    ser.timeout  = 1            #non-block read
    ser.xonxoff  = False     #disable software flow control

    try:
        ser.open()
        print("Serial port open for comms")
    except serial.SerialException as err:
        print("Error opening serial port: " + str(err))
        exit()

    while condition:
        generate_packet(ser, inFile, sim_period, sim_type)
        #serialize_data_from_file(inFile, serial_dev, serial_baud, sim_period, sim_type)
        if(sim_mode.strip('\'\"') != 'loop'):
            condition = False
    #close serial port
    ser.close();
    return;

pacific_scale_sim()
