import { useEffect, useState } from "react";

function App() {
  const [message, setMessage] = useState("");

  useEffect(() => {
    const evt = new EventSource("http://localhost:8000/stream");
    evt.onmessage = (e) => setMessage(e.data);
    return () => evt.close();
  }, []);

  return (
    <main className="p-8">
      <h1 className="text-2xl font-bold">ESP32 Live Serial</h1>
      <p className="mt-4 text-lg">{message || "Waiting for data..."}</p>
    </main>
  );
}

export default App;
