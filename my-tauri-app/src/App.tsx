import React from 'react';
import ExampleComponent from './components/ExampleComponent';

const App: React.FC = () => {
  return (
    <div className="App">
      <h1 className="text-2xl font-bold">Welcome to My Tauri App</h1>
      <ExampleComponent />
    </div>
  );
};

export default App;