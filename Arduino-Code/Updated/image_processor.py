import serial
import time

# Serial port configuration (update the serial_port variable as needed)
serial_port = 'COM4'        # e.g. 'COM3' on Windows or '/dev/ttyUSB0' on Linux
baud_rate = 115200          # Must match the sender/receiver settings

# Define the trigger markers (as bytes)
IMG_START = b"Received IMG_START"
IMG_END = b"Image sent to Serial"

# Output file to save the extracted image
output_file = 'received_image.jpg'

# Flags and buffer for image data accumulation
in_image = False          # True when we are currently capturing image bytes
image_data = bytearray()  # Buffer for holding all incoming image data

print("Waiting for image data on serial port...")

# Open the serial connection
ser = serial.Serial(serial_port, baud_rate, timeout=1)

try:
    while True:
        # Read any available data from the serial port
        if ser.in_waiting > 0:
            chunk = ser.read(ser.in_waiting)

            # If we're not already in image capture mode, look for the start trigger
            if not in_image:
                start_index = chunk.find(IMG_START)
                if start_index != -1:
                    in_image = True
                    print("IMG_START detected, beginning image capture.")
                    # Clear any data before the start trigger
                    image_data = bytearray()
                continue

            # We are in capture mode, so check if the end trigger appears in the chunk
            end_index = chunk.find(IMG_END)
            if end_index != -1:
                # End trigger found – append data up to the end trigger
                image_data += chunk[:end_index]
                print("IMG_END detected, finishing image capture.")

                # Write the clean JPEG data to file
                with open(output_file, 'wb') as f:
                    f.write(image_data)
                print(f"JPEG image successfully saved as {output_file}")

                # Reset the buffer and state for the next image
                image_data = bytearray()
                in_image = False
                break  # Exit after one image; remove if expecting multiple images
            else:
                # No end trigger in this chunk; append all incoming data to the buffer
                image_data += chunk

        else:
            # If no data is available, wait briefly before checking again
            time.sleep(0.1)

except KeyboardInterrupt:
    print("Keyboard interrupt detected. Exiting program.")

finally:
    ser.close()