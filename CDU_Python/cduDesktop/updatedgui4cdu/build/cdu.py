
import serial
import threading
from pathlib import Path
import time

from tkinter import Tk, Canvas, Entry, Text, Button, PhotoImage


OUTPUT_PATH = Path(__file__).parent
ASSETS_PATH = OUTPUT_PATH / Path(r"F:\projects\CDU_FW\CDU_Python\cduDesktop\updatedgui4cdu\build\assets\frame0")


N_volume = 0
N_obs = 0
N_Active_freq = 0
N_standby_freq = 0
N_Active_tx_Status = 0 
N_standby_tx_Status = 0

C_volume = 0
C_squelch = 0
C_micgain = 0
C_sidetone = 0
C_Active_freq = 0
C_standby_freq = 0
C_Active_tx_Status = 0 
C_standby_tx_Status = 0

A_Active_freq = 0
A_standby_freq = 0

T_Active_freq = 0
T_standby_freq = 0

def relative_to_assets(path: str) -> Path:
    return ASSETS_PATH / Path(path)

# Initialize serial communication
def init_serial():
    try:
        ser = serial.Serial('COM3', 9600, timeout=1)  # Replace 'COM3' with your actual port
        return ser
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        return None

# Function to read data from the serial port
def read_serial_data(serial_connection, datalist):
    global N_volume, N_obs, N_Active_freq, N_standby_freq, N_Active_tx_Status, N_standby_tx_Status
    global C_volume, C_squelch, C_micgain, C_sidetone, C_Active_freq, C_standby_freq, C_Active_tx_Status, C_standby_tx_Status
    global A_Active_freq, A_standby_freq
    global T_Active_freq, T_standby_freq

    while True:
        if serial_connection and serial_connection.in_waiting > 0:
            data = serial_connection.readline().decode('latin-1').strip()
            if data:
                print(f"Data received: {data}")

                if data.startswith("$PATNV"):                   # for nav ....................................................................
                    if data[6] == '7' and data[7] == '3':
                        N_volume = (ord(data[8]) - 48)
                        print("Receive Volume Value: ", N_volume)

                    elif data[6] == '3' and data[7] == '4':
                        N_obs = data[8]+data[9]+data[10]
                        print("Receive OBS Value: ", N_obs) 

                    elif data[6] == '2' and data[7] == '7':
                        Active_Mhz = (ord(data[8]) + 48)
                        Active_khz = (ord(data[9]) - 48) * 25
                        N_Active_freq = Active_Mhz + (Active_khz)/1000
                        N_Active_tx_Status = data[10]
                        print("Receive Active Freq(MHz):", N_Active_freq,", with Tx status is", N_Active_tx_Status)  # Output: Flag is set to: True

                    elif data[6] == '2' and data[7] == '8':
                        standby_Mhz = (ord(data[8]) + 48)
                        standby_khz = (ord(data[9]) - 48) * 25
                        N_standby_freq = standby_Mhz + (standby_khz)/1000
                        N_standby_tx_Status = data[10]
                        print("Receive Standby Freq(MHz): ", N_standby_freq,", with Tx status is", N_standby_tx_Status)  # Output: Flag is set to: True
                
                elif data.startswith("$PATCV"):                 # for com ....................................................................
                    if data[6] == '7' and data[7] == '1':
                        C_volume = (ord(data[8]) - 48)
                        print("Receive Volume Value: ", C_volume)
                        C_squelch = (ord(data[9]) - 48)
                        print("Receive Volume Value: ", C_squelch)

                    elif data[6] == '7' and data[7] == '2':
                        C_micgain = (ord(data[8]) - 48)
                        print("Receive Volume Value: ", C_micgain)
                        C_sidetone = (ord(data[9]) - 48)
                        print("Receive Volume Value: ", C_sidetone)

                    elif data[6] == '4' and data[7] == '2':
                        Active_Mhz = (ord(data[8]) + 48)
                        Active_khz = (ord(data[9]) - 48) * 25
                        C_Active_freq = Active_Mhz + (Active_khz)/1000
                        C_Active_tx_Status = data[10]
                        print("Receive Active Freq(MHz):", C_Active_freq,", with Tx status is", C_Active_tx_Status)  # Output: Flag is set to: True

                    elif data[6] == '2' and data[7] == '9':
                        standby_Mhz = (ord(data[8]) + 48)
                        standby_khz = (ord(data[9]) - 48) * 25
                        C_standby_freq = standby_Mhz + (standby_khz)/1000
                        C_standby_tx_Status = data[10]
                        print("Receive Standby Freq(MHz): ", C_standby_freq,", with Tx status is", C_standby_tx_Status)  # Output: Flag is set to: True

# Function to send data over the serial port
def send_serial_data(serial_connection, data_to_send):
    if serial_connection:
        try:
            serial_connection.write(data_to_send.encode('utf-8'))
            print(f"Sent: {data_to_send}")
        except Exception as e:
            print(f"Error sending data: {e}")
    else:
        print("Serial connection is not open.")


