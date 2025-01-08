import matplotlib.pyplot as plt
import networkx as nx
import numpy as np
from typing import Dict, List, Optional
import os
import time
from process import Process
from snapshot import SnapshotManager

class SystemVisualizer:
    def __init__(self, processes: Dict[int, Process]):
        self.processes = processes
        # Create a figure with three subplots:
        # 1. Network state (top left)
        # 2. Channel states (top right)
        # 3. Snapshot status (bottom)
        self.fig = plt.figure(figsize=(20, 15))
        gs = self.fig.add_gridspec(2, 2, height_ratios=[1, 1])
        self.ax_network = self.fig.add_subplot(gs[0, 0])  # Network topology
        self.ax_channels = self.fig.add_subplot(gs[0, 1])  # Channel states
        self.ax_snapshot = self.fig.add_subplot(gs[1, :])  # Snapshot visualization
        
        self.graph = nx.DiGraph()
        self.pos = None
        self.snapshot_data = None
        self.snapshot_started = False
        self.snapshot_start_step = None
        
        # Create data directory
        self.data_dir = os.path.join(os.getcwd(), "data")
        os.makedirs(self.data_dir, exist_ok=True)
        
        # Initialize graph
        self._init_graph()
        
    def _init_graph(self):
        """Initialize the network graph."""
        for pid in self.processes:
            self.graph.add_node(pid)
        
        for pid, process in self.processes.items():
            for target_pid in process.outgoing_channels:
                self.graph.add_edge(pid, target_pid)
        
        self.pos = nx.spring_layout(self.graph)
    
    def _draw_network(self, step: int):
        """Draw the network topology with process states."""
        self.ax_network.clear()
        title = f"Network State - Step {step}"
        if self.snapshot_started:
            title += "\n(Snapshot in Progress)"
        self.ax_network.set_title(title, pad=20, fontsize=12)
        
        # Draw nodes with different colors based on snapshot state
        node_colors = []
        for node in self.graph.nodes():
            if self.processes[node].snapshot_in_progress:
                node_colors.append('lightgreen')
            else:
                node_colors.append('lightblue')
        
        nx.draw_networkx_nodes(self.graph, self.pos,
                             node_color=node_colors,
                             node_size=2000,
                             ax=self.ax_network)
        
        # Draw edges
        nx.draw_networkx_edges(self.graph, self.pos,
                             edge_color='gray',
                             arrows=True,
                             arrowsize=20,
                             ax=self.ax_network)
        
        # Draw labels with process states
        labels = {pid: f"P{pid}\nState: {self.processes[pid].state}"
                 for pid in self.processes}
        nx.draw_networkx_labels(self.graph, self.pos, labels,
                              font_size=10,
                              ax=self.ax_network)
        
        self.ax_network.set_axis_off()
    
    def _draw_channel_states(self):
        """Draw the current state of all channels."""
        self.ax_channels.clear()
        self.ax_channels.set_title("Channel States", pad=20, fontsize=12)
        
        num_processes = len(self.processes)
        matrix = np.zeros((num_processes, num_processes))
        
        # Fill the matrix with channel message counts
        for pid, process in self.processes.items():
            for target_pid, channel in process.outgoing_channels.items():
                matrix[pid-1][target_pid-1] = len(channel.queue)
        
        # Create heatmap
        im = self.ax_channels.imshow(matrix, cmap='YlOrRd')
        
        # Add colorbar
        # plt.colorbar(im, ax=self.ax_channels, label='Messages in Channel')
        
        # Add labels
        self.ax_channels.set_xticks(range(num_processes))
        self.ax_channels.set_yticks(range(num_processes))
        self.ax_channels.set_xticklabels([f'P{i+1}' for i in range(num_processes)])
        self.ax_channels.set_yticklabels([f'P{i+1}' for i in range(num_processes)])
        
        self.ax_channels.set_xlabel('To Process')
        self.ax_channels.set_ylabel('From Process')
        
        # Add text annotations
        for i in range(num_processes):
            for j in range(num_processes):
                if matrix[i][j] > 0:
                    self.ax_channels.text(j, i, int(matrix[i][j]),
                                       ha='center', va='center')
                    
        # Mark recording channels
        for pid, process in self.processes.items():
            for from_pid, channel in process.incoming_channels.items():
                if channel.recording:
                    rect = plt.Rectangle((from_pid-1.5, pid-1.5), 1, 1, fill=False,
                                      edgecolor='green', linestyle='--', linewidth=2)
                    self.ax_channels.add_patch(rect)
    
    def _draw_snapshot_status(self):
        """Draw snapshot progress and recorded states."""
        self.ax_snapshot.clear()
        self.ax_snapshot.set_title("Snapshot Status", pad=20, fontsize=12)
        
        if not self.snapshot_started:
            self.ax_snapshot.text(0.5, 0.5, "No Snapshot Started",
                                ha='center', va='center',
                                fontsize=12)
            self.ax_snapshot.set_axis_off()
            return
        
        # Create a table of snapshot data
        data = []
        headers = ['Process', 'State', 'Recording Channels', 'Recorded Messages']
        
        for pid, process in self.processes.items():
            recorded_channels = []
            recorded_msgs = []
            
            for from_pid, channel in process.incoming_channels.items():
                if channel.recording:
                    recorded_channels.append(f'P{from_pid}→P{pid}')
                if hasattr(channel, 'recorded_messages'):
                    msgs = len(channel.recorded_messages)
                    if msgs > 0:
                        recorded_msgs.append(f'P{from_pid}: {msgs}')
            
            data.append([
                f'P{pid}',
                f'{process.snapshot_state if process.snapshot_in_progress else "Not taken"}',
                ', '.join(recorded_channels) if recorded_channels else 'None',
                ', '.join(recorded_msgs) if recorded_msgs else 'None'
            ])
        
        table = self.ax_snapshot.table(cellText=data,
                                     colLabels=headers,
                                     loc='center',
                                     cellLoc='center')
        table.auto_set_font_size(False)
        table.set_fontsize(9)
        table.scale(1.2, 1.5)
        
        if self.snapshot_start_step is not None:
            self.ax_snapshot.text(0.02, 0.95, 
                                f'Snapshot started at step {self.snapshot_start_step}',
                                transform=self.ax_snapshot.transAxes,
                                fontsize=10)
        
        self.ax_snapshot.set_axis_off()
    
    def update(self, step: int, snapshot_started: bool = False):
        """Update the visualization with current system state."""
        if snapshot_started and not self.snapshot_started:
            self.snapshot_started = True
            self.snapshot_start_step = step
            
        self._draw_network(step)
        self._draw_channel_states()
        self._draw_snapshot_status()
        
        plt.tight_layout()
    
    def save(self, filename: str):
        """Save the current visualization to a file."""
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        print(f"Saved visualization to {filename}")
        
    def show(self):
        """Display the current visualization."""
        plt.show()

