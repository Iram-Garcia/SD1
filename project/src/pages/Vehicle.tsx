import React, { useState, useEffect } from 'react';
import { Send, Download } from 'lucide-react';

function ParkingInformation() {
  const [formData, setFormData] = useState({
    licensePlate: '',
    carModel: '',
    studentName: '',
    studentId: '',
    studentEmail: '',
    zoneRestriction: '',
  });
  const [saveStatus, setSaveStatus] = useState<string>('');
  const [isSubmitting, setIsSubmitting] = useState(false);

  useEffect(() => {
    // Clear status message after 3 seconds
    if (saveStatus) {
      const timer = setTimeout(() => {
        setSaveStatus('');
      }, 3000);
      return () => clearTimeout(timer);
    }
  }, [saveStatus]);

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setIsSubmitting(true);
    console.log('Parking Information submitted:', formData);
    
    try {
      // Save to database via API endpoint
      await saveToDatabase();
      setSaveStatus('Information saved successfully!');
      
      // Removed the automatic download call
    } catch (error) {
      console.error('Error saving data:', error);
      setSaveStatus('Error saving information. Please try again.');
    } finally {
      setIsSubmitting(false);
    }
  };

  const saveToDatabase = async () => {
    // Simulate an API call to a backend endpoint that would write to database.csv
    // In a real implementation, you would create an actual API endpoint
    try {
      // Example API call:
      // const response = await fetch('/api/save-parking-info', {
      //   method: 'POST',
      //   headers: {
      //     'Content-Type': 'application/json',
      //   },
      //   body: JSON.stringify({
      //     ...formData,
      //     timestamp: new Date().toISOString()
      //   }),
      // });
      
      // if (!response.ok) {
      //   throw new Error('Network response was not ok');
      // }
      
      // For demo purposes, we'll simulate a successful API call with a delay
      return new Promise((resolve) => {
        setTimeout(() => {
          // Log what would be saved to database:
          console.log('Saved to database:', {
            ...formData,
            timestamp: new Date().toISOString()
          });
          resolve(true);
        }, 800);
      });
    } catch (error) {
      console.error('Error saving to database:', error);
      throw error;
    }
  };

  const handleChange = (e: React.ChangeEvent<HTMLInputElement | HTMLTextAreaElement | HTMLSelectElement>) => {
    const { name, value } = e.target;
    setFormData(prev => ({ ...prev, [name]: value }));
  };

  // Function to convert form data to CSV
  const convertToCSV = () => {
    const headers = [
      'License Plate',
      'Car Model',
      'Student Name',
      'Student ID',
      'Student Email',
      'Zone Restriction'
    ].join(',');
    
    const values = [
      formData.licensePlate,
      formData.carModel,
      formData.studentName,
      formData.studentId,
      formData.studentEmail,
      formData.zoneRestriction
    ].join(',');
    
    return `${headers}\n${values}`;
  };
  
  // Function to download CSV file
  const downloadCSV = () => {
    const csv = convertToCSV();
    const blob = new Blob([csv], { type: 'text/csv;charset=utf-8;' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    
    // Create filename with timestamp
    const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
    const filename = `parking_info_${formData.studentId}_${timestamp}.csv`;
    
    link.setAttribute('href', url);
    link.setAttribute('download', filename);
    link.style.visibility = 'hidden';
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  };

  return (
    <div className="bg-white rounded-lg shadow-xl p-8 max-w-2xl mx-auto">
      <h1 className="text-3xl font-bold text-center mb-8">Parking Information</h1>
      {saveStatus && (
        <div className={`p-4 mb-4 rounded-md ${saveStatus.includes('Error') ? 'bg-red-100 text-red-700' : 'bg-green-100 text-green-700'}`}>
          {saveStatus}
        </div>
      )}
      <form onSubmit={handleSubmit} className="space-y-6">
        <div>
          <label htmlFor="licensePlate" className="block text-sm font-medium text-gray-700 mb-1">
            License Plate Number
          </label>
          <input
            type="text"
            id="licensePlate"
            name="licensePlate"
            value={formData.licensePlate}
            onChange={handleChange}
            className="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-blue-500 focus:border-blue-500"
            required
          />
        </div>

        <div>
          <label htmlFor="carModel" className="block text-sm font-medium text-gray-700 mb-1">
            Car Model
          </label>
          <select
            id="carModel"
            name="carModel"
            value={formData.carModel}
            onChange={handleChange}
            className="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-blue-500 focus:border-blue-500"
            required
          >
            <option value="" disabled>Select vehicle type</option>
            <option value="car">Car</option>
            <option value="motorcycle">Motorcycle</option>
            <option value="pickup">Pickup Truck</option>
            <option value="other">Other</option>
          </select>
        </div>

        <div>
          <label htmlFor="studentName" className="block text-sm font-medium text-gray-700 mb-1">
            Student Name
          </label>
          <input
            type="text"
            id="studentName"
            name="studentName"
            value={formData.studentName}
            onChange={handleChange}
            className="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-blue-500 focus:border-blue-500"
            required
          />
        </div>

        <div>
          <label htmlFor="studentId" className="block text-sm font-medium text-gray-700 mb-1">
            Student ID
          </label>
          <input
            type="text"
            id="studentId"
            name="studentId"
            value={formData.studentId}
            onChange={handleChange}
            className="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-blue-500 focus:border-blue-500"
            required
          />
        </div>

        <div>
          <label htmlFor="studentEmail" className="block text-sm font-medium text-gray-700 mb-1">
            Student Email
          </label>
          <input
            type="email"
            id="studentEmail"
            name="studentEmail"
            value={formData.studentEmail}
            onChange={handleChange}
            className="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-blue-500 focus:border-blue-500"
            required
          />
        </div>

        <div>
          <label htmlFor="zoneRestriction" className="block text-sm font-medium text-gray-700 mb-1">
            Zone Restriction
          </label>
          <select
            id="zoneRestriction"
            name="zoneRestriction"
            value={formData.zoneRestriction}
            onChange={handleChange}
            className="w-full px-4 py-2 border border-gray-300 rounded-md focus:ring-blue-500 focus:border-blue-500"
            required
          >
            <option value="" disabled>Select zone</option>
            <option value="zone1">Zone 1</option>
            <option value="zone2">Zone 2</option>
            <option value="zone3">Zone 3</option>
          </select>
        </div>

        <div className="flex space-x-4">
          <button
            type="submit"
            disabled={isSubmitting}
            className={`flex-1 flex items-center justify-center px-4 py-2 border border-transparent rounded-md shadow-sm text-white ${isSubmitting ? 'bg-blue-400 cursor-not-allowed' : 'bg-blue-600 hover:bg-blue-700'} focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500`}
          >
            {isSubmitting ? (
              <>
                <svg className="animate-spin -ml-1 mr-3 h-5 w-5 text-white" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                  <circle className="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" strokeWidth="4"></circle>
                  <path className="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
                </svg>
                Processing...
              </>
            ) : (
              <>
                <Send className="w-5 h-5 mr-2" />
                Submit
              </>
            )}
          </button>
          
          <button
            type="button"
            onClick={downloadCSV}
            disabled={isSubmitting}
            className={`flex items-center justify-center px-4 py-2 border border-gray-300 rounded-md shadow-sm text-gray-700 bg-white ${isSubmitting ? 'opacity-50 cursor-not-allowed' : 'hover:bg-gray-50'} focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500`}
          >
            <Download className="w-5 h-5 mr-2" />
            Export CSV
          </button>
        </div>
      </form>
    </div>
  );
}

export default ParkingInformation;