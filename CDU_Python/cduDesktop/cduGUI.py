# import tkinter as tk

# # Function to increment and display the value
# def update_value():
#     global counter  # Using a global variable for the counter
#     counter += 1
#     label_var.set(f"Value: {counter}")  # Update label text
#     root.after(1000, update_value)  # Schedule the function to run again after 1 second (1000 ms)

# # Tkinter setup
# root = tk.Tk()
# root.title("Incremental Value Display")

# # Global counter variable
# counter = 0

# # Label to display the counter value
# label_var = tk.StringVar()
# label_var.set(f"Value: {counter}")
# label = tk.Label(root, textvariable=label_var, font=("Helvetica", 16))
# label.pack(pady=20)

# # Start the incremental update
# update_value()

# # Start the Tkinter event loop
# root.mainloop()

import tkinter as tk

window = tk.Tk()

# Entry box setup
entry_image_1 = tk.PhotoImage(file="path_to_image")  # Path to your image
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
