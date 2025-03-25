from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from sse_starlette.sse import EventSourceResponse
import serial_asyncio
import asyncio

app = FastAPI()

# Add CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

queue: asyncio.Queue[str] = asyncio.Queue()

# Background task: read from USB‑Serial
async def serial_reader():
    reader, _ = await serial_asyncio.open_serial_connection(url="COM4", baudrate=115200)
    while True:
        line = await reader.readline()
        await queue.put(line.decode().strip())

@app.on_event("startup")
async def startup():
    asyncio.create_task(serial_reader())

@app.get("/stream")
async def stream():
    async def generator():
        while True:
            msg = await queue.get()
            yield {"data": msg}
    return EventSourceResponse(generator())
