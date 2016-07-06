from tornado.tcpserver import TCPServer
from tornado.ioloop import IOLoop, PeriodicCallback
from tornado.iostream import StreamClosedError, IOStream


class Connection(object):

    def __init__(self, server, stream, address):
        self.server = server
        self.stream = stream
        self.address = address

        self.on_connect()
        self.stream.set_close_callback(self.on_close)
        self.stream.read_until_close(streaming_callback=self.on_receive)

    def send_message(self, msg):
        if not self.stream.closed():
            self.stream.write(msg + "\n")

    def on_connect(self):
        print "[New connection]", self.address
        self.stream.write(str(IOLoop.current().time()) + "\n")

    def on_close(self):
        print "[Connection close]", self.address
        self.server.handle_close(self)

    def on_receive(self, chunk):
        if self.stream.closed():
            return
            
        print "[Server receive]", self.address
        print "----------- chunk len = {:^5} -----------".format(len(chunk))
        print chunk
        print "-----------------------------------------"
        response = self.server.handle_message(chunk, self)
        if response is not None: 
            self.stream.write(response+"\n")



class MengineServer(TCPServer):

    def __init__(self, *args, **kwds):
        super(MengineServer, self).__init__(*args, **kwds)
        self.periodic = PeriodicCallback(self.write_time, 100)
        self.periodic.start()
        self.streams = []
        self.connections = []

    def handle_stream(self, stream, address):
        connection = Connection(self, stream, address)
        self.connections.append(connection)
    
    def handle_message(self, chunk, connection):
        if chunk == "get_count":
            return str(len(self.connections))
        elif chunk == "get_time":
            return str(IOLoop.current().time())
        elif chunk == "get_stream":
            self.streams.append(connection)
        elif chunk == "stop_stream":
            if connection in self.streams:
                self.streams.remove(connection)
                return "stream stopped"
            else:
                return "BAD_INPUT"
        else:
            return "BAD_INPUT"

    def handle_close(self, connection):
        if connection in self.streams:
            self.streams.remove(connection)

        self.connections.remove(connection)

    def write_time(self):
        for connection in self.streams:
            connection.send_message(str(IOLoop.current().time()))


def main():
    server = MengineServer()
    server.listen(8888)


    IOLoop.current().start()

if __name__ == '__main__':
    main()


