import matplotlib.pyplot as plt
import matplotlib.animation as animation
import networkx as nx
import numpy as np
from process import Process
from typing import Dict, List, Tuple
import math
import os
import time
from snapshot import SnapshotManager

class SystemVisualizer:
    def __init__(self, processes: Dict[int, Process]):
        self.processes = processes
        # Increase figure size and adjust subplot ratio
        self.fig = plt.figure(figsize=(20, 10))
        # Create grid spec to control subplot sizes
        gs = self.fig.add_gridspec(1, 2, width_ratios=[1.2, 0.8])
        self.ax1 = self.fig.add_subplot(gs[0])  # Network topology (larger)
        self.ax2 = self.fig.add_subplot(gs[1])  # State chart
        
        self.graph = nx.DiGraph()
        self.pos = None
        self.message_lines = []
        self.state_text = []
        self.snapshot_data = None
        
        # Color scheme
        self.colors = {
            'node_normal': '#ADD8E6',      # Light blue
            'node_snapshot': '#90EE90',     # Light green
            'node_border': '#4169E1',      # Royal blue
            'edge': '#808080',             # Gray
            'text': '#000000',             # Black
            'marker': '#FF6B6B'            # Coral for markers
        }
        
        # Create data directory if it doesn't exist
        self.data_dir = os.path.join(os.getcwd(), "data")
        os.makedirs(self.data_dir, exist_ok=True)
        
        # Initialize the graph
        self._init_graph()
        
    def _init_graph(self):
        """Initialize the network graph with improved layout."""
        # Add nodes
        for pid in self.processes:
            self.graph.add_node(pid)
            
        # Add edges based on channels
        for pid, process in self.processes.items():
            for target_pid in process.outgoing_channels:
                self.graph.add_edge(pid, target_pid)
        
        # Calculate layout based on number of nodes
        num_nodes = len(self.processes)
        if num_nodes <= 10:
            self.pos = nx.circular_layout(self.graph, scale=2)
        else:
            # For larger networks, use force-directed layout with adjusted parameters
            self.pos = nx.spring_layout(
                self.graph,
                k=1/math.sqrt(num_nodes),  # Adjust spacing
                iterations=50,              # More iterations for better layout
                scale=2                     # Larger scale
            )
        
    def _draw_network(self, snapshot_in_progress=False):
        """Draw the network topology with improved visibility."""
        self.ax1.clear()
        self.ax1.set_title("Network Topology", pad=20, fontsize=14)
        
        # Calculate node size based on number of nodes
        num_nodes = len(self.processes)
        node_size = max(3000 / math.sqrt(num_nodes), 500)
        font_size = max(10 / math.log10(num_nodes + 1), 8)
        
        # Draw nodes
        node_colors = []
        node_borders = []
        for node in self.graph.nodes():
            if snapshot_in_progress and self.processes[node].snapshot_in_progress:
                node_colors.append(self.colors['node_snapshot'])
            else:
                node_colors.append(self.colors['node_normal'])
            node_borders.append(self.colors['node_border'])
        
        # Draw nodes with borders
        nx.draw_networkx_nodes(self.graph, self.pos,
                             node_color=node_colors,
                             node_size=node_size,
                             edgecolors=node_borders,
                             linewidths=2,
                             ax=self.ax1)
        
        # Draw edges with curved arrows for clarity
        nx.draw_networkx_edges(self.graph, self.pos,
                             edge_color=self.colors['edge'],
                             arrows=True,
                             arrowsize=20,
                             connectionstyle='arc3,rad=0.2',
                             ax=self.ax1)
        
        # Draw labels with background for better visibility
        labels = {pid: f"P{pid}\n{self.processes[pid].state}"
                 for pid in self.processes}
        
        # Add white background to labels for better visibility
        for node, (x, y) in self.pos.items():
            self.ax1.text(x, y, labels[node],
                         horizontalalignment='center',
                         verticalalignment='center',
                         fontsize=font_size,
                         bbox=dict(facecolor='white',
                                 edgecolor='none',
                                 alpha=0.7,
                                 pad=2))
        
        # Remove axes
        self.ax1.set_axis_off()
        
    def _draw_state_chart(self):
        """Draw the state chart with improved readability."""
        self.ax2.clear()
        self.ax2.set_title("Process States", pad=20, fontsize=14)
        
        if self.snapshot_data:
            pids = list(self.snapshot_data.keys())
            states = [self.snapshot_data[pid]['local_state'] for pid in pids]
            
            # Calculate bar width based on number of processes
            bar_width = 0.8 / max(len(pids), 1)
            
            # Create bar chart with custom colors
            bars = self.ax2.bar([f'P{pid}' for pid in pids], states,
                              width=bar_width,
                              color=self.colors['node_normal'],
                              edgecolor=self.colors['node_border'],
                              linewidth=2)
            
            # Add value labels on top of bars
            for bar in bars:
                height = bar.get_height()
                self.ax2.text(bar.get_x() + bar.get_width()/2., height,
                            f'{int(height)}',
                            ha='center', va='bottom',
                            fontsize=10)
            
            # Add channel state information with improved formatting
            max_height = max(states) if states else 0
            y_pos = max_height * 1.2
            
            # Calculate font size based on number of processes
            font_size = max(8 / math.log10(len(pids) + 1), 6)
            
            for pid, data in self.snapshot_data.items():
                channel_info = []
                for from_pid, messages in data['channel_states'].items():
                    if messages:
                        channel_info.append(f"P{from_pid}: {len(messages)}")
                
                if channel_info:
                    channel_text = f"Msgs: {', '.join(channel_info)}"
                    self.ax2.text(pids.index(pid), y_pos, channel_text,
                                ha='center', va='bottom',
                                fontsize=font_size,
                                bbox=dict(facecolor='white',
                                        edgecolor='none',
                                        alpha=0.7,
                                        pad=2))
        
        self.ax2.set_ylabel("State Value", fontsize=12)
        self.ax2.tick_params(axis='both', which='major', labelsize=10)
        
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
        plt.savefig(filename, dpi=300, bbox_inches='tight')
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