# Start serial reading in a separate thread
def start_serial_read_thread(serial_connection, datalist):
    thread = threading.Thread(target=read_serial_data, args=(serial_connection, datalist))
    thread.daemon = True
    thread.start()

# for Nav data

N_prev_Active_freq= N_Active_freq
N_prev_Active_tx_Status= N_Active_tx_Status
N_prev_standby_freq= N_standby_freq
N_prev_standby_tx_Status= N_standby_tx_Status
N_prev_volume= N_volume
N_prev_obs = N_obs

N_old_Active_freq= N_Active_freq
N_old_Active_tx_Status= N_Active_tx_Status
N_old_standby_freq= N_standby_freq
N_old_standby_tx_Status= N_standby_tx_Status
N_old_volume= N_volume
N_old_obs = N_obs

# for Com data

C_prev_Active_freq= C_Active_freq
C_prev_Active_tx_Status= C_Active_tx_Status
C_prev_standby_freq= C_standby_freq
C_prev_standby_tx_Status= C_standby_tx_Status
C_prev_volume= C_volume
C_prev_squelch = C_squelch
C_prev_micgain = C_micgain
C_prev_sidetone = C_sidetone

C_old_Active_freq= C_Active_freq
C_old_Active_tx_Status= C_Active_tx_Status
C_old_standby_freq= C_standby_freq
C_old_standby_tx_Status= C_standby_tx_Status
C_old_volume= C_volume
C_old_squelch = C_squelch
C_old_micgain = C_micgain
C_old_sidetone = C_sidetone

# for ADF data

A_prev_Active_freq = A_Active_freq
A_prev_standby_freq = A_standby_freq

A_old_Active_freq = A_Active_freq
A_old_standby_freq = A_standby_freq

# for TACAN data

T_prev_Active_freq = T_Active_freq
T_prev_standby_freq = T_standby_freq

T_old_Active_freq = T_Active_freq
T_old_standby_freq = T_standby_freq

