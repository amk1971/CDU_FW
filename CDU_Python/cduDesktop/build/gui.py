
import serial
import threading
from pathlib import Path
import time

from tkinter import Tk, Canvas, Entry, Text, Button, PhotoImage


OUTPUT_PATH = Path(__file__).parent
ASSETS_PATH = OUTPUT_PATH / Path(r"C:\Users\AeroTrainB\Documents\GitHub\CDU_FW\CDU_Python\cduDesktop\build\assets\frame0")

flag = False      # sir now we have to connect the port 

volume = 0
obs = 0
Active_freq = 0
standby_freq = 0
Active_tx_Status = 0 
standby_tx_Status = 0

volume_update = False
obs_update = False
Active_freq_update = False
standby_freq_update = False
Active_tx_Status_update = False
standby_tx_Status_update = False


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
def read_serial_data(serial_connection, data_list):
    global volume, obs, Active_freq, standby_freq, Active_tx_Status, standby_tx_Status
    # global prev_Active_freq, prev_Active_tx_Status, prev_standby_freq, prev_standby_tx_Status, prev_volume, prev_obs
    while True:
        if serial_connection and serial_connection.in_waiting > 0:
            data = serial_connection.readline().decode('latin-1').strip()
            if data:
                print(f"Data received: {data}")
                if data.startswith("$PATNV"):
                    if data[6] == '7' and data[7] == '3':
                        volume = (ord(data[8]) - 48)
                        print("Receive Volume Value: ", volume)

                    elif data[6] == '3' and data[7] == '4':
                        obs = data[8]+data[9]+data[10]
                        print("Receive OBS Value: ", obs)  # Output: Flag is set to: True

                    elif data[6] == '2' and data[7] == '7':
                        Active_Mhz = (ord(data[8]) + 48)
                        Active_khz = (ord(data[9]) - 48) * 25
                        Active_freq = Active_Mhz + (Active_khz)/1000
                        # if data[10] == 'N':
                        #     Active_tx_Status = "normal"
                        # else:
                        #     Active_tx_Status = "0"
                        Active_tx_Status = data[10]
                        print("Receive Active Freq(MHz):", Active_freq,", with Tx status is", Active_tx_Status)  # Output: Flag is set to: True

                    elif data[6] == '2' and data[7] == '8':
                        standby_Mhz = (ord(data[8]) + 48)
                        standby_khz = (ord(data[9]) - 48) * 25
                        standby_freq = standby_Mhz + (standby_khz)/1000
                        # if data[10] == 'N':
                        #     standby_tx_Status = "normal"
                        # else:
                        #     standby_tx_Status = "0"
                        standby_tx_Status = data[10]
                        print("Receive Standby Freq(MHz): ", standby_freq,", with Tx status is", standby_tx_Status)  # Output: Flag is set to: True

            data_list.append(data)

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

