from process import Process
from snapshot import SnapshotManager
import time
import random

def simulate_system():
    # Create processes
    processes = {i: Process(i) for i in range(1, 4)}
    
    # Create connections (fully connected network)
    for pid1, p1 in processes.items():
        for pid2, p2 in processes.items():
            if pid1 < pid2:  # Avoid duplicate connections
                p1.connect_to(p2)
    
    # Create snapshot manager
    snapshot_manager = SnapshotManager(processes)
    
    # Simulate some message passing and state changes
    for _ in range(5):
        # Random state updates
        for p in processes.values():
            p.update_state(random.randint(1, 10))
        
        # Random message passing
        for pid1 in processes:
            for pid2 in processes:
                if pid1 != pid2 and random.random() < 0.3:  # 30% chance to send message
                    processes[pid1].send_message(pid2, f"Message from {pid1} to {pid2}")
        
        # Process messages
        for p in processes.values():
            for from_pid in p.incoming_channels:
                message = p.receive_message(from_pid)
                if message and not message.is_marker:
                    print(f"Process {p.pid} received: {message.content}")
    
    # Initiate snapshot from process 1
    print("\nInitiating snapshot from process 1...")
    snapshot_manager.initiate_snapshot(1)
    
    # Continue simulation while snapshot is in progress
    while not snapshot_manager.is_snapshot_complete():
        for p in processes.values():
            for from_pid in p.incoming_channels:
                message = p.receive_message(from_pid)
                if message and not message.is_marker:
                    print(f"Process {p.pid} received: {message.content}")
        time.sleep(0.1)
    
    # Collect and print snapshot
    snapshot = snapshot_manager.collect_snapshot()
    print("\nSnapshot result:")
    for pid, data in snapshot.items():
        print(f"\nProcess {pid}:")
        print(f"Local state: {data['local_state']}")
        print("Channel states:")
        for from_pid, messages in data['channel_states'].items():
            print(f"  From process {from_pid}: {messages}")

if __name__ == "__main__":
    simulate_system()
