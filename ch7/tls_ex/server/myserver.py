# https://github.com/FiloSottile/mkcert
# curl -k https://localhost:4443

import http.server
import ssl

class RequestHandler(http.server.BaseHTTPRequestHandler):
    
    def do_GET(self):

        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()

        self.wfile.write("<html><body>done!</body></html>".encode("utf-8"))

    do_POST = do_GET    
    do_PUT = do_POST
    do_DELETE = do_GET

context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
context.load_cert_chain('ch5_tls_ex_certca.pem', 'ch5_tls_ex.key')

httpd = http.server.HTTPServer(('localhost', 4443), RequestHandler)
httpd.socket = context.wrap_socket (httpd.socket, server_side=True)
httpd.serve_forever()