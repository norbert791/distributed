from sys import argv
from asyncio import create_task, TaskGroup, run, to_thread
from chat_pb2 import Empty, Note
from grpc import insecure_channel
from chat_pb2_grpc import ChatServerStub

async def handleChatStream(stub):
    stream = iter(stub.ChatStream(Empty()))
    while(True):
        message = await to_thread(next, stream)
        print(f"\n[{message.name}]: {message.text}")

async def handleUserInput(stub, name):
     while(True):
        text = await to_thread(input, "[you]: ")
        note = Note()
        note.name = name
        note.text = text
        stub.SendNote(note)

async def runClient(name: str):
    # NOTE(gRPC Python Team): .close() is possible on a channel and should be
    # used in circumstances in which the with statement does not fit the needs
    # of the code.
    with insecure_channel("localhost:50051") as channel:
        stub = ChatServerStub(channel)
        async with TaskGroup() as tg:
            _ = tg.create_task(handleChatStream(stub))
            _ = tg.create_task(handleUserInput(stub, name))
        

if __name__ == "__main__":
    if len(argv) != 2:
        print("wrong number of parameters")
        exit(1)
    name = argv[1]
    run(runClient(name))

