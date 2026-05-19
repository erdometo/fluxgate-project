import tkinter as tk
from PIL import ImageTk, Image
import math

# Create a new Tkinter window
window = tk.Tk()
window.title("Compass Dial")

# Load the compass image
compass_image = Image.open("dial.png")
compass_image = compass_image.resize((300, 300), Image.ANTIALIAS)
compass_photo = ImageTk.PhotoImage(compass_image)

# Create a canvas to display the compass image
canvas = tk.Canvas(window, width=300, height=300)
canvas.create_image(150, 150, image=compass_photo)
canvas.pack()
deg=45

# Function to update the compass needle position
def update_compass(heading):
    # Convert heading to radians
    angle = math.radians(heading)
    
    # Calculate the coordinates of the needle
    x = 150 + 120 * math.sin(angle)
    y = 150 - 120 * math.cos(angle)
    
    # Draw a line representing the needle
    canvas.delete("needle")
    canvas.create_line(150, 150, x, y, width=2, fill="red", tags="needle")

def find_north_pole(sensor_data, previous_data):
    # Calculate the average of the previous values
    avg_previous = sum(previous_data) / len(previous_data)

    # Determine the north pole direction based on the current value compared to the average of previous values
    if sensor_data > 2*avg_previous:
        deg+4
        update_compass(deg)
    else:
       update_compass(deg)



# Example usage: update the compass needle to point at 45 degrees
update_compass(45)

# Run the Tkinter event loop
window.mainloop()
