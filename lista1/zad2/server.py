from chat_pb2_grpc import ChatServerServicer, add_ChatServerServicer_to_server
from chat_pb2 import Empty
from grpc import server
from concurrent import futures
from queue import Queue
import logging

class Server(ChatServerServicer):
    def __init__(self):
        self.activeStreams = {}

    def ChatStream(self, request, context):
        counter = 0
        clientID = id(context)
        q = Queue()
        self.activeStreams[clientID] = q
        while True:
            print("stream")
            mes = q.get()
            yield mes
        q.shutdown()

    def SendNote(self, request, context):
        toDelete = set()
        for id in self.activeStreams.keys():
            try:
                q = self.activeStreams[id]
                q.put(request)
            except Exception as e:
                print(e)
                toDelete.add(id)
        for elem in toDelete:
            del self.activeStreams[elem]
        return Empty()

def serve():
    srv = server(futures.ThreadPoolExecutor(max_workers=10))
    add_ChatServerServicer_to_server(Server(), srv)
    srv.add_insecure_port("[::]:50051")
    srv.start()
    srv.wait_for_termination()

if __name__ == "__main__":
    logging.basicConfig()
    serve()