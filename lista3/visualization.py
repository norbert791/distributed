import matplotlib.pyplot as plt
import matplotlib.animation as animation
import networkx as nx
import numpy as np
from typing import Dict, List, Tuple
import math
import os
import time  # Added missing import
from process import Process
from snapshot import SnapshotManager

class SystemVisualizer:
    def __init__(self, processes: Dict[int, Process]):
        self.processes = processes
        self.fig, (self.ax1, self.ax2) = plt.subplots(1, 2, figsize=(15, 7))
        self.graph = nx.DiGraph()
        self.pos = None
        self.message_lines = []
        self.state_text = []
        self.snapshot_data = None
        
        # Create data directory if it doesn't exist
        self.data_dir = os.path.join(os.getcwd(), "data")
        os.makedirs(self.data_dir, exist_ok=True)
        
        # Initialize the graph
        self._init_graph()
        
    def _init_graph(self):
        """Initialize the network graph."""
        # Add nodes
        for pid in self.processes:
            self.graph.add_node(pid)
            
        # Add edges based on channels
        for pid, process in self.processes.items():
            for target_pid in process.outgoing_channels:
                self.graph.add_edge(pid, target_pid)
        
        # Calculate layout (positions of nodes)
        self.pos = nx.spring_layout(self.graph)
        
    def _draw_network(self, snapshot_in_progress=False):
        """Draw the network topology."""
        self.ax1.clear()
        self.ax1.set_title("Network Topology")
        
        # Draw nodes
        node_colors = []
        for node in self.graph.nodes():
            if snapshot_in_progress and self.processes[node].snapshot_in_progress:
                node_colors.append('lightgreen')
            else:
                node_colors.append('lightblue')
        
        nx.draw_networkx_nodes(self.graph, self.pos, 
                             node_color=node_colors,
                             node_size=1000,
                             ax=self.ax1)
        
        # Draw edges
        nx.draw_networkx_edges(self.graph, self.pos, 
                             edge_color='gray',
                             arrows=True,
                             ax=self.ax1)
        
        # Draw labels
        labels = {pid: f"P{pid}\n{self.processes[pid].state}"
                 for pid in self.processes}
        nx.draw_networkx_labels(self.graph, self.pos, labels, ax=self.ax1)
        
    def _draw_state_chart(self):
        """Draw the state chart showing process states over time."""
        self.ax2.clear()
        self.ax2.set_title("Process States")
        
        if self.snapshot_data:
            pids = list(self.snapshot_data.keys())
            states = [self.snapshot_data[pid]['local_state'] for pid in pids]
            
            # Create bar chart
            bars = self.ax2.bar([f'P{pid}' for pid in pids], states)
            
            # Add value labels on top of bars
            for bar in bars:
                height = bar.get_height()
                self.ax2.text(bar.get_x() + bar.get_width()/2., height,
                            f'{int(height)}',
                            ha='center', va='bottom')
            
            # Add channel state information
            y_pos = max(states) * 1.2
            for pid, data in self.snapshot_data.items():
                channel_text = f"P{pid} channels:"
                for from_pid, messages in data['channel_states'].items():
                    if messages:
                        channel_text += f"\nFrom P{from_pid}: {len(messages)} msgs"
                self.ax2.text(pids.index(pid), y_pos, channel_text,
                            ha='center', va='bottom', fontsize=8)
        
        self.ax2.set_ylabel("State Value")
        
    def update(self, snapshot_data=None):
        """Update the visualization."""
        if snapshot_data:
            self.snapshot_data = snapshot_data
        
        self._draw_network(snapshot_data is not None)
        self._draw_state_chart()
        plt.tight_layout()
        
    def show(self):
        """Display the visualization."""
        plt.show()
        
    def save(self, filename):
        """Save the visualization to a file."""
        plt.savefig(filename)
        print(f"Saved visualization to {filename}")

class EnhancedSimulation:
    def __init__(self, num_processes=5):
        # Create processes
        self.processes = {i: Process(i) for i in range(1, num_processes + 1)}
        
        # Create connections (fully connected network)
        for pid1, p1 in self.processes.items():
            for pid2, p2 in self.processes.items():
                if pid1 < pid2:  # Avoid duplicate connections
                    p1.connect_to(p2)
        
        self.snapshot_manager = SnapshotManager(self.processes)
        self.visualizer = SystemVisualizer(self.processes)
        
    def run_simulation(self, num_steps=10, snapshot_at_step=5):
        """Run the simulation with visualization."""
        print("Starting simulation...")
        
        # Create a subdirectory for this simulation run
        timestamp = time.strftime("%Y%m%d_%H%M%S")
        run_dir = os.path.join(self.visualizer.data_dir, f"run_{timestamp}")
        os.makedirs(run_dir, exist_ok=True)
        print(f"Saving simulation data to: {run_dir}")
        
        for step in range(num_steps):
            print(f"\nStep {step + 1}/{num_steps}")
            
            # Update process states
            for p in self.processes.values():
                p.update_state(np.random.randint(1, 10))
            
            # Random message passing
            for pid1 in self.processes:
                for pid2 in self.processes:
                    if pid1 != pid2 and np.random.random() < 0.3:
                        self.processes[pid1].send_message(
                            pid2, 
                            f"Msg_{step}_{pid1}->{pid2}"
                        )
            
            # Process messages
            for p in self.processes.values():
                for from_pid in p.incoming_channels:
                    message = p.receive_message(from_pid)
                    if message and not message.is_marker:
                        print(f"Process {p.pid} received: {message.content}")
            
            # Initiate snapshot at specified step
            if step == snapshot_at_step:
                print("\nInitiating snapshot from process 1...")
                self.snapshot_manager.initiate_snapshot(1)
            
            # Update visualization
            if self.snapshot_manager.is_snapshot_complete():
                snapshot_data = self.snapshot_manager.collect_snapshot()
                self.visualizer.update(snapshot_data)
            else:
                self.visualizer.update()
            
            # Save frame in run-specific directory
            frame_path = os.path.join(run_dir, f"step_{step:03d}.png")
            self.visualizer.save(frame_path)
        
        # Final snapshot collection
        if self.snapshot_manager.is_snapshot_complete():
            final_snapshot = self.snapshot_manager.collect_snapshot()
            print("\nFinal Snapshot:")
            for pid, data in final_snapshot.items():
                print(f"\nProcess {pid}:")
                print(f"Local state: {data['local_state']}")
                print("Channel states:")
                for from_pid, messages in data['channel_states'].items():
                    print(f"  From process {from_pid}: {messages}")
            
            # Save final snapshot data as text
            snapshot_file = os.path.join(run_dir, "final_snapshot.txt")
            with open(snapshot_file, "w") as f:
                f.write("Final Snapshot:\n")
                for pid, data in final_snapshot.items():
                    f.write(f"\nProcess {pid}:\n")
                    f.write(f"Local state: {data['local_state']}\n")
                    f.write("Channel states:\n")
                    for from_pid, messages in data['channel_states'].items():
                        f.write(f"  From process {from_pid}: {messages}\n")
        
        # Show final visualization
        self.visualizer.show()