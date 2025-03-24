import React from 'react';
import { Code2, Database, Globe } from 'lucide-react';

function About() {
  const features = [
    {
      icon: <Code2 className="h-8 w-8 text-blue-500" />,
      title: 'Modern Stack',
      description: 'Built with React, Vite, and FastAPI for optimal performance and developer experience.',
    },
    {
      icon: <Database className="h-8 w-8 text-blue-500" />,
      title: 'API Integration',
      description: 'Seamless integration between frontend and backend using REST API principles.',
    },
    {
      icon: <Globe className="h-8 w-8 text-blue-500" />,
      title: 'Responsive Design',
      description: 'Fully responsive interface that works beautifully on all devices.',
    },
  ];

  return (
    <div className="bg-white rounded-lg shadow-xl p-8 max-w-4xl mx-auto">
      <h1 className="text-3xl font-bold text-center mb-8">About Our Project</h1>
      <p className="text-gray-600 text-center mb-12">
        This demo showcases the integration of modern web technologies to create a seamless user experience.
      </p>

      <div className="grid md:grid-cols-3 gap-8">
        {features.map((feature, index) => (
          <div key={index} className="text-center p-6 rounded-lg bg-gray-50">
            <div className="flex justify-center mb-4">{feature.icon}</div>
            <h3 className="text-xl font-semibold mb-2">{feature.title}</h3>
            <p className="text-gray-600">{feature.description}</p>
          </div>
        ))}
      </div>
    </div>
  );
}

export default About;