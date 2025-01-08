from typing import Dict
from process import Process

class SnapshotManager:
    def __init__(self, processes: Dict[int, Process]):
        self.processes = processes
        self.snapshot_data = {}
    
    def initiate_snapshot(self, initiator_pid: int):
        """Initiate a snapshot starting from the specified process."""
        if initiator_pid in self.processes:
            self.processes[initiator_pid].take_snapshot()
    
    def is_snapshot_complete(self):
        """Check if the snapshot is complete for all processes."""
        return all(process.is_snapshot_complete() for process in self.processes.values())
    
    def collect_snapshot(self):
        """Collect the snapshot data from all processes."""
        self.snapshot_data = {
            pid: process.get_snapshot_data()
            for pid, process in self.processes.items()
            if process.snapshot_in_progress
        }
        return self.snapshot_data