# Main GUI application
def main():
    # Initialize serial port
    serial_connection = init_serial()

    # Function to handle the button click
    def on_send_button_click(serial_connection):
        
        global N_volume, N_obs, N_Active_freq, N_standby_freq, N_Active_tx_Status, N_standby_tx_Status
        global C_volume, C_squelch, C_micgain, C_sidetone, C_Active_freq, C_standby_freq, C_Active_tx_Status, C_standby_tx_Status

        global N_old_Active_freq, N_old_Active_tx_Status, N_old_standby_freq, N_old_standby_tx_Status, N_old_volume, N_old_obs
        global C_old_volume, C_old_squelch, C_old_micgain, C_old_sidetone, C_old_Active_freq, C_old_standby_freq, C_old_Active_tx_Status, C_old_standby_tx_Status

        global A_Active_freq, A_standby_freq
        global A_old_Active_freq, A_old_standby_freq

        global T_Active_freq, T_standby_freq
        global T_old_Active_freq, T_old_standby_freq

        # for nav ..................................................................................................

        Active_freq_entry = entry_1.get()
        Active_tx_Status_entry = entry_6.get()
        if Active_freq_entry[0:7] != N_old_Active_freq or Active_tx_Status_entry[0:6] != N_old_Active_tx_Status:
            N_Active_freq = Active_freq_entry[0:7]
            N_Active_tx_Status = Active_tx_Status_entry[0:6]
            N_old_Active_freq = N_Active_freq
            N_old_Active_tx_Status = N_Active_tx_Status
            temp_str = str(N_Active_freq)
            try:
                Amhz, Akhz = temp_str.split('.')
            except ValueError:
                Amhz = temp_str 
                Akhz = '0'    
            amhz_ascii = chr(int(Amhz) - 48)
            akhz2 = Akhz+"00"
            Akhz_num = int(akhz2[:3])
            Amhz_num = int(Amhz)

            if Akhz_num >= 950:
                start_mhz = Amhz_num - 1
                start_khz = 875
            elif Akhz_num <= 200:
                start_mhz = Amhz_num - 1
                start_khz = Akhz_num + 100
            else:
                start_mhz = Amhz_num - 1
                start_khz = Akhz_num  - 100

            current_mhz = start_mhz
            current_khz = start_khz

            while (abs(current_mhz - Amhz_num) > 0) or (abs(current_khz - Akhz_num) > 24):
                current_khz -= 25  * int((current_khz - Akhz_num) / abs(current_khz - Akhz_num))
                if (current_mhz < Amhz_num):
                    current_mhz += 1
                amhz_ascii = chr(current_mhz - 48)
                akhz_ascii = chr((current_khz // 25) + 48)

                temp_message = "$PATNV27" + amhz_ascii + akhz_ascii + N_Active_tx_Status + '\r' + '\n'
                send_serial_data(serial_connection, temp_message)

                time.sleep(0.5)

 

        standby_freq_entry = entry_11.get()
        standby_tx_Status_entry = entry_9.get()
        if standby_freq_entry[0:7] != N_old_standby_freq or standby_tx_Status_entry[0:6] != N_old_standby_tx_Status:
            N_standby_freq = standby_freq_entry[0:7]
            N_standby_tx_Status = standby_tx_Status_entry[0:6]
            N_old_standby_freq = N_standby_freq
            N_old_standby_tx_Status = N_standby_tx_Status        
            temp_str = str(N_standby_freq)
            try:
                Smhz, Skhz = temp_str.split('.')
            except:
                Smhz = temp_str
                Skhz = '0'
            smhz_ascii = chr(int(Smhz) - 48)
            skhz2 = Skhz+"00"
            Skhz_num = int(skhz2[:3])
            Smhz_num = int(Smhz)

            if Skhz_num >= 950:
                start_mhz = Smhz_num - 1
                start_khz = 875
            elif Skhz_num <= 200:
                start_mhz = Smhz_num - 1
                start_khz = Skhz_num + 100
            else:
                start_mhz = Smhz_num - 1
                start_khz = Skhz_num - 100

            current_mhz = start_mhz
            current_khz = start_khz 

            while (abs(current_mhz - Smhz_num)) or (abs(current_khz - Skhz_num) > 24):
    
                current_khz -= 25  * int((current_khz - Skhz_num) / abs(current_khz - Skhz_num))

                if (current_mhz < Smhz_num):
                    current_mhz += 1

                smhz_ascii = chr(current_mhz - 48)
                skhz_ascii = chr((current_khz // 25) + 48)

                temp_message = "$PATNV28" + smhz_ascii + skhz_ascii + N_standby_tx_Status + '\r' + '\n'
                send_serial_data(serial_connection, temp_message)

                time.sleep(0.5)

        volume_entry = entry_22.get()
        if volume_entry[0:2] != N_old_volume:
            N_volume = volume_entry[0:2]
            N_old_volume = N_volume
            temp_message = "$PATNV73" + str(N_volume) +'\r' +'\n'
            send_serial_data(serial_connection, temp_message)   

        obs_entry = entry_23.get()
        if obs_entry[0:3] != N_old_obs:
            N_obs = obs_entry[0:3]
            N_old_obs = N_obs
            temp_message = "$PATNV34" + N_obs +'\r' +'\n'
            send_serial_data(serial_connection, temp_message)

        # for com ..................................................................................................

        C_Active_freq_entry = entry_10.get()
        C_Active_tx_Status_entry = entry_7.get()
        if C_Active_freq_entry[0:7] != C_old_Active_freq or C_Active_tx_Status_entry[0:6] != C_old_Active_tx_Status:
            C_Active_freq = C_Active_freq_entry[0:7]
            C_Active_tx_Status = C_Active_tx_Status_entry[0:6]
            C_old_Active_freq = C_Active_freq
            C_old_Active_tx_Status = C_Active_tx_Status
            temp_str = str(C_Active_freq)
            try:
                Amhz, Akhz = temp_str.split('.')
            except ValueError:
                Amhz = temp_str 
                Akhz = '0'    
            amhz_ascii = chr(int(Amhz) - 48)
            akhz2 = Akhz+"00"
            Akhz_num = int(akhz2[:3])
            Amhz_num = int(Amhz)

            if Akhz_num >= 950:
                start_mhz = Amhz_num - 1
                start_khz = 875
            elif Akhz_num <= 200:
                start_mhz = Amhz_num - 1
                start_khz = Akhz_num + 100
            else:
                start_mhz = Amhz_num - 1
                start_khz = Akhz_num  - 100

            current_mhz = start_mhz
            current_khz = start_khz

            while (abs(current_mhz - Amhz_num) > 0) or (abs(current_khz - Akhz_num) > 24):
                current_khz -= 25  * int((current_khz - Akhz_num) / abs(current_khz - Akhz_num))
                if (current_mhz < Amhz_num):
                    current_mhz += 1
                amhz_ascii = chr(current_mhz - 48)
                akhz_ascii = chr((current_khz // 25) + 48)

                temp_message = "$PATCV42" + amhz_ascii + akhz_ascii + C_Active_tx_Status + '\r' + '\n'
                send_serial_data(serial_connection, temp_message)

                time.sleep(0.5)

        C_standby_freq_entry = entry_12.get()
        C_standby_tx_Status_entry = entry_8.get()
        if C_standby_freq_entry[0:7] != C_old_standby_freq or C_standby_tx_Status_entry[0:6] != C_old_standby_tx_Status:
            C_standby_freq = C_standby_freq_entry[0:7]
            C_standby_tx_Status = C_standby_tx_Status_entry[0:6]
            C_old_standby_freq = C_standby_freq
            C_old_standby_tx_Status = C_standby_tx_Status        
            temp_str = str(C_standby_freq)
            try:
                Smhz, Skhz = temp_str.split('.')
            except:
                Smhz = temp_str
                Skhz = '0'
            smhz_ascii = chr(int(Smhz) - 48)
            skhz2 = Skhz+"00"
            Skhz_num = int(skhz2[:3])
            Smhz_num = int(Smhz)

            if Skhz_num >= 950:
                start_mhz = Smhz_num - 1
                start_khz = 875
            elif Skhz_num <= 200:
                start_mhz = Smhz_num - 1
                start_khz = Skhz_num + 100
            else:
                start_mhz = Smhz_num - 1
                start_khz = Skhz_num - 100

            current_mhz = start_mhz
            current_khz = start_khz 

            while (abs(current_mhz - Smhz_num)) or (abs(current_khz - Skhz_num) > 24):
    
                current_khz -= 25  * int((current_khz - Skhz_num) / abs(current_khz - Skhz_num))

                if (current_mhz < Smhz_num):
                    current_mhz += 1

                smhz_ascii = chr(current_mhz - 48)
                skhz_ascii = chr((current_khz // 25) + 48)

                temp_message = "$PATCV29" + smhz_ascii + skhz_ascii + C_standby_tx_Status + '\r' + '\n'
                send_serial_data(serial_connection, temp_message)

                time.sleep(0.5)

        C_volume_entry = entry_20.get()
        C_squelch_entry = entry_24.get()
        if C_volume_entry[0:2] != C_old_volume or C_squelch_entry[0:2] != C_old_squelch:
            C_volume = C_volume_entry[0:2]
            C_squelch = C_squelch_entry[0:2]
            C_old_volume = C_volume
            C_old_squelch = C_squelch
            temp_message = "$PATCV71" + str(C_volume) + str(C_squelch) +'\r' +'\n'
            send_serial_data(serial_connection, temp_message)

        C_micgain_entry = entry_21.get()
        C_sidetone_entry = entry_25.get()
        if C_micgain_entry[0:2] != C_old_micgain or C_sidetone_entry[0:2] != C_old_sidetone:
            C_micgain = C_micgain_entry[0:2]
            C_sidetone = C_sidetone_entry[0:2]
            C_old_micgain = C_micgain
            C_old_sidetone = C_sidetone
            temp_message = "$PATCV72" + str(C_micgain) + str(C_sidetone) +'\r' +'\n'
            send_serial_data(serial_connection, temp_message)

        # for ADF ..................................................................................................

        Active_freq_entry = entry_2.get()
        if Active_freq_entry[0:7] != A_old_Active_freq:
            A_Active_freq = Active_freq_entry[0:7]
            A_old_Active_freq = A_Active_freq
            temp_str = str(A_Active_freq)
            try:
                Amhz, Akhz = temp_str.split('.')
            except ValueError:
                Amhz = temp_str 
                Akhz = '0'    
            amhz_ascii = chr(int(Amhz) - 48)
            akhz2 = Akhz+"00"
            Akhz_num = int(akhz2[:3])
            Amhz_num = int(Amhz)

            if Akhz_num >= 950:
                start_mhz = Amhz_num - 1
                start_khz = 875
            elif Akhz_num <= 200:
                start_mhz = Amhz_num - 1
                start_khz = Akhz_num + 100
            else:
                start_mhz = Amhz_num - 1
                start_khz = Akhz_num  - 100

            current_mhz = start_mhz
            current_khz = start_khz

            while (abs(current_mhz - Amhz_num) > 0) or (abs(current_khz - Akhz_num) > 24):
                current_khz -= 25  * int((current_khz - Akhz_num) / abs(current_khz - Akhz_num))
                if (current_mhz < Amhz_num):
                    current_mhz += 1
                amhz_ascii = chr(current_mhz - 48)
                akhz_ascii = chr((current_khz // 25) + 48)

                temp_message = "$PATAV27" + amhz_ascii + akhz_ascii + '\r' + '\n'
                send_serial_data(serial_connection, temp_message)

                time.sleep(0.5)

        standby_freq_entry = entry_3.get()
        if standby_freq_entry[0:7] != A_old_standby_freq:
            A_standby_freq = standby_freq_entry[0:7]
            A_old_standby_freq = A_standby_freq      
            temp_str = str(A_standby_freq)
            try:
                Smhz, Skhz = temp_str.split('.')
            except:
                Smhz = temp_str
                Skhz = '0'
            smhz_ascii = chr(int(Smhz) - 48)
            skhz2 = Skhz+"00"
            Skhz_num = int(skhz2[:3])
            Smhz_num = int(Smhz)

            if Skhz_num >= 950:
                start_mhz = Smhz_num - 1
                start_khz = 875
            elif Skhz_num <= 200:
                start_mhz = Smhz_num - 1
                start_khz = Skhz_num + 100
            else:
                start_mhz = Smhz_num - 1
                start_khz = Skhz_num - 100

            current_mhz = start_mhz
            current_khz = start_khz 

            while (abs(current_mhz - Smhz_num)) or (abs(current_khz - Skhz_num) > 24):

                current_khz -= 25  * int((current_khz - Skhz_num) / abs(current_khz - Skhz_num))

                if (current_mhz < Smhz_num):
                    current_mhz += 1

                smhz_ascii = chr(current_mhz - 48)
                skhz_ascii = chr((current_khz // 25) + 48)

                temp_message = "$PATAV28" + smhz_ascii + skhz_ascii + '\r' + '\n'
                send_serial_data(serial_connection, temp_message)

                time.sleep(0.5)   

        # for TACAN ..................................................................................................

        Active_freq_entry = entry_4.get()
        if Active_freq_entry[0:7] != T_old_Active_freq:
            T_Active_freq = Active_freq_entry[0:7]
            T_old_Active_freq = T_Active_freq
            temp_str = str(T_Active_freq)
            try:
                Amhz, Akhz = temp_str.split('.')
            except ValueError:
                Amhz = temp_str 
                Akhz = '0'    
            amhz_ascii = chr(int(Amhz) - 48)
            akhz2 = Akhz+"00"
            Akhz_num = int(akhz2[:3])
            Amhz_num = int(Amhz)

            if Akhz_num >= 950:
                start_mhz = Amhz_num - 1
                start_khz = 875
            elif Akhz_num <= 200:
                start_mhz = Amhz_num - 1
                start_khz = Akhz_num + 100
            else:
                start_mhz = Amhz_num - 1
                start_khz = Akhz_num  - 100

            current_mhz = start_mhz
            current_khz = start_khz

            while (abs(current_mhz - Amhz_num) > 0) or (abs(current_khz - Akhz_num) > 24):
                current_khz -= 25  * int((current_khz - Akhz_num) / abs(current_khz - Akhz_num))
                if (current_mhz < Amhz_num):
                    current_mhz += 1
                amhz_ascii = chr(current_mhz - 48)
                akhz_ascii = chr((current_khz // 25) + 48)

                temp_message = "$PATTV27" + amhz_ascii + akhz_ascii + '\r' + '\n'
                send_serial_data(serial_connection, temp_message)

                time.sleep(0.5)

        standby_freq_entry = entry_5.get()
        if standby_freq_entry[0:7] != T_old_standby_freq:
            T_standby_freq = standby_freq_entry[0:7]
            T_old_standby_freq = T_standby_freq      
            temp_str = str(T_standby_freq)
            try:
                Smhz, Skhz = temp_str.split('.')
            except:
                Smhz = temp_str
                Skhz = '0'
            smhz_ascii = chr(int(Smhz) - 48)
            skhz2 = Skhz+"00"
            Skhz_num = int(skhz2[:3])
            Smhz_num = int(Smhz)

            if Skhz_num >= 950:
                start_mhz = Smhz_num - 1
                start_khz = 875
            elif Skhz_num <= 200:
                start_mhz = Smhz_num - 1
                start_khz = Skhz_num + 100
            else:
                start_mhz = Smhz_num - 1
                start_khz = Skhz_num - 100

            current_mhz = start_mhz
            current_khz = start_khz 

            while (abs(current_mhz - Smhz_num)) or (abs(current_khz - Skhz_num) > 24):

                current_khz -= 25  * int((current_khz - Skhz_num) / abs(current_khz - Skhz_num))

                if (current_mhz < Smhz_num):
                    current_mhz += 1

                smhz_ascii = chr(current_mhz - 48)
                skhz_ascii = chr((current_khz // 25) + 48)

                temp_message = "$PATTV28" + smhz_ascii + skhz_ascii + '\r' + '\n'
                send_serial_data(serial_connection, temp_message)

                time.sleep(0.5)


    # Function to increment and display the value
    def update_value():

        global N_volume, N_obs, N_Active_freq, N_standby_freq, N_Active_tx_Status, N_standby_tx_Status
        global C_volume, C_squelch, C_micgain, C_sidetone, C_Active_freq, C_standby_freq, C_Active_tx_Status, C_standby_tx_Status
        
        global N_prev_Active_freq, N_prev_Active_tx_Status, N_prev_standby_freq, N_prev_standby_tx_Status, N_prev_volume, N_prev_obs
        global C_prev_volume, C_prev_squelch, C_prev_micgain, C_prev_sidetone, C_prev_Active_freq, C_prev_standby_freq, C_prev_Active_tx_Status, C_prev_standby_tx_Status

        global T_prev_Active_freq, T_prev_standby_freq
        global T_Active_freq, T_standby_freq

        global A_Active_freq, A_standby_freq
        global A_prev_Active_freq, A_prev_standby_freq

        # Input Outputs:

        # for nav ..........................................................

        if N_Active_freq != N_prev_Active_freq:
            print(N_Active_freq)
            entry_1.delete(0,7)
            entry_1.insert(0,N_Active_freq)
            N_prev_Active_freq = N_Active_freq

        if N_Active_tx_Status != N_prev_Active_tx_Status:
            print(N_Active_tx_Status)
            entry_6.delete(0,6)
            entry_6.insert(0,N_Active_tx_Status)
            N_prev_Active_tx_Status = N_Active_tx_Status

        if N_standby_freq != N_prev_standby_freq:
            print(N_standby_freq)
            entry_11.delete(0,7)
            entry_11.insert(0,N_standby_freq)
            N_prev_standby_freq = N_standby_freq

        if N_standby_tx_Status != N_prev_standby_tx_Status:
            print(N_standby_tx_Status)
            entry_9.delete(0,6)
            entry_9.insert(0,N_standby_tx_Status)
            N_prev_standby_tx_Status = N_standby_tx_Status

        if N_volume != N_prev_volume:
            print(N_volume)
            entry_22.delete(0,2)
            entry_22.insert(0,N_volume)
            N_prev_volume = N_volume

        if N_obs != N_prev_obs:
            print(N_obs)
            entry_23.delete(0,3)
            entry_23.insert(0,N_obs)
            N_prev_obs = N_obs

        # for com ..............................................................

        if C_Active_freq != C_prev_Active_freq:
            print(C_Active_freq)
            entry_10.delete(0,7)
            entry_10.insert(0,C_Active_freq)
            C_prev_Active_freq = C_Active_freq

        if C_Active_tx_Status != C_prev_Active_tx_Status:
            print(C_Active_tx_Status)
            entry_7.delete(0,6)
            entry_7.insert(0,C_Active_tx_Status)
            C_prev_Active_tx_Status = C_Active_tx_Status

        if C_standby_freq != C_prev_standby_freq:
            print(C_standby_freq)
            entry_12.delete(0,7)
            entry_12.insert(0,C_standby_freq)
            C_prev_standby_freq = C_standby_freq

        if C_standby_tx_Status != C_prev_standby_tx_Status:
            print(C_standby_tx_Status)
            entry_8.delete(0,6)
            entry_8.insert(0,C_standby_tx_Status)
            C_prev_standby_tx_Status = C_standby_tx_Status

        if C_volume != C_prev_volume:
            print(C_volume)
            entry_20.delete(0,2)
            entry_20.insert(0,C_volume)
            C_prev_volume = C_volume

        if C_squelch != C_prev_squelch:
            print(C_squelch)
            entry_24.delete(0,2)
            entry_24.insert(0,C_squelch)
            C_prev_squelch = C_squelch

        if C_micgain != C_prev_micgain:
            print(C_micgain)
            entry_21.delete(0,2)
            entry_21.insert(0,C_micgain)
            C_prev_micgain = C_micgain

        if C_sidetone != C_prev_sidetone:
            print(C_sidetone)
            entry_25.delete(0,2)
            entry_25.insert(0,C_sidetone)
            C_prev_sidetone = C_sidetone 

        # for ADF ..............................................................

        if A_Active_freq != A_prev_Active_freq:
            print(A_Active_freq)
            entry_2.delete(0,7)
            entry_2.insert(0,A_Active_freq)
            A_prev_Active_freq = A_Active_freq

        if A_standby_freq != A_prev_standby_freq:
            print(A_standby_freq)
            entry_3.delete(0,7)
            entry_3.insert(0,A_standby_freq)
            A_prev_standby_freq = A_standby_freq

        # for TACAN ..............................................................

        if T_Active_freq != T_prev_Active_freq:
            print(T_Active_freq)
            entry_4.delete(0,7)
            entry_4.insert(0,T_Active_freq)
            T_prev_Active_freq = T_Active_freq

        if T_standby_freq != T_prev_standby_freq:
            print(T_standby_freq)
            entry_5.delete(0,7)
            entry_5.insert(0,T_standby_freq)
            T_prev_standby_freq = T_standby_freq

        window.after(1000, update_value)


    # List to store received data
    received_data = []

    # Start serial read thread
    if serial_connection:
        start_serial_read_thread(serial_connection, received_data)   

    window = Tk()
    window.title("CDU Simulator")


    window.geometry("700x500")
    window.configure(bg = "#C8F0F5")


    canvas = Canvas(
        window,
        bg = "#C8F0F5",
        height = 500,
        width = 700,
        bd = 0,
        highlightthickness = 0,
        relief = "ridge"
    )

    canvas.place(x = 0, y = 0)
    canvas.create_rectangle(
        26.0,
        23.0,
        339.0,
        234.0,
        fill="#8E98A7",
        outline="")

    canvas.create_rectangle(
        26.0,
        282.0,
        339.0,
        493.0,
        fill="#8E98A7",
        outline="")

    canvas.create_rectangle(
        362.0,
        282.0,
        675.0,
        493.0,
        fill="#8E98A7",
        outline="")

    canvas.create_rectangle(
        362.0,
        23.0,
        675.0,
        234.0,
        fill="#8E98A7",
        outline="")

    canvas.create_text(
        155.0,
        0.0,
        anchor="nw",
        text="NAV",
        fill="#000000",
        font=("Inter SemiBold", 20 * -1)
    )

    canvas.create_text(
        153.0,
        256.0,
        anchor="nw",
        text="ADF",
        fill="#000000",
        font=("Inter SemiBold", 20 * -1)
    )

    canvas.create_text(
        496.0,
        256.0,
        anchor="nw",
        text="TACAN",
        fill="#000000",
        font=("Inter SemiBold", 20 * -1)
    )

    canvas.create_text(
        507.0,
        0.0,
        anchor="nw",
        text="COM",
        fill="#000000",
        font=("Inter SemiBold", 20 * -1)
    )

    canvas.create_text(
        37.0,
        42.0,
        anchor="nw",
        text="Active Freq, Tx status",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        37.0,
        307.0,
        anchor="nw",
        text="Active Freq",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        37.0,
        344.0,
        anchor="nw",
        text="Standby Freq",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        369.0,
        307.0,
        anchor="nw",
        text="Active Freq",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        369.0,
        344.0,
        anchor="nw",
        text="Standby Freq",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        368.0,
        42.0,
        anchor="nw",
        text="Active Freq, Tx Status",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        37.0,
        77.0,
        anchor="nw",
        text="Standby Freq, Tx status",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        368.0,
        77.0,
        anchor="nw",
        text="Standby Freq, Tx Status",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        37.0,
        112.0,
        anchor="nw",
        text="OBS Value",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        248.0,
        187.0,
        anchor="nw",
        text="Rx Status",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        368.0,
        112.0,
        anchor="nw",
        text="Mic Gain, Sidetone",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        37.0,
        147.0,
        anchor="nw",
        text="Volume Level",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        368.0,
        147.0,
        anchor="nw",
        text="Volume, Squelch",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        37.0,
        182.0,
        anchor="nw",
        text="Rx Active Freq.",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        368.0,
        183.0,
        anchor="nw",
        text="Rx Active Freq, Status",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        37.0,
        205.0,
        anchor="nw",
        text="Rx Standby Freq.",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    canvas.create_text(
        368.0,
        206.0,
        anchor="nw",
        text="Rx Standby Freq, Test Set",
        fill="#000000",
        font=("Inter SemiBold", 12 * -1)
    )

    entry_image_1 = PhotoImage(
        file=relative_to_assets("entry_1.png"))
    entry_bg_1 = canvas.create_image(
        231.0,
        48.0,
        image=entry_image_1
    )
    entry_1 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_1.place(
        x=194.0,
        y=33.0,
        width=74.0,
        height=28.0
    )

    entry_image_2 = PhotoImage(
        file=relative_to_assets("entry_2.png"))
    entry_bg_2 = canvas.create_image(
        231.0,
        315.0,
        image=entry_image_2
    )
    entry_2 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_2.place(                  # adf active 
        x=194.0,
        y=300.0,
        width=74.0,
        height=28.0
    )

    entry_image_3 = PhotoImage(
        file=relative_to_assets("entry_3.png"))
    entry_bg_3 = canvas.create_image(
        231.0,
        352.0,
        image=entry_image_3
    )
    entry_3 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_3.place(                  # adf standby 
        x=194.0,
        y=337.0,
        width=74.0,
        height=28.0
    )

    entry_image_4 = PhotoImage(
        file=relative_to_assets("entry_4.png"))
    entry_bg_4 = canvas.create_image(
        570.0,
        315.0,
        image=entry_image_4
    )
    entry_4 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_4.place(              # tacan active 
        x=533.0,
        y=300.0,
        width=74.0,
        height=28.0
    )

    entry_image_5 = PhotoImage(
        file=relative_to_assets("entry_5.png"))
    entry_bg_5 = canvas.create_image(
        570.0,
        352.0,
        image=entry_image_5
    )
    entry_5 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_5.place(              # tacan standby
        x=533.0,
        y=337.0,
        width=74.0,
        height=28.0
    )

    entry_image_6 = PhotoImage(
        file=relative_to_assets("entry_6.png"))
    entry_bg_6 = canvas.create_image(
        299.5,
        48.0,
        image=entry_image_6
    )
    entry_6 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_6.place(
        x=272.0,
        y=33.0,
        width=55.0,
        height=28.0
    )

    entry_image_7 = PhotoImage(
        file=relative_to_assets("entry_7.png"))
    entry_bg_7 = canvas.create_image(
        638.5,
        50.0,
        image=entry_image_7
    )
    entry_7 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_7.place(
        x=611.0,
        y=35.0,
        width=55.0,
        height=28.0
    )

    entry_image_8 = PhotoImage(
        file=relative_to_assets("entry_8.png"))
    entry_bg_8 = canvas.create_image(
        638.5,
        85.0,
        image=entry_image_8
    )
    entry_8 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_8.place(
        x=611.0,
        y=70.0,
        width=55.0,
        height=28.0
    )

    entry_image_9 = PhotoImage(
        file=relative_to_assets("entry_9.png"))
    entry_bg_9 = canvas.create_image(
        299.5,
        83.0,
        image=entry_image_9
    )
    entry_9 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_9.place(
        x=272.0,
        y=68.0,
        width=55.0,
        height=28.0
    )

    entry_image_10 = PhotoImage(
        file=relative_to_assets("entry_10.png"))
    entry_bg_10 = canvas.create_image(
        570.0,
        51.0,
        image=entry_image_10
    )
    entry_10 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_10.place(
        x=533.0,
        y=36.0,
        width=74.0,
        height=28.0
    )

    entry_image_11 = PhotoImage(
        file=relative_to_assets("entry_11.png"))
    entry_bg_11 = canvas.create_image(
        231.0,
        83.0,
        image=entry_image_11
    )
    entry_11 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_11.place(
        x=194.0,
        y=68.0,
        width=74.0,
        height=28.0
    )

    entry_image_12 = PhotoImage(
        file=relative_to_assets("entry_12.png"))
    entry_bg_12 = canvas.create_image(
        570.0,
        85.0,
        image=entry_image_12
    )
    entry_12 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_12.place(
        x=533.0,
        y=70.0,
        width=74.0,
        height=28.0
    )

    entry_image_13 = PhotoImage(
        file=relative_to_assets("entry_13.png"))
    entry_bg_13 = canvas.create_image(
        570.0,
        190.5,
        image=entry_image_13
    )
    entry_13 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_13.place(
        x=533.0,
        y=178.0,
        width=74.0,
        height=23.0
    )

    entry_image_14 = PhotoImage(
        file=relative_to_assets("entry_14.png"))
    entry_bg_14 = canvas.create_image(
        570.0,
        217.5,
        image=entry_image_14
    )
    entry_14 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_14.place(
        x=533.0,
        y=205.0,
        width=74.0,
        height=23.0
    )

    entry_image_15 = PhotoImage(
        file=relative_to_assets("entry_15.png"))
    entry_bg_15 = canvas.create_image(
        638.5,
        217.5,
        image=entry_image_15
    )
    entry_15 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_15.place(
        x=611.0,
        y=205.0,
        width=55.0,
        height=23.0
    )

    entry_image_16 = PhotoImage(
        file=relative_to_assets("entry_16.png"))
    entry_bg_16 = canvas.create_image(
        638.5,
        190.5,
        image=entry_image_16
    )
    entry_16 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_16.place(
        x=611.0,
        y=178.0,
        width=55.0,
        height=23.0
    )

    entry_image_17 = PhotoImage(
        file=relative_to_assets("entry_17.png"))
    entry_bg_17 = canvas.create_image(
        194.0,
        189.5,
        image=entry_image_17
    )
    entry_17 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_17.place(
        x=157.0,
        y=177.0,
        width=74.0,
        height=23.0
    )

    entry_image_18 = PhotoImage(
        file=relative_to_assets("entry_18.png"))
    entry_bg_18 = canvas.create_image(
        276.0,
        216.5,
        image=entry_image_18
    )
    entry_18 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_18.place(
        x=239.0,
        y=204.0,
        width=74.0,
        height=23.0
    )

    entry_image_19 = PhotoImage(
        file=relative_to_assets("entry_19.png"))
    entry_bg_19 = canvas.create_image(
        194.0,
        216.5,
        image=entry_image_19
    )
    entry_19 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_19.place(
        x=157.0,
        y=204.0,
        width=74.0,
        height=23.0
    )

    entry_image_20 = PhotoImage(
        file=relative_to_assets("entry_20.png"))
    entry_bg_20 = canvas.create_image(
        579.5,
        152.0,
        image=entry_image_20
    )
    entry_20 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_20.place(
        x=552.0,
        y=137.0,
        width=55.0,
        height=28.0
    )

    entry_image_21 = PhotoImage(
        file=relative_to_assets("entry_21.png"))
    entry_bg_21 = canvas.create_image(
        579.5,
        120.0,
        image=entry_image_21
    )
    entry_21 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_21.place(
        x=552.0,
        y=105.0,
        width=55.0,
        height=28.0
    )

    entry_image_22 = PhotoImage(
        file=relative_to_assets("entry_22.png"))
    entry_bg_22 = canvas.create_image(
        299.5,
        156.0,
        image=entry_image_22
    )
    entry_22 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_22.place(
        x=272.0,
        y=141.0,
        width=55.0,
        height=28.0
    )

    entry_image_23 = PhotoImage(
        file=relative_to_assets("entry_23.png"))
    entry_bg_23 = canvas.create_image(
        299.5,
        121.0,
        image=entry_image_23
    )
    entry_23 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_23.place(
        x=272.0,
        y=106.0,
        width=55.0,
        height=28.0
    )

    entry_image_24 = PhotoImage(
        file=relative_to_assets("entry_24.png"))
    entry_bg_24 = canvas.create_image(
        638.5,
        152.0,
        image=entry_image_24
    )
    entry_24 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_24.place(
        x=611.0,
        y=137.0,
        width=55.0,
        height=28.0
    )

    entry_image_25 = PhotoImage(
        file=relative_to_assets("entry_25.png"))
    entry_bg_25 = canvas.create_image(
        638.5,
        120.0,
        image=entry_image_25
    )
    entry_25 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_25.place(
        x=611.0,
        y=105.0,
        width=55.0,
        height=28.0
    )

    button_image_1 = PhotoImage(
        file=relative_to_assets("button_1.png"))
    button_1 = Button(
        image=button_image_1,
        borderwidth=0,
        highlightthickness=0,
        command=lambda: on_send_button_click(serial_connection),
        relief="flat"
    )
    button_1.place(
        x=310.0,
        y=244.0,
        width=80.0,
        height=27.0
    )
    window.resizable(False, False)
    window.mainloop()

if __name__ == "__main__":
    main()