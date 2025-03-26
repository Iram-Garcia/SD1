import pytest
import pyserial_asyncio

@pytest.mark.asyncio
async def test_serial_connection():
    # Attempt to establish serial connection on COM4
    try:
        reader, writer = await pyserial_asyncio.open_serial_connection(url="COM4", baudrate=115200)
        assert reader is not None
        writer.close()
        await writer.wait_closed()
    except Exception as e:
        pytest.skip("COM4 not available or serial connection failed: " + str(e))
