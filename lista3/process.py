from typing import Dict, List, Set, Optional
from channel import Channel, Message

class Process:
    def __init__(self, pid: int):
        self.pid = pid
        self.state = 0  # Local state (could be more complex)
        self.incoming_channels: Dict[int, Channel] = {}  # from_pid -> Channel
        self.outgoing_channels: Dict[int, Channel] = {}  # to_pid -> Channel
        self.snapshot_state = None
        self.recorded_channels: Set[int] = set()
        self.marker_received_from: Set[int] = set()
        self.snapshot_in_progress = False
    
    def connect_to(self, other_process: 'Process'):
        """Establish a bidirectional connection with another process."""
        # Create outgoing channel
        outgoing = Channel(self.pid, other_process.pid)
        self.outgoing_channels[other_process.pid] = outgoing
        other_process.incoming_channels[self.pid] = outgoing
        
        # Create incoming channel
        incoming = Channel(other_process.pid, self.pid)
        self.incoming_channels[other_process.pid] = incoming
        other_process.outgoing_channels[self.pid] = incoming
    
    def send_message(self, to_pid: int, message: any, is_marker: bool = False):
        """Send a message to another process."""
        if to_pid in self.outgoing_channels:
            self.outgoing_channels[to_pid].send(message, is_marker)
    
    def receive_message(self, from_pid: int):
        """Receive a message from a specific process."""
        if from_pid in self.incoming_channels:
            message = self.incoming_channels[from_pid].receive()
            if message and message.is_marker:
                self._handle_marker(from_pid)
            return message
        return None
    
    def update_state(self, amount: int):
        """Update the process's local state."""
        self.state += amount
    
    def take_snapshot(self):
        """Initiate a snapshot from this process."""
        if not self.snapshot_in_progress:
            self.snapshot_in_progress = True
            self.snapshot_state = self.state
            self.marker_received_from = {self.pid}
            
            # Start recording on all incoming channels
            for pid in self.incoming_channels:
                self.incoming_channels[pid].start_recording()
            
            # Send markers on all outgoing channels
            for pid in self.outgoing_channels:
                self.send_message(pid, f"MARKER_{self.pid}", is_marker=True)
    
    def _handle_marker(self, from_pid: int):
        """Handle receiving a marker message."""
        if not self.snapshot_in_progress:
            # First marker received - take local snapshot
            self.take_snapshot()
        
        self.marker_received_from.add(from_pid)
        
        # Stop recording on this channel
        if from_pid in self.incoming_channels:
            self.incoming_channels[from_pid].stop_recording()
            self.recorded_channels.add(from_pid)
    
    def is_snapshot_complete(self):
        """Check if the snapshot is complete for this process."""
        return self.snapshot_in_progress and len(self.marker_received_from) == len(self.incoming_channels) + 1
    
    def get_snapshot_data(self):
        """Get the snapshot data for this process."""
        if not self.snapshot_in_progress:
            return {}
        
        channel_states = {}
        for pid, channel in self.incoming_channels.items():
            if pid in self.recorded_channels:
                channel_states[pid] = [msg.content for msg in channel.get_recorded_messages()]
        
        return {
            'process_id': self.pid,
            'local_state': self.snapshot_state,
            'channel_states': channel_states
        }
