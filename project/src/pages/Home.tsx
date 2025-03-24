import React, { useEffect, useState } from 'react';
import axios from 'axios';
import { Server } from 'lucide-react';

function Home() {
  const [message, setMessage] = useState<string>('');
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<string>('');

  useEffect(() => {
    const fetchData = async () => {
      try {
        const response = await axios.get('http://localhost:8000/api/hello');
        setMessage(response.data.message);
      } catch (err) {
        setError('Failed to fetch data from the server');
      } finally {
        setLoading(false);
      }
    };

    fetchData();
  }, []);

  return (
    <div className="bg-black rounded-lg shadow-xl p-8 max-w-md mx-auto">
      <div className="flex items-center justify-center mb-6">
        <Server className="h-12 w-12 text-blue-500" />
      </div>
      <h1 className="text-2xl font-bold text-center mb-6">Welcome to FastAPI + Vite Demo</h1>
      
      {loading ? (
        <div className="text-center text-gray-600">Loading...</div>
      ) : error ? (
        <div className="text-center text-red-500">{error}</div>
      ) : (
        <div className="text-center">
          <p className="text-lg text-gray-700">{message}</p>
          <p className="mt-4 text-sm text-gray-500">
            This message is fetched from the FastAPI backend
          </p>
        </div>
      )}
    </div>
  );
}

export default Home;