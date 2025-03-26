import React, { useEffect, useState } from 'react';
import { Server } from 'lucide-react';

function Home() {
  const [message, setMessage] = useState<string>('');
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<string>('');

  useEffect(() => {
    setLoading(true);
    
    // Connect to the SSE endpoint
    const eventSource = new EventSource('http://localhost:8000/stream');
    
    eventSource.onmessage = (event) => {
      setMessage(event.data);
      setLoading(false);
    };
    
    eventSource.onerror = () => {
      setError('Failed to connect to the ESP32 data stream');
      setLoading(false);
      eventSource.close();
    };
    
    return () => {
      eventSource.close();
    };
  }, []);

  return (
    <div className="bg-white rounded-lg shadow-xl p-8 max-w-md mx-auto">
      <div className="flex items-center justify-center mb-6">
        <Server className="h-12 w-12 text-blue-500" />
      </div>
      <h1 className="text-2xl font-bold text-center mb-6">ESP32 Live Serial Monitor</h1>
      
      {loading ? (
        <div className="text-center text-gray-600">Connecting to ESP32...</div>
      ) : error ? (
        <div className="text-center text-red-500">{error}</div>
      ) : (
        <div className="text-center">
          <div className="bg-gray-100 p-4 rounded-lg text-left">
            <p className="font-mono text-lg break-words">{message || "Waiting for data..."}</p>
          </div>
          <p className="mt-4 text-sm text-gray-500">
            Live data stream from ESP32 on COM4
          </p>
        </div>
      )}
    </div>
  );
}

export default Home;