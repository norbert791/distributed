from collections import deque
from dataclasses import dataclass
from typing import Any, Optional, List

@dataclass
class Message:
    content: Any
    is_marker: bool = False

class Channel:
    def __init__(self, from_pid: int, to_pid: int):
        self.from_pid = from_pid
        self.to_pid = to_pid
        self.queue = deque()
        self.recording = False
        self.recorded_messages: List[Message] = []
    
    def send(self, message: Any, is_marker: bool = False) -> None:
        """Send a message through the channel."""
        self.queue.append(Message(content=message, is_marker=is_marker))
    
    def receive(self) -> Optional[Message]:
        """Receive a message from the channel if available."""
        if not self.queue:
            return None
        
        message = self.queue.popleft()
        if self.recording and not message.is_marker:
            self.recorded_messages.append(message)
        return message
    
    def start_recording(self) -> None:
        """Start recording messages passing through the channel."""
        self.recording = True
        self.recorded_messages = []
    
    def stop_recording(self) -> None:
        """Stop recording messages."""
        self.recording = False
    
    def get_recorded_messages(self) -> List[Message]:
        """Return all recorded messages."""
        return self.recorded_messages
    
    def has_messages(self) -> bool:
        """Check if there are any messages in the channel."""
        return len(self.queue) > 0
