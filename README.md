# ESP32 Serial Monitor Application

This application displays real-time data from an ESP32 device connected via serial port.

## Prerequisites

- Node.js (>= version 14)
- Python (>= 3.8)
- ESP32 device connected to COM4

## Installation

```bash
# Clone the repository
git clone <repository-url>
cd <repository-directory>

# Install dependencies
npm install
```

This will also install Python dependencies through the postinstall script.

## Running the Application

To start both the backend and frontend:

```bash
# Start the backend server first
npm run start

# In a separate terminal, start the frontend
npm run dev
```

## Features

- Real-time data streaming from ESP32
- Responsive interface for different devices
- About and Contact pages

## Building for Production

```bash
npm run build
```

## Testing

```bash
npm run test
```

## Additional Notes

- The backend runs on http://localhost:8000
- The frontend runs on http://localhost:5173 (or another port if 5173 is in use)
- Make sure your ESP32 is connected to COM4 (or update the port in server/main.py)
