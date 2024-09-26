import tkinter as tk
from pathlib import Path

window = tk.Tk()

OUTPUT_PATH = Path(__file__).parent
ASSETS_PATH = OUTPUT_PATH / Path(r"D:\Projects\CDU\Demo_RCP_CDU\cduDesktop\Tkinter-Designer-master\build\assets\frame0")

def relative_to_assets(path: str) -> Path:
    return ASSETS_PATH / Path(path)

# Entry box setup
entry_image_1 = tk.PhotoImage(file=relative_to_assets("entry_1.png"))  # Path to your image
entry_bg_1 = tk.Canvas(window, width=300, height=200)  # Create canvas to hold image
entry_bg_1.create_image(231.0, 48.0, image=entry_image_1)
entry_bg_1.pack()

entry_1 = tk.Entry(window, bd=0, bg="#FFFFFF", fg="#000716", highlightthickness=0)
entry_1.place(x=194.0, y=33.0, width=74.0, height=28.0)

# Active frequency value
Active_freq = "123.45"
previous_freq = Active_freq  # Initialize to track changes


# Function to update entry widget only if Active_freq changes
def update_value():
    global previous_freq, Active_freq

    # Check if Active_freq has changed
    if Active_freq != previous_freq:
        print(Active_freq)
        entry_1.delete(0, tk.END)  # Clear the entry box
        entry_1.insert(0, Active_freq)  # Insert the new Active_freq
        previous_freq = Active_freq  # Update previous_freq to current

    window.after(1000, update_value)  # Schedule the function to run again after 1 sec


# Function to capture user input from the entry box and assign it to Active_freq
def handle_user_input(event=None):
    global Active_freq
    user_input = entry_1.get()  # Get text from entry box
    Active_freq = user_input  # Update Active_freq variable with user input


# Bind the <Return> key (Enter key) to handle_user_input
entry_1.bind("<Return>", handle_user_input)

# Start the update loop
window.after(1000, update_value)

window.mainloop()