# Function to handle the button click
def on_send_button_click(serial_connection):
    # message = []
    
    global Active_freq_update, standby_freq_update, Active_tx_Status_update, standby_tx_Status_update, volume_update, obs_update

    # Amhz, Akhz = tx_activef.split('.')
    # amhz_ascii = chr(int(Amhz) - 48)
    # Akhz_num = int(Akhz)
    # if Akhz_num % 25 != 0:
    #     Akhz_num += 25 - (Akhz_num % 25)
    # akhz_ascii = chr((Akhz_num // 25) + 48) 
    # Smhz, Skhz = tx_standbyf.split('.')
    # smhz_ascii = chr(int(Smhz) - 48)
    # Skhz_num = int(Skhz)
    # if Skhz_num % 25 != 0:
    #     Skhz_num += 25 - (Skhz_num % 25)
    # skhz_ascii = chr((Skhz_num // 25) + 48)
    # message = "$PATNV28" + amhz_ascii + akhz_ascii + smhz_ascii + skhz_ascii + tx_status

    if Active_freq_update or Active_tx_Status_update:
        temp_str = str(Active_freq)
        Amhz, Akhz = temp_str.split('.')
        amhz_ascii = chr(int(Amhz) - 48)
        Akhz_num = int(Akhz)
        if Akhz_num % 25 != 0:
            Akhz_num += 25 - (Akhz_num % 25)
        akhz_ascii = chr((Akhz_num // 25) + 48) 
        temp_message = "$PATNV27" + amhz_ascii + akhz_ascii + Active_tx_Status +'\r' +'\n'
        send_serial_data(serial_connection, temp_message)
        # message.append(temp_message)
        Active_freq_update = False
        Active_tx_Status_update = False

    elif standby_freq_update or standby_tx_Status_update:
        temp_str = str(standby_freq)
        Smhz, Skhz = temp_str.split('.')
        smhz_ascii = chr(int(Smhz) - 48)
        Skhz_num = int(Skhz)
        if Skhz_num % 25 != 0:
            Skhz_num += 25 - (Skhz_num % 25)
        skhz_ascii = chr((Skhz_num // 25) + 48)
        temp_message = "$PATNV28" + smhz_ascii + skhz_ascii + standby_tx_Status +'\r' +'\n'
        send_serial_data(serial_connection, temp_message)
        # message.append(temp_message)
        standby_freq_update = False
        standby_tx_Status_update = False

    elif volume_update:
        temp_message = "$PATNV73" + str(volume) +'\r' +'\n'
        send_serial_data(serial_connection, temp_message)   
        # message.append(temp_message)
        volume_update = False

    elif obs_update:
        temp_message = "$PATNV34" + obs +'\r' +'\n'
        send_serial_data(serial_connection, temp_message)
        # message.append(temp_message)
        obs_update = False

    # for i in message:
    #     send_serial_data(serial_connection, i)
    #     time.sleep(0.25)

# Start serial reading in a separate thread
def start_serial_read_thread(serial_connection, data_list):
    thread = threading.Thread(target=read_serial_data, args=(serial_connection, data_list))
    thread.daemon = True
    thread.start()

prev_Active_freq= Active_freq
prev_Active_tx_Status= Active_tx_Status
prev_standby_freq= standby_freq
prev_standby_tx_Status= standby_tx_Status
prev_volume= volume
prev_obs = obs



# Main GUI application
def main():
    # Initialize serial port
    serial_connection = init_serial()

    # Function to capture user input from the entry box and assign it to its respective variables
    def handle_user_input_activefreq(event=None):
        global Active_freq, Active_freq_update

        Active_freq_entry = entry_1.get()
        Active_freq = Active_freq_entry[0:7]
        
        Active_freq_update = True

    def handle_user_input_standbyfreq(event=None):
        global standby_freq , standby_freq_update

        standby_freq_entry = entry_7.get() 
        standby_freq = standby_freq_entry[0:7]

        standby_freq_update = True

    def handle_user_input_volume(event=None):
        global volume, volume_update

        volume_entry = entry_18.get()
        volume = volume_entry[0:2]

        volume_update = True

    def handle_user_input_obs(event=None):
        global obs, obs_update

        obs_entry = entry_19.get() 
        obs = obs_entry[0:3]
        obs_update = True

    def handle_user_input_activeStatus(event=None):
        global Active_tx_Status, Active_tx_Status_update

        Active_tx_Status_entry = entry_2.get()
        Active_tx_Status = Active_tx_Status_entry[0:6]

        Active_tx_Status_update = True

    def handle_user_input_standbyStatus(event=None):
        global standby_tx_Status, standby_tx_Status_update

        standby_tx_Status_entry = entry_5.get()
        standby_tx_Status = standby_tx_Status_entry[0:6]

        standby_tx_Status_update = True
 
    
    # Function to increment and display the value
    def update_value():

        global tx_status, tx_activef, tx_standbyf

        global prev_Active_freq, prev_Active_tx_Status, prev_standby_freq, prev_standby_tx_Status, prev_volume, prev_obs

        # Input Outputs:

        if Active_freq != prev_Active_freq:
            print(Active_freq)
            entry_1.delete(0,7)
            entry_1.insert(0,Active_freq)
            prev_Active_freq = Active_freq

        if Active_tx_Status != prev_Active_tx_Status:
            print(Active_tx_Status)
            entry_2.delete(0,6)
            entry_2.insert(0,Active_tx_Status)
            prev_Active_tx_Status = Active_tx_Status

        if standby_freq != prev_standby_freq:
            print(standby_freq)
            entry_7.delete(0,7)
            entry_7.insert(0,standby_freq)
            prev_standby_freq = standby_freq

        if standby_tx_Status != prev_standby_tx_Status:
            print(standby_tx_Status)
            entry_5.delete(0,6)
            entry_5.insert(0,standby_tx_Status)
            prev_standby_tx_Status = standby_tx_Status

        if volume != prev_volume:
            print(volume)
            entry_18.delete(0,2)
            entry_18.insert(0,volume)
            prev_volume = volume

        if obs != prev_obs:
            print(obs)
            entry_19.delete(0,3)
            entry_19.insert(0,obs)
            prev_obs = obs

        # tx_status_entry = entry_14.get("1.0", "2.0")  # Get letter N or M
        # tx_status = tx_status_entry[0:1]
        
        # tx_activef_entry = entry_13.get("1.0", "2.0")
        # tx_activef = tx_activef_entry[0:7]

        # tx_standbyf_entry = entry_15.get("1.0", "2.0")
        # tx_standbyf = tx_standbyf_entry[0:7]

        # Input only
        # entry_1.delete(0,7)
        # entry_1.insert(0,ip_Active_freq)        
        # entry_2.delete(0,6)
        # entry_2.insert(0,ip_Status)
        # entry_7.delete(0,7)
        # entry_7.insert(0,ip_standby_freq)

        # Bind the <Return> key (Enter key) to handle_user_input
        entry_1.bind("<Return>", handle_user_input_activefreq)
        entry_18.bind("<Return>", handle_user_input_volume)
        entry_19.bind("<Return>", handle_user_input_obs)
        entry_7.bind("<Return>", handle_user_input_standbyfreq)
        entry_2.bind("<Return>", handle_user_input_activeStatus)
        entry_5.bind("<Return>", handle_user_input_standbyStatus)



        window.after(1000, update_value)  # Schedule the function to run again after 1 second (1000 ms)


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
        text="Active Freq, status",
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
        text="Standby Freq, status",
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
        text="Status(N/M)",
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
        text="Active Freq.(in MHz)",
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
        text="Standby Freq.(in MHz)",
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

    entry_image_1 = PhotoImage(                             # op nav active
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

    entry_image_2 = PhotoImage(                             # op nav active tx status
        file=relative_to_assets("entry_2.png"))
    entry_bg_2 = canvas.create_image(
        299.5,
        48.0,
        image=entry_image_2
    )
    entry_2 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_2.place(
        x=272.0,
        y=33.0,
        width=55.0,
        height=28.0
    )

    entry_image_3 = PhotoImage(
        file=relative_to_assets("entry_3.png"))
    entry_bg_3 = canvas.create_image(
        638.5,
        50.0,
        image=entry_image_3
    )
    entry_3 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_3.place(
        x=611.0,
        y=35.0,
        width=55.0,
        height=28.0
    )

    entry_image_4 = PhotoImage(
        file=relative_to_assets("entry_4.png"))
    entry_bg_4 = canvas.create_image(
        638.5,
        85.0,
        image=entry_image_4
    )
    entry_4 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_4.place(
        x=611.0,
        y=70.0,
        width=55.0,
        height=28.0
    )

    entry_image_5 = PhotoImage(                             # op nav standby tx status
        file=relative_to_assets("entry_5.png"))
    entry_bg_5 = canvas.create_image(
        299.5,
        83.0,
        image=entry_image_5
    )
    entry_5 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_5.place(
        x=272.0,
        y=68.0,
        width=55.0,
        height=28.0
    )

    entry_image_6 = PhotoImage(
        file=relative_to_assets("entry_6.png"))
    entry_bg_6 = canvas.create_image(
        570.0,
        51.0,
        image=entry_image_6
    )
    entry_6 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_6.place(
        x=533.0,
        y=36.0,
        width=74.0,
        height=28.0
    )

    entry_image_7 = PhotoImage(                         # op nav standby
        file=relative_to_assets("entry_7.png"))
    entry_bg_7 = canvas.create_image(
        231.0,
        83.0,
        image=entry_image_7
    )
    entry_7 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_7.place(
        x=194.0,
        y=68.0,
        width=74.0,
        height=28.0
    )

    entry_image_8 = PhotoImage(
        file=relative_to_assets("entry_8.png"))
    entry_bg_8 = canvas.create_image(
        570.0,
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
        x=533.0,
        y=70.0,
        width=74.0,
        height=28.0
    )

    entry_image_9 = PhotoImage(
        file=relative_to_assets("entry_9.png"))
    entry_bg_9 = canvas.create_image(
        570.0,
        190.5,
        image=entry_image_9
    )
    entry_9 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_9.place(
        x=533.0,
        y=178.0,
        width=74.0,
        height=23.0
    )

    entry_image_10 = PhotoImage(
        file=relative_to_assets("entry_10.png"))
    entry_bg_10 = canvas.create_image(
        570.0,
        217.5,
        image=entry_image_10
    )
    entry_10 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_10.place(
        x=533.0,
        y=205.0,
        width=74.0,
        height=23.0
    )

    entry_image_11 = PhotoImage(
        file=relative_to_assets("entry_11.png"))
    entry_bg_11 = canvas.create_image(
        638.5,
        217.5,
        image=entry_image_11
    )
    entry_11 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_11.place(
        x=611.0,
        y=205.0,
        width=55.0,
        height=23.0
    )

    entry_image_12 = PhotoImage(
        file=relative_to_assets("entry_12.png"))
    entry_bg_12 = canvas.create_image(
        638.5,
        190.5,
        image=entry_image_12
    )
    entry_12 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_12.place(
        x=611.0,
        y=178.0,
        width=55.0,
        height=23.0
    )

    entry_image_13 = PhotoImage(                # ip nav active
        file=relative_to_assets("entry_13.png"))
    entry_bg_13 = canvas.create_image(
        194.0,
        189.5,
        image=entry_image_13
    )
    entry_13 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_13.place(
        x=157.0,
        y=177.0,
        width=74.0,
        height=23.0
    )

    entry_image_14 = PhotoImage(                        # ip nav status
        file=relative_to_assets("entry_14.png"))
    entry_bg_14 = canvas.create_image(
        276.0,
        216.5,
        image=entry_image_14
    )
    entry_14 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_14.place(
        x=239.0,
        y=204.0,
        width=74.0,
        height=23.0
    )
    # txt = entry_14.get(0)
    # print(txt)

    entry_image_15 = PhotoImage(                        # ip nav standby
        file=relative_to_assets("entry_15.png"))
    entry_bg_15 = canvas.create_image(
        194.0,
        216.5,
        image=entry_image_15
    )
    entry_15 = Text(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_15.place(
        x=157.0,
        y=204.0,
        width=74.0,
        height=23.0
    )

    entry_image_16 = PhotoImage(
        file=relative_to_assets("entry_16.png"))
    entry_bg_16 = canvas.create_image(
        579.5,
        152.0,
        image=entry_image_16
    )
    entry_16 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_16.place(
        x=552.0,
        y=137.0,
        width=55.0,
        height=28.0
    )

    entry_image_17 = PhotoImage(
        file=relative_to_assets("entry_17.png"))
    entry_bg_17 = canvas.create_image(
        579.5,
        120.0,
        image=entry_image_17
    )
    entry_17 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_17.place(
        x=552.0,
        y=105.0,
        width=55.0,
        height=28.0
    )

    entry_image_18 = PhotoImage(                        # op nav vol level
        file=relative_to_assets("entry_18.png"))
    entry_bg_18 = canvas.create_image(
        299.5,
        156.0,
        image=entry_image_18
    )
    entry_18 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_18.place(
        x=272.0,
        y=141.0,
        width=55.0,
        height=28.0
    )
    # entry_18.insert(0, volume)

    entry_image_19 = PhotoImage(                            # op nav obs val
        file=relative_to_assets("entry_19.png"))
    entry_bg_19 = canvas.create_image(
        299.5,
        121.0,
        image=entry_image_19
    )
    entry_19 = Entry(
        bd=0,
        bg="#FFFFFF",
        fg="#000716",
        highlightthickness=0
    )
    entry_19.place(
        x=272.0,
        y=106.0,
        width=55.0,
        height=28.0
    )

    entry_image_20 = PhotoImage(
        file=relative_to_assets("entry_20.png"))
    entry_bg_20 = canvas.create_image(
        638.5,
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
        x=611.0,
        y=137.0,
        width=55.0,
        height=28.0
    )

    entry_image_21 = PhotoImage(
        file=relative_to_assets("entry_21.png"))
    entry_bg_21 = canvas.create_image(
        638.5,
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
    update_value()
    window.resizable(False, False)
    window.mainloop()

if __name__ == "__main__":
    main()