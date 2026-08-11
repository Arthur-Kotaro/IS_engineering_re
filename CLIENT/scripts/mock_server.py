#!/usr/bin/env python3
"""
Mock HTTP server for testing User Service
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import time

class MockUserServiceHandler(BaseHTTPRequestHandler):
    
    def do_POST(self):
        if self.path == "/api/v1/auth/login":
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data.decode('utf-8'))
            
            # Проверка credentials
            if data.get('email') == 'test@example.com' and data.get('password') == 'password123':
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                response = {
                    "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjMiLCJlbWFpbCI6InRlc3RAZXhhbXBsZS5jb20iLCJmdWxsX25hbWUiOiJUZXN0IFVzZXIiLCJwb3NpdGlvbiI6IkVuZ2luZWVyIn0.signature",
                    "refresh_token": "mock_refresh_token_123"
                }
                self.wfile.write(json.dumps(response).encode())
            else:
                self.send_response(401)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                response = {"error": "Invalid credentials"}
                self.wfile.write(json.dumps(response).encode())
        
        elif self.path == "/api/v1/auth/refresh":
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            response = {
                "access_token": "new_mock_access_token",
                "refresh_token": "new_mock_refresh_token"
            }
            self.wfile.write(json.dumps(response).encode())
        
        else:
            self.send_response(404)
            self.end_headers()
    
    def do_GET(self):
        if self.path == "/health":
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            response = {"status": "ok", "timestamp": time.time()}
            self.wfile.write(json.dumps(response).encode())
        
        elif self.path == "/api/v1/auth/password-expiry":
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            response = {
                "days_remaining": 15,
                "is_expired": False,
                "expires_at": "2025-12-31T23:59:59Z"
            }
            self.wfile.write(json.dumps(response).encode())
        
        else:
            self.send_response(404)
            self.end_headers()
    
    def log_message(self, format, *args):
        print(f"[MOCK] {args[0]}")

if __name__ == "__main__":
    port = 8000
    server = HTTPServer(("localhost", port), MockUserServiceHandler)
    print(f"Mock User Service running on http://localhost:{port}")
    print("Endpoints:")
    print("  POST /api/v1/auth/login - Login with email/password")
    print("  POST /api/v1/auth/refresh - Refresh token")
    print("  GET  /health - Health check")
    print("  GET  /api/v1/auth/password-expiry - Check password expiry")
    print("\nTest credentials: test@example.com / password123")
    print("\nPress Ctrl+C to stop")
    
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down mock server...")
        server.shutdown()
