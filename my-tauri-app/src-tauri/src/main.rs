// This file is the main entry point for the Tauri application. It sets up the Tauri API and initializes the application.

use tauri::Manager;

fn main() {
    tauri::Builder::default()
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}