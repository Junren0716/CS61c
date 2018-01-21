#!/usr/bin/python
import BaseHTTPServer
import SimpleHTTPServer
import json
import os
import sys

from cnnModule import *

if len(sys.argv) == 1:
    web_port_number = 12345
else:
    web_port_number = int(sys.argv[1])

class webHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
	def do_POST(self):
		data_string = self.rfile.read(int(self.headers['Content-Length']))

		self.send_response(200)
		self.send_header('Content-type','text/html')
		self.end_headers()

		samples = json.loads(data_string)
		print '--------------------------------------------------------------------------------'
		print 'RECEIVED CLASSIFICATION REQUEST: ' + ','.join([str(x) for x in samples])

		dt = RunCNNClassifier(samples)

		responses = samples

		print 'SENDING RESPONSES: ' + ','.join([str(x) for x in responses])
		self.wfile.write(json.dumps({'dt':dt, 'r':responses}))
		print '--------------------------------------------------------------------------------'

		return

# -----------------------------------------------------------------------------

print
print '          *** CS 61C, Summer 2015: Project 4 ***'
print

try:
	server = BaseHTTPServer.HTTPServer(('', web_port_number), webHandler)
	print 'Launched web server! Open your browser and open the following page:'
	print
	print 'http://localhost:%d' % web_port_number
	print
	print 'Press CTRL+C to terminate'
	
	os.chdir('web')
	server.serve_forever()

except KeyboardInterrupt:
	print 'CTRL+C received, shutting down the web server'
	server.socket.close()