class Simulation:
    def __init__(self, num_processes=5):
        self.processes = {i: Process(i) for i in range(1, num_processes + 1)}
        
        # Create cycle connections (C_n graph)
        num_processes = len(self.processes)
        for i in range(1, num_processes + 1):
            next_pid = i % num_processes + 1
            self.processes[i].connect_to(self.processes[next_pid])
        
        # Add additional random edges
        additional_edges = np.random.randint(1, num_processes)
        for _ in range(additional_edges):
            pid1, pid2 = np.random.choice(range(1, num_processes + 1), 2, replace=False)
            self.processes[pid1].connect_to(self.processes[pid2])
        
        self.snapshot_manager = SnapshotManager(self.processes)
        self.visualizer = SystemVisualizer(self.processes)
        
    def run_simulation(self, num_steps=10, snapshot_at_step=5):
        """Run the simulation with enhanced visualization."""
        print("Starting simulation...")
        
        # Create run directory
        timestamp = time.strftime("%Y%m%d_%H%M%S")
        run_dir = os.path.join(self.visualizer.data_dir, f"run_{timestamp}")
        os.makedirs(run_dir, exist_ok=True)
        print(f"Saving simulation data to: {run_dir}")
        
        for step in range(num_steps * 2):
            print(f"\nStep {step//2 + 1}/{num_steps} " + "send" if step % 2 == 0 else "receive")
            
            # Update states
            for p in self.processes.values():
                p.update_state(np.random.randint(1, 10))
            
            if step % 2 == 0:
                # Send messages
                for pid1 in self.processes:
                    for pid2 in self.processes:
                        if pid1 != pid2 and np.random.random() < 0.3:
                            self.processes[pid1].send_message(
                                pid2, 
                                f"Msg_{step}_{pid1}->{pid2}"
                        )
            else:
                # Process messages
                for p in self.processes.values():
                    for from_pid in p.incoming_channels:
                        message = p.receive_message(from_pid)
                        if message and not message.is_marker:
                            print(f"Process {p.pid} received: {message.content}")
            
            # Start snapshot if it's time
            snapshot_started = step == snapshot_at_step
            if snapshot_started:
                print("\nInitiating snapshot from process 1...")
                self.snapshot_manager.initiate_snapshot(1)
            
            # Update and save visualization
            self.visualizer.update(step, snapshot_started)
            frame_path = os.path.join(run_dir, f"step_{step:03d}.png")
            self.visualizer.save(frame_path)
            
        # Show final state
        self.visualizer.show()