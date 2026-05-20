from http.server import BaseHTTPRequestHandler, HTTPServer
import os

ASTERISK_FILE = "/etc/asterisk/sip.conf"

def add_user(username, password):
    with open(ASTERISK_FILE, "a") as f:
        f.write(f"\n[{username}]\n")
        f.write("type=friend\n")
        f.write(f"secret={password}\n")
        f.write("host=dynamic\n")

    os.system("asterisk -rx 'sip reload'")

def get_users():
    users = []
    with open(ASTERISK_FILE, "r") as f:
        for line in f:
            if line.startswith("[") and "]" in line:
                users.append(line.strip()[1:-1])
    return users

class MyServer(BaseHTTPRequestHandler):

    def do_GET(self):
        if self.path == "/":
            users = get_users()

            html = "<h1>Gestion Asterisk</h1>"
            html += "<h2>Créer un utilisateur</h2>"
            html += '''
                <form method="POST" action="/add">
                    Username: <input name="username"><br>
                    Password: <input name="password"><br>
                    <input type="submit" value="Créer">
                </form>
            '''

            html += "<h2>Utilisateurs</h2><ul>"
            for u in users:
                html += f"<li>{u}</li>"
            html += "</ul>"

            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(html.encode())

    def do_POST(self):
        if self.path == "/add":
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode()

            data = dict(x.split("=") for x in post_data.split("&"))
            username = data.get("username")
            password = data.get("password")

            add_user(username, password)

            self.send_response(302)
            self.send_header("Location", "/")
            self.end_headers()

server = HTTPServer(("0.0.0.0", 8000), MyServer)
print("Interface web lancée ")
server.serve_forever()
