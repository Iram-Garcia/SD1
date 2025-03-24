// This file exports TypeScript types and interfaces used throughout the application. 
// It provides type safety for props and state.

export interface ExampleProps {
    title: string;
    description?: string;
}

export type ExampleState = {
    count: number;
};