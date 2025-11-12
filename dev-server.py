#!/usr/bin/env python3
"""
Simple HTTP development server for OpenWanzer
Serves the game with proper MIME types and CORS headers
"""

import http.server
import socketserver
import os

PORT = 8000

class GameHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    """Custom handler with proper MIME types"""

    def end_headers(self):
        # Add CORS headers for local development
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate')
        super().end_headers()

    def log_message(self, format, *args):
        """Customize log format"""
        print(f"[{self.log_date_time_string()}] {format % args}")

def main():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    with socketserver.TCPServer(("", PORT), GameHTTPRequestHandler) as httpd:
        print(f"""
╔══════════════════════════════════════════════════════════╗
║           OpenWanzer Development Server                  ║
╚══════════════════════════════════════════════════════════╝

Server running at: http://localhost:{PORT}

Open this URL in Firefox to play the game:
  → http://localhost:{PORT}/index.html

Press Ctrl+C to stop the server
""")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n\nShutting down server...")

if __name__ == '__main__':
    main()
